import os
import os.path
import psycopg2
from flask import Flask, request
from werkzeug import secure_filename

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
    return 'Hello World!'

@app.route('/overview/<sceneid>', methods=['GET', 'POST'])
def overview(sceneid):
    if request.method == 'POST':
        for name, file in request.files.items():
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
        return 'Success POST'
    else:
        return 'Success GET'

if __name__ == '__main__':
    #app.debug = True
    app.run(host='0.0.0.0')
