import sys
sys.path.insert(0, '/opt/geoportal/geoportal/server')

import os
os.environ['GEOPORTAL_UPLOAD_FOLDER']='/opt/geoportal/temp'
os.environ['GEOPORTAL_PUBLIC_FOLDER']='/opt/virtualglobe.ru/geoportal'
os.environ['GEOPORTAL_SCENES_FOLDER']='/opt/geoportal/Hyperion/scenes'

from server import app as application

import logging
application.logger.setLevel(logging.INFO)

from logging import FileHandler
file_handler = FileHandler('/opt/geoportal/geoportal/server/pylog.txt')
file_handler.setLevel(logging.INFO)
application.logger.addHandler(file_handler)
