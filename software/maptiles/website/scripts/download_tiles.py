#!/usr/bin/env python3
# Description: This script downloads map tiles of a specified area from a specified server.

import argparse
import logging
import math
import mysql.connector
import os
import shutil
import sys
import tarfile
import urllib.request
import urllib.parse

from logging.handlers import RotatingFileHandler
from mysql.connector import Error

from config.db_config import DB_CONFIG

def initLogger(logFile, maxBytes, backupCount, encoding, logLevel):
    os.makedirs(os.path.dirname(logFile), exist_ok=True)
    
    logger = logging.getLogger()
    logger.setLevel(logLevel)

    if not logger.handlers:
        file_handler = RotatingFileHandler(filename=logFile, maxBytes=maxBytes, backupCount=backupCount, encoding=encoding)
        formatter = logging.Formatter('%(process)d %(asctime)s %(levelname)s: %(filename)s(%(lineno)d): %(message)s')
        file_handler.setFormatter(formatter)
        logger.addHandler(file_handler)

class maptilesDB:
    def __init__(self):
        self.config = DB_CONFIG
        self.connection = None
        self.cursor = None

    def update_progress(self, task_id: int, percentage: float) -> bool:
        if not (self.connection and self.connection.is_connected()):
            try:
                self.connection = mysql.connector.connect(** self.config)
                self.cursor = self.connection.cursor()
                logging.info("Database connection established.")
            except Error as e:
                logging.error(f"Database connection failed: {str(e)}")
                return False
        try:
            sql = """
                UPDATE tasks 
                SET progress = %s 
                WHERE id = %s
            """
            self.cursor.execute(sql, (percentage, task_id))
            self.connection.commit()
            
            if self.cursor.rowcount == 0:
                logging.debug(f"Task {task_id} does not exist, no update performed.")
                return False
                
            logging.debug(f"Task {task_id} progress updated to {percentage}")
            return True
        except Error as e:
            logging.error(f"Task {task_id} update failed: {str(e)}")
            self.connection.rollback()
            return False

    def __del__(self):
        if self.connection and self.connection.is_connected():
            try:
                self.cursor.close()
                self.connection.close()
            except Error as e:
                logging.error(f"Close connection failed: {str(e)}")

def get_extension_from_url(url):
    temp_url = url.format(x=0, y=0, z=0)
    path = urllib.parse.urlparse(temp_url).path
    filename = os.path.basename(path)
    _, ext = os.path.splitext(filename)
    return ext[1:] if ext else ""

def download_file(url:str, destination:str):
    try:
        tmp_path, _ = urllib.request.urlretrieve(url)
        shutil.move(tmp_path, destination)
        print("Download %s form %s success." % (destination, url))
    except Exception as e:
        print("Download %s form %s failed." % (destination, url), str(e))

def get_tile_index(longitude:float, latitude:float, zoom:int) -> list[int]:
    x_index = int((longitude + 180.0) / 360.0 * (1 << zoom))
    y_index = int((1.0 - math.asinh(math.tan(math.radians(latitude))) / math.pi) / 2.0 * (1 << zoom))
    return [x_index, y_index]

if __name__ == '__main__':
    initLogger('logs/download_tiles.log', 1024*1024*10, 10, 'utf-8', logging.INFO)

    parser = argparse.ArgumentParser(description='Download map tiles of a specified area.')

    parser.add_argument('coordinates', type=float, nargs=4, help='Coordinates of the boundary of area (order: west, south, east, north)')
    parser.add_argument('-z', '--zoom', type=int, nargs=2, help='Zoom level range of the map tiles (min, max)', default=[12, 17])
    parser.add_argument('-u', '--url', type=str, help='Server URL ({x}, {y} and {z} will be replaced with the tile index), e.g.: https://tile.example.org/{z}/{x}/{y}.png?key=12345')
    parser.add_argument('-o', '--output-folder', type=str, help='Destination folder to save map tiles', default='.')
    parser.add_argument('-e', '--extension', type=str, help='File extension of the map tiles', default='')
    parser.add_argument('-t','--task-id', type=int, help='Task ID in the database', default=-1)
    parser.add_argument('--overwrite', action='store_true', help='Overwrite existing files', default=False)

    args = parser.parse_args()

    if args.extension == '':
        args.extension = get_extension_from_url(args.url)

    logging.info(args)

    tasks = {'total': 0, 'downloaded': 0, 'tiles': {}}
    db = None

    if args.task_id >= 0:
        db = maptilesDB()
    else:
        logging.warning("No task ID specified, database progress will not be updated.")

    for zoom in range(args.zoom[0], args.zoom[1]+1):
        west, north = get_tile_index(args.coordinates[0], args.coordinates[1], zoom)
        east, south = get_tile_index(args.coordinates[2], args.coordinates[3], zoom)
        tasks['total'] += (east - west + 1) * (south - north + 1)
        tasks['tiles'][f'{zoom}'] = {}
        url_zoom = args.url.replace('{z}', str(zoom))
        for x in range(west, east+1):
            os.makedirs('%s/%d/%d' % (args.output_folder, zoom, x), exist_ok=True)
            url_x = url_zoom.replace('{x}', str(x))
            tasks['tiles'][f'{zoom}'][f'{x}'] = {}
            for y in range(north, south+1):
                url = url_x.replace('{y}', str(y))
                destination = '%s/%d/%d/%d.%s' % (args.output_folder, zoom, x, y, args.extension)
                status = 0 if (args.overwrite or not os.path.exists(destination)) else 1
                tasks['downloaded'] += status
                tasks['tiles'][f'{zoom}'][f'{x}'][f'{y}'] = {
                    'url': url,
                    'destination': destination,
                    'status': status,
                }

    for zoom in tasks['tiles']:
        for x in tasks['tiles'][zoom]:
            for y in tasks['tiles'][zoom][x]:
                if tasks['tiles'][zoom][x][y]['status'] == 0:
                    download_file(tasks['tiles'][zoom][x][y]['url'], tasks['tiles'][zoom][x][y]['destination'])
                    if os.path.exists(tasks['tiles'][zoom][x][y]['destination']):
                        tasks['tiles'][zoom][x][y]['status'] = 1
                        tasks['downloaded'] += 1
                        if (args.task_id > 0 and db):
                            percentage = (tasks['downloaded'] / tasks['total'] * 100) if tasks['total'] else 0
                            db.update_progress(args.task_id, percentage)
                    else:
                        logging.error(f"Download {tasks['tiles'][zoom][x][y]['url']} to {tasks['tiles'][zoom][x][y]['destination']} failed")
                else:
                    logging.debug(f"Tile {zoom}/{x}/{y} already exists, skip.")

    if (tasks['downloaded'] == tasks['total']):
        tgz_file = f'{args.output_folder}/tiles.tar.gz'
        with tarfile.open(tgz_file, 'w:gz') as tar:
            for zoom in tasks['tiles']:
                tar.add(f'{args.output_folder}/{zoom}', f'{zoom}')
        logging.info(f"All map tiles downloaded. Tar file: {tgz_file}")
    else:
        logging.warning(f"Not all map tiles downloaded. {tasks['downloaded']} of {tasks['total']} tiles downloaded.")

    if (args.task_id > 0 and db):
        del db

# End of download_tiles.py