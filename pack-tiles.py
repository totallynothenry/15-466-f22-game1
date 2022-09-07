from PIL import Image
import numpy as np

DEBUG = False
PALETTE_SIZE = 4
TILE_SIZE = 8

'''
This code assumes the input images are rgba images already reduced to
4 colors and that all images are made up of 8x8 tiles. Unfortunately, gimp does
not produce indexed color images where colors are RGBA (only RGB)...
'''

def img_to_tiles(filename):
    img = Image.open(filename)
    w, h = img.size

    assert w % TILE_SIZE == 0 and h % TILE_SIZE == 0

    color_count = 0
    color_map = {}
    indices = np.zeros((h, w), dtype=np.uint8)
    for row in range(h):
        for col in range(w):
            color = img.getpixel((col,row))
            idx = color_map.get(color)
            if idx == None:
                # new color
                idx = color_count
                color_count += 1
                if color_count > 4:
                    assert False
                color_map[color] = idx
            indices[row, col] = idx
    if DEBUG:
        print(color_map)
        print(indices)

    # color map to palette array
    palette = np.zeros((4,4), dtype=np.uint8)
    for color in color_map:
        idx = color_map[color]
        palette[idx, :] = color
    if DEBUG:
        print(palette)

    tiles = []
    for i in range(0, h, TILE_SIZE):
        for j in range(0, w, TILE_SIZE):
            tile = indices[i:(i+TILE_SIZE), j:(j+TILE_SIZE)]
            if DEBUG:
                print(tile)
            tiles.append(tile)
    return tiles, palette

def bitpack(tile):
    '''
    Bitpack an 8x8 uint2 tile into 2 arrays of 8 uint8
    '''
    binary = np.unpackbits(tile[..., None], axis=2)[:,:,6:] # 8x8x2 bits
    if DEBUG:
        print(binary[:,:,0])
        print(binary[:,:,1])
    bit0 = np.packbits(binary[:,:,0], axis=1).flatten()
    bit1 = np.packbits(binary[:,:,1], axis=1).flatten()
    packed = np.vstack([bit0, bit1])
    if DEBUG:
        print(packed)
    return packed

def convert(asset_name):
    tiles, palette = img_to_tiles('resources/%s.png' % asset_name)
    # tiles and palettes are stored in csv format since it's easier to work
    # with than binary format. All 2-day arrays get flattened in the process,
    # with palette on the first line and tiles in the following lines
    palette_str = '\n'.join([
        ','.join([str(i) for i in row]) for row in palette
    ])

    tiles_str = '\n\n'.join([
        '\n'.join([
            ','.join([str(i) for i in row]) for row in bitpack(tile)
        ]) for tile in tiles
    ])
    with open('resources/ppu4_%s.csv' % asset_name, 'w+') as file:
        file.write(palette_str)
        file.write('\n\n')
        file.write(tiles_str)


def main():
    assets = [
        'player',
        'enemy',
        'bombs',
        'lasers',
        'explosion',
        'misc',
        'water_tiles',
        'road_tiles'
    ]
    for asset in assets:
        convert(asset)

if __name__ == '__main__':
    main()