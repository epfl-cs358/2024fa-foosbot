# Original Src : https://gist.github.com/j-marjanovic/5319384ad6606d6fd6d6
'''
 BMP Inspector: Prints BPM file information and pixel array statistics

 Copyright (C) 2015 Jan Marjanovic <jan@marjanovic.pro>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
'''

import sys
import struct
from collections import OrderedDict

METER_IN_INCH = 39.3700787


class BMP_inspector(object):
    ''' Gets BMP (bitmap file) information, such as size, bits per color, ...
        and also parses the pixel array '''

    _dib_header_type = {
         12: 'BITMAPCOREHEADER',
         64: 'OS22XBITMAPHEADER',
         40: 'BITMAPINFOHEADER',
         52: 'BITMAPV2INFOHEADER',
         56: 'BITMAPV3INFOHEADER',
        108: 'BITMAPV4HEADER',
        124: 'BITMAPV5HEADER'
    }

    _dib_method_type = {
         0: 'BI_RGB (none)',
         1: 'BI_RLE8',
         2: 'BI_RLE4',
         3: 'BI_BITFIELDS',
         4: 'BI_JPEG',
         5: 'BI_PNG',
         6: 'BI_ALPHABITFIELDS',
        11: 'BI_CMYK',
        12: 'BI_CMYKRLE8',
        13: 'BI_CMYKRLE4',
    }

    def __init__(self,
                 filename,
                 buffer  = None,
                 verbose = False,
                 fromBuf = True):
        ''' Open file, parses information and shows some information '''
        self.array_offset   = 0
        self.image_size     = 0
        self.bits_per_pixel = 0
        self.bitmap_width   = 0
        self.bitmap_height  = 0
        self.verbose        = verbose

        if fromBuf:
            self.buff = buffer
        else :
            print('Opening', filename)
            f = open(filename, 'rb')
            self.buff = f.read()
            f.close()

        self.parse_header()
        self.parse_dib_header()
        self.parse_pixel_array()
        #if self.verbose:
            #self.get_color_stats()

    def parse_header(self):
        ''' Parses header -> gets pixel array offset '''
        header = self.buff[0:14]

        file_format = header[0:2].decode('ascii')
        file_size = struct.unpack('<I', header[2:6])[0]
        array_offset = struct.unpack('<I', header[10:14])[0]

        self.array_offset = array_offset

        if self.verbose:
            print('File header')
            print('  File format:', file_format)
            print('  File size:', file_size, 'bytes')
            print('  Array offset:', array_offset, 'bytes')

    def parse_dib_header(self):
        ''' Parses DIB header structure -> get image size, bits per pixel '''
        dib_header_size = struct.unpack('<I', self.buff[14:18])[0]

        if self.verbose:
            print('DIB header size:', dib_header_size, 'bytes')
            print('DIB type:', self._dib_header_type[dib_header_size])

        dib_header     = self.buff[14:14+dib_header_size]
        bitmap_width   = struct.unpack('<i', dib_header[ 4: 8])[0]
        bitmap_height  = struct.unpack('<i', dib_header[ 8:12])[0]
        color_planes   = struct.unpack('<H', dib_header[12:14])[0]
        bits_per_pixel = struct.unpack('<H', dib_header[14:16])[0]
        comp_method    = struct.unpack('<I', dib_header[16:20])[0]
        image_size     = struct.unpack('<I', dib_header[20:24])[0]
        hor_resolution = struct.unpack('<i', dib_header[24:28])[0]
        ver_resolution = struct.unpack('<i', dib_header[28:32])[0]
        # color_palette = struct.unpack('<I', dib_header[32:36])[0]
        # imp_colors_used = struct.unpack('<I', dib_header[36:40])[0]

        self.image_size     = image_size
        self.bits_per_pixel = bits_per_pixel
        self.bitmap_width   = bitmap_width
        self.bitmap_height  = bitmap_height

        if self.verbose:
            print('DIB header')
            print('  Bitmap width:'  , bitmap_width)
            print('  Bitmap height:' , bitmap_height)
            print('  Color planes:'  , color_planes)
            print('  Bits per pixel:', bits_per_pixel)
            print('  Compr method:'  , self._dib_method_type[comp_method])
            print('  Image size:'    , image_size)
            print('  Hor resolution:', hor_resolution // METER_IN_INCH, 'DPI')
            print('  Ver resolution:', ver_resolution // METER_IN_INCH, 'DPI')

    def parse_pixel_array(self):
        ''' Parses pixel array and stores it to list of bytes '''
        pixel_array = self.buff[self.array_offset:
                                self.array_offset+self.image_size]

        def chunks(l, n):
            """ Yield successive n-sized chunks from l.
            """
            for i in range(0, len(l), n):
                yield l[i:i+n]

        self.pixel_array = list(chunks(pixel_array, self.bits_per_pixel//8))

    def get_color_stats(self):
        ''' Finds 10 most used colors in bitmap '''
        colors = dict()

        for pixel in self.pixel_array:
            try:
                colors[pixel] += 1
            except KeyError:
                colors[pixel] = 1

        ord_colors = OrderedDict(sorted(colors.items(),
                                        key=lambda t: t[1],
                                        reverse=True))

        print('Color statistics')
        for c, v in list(ord_colors.items())[0:10]:
            _nr_pixels = self.bitmap_width * self.bitmap_height
            if self.bits_per_pixel == 24:
                b, g, r = struct.unpack('BBB', c)
                percent = v/_nr_pixels*100
                print('  R: {0:3}, G: {1:3}, B: {2:3}, {3:.3}%'.
                      format(r, g, b, percent))
            elif self.bits_per_pixel == 32:
                # pylint: disable=unused-variable
                b, g, r, a = struct.unpack('BBBB', c)
                percent = v/_nr_pixels*100
                print('  R: {0:3}, G: {1:3}, B: {2:3}, {3:.3}%'.
                      format(r, g, b, percent))
            else:
                raise NotImplementedError


if __name__ == '__main__':

    if len(sys.argv) != 2:
        print('Usage: ', sys.argv[0], 'bmp-file')
        sys.exit(1)

    filename = sys.argv[1]
    bpmInspector = BMP_inspector(filename)
