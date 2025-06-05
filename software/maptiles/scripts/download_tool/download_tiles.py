#!/usr/bin/env python3
# Description: This script downloads map tiles of a specified area from a specified server.

import argparse
import math
import urllib.request
import os

def get_first_non_blank_line_from_file(file:str) -> str:
    with open(file, 'r') as f:
        while (line := f.readline()) != '':
            if (line := line.strip('\r\n\t ')) == '':
                continue
            else:
                break
    return line

def download_file(url:str, destination:str):
    try:
        urllib.request.urlretrieve(url, destination)
        print("File %s download success." % destination)
    except Exception as e:
        print("File %s download failed." % destination, str(e))

def get_tile_index(longitude:float, latitude:float, zoom:int) -> list[int]:
    x_index = int((longitude + 180.0) / 360.0 * (1 << zoom))
    y_index = int((1.0 - math.asinh(math.tan(math.radians(latitude))) / math.pi) / 2.0 * (1 << zoom))
    return [x_index, y_index]

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Download map tiles of a specified area.')
    parser.add_argument('coordinates', type=float, nargs=4, help='Coordinates of the boundary of area (order: west, south, east, north)')
    parser.add_argument('-z', '--zoom', nargs=2, type=int, help='Zoom level of the map (min, max)', default=[12, 17])
    parser.add_argument('-p', '--protocol', type=str, help='Protocol to use for downloading map tiles', default='https', choices=['http', 'https'])
    parser.add_argument('-s', '--server', type=str, help='Server (and path befor {z}/{x}/{y} if any) to download map tiles', default='tile.thunderforest.com/cycle')
    parser.add_argument('-e', '--extension', type=str, help='Extension of map tile files', default='png')
    parser.add_argument('-k', '--apikey-file', type=str, help='Fiel contains API key string in first non-blank line, entire "key=value" string needed, if required', default='apikey.thunderfotest')
    parser.add_argument('-o', '--output-folder', type=str, help='Destination folder to save map tiles', default='.')
    parser.add_argument('--overwrite', action='store_true', help='Overwrite existing files')
    args = parser.parse_args()

    print(args)

    apikey_string = get_first_non_blank_line_from_file(args.apikey_file)

    for zoom in range(args.zoom[0], args.zoom[1]+1):
        west, south = get_tile_index(args.coordinates[0], args.coordinates[1], zoom)
        east, north = get_tile_index(args.coordinates[2], args.coordinates[3], zoom)
        print("Proccessing zoom level: %d, x range from %d to %d, y range from %d to %d." % (zoom, west, east, north, south))
        for x in range(west, east+1):
            for y in range(north, south+1):
                url = '%s://%s/%d/%d/%d.%s?%s' % (args.protocol, args.server, zoom, x, y, args.extension, apikey_string)
                destination = '%s/%d/%d/%d.%s' % (args.output_folder, zoom, x, y, args.extension)
                if not os.path.exists('%s/%d/%d' % (args.output_folder, zoom, x)):
                    os.makedirs('%s/%d/%d' % (args.output_folder, zoom, x))
                if args.overwrite or not os.path.exists(destination):
                    pass
                    download_file(url, destination)
                    print("Downloading %s from %s://%s/%d/%d/%d.%s success." % (destination, args.protocol, args.server, zoom, x, y, args.extension))
                else:
                    print("File %s already exists." % destination)

    print("All map tiles downloaded.")

# End of download_tiles.py