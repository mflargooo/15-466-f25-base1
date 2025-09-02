from os import path, getcwd, mkdir
import sys
from PIL import Image

import numpy as np
from scipy.signal import convolve2d

'''
Takes a a sequence of PNG image generated in "Indexed" color mode and converts 
into data that matches the tiletable defined in the PPU headers. A given image 
is split into tiles in left-to-right, top-to-bottom order, and a continguous 
block of data containing the tile information is returned.

The returned tiletable contains the all the data in the order the tilesheets were
provided. Tiles within each tilesheet are placed in tiletable in left-to-right,
top-to-bottom order.

Ex. Each of the following entries in tilesheet_i label tile t of 8x8 pixels in 
    that tilesheet. Thus, tilesheet_0 has image width 32 px and image height 16 
    px, and tilesheet_1 has image width 16 px and image height 24 px.

    The result, tiletable, labels i_t for the 16 byte data corresponding to 
    tile t of tilesheet_i as PPU requires 

tilesheet_0          tilesheet_1                tiletable 
[[0,1,2,3],    +     [[0,1],            =       [0_0, 0_1, ..., 0_7, 1_0, 1_1, ..., 1_5]
 [4,5,6,7]]           [2,3],
                      [4,5]]

if any tile in the tilesheet is empty (all transparent), then that tile is 
skipped, but tile data is still contiguous.
'''


def lookat_tile_i(tiletable, i):
    for j in range(8):
        res = []
        r0 = tiletable[i,j]
        r1 = tiletable[i,j+8]
        for _ in range(8):
            res.append((r0 & 1) | ((r1 & 1) << 1))
            r0 >>= 1
            r1 >>= 1

        print(res)

help = "Usage: python ./indexed_to_ppu.py [options] <tilesheet_0.png> ... <tilesheet_n.png>\n" \
"options:\n" \
"   -h              \t help\n" \

if sys.argv == 1 or "-h" in sys.argv:
    print(help)
    sys.exit()

cwd = getcwd()

# We do not want to return output to user if they are expecting all 
# tilesheets to be split correctly and cleanly into a single tiletable

# We check that
#   1. All tilesheets can be found
#   2. All tilesheets are in Pillow's color mode "P" (indexed mode)
#   3. All tilesheets have width and height divisible by 8
#   4. All tilesheets contain indicies only in {0, 1, 2, 3}
#   5. The sum of all non-empty 8x8 tiles in all tilesheets does not exceed
#      256 (the PPU tiletable capacity)

paths = []
nonempty_tiles = []
total_nonempty_tiles = 0

for i in range(len(sys.argv) - 1):
    if cwd + "/" + sys.argv[i + 1] in paths:
        continue

    paths.append(cwd + "/" + sys.argv[i + 1])

    if not path.exists(paths[-1]):
        raise FileNotFoundError("Could not find " + sys.argv[-1] + "! Aborting program.")
    
    with Image.open(paths[-1]) as ts:
        if ts.mode != "P":
            raise AssertionError("Image " + sys.argv[i + 1] + " does not have Pillow's color mode \"P\"")
                
        w, h = ts.size
        if ((w // 8) * 8 != w):
            raise AssertionError("Image " + sys.argv[i + 1] + " does not have width divisible by 8")
        if ((h // 8) * 8 != h):
            raise AssertionError("Image " + sys.argv[i + 1] + " does not have height divisible by 8")

        if(np.min(ts) < 0 or np.max(ts) > 3):
            raise ValueError("Image " + sys.argv[i + 1] + " has an index not in range [0, 3]")
        

        nonempty_tiles.append([0, (convolve2d(ts, in2=np.ones((8,8), dtype=np.uint8), mode='valid')[::8, ::8] > 0).astype(np.uint8)])
        nonempty_tiles[-1][0] = np.sum(nonempty_tiles[-1][1], dtype=int)
        total_nonempty_tiles += nonempty_tiles[-1][0]

    if total_nonempty_tiles > 256:
        raise RuntimeError("Sum of nonempty tiles among all provided tilesheets >256")
    
    
# now we process all the tilesheets into one tiletable

# for each tile up to 256 tiles, store bit0 and bit1 of each flattened 8x8 tile.
uncompressed_tiletable = np.zeros((256, 2, 8 * 8), dtype=np.uint8)

total_tiles_filled = 0
for i in range(len(paths)):
    with Image.open(paths[i]) as ts:
        w, h = ts.size
        ts_arr = np.array(ts)

        # get the index of the tile each pixel belongs to, such that
        # each tile is listed in left-to-right, top-to-bottom order
        tile_idxs = (np.cumsum(nonempty_tiles[i][1]).reshape(nonempty_tiles[i][1].shape) - 1 + total_tiles_filled) * nonempty_tiles[i][1]
        tile_px_idxs = np.kron(tile_idxs, np.ones((8,8), dtype=np.uint8))
        offset_px_idxs = np.tile(np.arange(64, dtype=np.uint8).reshape((8,8)), (h // 8, w // 8))

        valid_tile_pxs = np.where(np.kron(nonempty_tiles[i][1], np.ones((8,8), dtype=np.uint8)) == 1)

        # split the tiles into bit0 and bit1
        uncompressed_tiletable[tile_px_idxs[valid_tile_pxs], 0, offset_px_idxs[valid_tile_pxs]] = np.bitwise_and(ts_arr[valid_tile_pxs], 0b1)
        uncompressed_tiletable[tile_px_idxs[valid_tile_pxs], 1, offset_px_idxs[valid_tile_pxs]] = np.bitwise_and(np.right_shift(ts_arr[valid_tile_pxs], 1), 0b1)

        total_tiles_filled += nonempty_tiles[i][0]


# packbits into one uint8 per row, flip rows because PPU indexes and numpy index image rows differently, and convert to bytes
compressed_tiletable = np.packbits(uncompressed_tiletable.reshape(256, 2, 8, 8), axis=3, bitorder='little')[:,:,::-1].reshape((256, 16))
tt_bytes = compressed_tiletable.tobytes()

if not path.exists("./assets"):
    mkdir("./assets")
    
with open("assets/tiletable.tt", "wb") as f:
    f.write(tt_bytes)