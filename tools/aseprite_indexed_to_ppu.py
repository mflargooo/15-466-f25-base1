from os import path, getcwd
import sys
from PIL import Image

import numpy as np
from scipy.signal import convolve2d

import struct

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
'''





help = "Usage: python ./aseprite_indexed_to_ppu.py [options] <tilesheet_0.png> ... <tilesheet_n.png>\n" \
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

paths = ["" for _ in range(len(sys.argv) - 1)]
total_nonempty_tiles = 0

for i in range(len(sys.argv) - 1):
    if (cwd + "/" + sys.argv[i + 1]) in paths:
        continue

    paths[i] = cwd + "/" + sys.argv[i + 1]

    if not path.exists(paths[i]):
        raise FileNotFoundError("Could not find " + sys.argv[-1] + "! Aborting program.")
    
    with Image.open(paths[i]) as ts:
        if ts.mode != "P":
            raise AssertionError("Image " + sys.argv[i + 1] + " does not have Pillow's color mode \"P\"")
                
        w, h = ts.size
        if ((w // 8) * 8 != w):
            raise AssertionError("Image " + sys.argv[i + 1] + " does not have width divisible by 8")
        if ((h // 8) * 8 != h):
            raise AssertionError("Image " + sys.argv[i + 1] + " does not have height divisible by 8")

        if(np.min(ts) < 0 or np.max(ts) > 3):
            raise ValueError("Image " + sys.argv[i + 1] + " has an index not in range [0, 3]")
        
        tile_sum = convolve2d(ts, in2=np.ones((8,8), dtype=np.uint8), mode='valid')[::8, ::8]
        total_nonempty_tiles += np.sum(tile_sum > 0)

    if total_nonempty_tiles > 256:
        raise RuntimeError("Sum of nonempty tiles among all provided tilesheets >256")
    
# now we process all the tilesheets into one tiletable
tilesheet_path = cwd + "/" + sys.argv[-1]
if not path.exists(tilesheet_path):
    raise FileNotFoundError("Could not find " + sys.argv[-1])

with Image.open(tilesheet_path) as ts:
    w, h = ts.size

    # for each tile up to 256 tiles, store bit0 and bit1 of each flattened 8x8 tile.
    uncompressed_tiletable = np.zeros((256, 2, 8 * 8), dtype=np.uint8)

    ts_arr = np.array(ts)
    pixel_row, pixel_col = np.indices(ts_arr.shape)

    # get the index of the tile each pixel belongs to, such that
    # each tile is listed in left-to-right, top-to-bottom order
    tile_idxs = pixel_row // 8 * (w // 8) + pixel_col // 8
    offset_idxs = np.tile(np.arange(64, dtype=int).reshape((8,8)), (h // 8, w // 8))

    # split the tiles into bit0 and bit1
    uncompressed_tiletable[tile_idxs, 0, offset_idxs] = np.bitwise_and(ts_arr, 0b1)
    uncompressed_tiletable[tile_idxs, 1, offset_idxs] = np.bitwise_and(np.right_shift(ts_arr, 1), 0b1)

    # packbits into one uint8 per row
    compressed_tiletable = np.packbits(uncompressed_tiletable.reshape(256, 2, 8, 8), axis=3, bitorder='little').reshape((256, 16))

    tt_bytes = compressed_tiletable.tobytes()