import os
dir = os.path.dirname(os.path.abspath(__file__))

import sys
sys.path.insert(0, dir)

os.environ['GEOPORTAL_UPLOAD_FOLDER']='/opt/hyexplorer/temp'
os.environ['GEOPORTAL_PUBLIC_FOLDER']='/opt/www/geoportal'
os.environ['GEOPORTAL_SCENES_FOLDER']='/opt/hyexplorer/Hyperion/scenes'

from server import app as application

import logging
application.logger.setLevel(logging.INFO)

from logging import FileHandler
file_handler = FileHandler(dir + '/pylog.txt')
file_handler.setLevel(logging.INFO)
application.logger.addHandler(file_handler)
