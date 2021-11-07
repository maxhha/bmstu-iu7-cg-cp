DAT_FILE_NAME = "sphere.dat"
RAW_FILE_NAME = "sphere.raw"

DIM_X = 128
DIM_Y = 128
DIM_Z = 128

SCALE_X = 1
SCALE_Y = 1
SCALE_Z = 1

CENTER_X = 64
CENTER_Y = 64
CENTER_Z = 64

with open(DAT_FILE_NAME, "w") as f:
    f.write(f"ObjectFileName: {RAW_FILE_NAME}\n")
    f.write(f"Resolution: {DIM_X} {DIM_Y} {DIM_Z}\n")
    f.write(f"SliceThickness: {SCALE_X} {SCALE_Y} {SCALE_Z}\n")
    f.write(f"Format: UCHAR\n")

result = []

for z in range(DIM_Z):
    for y in range(DIM_Y):
        for x in range(DIM_X):
            result.append(
                (x - CENTER_X) ** 2 +
                (y - CENTER_Y) ** 2 +
                (z - CENTER_Z) ** 2
            )

max_v = max(result)
min_v = min(result)
d = max_v - min_v

result = bytes(int((i - min_v) / d * 255) for i in result)

with open(RAW_FILE_NAME, "wb") as f:
    f.write(result)
