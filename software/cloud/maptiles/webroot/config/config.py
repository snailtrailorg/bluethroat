from pathlib import Path

DB_CONFIG = {
    "host": "localhost",
    "user": "worker",
    "password": "(xQRy/)PQ!pzkBXE",
    "database": "maptiles"
}

TILE_DESCRIPTION_FILE = ".description"
PID_FILE = ".pid"

LOG_CONFIG = {
    "log_file": f"{Path(__file__).resolve().parent.parent}/logs/python_scripts.log",
    "max_bytes": 1024 * 1024 * 10,
    "backup_count": 5,
    "encoding": "utf-8",
    "level": "INFO",
    "format": "%(process)d %(asctime)s %(levelname)s: %(filename)s(%(lineno)d): %(message)s",
    "datefmt": "%Y-%m-%d %H:%M:%S"
}