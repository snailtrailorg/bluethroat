#!/usr/bin/env python3
# Description: This script downloads map tiles of a specified area from a specified server.

import argparse
import logging
import math
import mysql.connector
import os
import shutil
import tarfile
import urllib.request
import urllib.parse

from logging.handlers import RotatingFileHandler
from mysql.connector import Error
from pathlib import Path
from urllib.parse import urlparse

from config.config import DB_CONFIG, LOG_CONFIG, TILE_DESCRIPTION_FILE

def initLogger(file=LOG_CONFIG['log_file'], bytes=LOG_CONFIG['max_bytes'], backup=LOG_CONFIG['backup_count'], coding=LOG_CONFIG['encoding'], level=LOG_CONFIG['level'], format=LOG_CONFIG['format'], date_format=LOG_CONFIG['datefmt']):
    os.makedirs(os.path.dirname(file), exist_ok=True)
    
    logger = logging.getLogger()
    logger.setLevel(level)

    if not logger.handlers:
        file_handler = RotatingFileHandler(filename=file, maxBytes=bytes, backupCount=backup, encoding=coding)
        formatter = logging.Formatter(fmt=format, datefmt=date_format)
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
                logging.debug(f"Update task {task_id}, database connection established.")
            except Error as e:
                logging.error(f"Update task {task_id}, database connection failed: {str(e)}")
                return False
        try:
            sql = """
                UPDATE tasks 
                SET progress = %s 
                WHERE id = %s
            """
            self.cursor.execute(sql, (percentage, task_id))
            self.connection.commit()

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
                logging.error(f"Close database connection failed: {str(e)}")

def get_extension_from_url(url):
    temp_url = url.format(x=0, y=0, z=0)
    path = urllib.parse.urlparse(temp_url).path
    filename = os.path.basename(path)
    _, ext = os.path.splitext(filename)
    return ext[1:] if ext else ""

def download_file(url:str, destination:str):
    try:
        tmp_path, _ = urllib.request.urlretrieve(url)
        os.makedirs(os.path.dirname(destination), exist_ok=True)
        shutil.move(tmp_path, destination)
        os.chmod(destination, 0o644)
        logging.debug("Download %s form %s success." % (destination, url))
    except Exception as e:
        logging.error("Download %s form %s failed." % (destination, url), str(e))

def get_tile_index(longitude:float, latitude:float, zoom:int) -> list[int]:
    x_index = int((longitude + 180.0) / 360.0 * (1 << zoom))
    y_index = int((1.0 - math.asinh(math.tan(math.radians(latitude))) / math.pi) / 2.0 * (1 << zoom))
    return [x_index, y_index]

if __name__ == '__main__':
    initLogger()
    logging.info("Download tiles script started.")

    parser = argparse.ArgumentParser(description='Download map tiles of a specified area.')

    parser.add_argument('coordinates', type=float, nargs=4, help='Coordinates of the boundary of area (order: west, south, east, north)')
    parser.add_argument('-z', '--zoom', type=int, nargs=2, help='Zoom level range of the map tiles (min, max)', default=[12, 17])
    parser.add_argument('-u', '--url', type=str, help='Server URL ({x}, {y} and {z} will be replaced with the tile index), e.g.: https://tile.example.org/{z}/{x}/{y}.png?key=12345')
    parser.add_argument('-o', '--output-folder', type=str, help='Destination folder to save map tiles', default='.')
    parser.add_argument('-t','--task-id', type=int, help='Task ID in the database', default=-1)
    parser.add_argument('--overwrite', action='store_true', help='Overwrite existing files', default=False)

    args = parser.parse_args()
    logging.debug(args)
    print(args)

    clean_url = args.url
    if clean_url:
        p = urlparse(clean_url)
        clean_url = f"{p.scheme}://{p.netloc}{p.path}"

    try:
        os.makedirs(args.output_folder, exist_ok=True)
        with open(os.path.join(args.output_folder, TILE_DESCRIPTION_FILE), 'w') as f:
            f.write(f"Range coordinates[west, south, east, north]: {args.coordinates}\n")
            f.write(f"Zoom[min, max]: {args.zoom}\n")
            f.write(f"Server URL: {clean_url}\n")
            f.write(f"Task-ID: {args.task_id}\n")
    except Exception as e:
        logging.error(f"Write tile description file failed: {str(e)}")

    tasks = {'total': 0, 'downloaded': 0, 'tiles': {}}
    db = None

    if args.task_id >= 0:
        db = maptilesDB()
    else:
        logging.warning("No task ID specified, database progress will not be updated.")

    for z in range(args.zoom[0], args.zoom[1]+1):
        west, north = get_tile_index(args.coordinates[0], args.coordinates[1], z)
        east, south = get_tile_index(args.coordinates[2], args.coordinates[3], z)
        tasks['total'] += (east - west + 1) * (south - north + 1)
        url_z = args.url.replace('{z}', str(z))
        destination_z = args.output_folder + urlparse(args.url).path.replace('{z}', str(z))
        for x in range(west, east+1):
            url_x = url_z.replace('{x}', str(x))
            destination_x = destination_z.replace('{x}', str(x))
            for y in range(north, south+1):
                url = url_x.replace('{y}', str(y))
                destination = destination_x.replace('{y}', str(y))
                status = 0 if (args.overwrite or not os.path.exists(destination)) else 1
                tasks['downloaded'] += status
                tasks['tiles'][f'{z}_{x}_{y}'] = {
                    'url': url,
                    'destination': destination,
                    'status': status,
                }

    last_percentage = 0
    for tile in tasks['tiles']:
        if tasks['tiles'][tile]['status'] == 0:
            download_file(tasks['tiles'][tile]['url'], tasks['tiles'][tile]['destination'])
            if os.path.exists(tasks['tiles'][tile]['destination']):
                tasks['tiles'][tile]['status'] = 1
                tasks['downloaded'] += 1
                if (args.task_id > 0 and db):
                    percentage = (tasks['downloaded'] / tasks['total'] * 100) if tasks['total'] else 0
                    if percentage - last_percentage >= 0.01:
                        db.update_progress(args.task_id, percentage)
                        last_percentage = percentage

    if (tasks['downloaded'] == tasks['total']):
        tgz_file = f'{args.output_folder}/tiles.tar.gz'
        with tarfile.open(tgz_file, 'w:gz') as tar:
            for subdir in Path(args.output_folder).iterdir():
                if subdir.is_dir():
                    tar.add(subdir, subdir.name)
        logging.info(f"All of {tasks['total']} map tiles downloaded. Tar file: {tgz_file}")
    else:
        logging.warning(f"Partial {tasks['downloaded']} of {tasks['total']} map tiles downloaded.")

    if (args.task_id > 0 and db):
        del db

# End of download_tiles.py