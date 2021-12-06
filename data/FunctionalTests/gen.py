
def s(v, W):
    return (
        1 if v < -W else
        (W - v) / 2 / W if -W <= v < W else
        0
    ) 

def generate(name, fn):
    DAT_FILE_NAME = f"{name}.dat"
    RAW_FILE_NAME = f"{name}.raw"

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
                    fn(x - CENTER_X, y - CENTER_Y, z - CENTER_Z)
                )

    max_v = max(result)
    min_v = min(result)
    d = max_v - min_v

    result = bytes(int((i - min_v) / d * 255) for i in result)

    with open(RAW_FILE_NAME, "wb") as f:
        f.write(result)

# generate("ball", lambda x, y, z: s((x**2 + y**2 + z**2)**0.5 - 40, 5))
# generate("cube", lambda x, y, z: s(max(abs(x), abs(y), abs(z)) - 40, 0))
# generate("cylinder", lambda x, y, z: s((x**2 + y**2)**0.5 - 40, 5)*s(abs(z) - 60, 0))
generate("cube_sub_ball", lambda x, y, z: s(max(abs(x), abs(y), abs(z)) - 40, 0)*(1 - s((x**2 + y**2 + z**2)**0.5 - 50, 5)))
