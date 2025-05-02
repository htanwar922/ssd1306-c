#!/usr/bin/env python3

from abc import ABC, abstractmethod
from typing import List, Union

class FontBase(ABC):
    """
    Abstract base class for fonts.
    """
    @property
    @abstractmethod
    def height(self) -> int:
        """
        Returns the pixel height of the font.
        """
        pass

    @property
    @abstractmethod
    def width(self) -> int:
        """
        Returns the pixel width of the font.
        """
        pass

    @classmethod
    @abstractmethod
    def max_height(cls) -> int:
        """
        Returns the pixel width of the font.
        """
        pass

    @classmethod
    @abstractmethod
    def max_width(cls) -> int:
        """
        Returns the pixel width of the font.
        """
        pass

    @abstractmethod
    def get_columns(self) -> Union[List[int], List[List[int]]]:
        """
        Returns the columns of the font.
        """
        pass

    @abstractmethod
    def __repr__(self) -> str:
        """
        Returns a string representation of the font.
        """
        pass

class Font6x4(FontBase):
    """
    # [Tiny Monospaced Font](https://robey.lag.net/2010/01/23/tiny-monospace-font.html)
    ## Tom Thumb: A very tiny, monospace, bitmap font
    ## Font data: 96 printable ASCII characters (0x20–0x7E)
    ## Each character is 4 bytes (4 columns × 6 rows), LSB = top row
    """
    def __init__(self, swidth, dwidth, bbx, bitmap):
        self.swidth = swidth
        self.dwidth = dwidth
        self.bbx = bbx
        self.bitmap = bitmap
    @property
    def height(self):
        return self.bbx[1] - self.bbx[3] + 1
    @property
    def width(self):
        return self.dwidth[0]
    @classmethod
    def max_height(cls) -> int:
        return 6
    @classmethod
    def max_width(cls) -> int:
        return 4
    def get_columns(self) -> List[int]:
        columns = list([0] * 4)
        for row in range(len(self.bitmap)):
            byte = self.bitmap[row] >> 4
            for col in range(4):
                columns[col] |= (byte >> (3 - col) & 1) << (row + (6 - len(self.bitmap)))
        if any(columns):
            while len(columns) >= 2 and columns[-1] == 0 and columns[-2] == 0:
                columns.pop()
        return columns
    def __repr__(self):
        bitmap = '[' + ', '.join(f'0x{b:02x}' for b in self.bitmap) + ']'
        return f'{__class__.__name__}({self.swidth}, {self.dwidth}, {self.bbx}, {bitmap})'


class Font8x9(FontBase):
    """
    # [Pixelated Elegance Font](https://www.fontspace.com/pixelated-elegance-font-f126145)
    ## The font Pixelated Elegance - CC0 contains 206 glyphs.
    Font data: 206 printable ASCII, Latin-1, and more characters.
    """
    def __init__(self, advance, auto_update_advance, auto_advance_amount, pixels):
        self.advance = advance
        self.auto_update_advance = auto_update_advance
        self.auto_advance_amount = auto_advance_amount
        self.pixels = pixels
    @property
    def height(self):
        return 8
    @property
    def width(self):
        return self.advance
    @classmethod
    def max_height(cls) -> int:
        return 8
    @classmethod
    def max_width(cls) -> int:
        return 9
    def get_columns(self) -> List[int]:
        pixels = self.pixels
        pixels = sorted(pixels)
        if pixels and self.width <= pixels[-1][0]:
            print(f'Character has width {self.width} <= max {pixels[-1][0]}. Will throw...')
        columns = list([0] * self.width)
        for pixel in pixels:
            columns[pixel[0]] |= (1 << (6 - pixel[1]))
        return columns
    def __repr__(self):
        return f'{__class__.__name__}({self.advance}, {self.auto_update_advance}, {self.auto_advance_amount}, {self.pixels})'


class Font16x8(FontBase):
    """
    # [Bizcat 16 × 8 font](https://github.com/tomwaitsfornoman/lawrie-nes_ecp5/blob/master/osd/font_bizcat8x16.mem)
    """
    def __init__(self, bitmap):
        self.bitmap = bitmap        # Row-major order - 16 rows, 8 columns - first byte is top row
    @property
    def height(self):
        return 16
    @property
    def width(self):
        return 8
    @classmethod
    def max_height(cls) -> int:
        return 16
    @classmethod
    def max_width(cls) -> int:
        return 8
    def get_columns(self) -> List[List[int]]:
        # columns = [[0] * 8 for _ in range(16 // 8)]
        # for row in range(16):
        #     byte = self.bitmap[row]
        #     for col in range(8):
        #         columns[row // 8][col] |= (byte >> (7 - col) & 1) << row % 8
        # return columns
        columns = [[0] * 2 for _ in range(8)]
        for row in range(16):
            byte = self.bitmap[row]
            for col in range(8):
                columns[col][row // 8] |= (byte >> (7 - col) & 1) << row % 8
        return columns
    def __repr__(self):
        bitmap = '[' + ', '.join(f'0x{b:02x}' for b in self.bitmap) + ']'
        return f'{__class__.__name__}({bitmap})'


if __name__ == '__main__':
    import string

    # font6x4 = {}
    # with open('tom-thumb.bdf', 'r') as f:
    #     while True:
    #         line = f.readline()
    #         if not line:
    #             break
    #         if line.startswith('STARTCHAR'):
    #             line = f.readline()
    #             encoding = int(line.split()[1])
    #             line = f.readline()
    #             swidth = tuple(map(int, line.split()[1:]))
    #             line = f.readline()
    #             dwidth = tuple(map(int, line.split()[1:]))
    #             line = f.readline()
    #             bbx = tuple(map(int, line.split()[1:]))
    #             f.readline()  # Skip BITMAP line
    #             bitmap = []
    #             while True:
    #                 line = f.readline()
    #                 if line.startswith('ENDCHAR'):
    #                     break
    #                 bitmap.append(int(line.strip(), 16))
    #             font6x4[chr(encoding)] = Font6x4(swidth, dwidth, bbx, bitmap)

    # with open('font_6x4.txt', 'w', encoding='utf-8') as f:
    #     for k, v in font6x4.items():
    #         if k not in string.printable:
    #             continue
    #         f.write("'" + (k if k not in ("'", "\\") else f'\\{k}') + f"': {v},\n")
    #     print(f'Generated font_6x4.txt for {len(font6x4)} characters')


    font8x9 = {}
    with open('Pixelated Elegance v0.3-7344.pxf', 'r') as f:
        num_glyphs = 0
        while True:
            line = f.readline()
            if not line:
                break
            if line.startswith('num_glyphs'):
                num_glyphs = int(line.split()[1])
            if line.startswith('glyphs'):
                break
        assert num_glyphs, f'Expected num_glyphs to be > 0, but got {num_glyphs}'

        for _ in range(num_glyphs):
            line = f.readline()
            if line.startswith('\t'):
                encoding = int(line.strip('\t\r\n:'))
                line = f.readline()
                advance = int(line.split()[1])
                line = f.readline()
                auto_update_advance = bool(line.split()[1])
                line = f.readline()
                auto_advance_amount = int(line.split()[1])
                line = f.readline()
                line = line.strip().split(':')[1].strip()
                pixels = []
                for pair in line.split(','):
                    if not pair:
                        continue
                    x, y = map(int, pair.split())
                    pixels.append((x, y))
                font8x9[chr(encoding)] = Font8x9(advance, auto_update_advance, auto_advance_amount, pixels)

    with open('font_8x9.c', 'w', encoding='utf-8') as f:
        index_map = {0x21: 0}
        f.write('// Font data for Pixelated Elegance v0.3-7344\n')
        f.write('// Generated by generator.py\n')
        f.write('// Font data: 95 printable ASCII characters (0x21–0x7E)\n')
        f.write('// Address map: 0x21–0x7E\n')
        f.write('// Author: Himanshu\n')
        f.write('// License: CC0 1.0 Universal (CC0 1.0) Public Domain Dedication\n')
        f.write('// https://www.fontspace.com/pixelated-elegance-font-f126145\n')
        f.write('\n')
        f.write('#include <stdint.h>\n')
        f.write('#include <string.h>\n')
        f.write('\n')
        f.write('#define BEGIN(x)\n')
        f.write('#define END(x)\n')
        f.write('\n')

        prefix = ' ' * 4

        f.write('const uint8_t font_8x9_columns_blob[] = {\n')
        for k in range(0x21, 0x7F):
            columns = font8x9[chr(k)].get_columns()
            index_map[k + 1] = index_map[k] + len(columns)
            ks = chr(k)
            ks = "'" + (ks if ks not in ("'", "\\") else f'\\{ks}') + "'"
            f.write(prefix + f'BEGIN({ks}) ')
            f.write(', '.join(map(lambda x: f'0x{x:02x}', columns)))
            f.write(f' END({ks}),\n')
        f.write('};\n')
        f.write('\n')

        f.write('const uint8_t font_8x9_index_map[] = {\n')
        for k in range(0x21, 0x7F):
            ks = chr(k)
            ks = "'" + (ks if ks not in ("'", "\\") else f'\\{ks}') + "'"
            f.write(prefix + f'BEGIN({ks}) ')
            f.write(f'{index_map[k]}')
            f.write(f' END({ks}),\n')
        f.write(prefix + 'sizeof(font_8x9_columns_blob) / sizeof(font_8x9_columns_blob[0])\n')
        f.write('};\n')
        f.write('\n')

        f.write('int8_t font_8x9_get_columns(uint8_t c, uint8_t *buf) {\n')
        f.write('    if (c < 0x21 || c > 0x7E)\n')
        f.write('        return -1;\n')
        f.write('    c -= 0x21;\n')
        f.write('    int8_t len = font_8x9_index_map[c + 1] - font_8x9_index_map[c];\n')
        f.write('    if (len > 0)\n')
        f.write('        memcpy(buf, font_8x9_columns_blob + font_8x9_index_map[c], len);\n')
        f.write('    else\n')
        f.write('        return -1;\n')
        f.write('    return len;\n')
        f.write('}\n')
        # f.write('\n')
        print(f'Generated font_8x9.c for {len(font8x9)} characters')

    font16x8 = {}
    with open('font_bizcat8x16.mem', 'r') as f:
        f.readline()  # Skip first line
        for line in f:
            if line.startswith('// 0x'):
                char = chr(int(line.split()[1], 16))
                bitmap = []
                for _ in range(16):
                    line = f.readline()
                    bitmap.append(int(line.strip(), 2))
                if char in (' ', '\n', '\r', '\t', '\x0b', '\x0c', '\x00'):
                    continue
                font16x8[char] = Font16x8(bitmap)
            else:
                print(f'Unexpected line: {line.strip()}')

    with open('font_16x8.c', 'w', encoding='utf-8') as f:
        index_map = {0x21: 0}
        f.write('// Font data for Bizcat 16 × 8 font\n')
        f.write('// Generated by generator.py\n')
        f.write('// Font data: 95 printable ASCII characters (0x21–0x7E)\n')
        f.write('// Author: Himanshu\n')
        f.write('// License: CC0 1.0 Universal (CC0 1.0) Public Domain Dedication\n')
        f.write('// https://github.com/tomwaitsfornoman/lawrie-nes_ecp5/blob/master/osd/font_bizcat8x16.mem\n')
        f.write('\n')
        f.write('#include <stdint.h>\n')
        f.write('\n')
        f.write('#define BEGIN(x) {\n')
        f.write('#define END(x) }\n')
        f.write('\n')

        prefix = ' ' * 4

        f.write('const uint16_t font_16x8_columns[0x7E - 0x20][8] = {\n')
        for k in range(0x21, 0x7F):
            columns = font16x8[chr(k)].get_columns()
            index_map[k + 1] = index_map[k] + sum(map(len, columns))
            ks = chr(k)
            ks = "'" + (ks if ks not in ("'", "\\") else f'\\{ks}') + "'"
            f.write(prefix + f'BEGIN({ks}) ')
            f.write(', '.join(f'0x{column[1]:02x}{column[0]:02x}' for column in columns))
            f.write(f' END({ks}),\n')
        f.write('};\n')
        f.write('\n')

        f.write('int8_t font_16x8_get_columns(uint8_t c, uint8_t *buf) {\n')
        f.write('    if (c < 0x21 || c > 0x7E)\n')
        f.write('        return -1;\n')
        f.write('    c -= 0x21;\n')
        f.write('    int8_t len = 8;\n')
        f.write('    if (len > 0)\n')
        f.write('        memcpy(buf, font_16x8_columns[c], len);\n')
        f.write('    else\n')
        f.write('        return -1;\n')
        f.write('    return len;\n')
        f.write('}\n')
        print(f'Generated font_16x8.c for {len(font16x8)} characters')

    print('Done.')
