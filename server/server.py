import os
import os.path
import psycopg2
from zipfile import ZipFile
from flask import Flask, request, redirect, url_for
from werkzeug import secure_filename

SCENES_FOLDER = os.environ['GEOPORTAL_SCENES_FOLDER']
SCENES_EXTRACT_FOLDER = os.environ['GEOPORTAL_SCENES_EXTRACT_FOLDER']

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

@app.route('/scene/<sceneid>/<int:minband>/<int:maxband>')
def scene(sceneid, minband, maxband):
    app.logger.info('Scene %s %d %d', sceneid, minband, maxband)

    zipfilepath = SCENES_FOLDER + "/" + sceneid[:23] + "1T.ZIP"

    if not os.path.isfile(zipfilepath):
        app.logger.warning('Filepath %s is not found!', zipfilepath)
        return 'NO FILE'

    app.logger.info('Filepath %s is found', zipfilepath)

    extractfolder = SCENES_EXTRACT_FOLDER + "/" + sceneid

    output = 'SUCCESS\n'

    with ZipFile(zipfilepath) as zip:
        for i in range(minband, maxband + 1):
            filename = sceneid[:23] + "B{0:0>3}_L1T.TIF".format(i)

            if not os.path.isfile(extractfolder + "/" + filename):
                zip.extract(filename, extractfolder)

            output += 'http://virtualglobe.ru/geoportal/hyperion/scenes/' + sceneid + '/' + filename + '\n'

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
