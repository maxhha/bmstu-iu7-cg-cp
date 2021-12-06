from PIL import Image
import sys
from pathlib import Path

if len(sys.argv) < 3:
    print("Usage: python3 raw2png.py <path to .dat> <dir to store pngs>")
    exit(1)

DAT_FILE_NAME = Path(sys.argv[1])
SAVE_FILE_DIR = Path(sys.argv[2])

if not SAVE_FILE_DIR.is_dir():
    print(f"{SAVE_FILE_DIR} must be a dir")
    exit(1)

raw_file_name = None
shape = None

with open(DAT_FILE_NAME, "r") as f:
    for line in f:
        token, value = line.split(":")
        value = value.strip(" \n\t")
        if token == "Resolution":
            shape = tuple(map(int, value.split(" ")))
        if token == "ObjectFileName":
            raw_file_name = value

if raw_file_name is None:
    print("ObjectFileName not found in .dat file")
    exit(1)

if shape is None:
    print("Resolution not found in .dat file")
    exit(1)

with open(DAT_FILE_NAME.joinpath(f"../{raw_file_name}"), "rb") as f:
    for z in range(shape[2]):
        file_name = SAVE_FILE_DIR.joinpath(f"{z}.png")
        img = Image.frombytes("L", shape[0:2],  f.read(shape[0]*shape[1]))
        img.save(str(file_name))
        print(f"Created {file_name}")
    
print("Finished")
