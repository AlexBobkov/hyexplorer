# -*- coding: utf-8 -*-

import os
import os.path
import psycopg2
import subprocess
import tempfile
from osgeo import gdal
from osgeo import osr
from zipfile import ZipFile
from flask import Flask, request, redirect, url_for
from werkzeug import secure_filename

UPLOAD_FOLDER = os.environ['GEOPORTAL_UPLOAD_FOLDER']
PUBLIC_FOLDER = os.environ['GEOPORTAL_PUBLIC_FOLDER']
SCENES_FOLDER = os.environ['GEOPORTAL_SCENES_FOLDER']
#SCENES_EXTRACT_FOLDER = os.environ['GEOPORTAL_SCENES_EXTRACT_FOLDER']
#SCENES_CLIPS_FOLDER = os.environ['GEOPORTAL_SCENES_CLIPS_FOLDER']

ALLOWED_EXTENSIONS = set(['jpg', 'jpeg', 'ZIP', 'TIF'])

tempfile.tempdir = UPLOAD_FOLDER

###############################

app = Flask(__name__)
#app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = 1024 * 1024 * 1024 #1Gb

###############################

def allowed_file(filename):
    return '.' in filename and \
        filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

###############################
        
@app.route('/')
def hello_world():
    app.logger.info('Hello World!')
    return 'Hello World!'

@app.route('/overview/<sensor>/<sceneid>', methods=['GET', 'POST'])
def overview(sensor, sceneid):
    app.logger.info('Upload overview %s method %s', sceneid, request.method)
    
    if sensor != 'Hyperion' and sensor != 'AVIRIS':
        return 'Wrong sensor'

    if request.method == 'POST':
        file = request.files['file']
        app.logger.info('File %s ', file.filename)

        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file.save(os.path.join(PUBLIC_FOLDER + "/" + sensor + "/overviews", filename))

            #conn = psycopg2.connect("host=localhost dbname=GeoPortal user=user password=user")
            conn = psycopg2.connect("host=178.62.140.44 dbname=GeoPortal user=portal password=PortalPass")

            cur = conn.cursor()
            cur.execute("update scenes set hasoverview=TRUE, overviewname=%s where sceneid=%s;", (filename, sceneid))
            conn.commit()
            cur.close()
            conn.close()

            app.logger.info('Scene is ready %s', sceneid)
        else:
            return 'Wrong file'

        return 'Success overview POST'
    else:
        return 'Success overview GET'

@app.route('/scene/<sceneid>', methods=['GET', 'POST'])
def scene_upload(sceneid):
    app.logger.info('Upload scene %s method %s', sceneid, request.method)

    if request.method == 'POST':
        file = request.files['file']
        app.logger.info('File %s ', file.filename)

        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file.save(os.path.join(SCENES_FOLDER, filename))

            #conn = psycopg2.connect("host=localhost dbname=GeoPortal user=user password=user")
            conn = psycopg2.connect("host=178.62.140.44 dbname=GeoPortal user=portal password=PortalPass")

            cur = conn.cursor()
            cur.execute("update scenes set hasscene=TRUE where sceneid=%s;", (sceneid,))
            conn.commit()
            cur.close()
            conn.close()

            app.logger.info('Scene is ready %s', sceneid)

        return 'Success scene POST'
    else:
        return 'Success scene GET'

def extract_bands(sceneid, minband, maxband):
    zipfilepath = SCENES_FOLDER + "/" + sceneid[:23] + "1T.ZIP"

    if not os.path.isfile(zipfilepath):
        app.logger.error('Filepath %s is not found!', zipfilepath)
        return False

    app.logger.info('Filepath %s is found', zipfilepath)

    extractfolder = PUBLIC_FOLDER + "/Hyperion/scenes/" + sceneid + "/original"

    with ZipFile(zipfilepath) as zip:
        for i in range(minband, maxband + 1):
            filename = sceneid[:23] + "B{0:0>3}_L1T.TIF".format(i)

            if not os.path.isfile(extractfolder + "/" + filename):
                zip.extract(filename, extractfolder)

    return True

@app.route('/scene/<sceneid>/<int:minband>/<int:maxband>')
def scene(sceneid, minband, maxband):
    app.logger.info('Request scene %s min band %d max band %d', sceneid, minband, maxband)

    if minband < 1 or maxband > 242:
        app.logger.error('Wrong band')
        return 'WRONG BAND'

    if not extract_bands(sceneid, minband, maxband):
        app.logger.error('ZIP is not found')
        return 'NO FILE'

    output = 'SUCCESS\n'
    for i in range(minband, maxband + 1):
        filename = sceneid[:23] + "B{0:0>3}_L1T.TIF".format(i)
        output += 'http://virtualglobe.ru/geoportal/Hyperion/scenes/' + sceneid + '/original/' + filename + '\n'

    return output

@app.route('/sceneclip/<sceneid>/<int:minband>/<int:maxband>')
def sceneclip(sceneid, minband, maxband):
    app.logger.info('Request scene clip %s min band %d max band %d', sceneid, minband, maxband)

    if minband < 1 or maxband > 242:
        app.logger.error('Wrong band')
        return 'WRONG BAND'

    if not 'leftgeo' in request.args or \
        not 'upgeo' in request.args or \
        not 'rightgeo' in request.args or \
        not 'downgeo' in request.args:
        app.logger.error('Bad request')
        return 'BAD REQUEST'

    leftgeo = request.args.get('leftgeo', None, float)
    upgeo = request.args.get('upgeo', None, float)
    rightgeo = request.args.get('rightgeo', None, float)
    downgeo = request.args.get('downgeo', None, float)

    if leftgeo == None or upgeo == None or rightgeo == None or downgeo == None:
        app.logger.error('Wrong bounds')
        return 'WRONG BOUNDS'

    #######################################

    if not extract_bands(sceneid, minband, maxband):
        app.logger.error('ZIP is not found')
        return 'NO FILE'

    extractfolder = PUBLIC_FOLDER + "/Hyperion/scenes/" + sceneid + "/original"
    clipsfolder = PUBLIC_FOLDER + "/Hyperion/scenes/" + sceneid + "/clips"

    if not os.path.exists(clipsfolder):
        os.makedirs(clipsfolder)

    clipNum = len(os.listdir(clipsfolder)) #Количество клипов для этой сцены
    clipsfolder += "/clip%d" % clipNum

    if not os.path.exists(clipsfolder):
        os.makedirs(clipsfolder)

    app.logger.info("Create clips folder %s", clipsfolder)

    #######################################
    #Открываем первый из файлов с помощью GDAL
    #Читаем оттуда описание КСО
    #Преобразуем координаты фрагмента из WGS84 в КСО сцены

    firstFilename = sceneid[:23] + "B{0:0>3}_L1T.TIF".format(minband)

    dataset = gdal.Open(extractfolder + "/" + firstFilename, gdal.GA_ReadOnly)
    if dataset is None:
        app.logger.error('Failed to open file %s', firstFilename)
        return 'FAILED OPEN FILE'

    projection = dataset.GetProjectionRef()
    if projection is None:
        app.logger.error('Projection is null for file %s', firstFilename)
        return 'PROJECTION IS NULL'

    srsOut = osr.SpatialReference()
    if srsOut.ImportFromWkt(projection) != gdal.CE_None:
        app.logger.error('Failed to import srs from file %s', firstFilename)
        return 'FAILED IMPORT SRS'

    srsIn = osr.SpatialReference()
    srsIn.ImportFromEPSG(4326)

    ct = osr.CoordinateTransformation(srsIn, srsOut)
    (left, up, h) = ct.TransformPoint(leftgeo, upgeo)
    (right, down, h) = ct.TransformPoint(rightgeo, downgeo)

    app.logger.info("ClipGeo L = %.10f U = %.10f R = %.10f D = %.10f", leftgeo, upgeo, rightgeo, downgeo)
    app.logger.info("Clip L = %f U = %f R = %f D = %f", left, up, right, down)

    #######################################

    output = 'SUCCESS_CLIP\n'
    for i in range(minband, maxband + 1):
        inFilepath = extractfolder + "/" + sceneid[:23] + "B{0:0>3}_L1T.TIF".format(i)

        outFilename = sceneid[:23] + "B{0:0>3}_L1T_clip.TIF".format(i)
        outFilepath = clipsfolder + "/" + outFilename

        subprocess.call(["gdal_translate", "-projwin", '{0}'.format(left), '{0}'.format(up), '{0}'.format(right), '{0}'.format(down), inFilepath, outFilepath])

        output += 'http://virtualglobe.ru/geoportal/Hyperion/scenes/{0}/clips/clip{1}/{2}\n'.format(sceneid, clipNum, outFilename)

    return output
    
def ConvertToGeo(transform, adfGeoTransform, x, y):
    dfGeoX = adfGeoTransform[0] + adfGeoTransform[1] * x + adfGeoTransform[2] * y
    dfGeoY = adfGeoTransform[3] + adfGeoTransform[4] * x + adfGeoTransform[5] * y
    pnt = transform.TransformPoint(dfGeoX, dfGeoY, 0)
    return pnt
    
@app.route('/processed/<sceneid>', methods=['GET', 'POST'])
def processed_upload(sceneid):
    app.logger.info('Upload processed file for scene %s method %s', sceneid, request.method)

    if request.method == 'POST':
        file = request.files['file']
        app.logger.info('File %s ', file.filename)
        
        processedfolder = PUBLIC_FOLDER + "/Hyperion/scenes/" + sceneid + "/processed"
        
        if not os.path.exists(processedfolder):
            os.makedirs(processedfolder)

        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file.save(os.path.join(processedfolder, filename))
            
            ####################
            
            try:
                band = int(request.form['band'])
                contrast = float(request.form['contrast'])
                sharpness = float(request.form['sharpness'])
                blocksize = int(request.form['blocksize'])
            except ValueError:
                return 'Failed to parse params'
            
            app.logger.info('Params %s %s %s %s', band, contrast, sharpness, blocksize)
            
            ####################
            
            dataset = gdal.Open(os.path.join(processedfolder, filename), gdal.GA_ReadOnly)
            if dataset is None:
                app.logger.error('Failed to open file %s', filename)
                return 'FAILED OPEN FILE'

            projection = dataset.GetProjectionRef()
            if projection is None or len(projection) == 0:
                app.logger.error('Projection is null for file %s', filename)
                return 'PROJECTION IS NULL'

            srs = osr.SpatialReference()
            if srs.ImportFromWkt(projection) != gdal.CE_None:
                app.logger.error('Failed to import srs from file %s', filename)
                return 'FAILED IMPORT SRS'
            
            srsLatLong = srs.CloneGeogCS()
            if srsLatLong is None:
                app.logger.error('Failed to create latlong srs from file %s', filename)
                return 'LATLONG IS NULL'               
                    
            transform = osr.CoordinateTransformation(srs, srsLatLong)
                    
            adfGeoTransform = dataset.GetGeoTransform(can_return_null = True)
            if adfGeoTransform is None:
                app.logger.error('GeoTransform is null from file %s', filename)
                return 'GEOTRANSFORM IS NULL'    
            
            ul = ConvertToGeo(transform, adfGeoTransform, 0, 0) #upper left
            ll = ConvertToGeo(transform, adfGeoTransform, 0, dataset.RasterYSize) #lower left
            ur = ConvertToGeo(transform, adfGeoTransform, dataset.RasterXSize, 0) #upper right
            lr = ConvertToGeo(transform, adfGeoTransform, dataset.RasterXSize, dataset.RasterYSize) #lower right
            
            polystr = "SRID=4326;POLYGON(({0:.12f} {1:.12f}, {2:.12f} {3:.12f}, {4:.12f} {5:.12f}, {6:.12f} {7:.12f}, {0:.12f} {1:.12f}))".format(ll[0], ll[1], lr[0], lr[1], ur[0], ur[1], ul[0], ul[1])
            
            app.logger.info(polystr)
            
            ####################
            
            #conn = psycopg2.connect("host=localhost dbname=GeoPortal user=user password=user")
            conn = psycopg2.connect("host=178.62.140.44 dbname=GeoPortal user=portal password=PortalPass")

            cur = conn.cursor()
            cur.execute("insert into public.processedimages (sceneid, bounds, band, contrast, sharpness, blocksize, filename) values (%s, ST_GeographyFromText(%s), %s, %s, %s, %s, %s);", (sceneid, polystr, band, contrast, sharpness, blocksize, filename))
            conn.commit()
            cur.close()
            conn.close()

            app.logger.info('Processed is ready %s', sceneid)

        return 'Success processed POST'
    else:
        return 'Success processed GET'

if __name__ == '__main__':
    app.debug = False

    if not app.debug:
        import logging
        app.logger.setLevel(logging.INFO)

        from logging import FileHandler
        file_handler = FileHandler('pylog.txt')
        file_handler.setLevel(logging.INFO)
        app.logger.addHandler(file_handler)

    app.run(host='0.0.0.0')
