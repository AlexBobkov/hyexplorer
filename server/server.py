import os
import os.path
import psycopg2
import subprocess
from osgeo import gdal
from osgeo import osr
from zipfile import ZipFile
from flask import Flask, request, redirect, url_for
from werkzeug import secure_filename

SCENES_FOLDER = os.environ['GEOPORTAL_SCENES_FOLDER']
SCENES_EXTRACT_FOLDER = os.environ['GEOPORTAL_SCENES_EXTRACT_FOLDER']
SCENES_CLIPS_FOLDER = os.environ['GEOPORTAL_SCENES_CLIPS_FOLDER']

UPLOAD_FOLDER = os.environ['GEOPORTAL_UPLOAD_FOLDER']
ALLOWED_EXTENSIONS = set(['jpeg'])

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = 1 * 1024 * 1024 #1mb

def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

@app.route('/')
def hello_world():
    app.logger.info('Hello World!')
    return 'Hello World!'

@app.route('/overview/<sceneid>', methods=['GET', 'POST'])
def overview(sceneid):
    app.logger.info('Overview %s %s', sceneid, request.method)

    if request.method == 'POST':
        for name, file in request.files.items():
            app.logger.info('File %s ', file.filename)

            if allowed_file(file.filename):
                filename = secure_filename(file.filename)
                file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))

                #conn = psycopg2.connect("host=localhost dbname=GeoPortal user=user password=user")
                conn = psycopg2.connect("host=178.62.140.44 dbname=GeoPortal user=portal password=PortalPass")

                cur = conn.cursor()
                cur.execute("update scenes set hasoverview=TRUE, overviewname=%s where sceneid=%s;", (filename, sceneid))
                conn.commit()
                cur.close()
                conn.close()

                app.logger.info('Scene is ready %s', sceneid)

        return 'Success POST'
    else:
        return 'Success GET'

def extract_bands(sceneid, minband, maxband):
    zipfilepath = SCENES_FOLDER + "/" + sceneid[:23] + "1T.ZIP"

    if not os.path.isfile(zipfilepath):
        app.logger.error('Filepath %s is not found!', zipfilepath)
        return False

    app.logger.info('Filepath %s is found', zipfilepath)

    extractfolder = SCENES_EXTRACT_FOLDER + "/" + sceneid

    with ZipFile(zipfilepath) as zip:
        for i in range(minband, maxband + 1):
            filename = sceneid[:23] + "B{0:0>3}_L1T.TIF".format(i)

            if not os.path.isfile(extractfolder + "/" + filename):
                zip.extract(filename, extractfolder)

    return True

@app.route('/scene/<sceneid>/<int:minband>/<int:maxband>')
def scene(sceneid, minband, maxband):
    app.logger.info('Scene %s %d %d', sceneid, minband, maxband)

    if minband < 1 or maxband > 242:
        app.logger.error('Wrong band')
        return 'WRONG BAND'

    if not extract_bands(sceneid, minband, maxband):
        app.logger.error('ZIP is not found')
        return 'NO FILE'

    output = 'SUCCESS\n'
    for i in range(minband, maxband + 1):
        filename = sceneid[:23] + "B{0:0>3}_L1T.TIF".format(i)
        output += 'http://virtualglobe.ru/geoportal/hyperion/scenes/' + sceneid + '/' + filename + '\n'

    return output

@app.route('/sceneclip/<sceneid>/<int:minband>/<int:maxband>')
def sceneclip(sceneid, minband, maxband):
    app.logger.info('Scene clip %s %d %d', sceneid, minband, maxband)

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

    extractfolder = SCENES_EXTRACT_FOLDER + "/" + sceneid
    clipsfolder = SCENES_CLIPS_FOLDER + "/" + sceneid

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

        output += 'http://virtualglobe.ru/geoportal/hyperion/scenes/clips/{0}/clip{1}/{2}\n'.format(sceneid, clipNum, outFilename)

    return output

if __name__ == '__main__':
    #app.debug = True

    if not app.debug:
        import logging
        app.logger.setLevel(logging.INFO)

        from logging import FileHandler
        file_handler = FileHandler('pylog.txt')
        file_handler.setLevel(logging.INFO)
        app.logger.addHandler(file_handler)

    app.run(host='0.0.0.0')
