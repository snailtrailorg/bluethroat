#!/usr/bin/env python3
# Description: This script downloads map tiles of a specified area from a specified server.

import argparse
import math
import urllib.request
import os
import urllib.parse
import shutil
import json

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
    parser = argparse.ArgumentParser(description='Download map tiles of a specified area.')
    parser.add_argument('coordinates', type=float, nargs=4, help='Coordinates of the boundary of area (order: west, south, east, north)')
    parser.add_argument('-z', '--zoom', type=int, nargs=2, help='Zoom level range of the map tiles (min, max)', default=[12, 17])
    parser.add_argument('-u', '--url', type=str, help='Server URL ({x}, {y} and {z} will be replaced with the tile index), e.g.: https://tile.example.org/{z}/{x}/{y}.png?key=12345')
    parser.add_argument('-o', '--output-folder', type=str, help='Destination folder to save map tiles', default='.')
    parser.add_argument('-e', '--extension', type=str, help='File extension of the map tiles', default='')
    parser.add_argument('--overwrite', action='store_true', help='Overwrite existing files', default=False)
    args = parser.parse_args()
    if args.extension == '':
        args.extension = get_extension_from_url(args.url)

    print(args)

    progress = {
        'total': 0,
        'processed': 0,
        'failed': [],
        'success': 0,
        'z': {
            'min': args.zoom[0],
            'max': args.zoom[1],
        }
    }

    for zoom in range(args.zoom[0], args.zoom[1]+1):
        west, north = get_tile_index(args.coordinates[0], args.coordinates[1], zoom)
        east, south = get_tile_index(args.coordinates[2], args.coordinates[3], zoom)
        progress['total'] += (east - west + 1) * (south - north + 1)
        progress['z'][zoom] = {
            'x': {
                'min': west,
                'max': east,
            },
            'y': {
                'min': north,
                'max': south,
            },
            'total': (east - west + 1) * (south - north + 1),
            'processed': 0,
            'failed': [],
            'success': 0,
        }
        print(progress)

    for zoom in range(args.zoom[0], args.zoom[1]+1):
        for x in range(progress['z'][zoom]['x']['min'], progress['z'][zoom]['x']['max']+1):
            for y in range(progress['z'][zoom]['y']['min'], progress['z'][zoom]['y']['max']+1):
                url = args.url.format(x=x, y=y, z=zoom)
                destination = '%s/%d/%d/%d.%s' % (args.output_folder, zoom, x, y, args.extension)
                if not os.path.exists('%s/%d/%d' % (args.output_folder, zoom, x)):
                    os.makedirs('%s/%d/%d' % (args.output_folder, zoom, x))
                if args.overwrite or not os.path.exists(destination):
                    download_file(url, destination)
                else:
                    print("File %s already exists." % destination)
                progress['processed'] += 1
                progress['z'][zoom]['processed'] += 1
                if os.path.exists(destination):
                    progress['success'] += 1
                    progress['z'][zoom]['success'] += 1
                else:
                    progress['failed'].append(z + '/' + x + "/" + y + "." + args.extension)
                    progress['z'][zoom]['failed'].append(z + '/' + x + "/" + y + "." + args.extension)
                with open(args.output_folder + '/.progress', 'w') as f_progress:
                    percentage = (progress['success'] / progress['total'] * 100) if progress['total'] else 0
                    f_progress.write(f"{(f'{percentage:.2f}'.rstrip('0').rstrip('.') or '0')}%")

            with open(args.output_folder + '/.detail.json', 'w') as f_detail:
                json.dump(progress, f_detail, indent=4)

    print("All map tiles processed.")

# End of download_tiles.py