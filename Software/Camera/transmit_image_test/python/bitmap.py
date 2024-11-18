# Orig: https://github.com/jtkirkpatrick/bitmap
"""
The bitmap module provides the Bitmap class. A Bitmap object represents
an image as a matrix of pixels. Bitmap methods allow pixels to be edited
and the image saved.

For information on file format, refer to:
    https://en.wikipedia.org/wiki/BMP_file_format
"""
import itertools
import struct


class Bitmap(object):

    """Create a bitmap image file (.bmp).

    Usage is simple:
    >>> from bitmap import Bitmap
    >>> bmp = Bitmap(10, 10, fill=(0, 100, 255))  # (r, g, b) background
    >>> bmp.pencil(1, 1, (128, 128, 128))
    >>> bmp.save('test.bmp')
    """

    def __init__(self, width, height, fill):
        """Intialize the bitmap file."""
        self._width  = width
        self._height = height

        self._pixels = fill

        self._row_pad   = (self._width*3) % 4

    def pencil(self, x, y, color):
        """Color in a single pixel at x, y."""
        assert(0 <= x < self._width)
        assert(0 <= y < self._height)
        assert(isinstance(color, tuple))
        assert(len(color) == 3)
        assert(set(type(x) for x in color) == {int})
        assert(min(color) >= 0)
        assert(max(color) <= 255)

        self._pixels[x][y] = color

    def save(self, path):
        """Write the bitmap data to the file path."""
        with open(path, 'wb') as f:
            self._write_header(f)
            self._write_pixels(f)

    def _write_header(self, f):
        """Write the bitmap file header."""
        file_size = 0x36 + ((self._width + self._row_pad)*self._height*3)

        map(
            f.write,
            [
                # Bitmap file header
                bytearray([0x42, 0x4d]),              # Header field
                struct.pack('<I', file_size),         # File size
                bytearray([0x00, 0x00, 0x00, 0x00]),  # Reserved/unused
                bytearray([0x36, 0x00, 0x00, 0x00]),  # Data offset
                # DIB header, BITMAPINFOHEADER
                bytearray([0x28, 0x00, 0x00, 0x00]),  # DIB header size
                struct.pack('<I', self._width),       # Image width
                struct.pack('<I', self._height),      # Image height
                bytearray([0x01, 0x00]),              # Color planes
                bytearray([0x18, 0x00]),              # Bits per pixel
                bytearray([0x00, 0x00, 0x00, 0x00]),  # Compression
                struct.pack('<I', file_size - 0x36),  # Pixel data size
                bytearray([0x13, 0x0b, 0x00, 0x00]),  # Horz. resolution
                bytearray([0x13, 0x0b, 0x00, 0x00]),  # Vert. resolution
                bytearray([0x00, 0x00, 0x00, 0x00]),  # Number of colors
                bytearray([0x00, 0x00, 0x00, 0x00])   # Important colors
            ]
        )

    def _write_pixels(self, f):
        """Write the array of pixels to the file."""
        for y in reversed(range(self._height)):
            row = [self._pixels[x][y] for x in range(self._width)]
            row = [reversed(color) for color in row]
            row = list(itertools.chain.from_iterable(row))
            row.extend([0x00]*self._row_pad)

            f.write(bytearray(row))


if __name__ == '__main__':
    bmp = Bitmap(10, 10, fill=(0, 100, 200))
    for i in range(8):
        bmp.pencil(i + 1,     1, (255, 255, 255))
        bmp.pencil(    1, i + 1, (255, 255, 255))
        bmp.pencil(i + 1, 8 - i, (255, 255, 255))
    bmp.save('bitmap_test.bmp')
