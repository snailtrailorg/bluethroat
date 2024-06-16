#!/usr/bin/env python3
# Description: This script downloads a file from a URL and saves it to a specified destination.

import argparse
import math
import urllib.request

def download_file(url:str, destination:str):
    try:
        urllib.request.urlretrieve(url, destination)
        print("File downloaded successfully!")
    except Exception as e:
        print("Error downloading file:", str(e))

def get_tile_index(longitude:float, latitude:float, zoom:int) -> list[int]:
    x_index = int((longitude + 180.0) / 360.0 * (1 << zoom))
    y_index = int((1 - math.log(math.tan(latitude.toRad()) + 1 / math.cos(latitude.toRad())) / math.PI()) / 2 * (1 << zoom))
    return [x_index, y_index]

parser = argparse.ArgumentParser(description='Download map tiles of a specified area.')
parser.add_argument('coordinates', type=float, nargs=4, help='Coordinates of the boundary of area (order: west, north, east, south)')
parser.add_argument('-z', '--zoom', nargs=2, type=int, help='Zoom level of the map (min, max)', default=[12, 17])
parser.add_argument('-p', '--protocol', type=str, help='Protocol to use for downloading map tiles', default='https', choices=['http', 'https'])
parser.add_argument('-s', '--server', type=str, help='Server (and path if any befor {z}/{x}/{y}) to download map tiles', default='tile.thunderforest.com/cycle')
parser.add_argument('-e', '--extension', type=str, help='Extension of map tile files', default='png')
parser.add_argument('-k', '--apikey', type=str, help='API key to use for downloading map tiles, entire "key=value" string, if required', default='apikey=cb3553b5305749f8a585e4e7ce4d56f1')
parser.add_argument('-o', '--output-folder', type=str, help='Destination folder to save map tiles', default='.')
parser.add_argument('--overwrite', action='store_true', help='Overwrite existing files')

if __name__ == '__main__':
    args = parser.parse_args()
