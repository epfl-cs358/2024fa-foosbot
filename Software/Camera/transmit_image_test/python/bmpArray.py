import numpy as np

from bmp_inspector import BMP_inspector
#from bitmap import Bitmap
from matplotlib import pyplot as plt

class BmpArray():

    """ BMP image represented by a `numpy.ndarray`. """

    def __init__(self, img: bytes):
        """
        Initialise a `numpy.ndarray` from the pixel array of a BMP
        file.

        Parameters:
            img (bytes): The binary file representing the BMP image
        """

        self.bmpIns = BMP_inspector("", img, False, True)
        padding = 3*self.bmpIns.bitmap_width % 4
        self.array = np.array(self.bmpIns.pixel_array).reshape((
            abs(self.bmpIns.bitmap_height),
            abs(self.bmpIns.bitmap_width)
        ))[:][:-padding if padding != 0 else None]

    def getBallPosition(self,
                        red_b:     bytes = b'\xff',
                        green_b:   bytes = b'\xff',
                        blue_b:    bytes = b'\x84',
                        tolerance: int   = 20):
        """
        Get the position of the center of the ball.

        Parameters:
            clr       (bytes): The colour of the ball to look for
            tolerance (int)  : The tolerance for the colour
        Returns:
            The position of the center of the ball.
        """

        red   = int.from_bytes(  red_b, "little")
        green = int.from_bytes(green_b, "little")
        blue  = int.from_bytes( blue_b, "little")

        rslt = (-1, -1)

        print("Getting ball position.")
        rangeRed    = range(  red-tolerance,   red+tolerance)
        rangeGreen  = range(green-tolerance, green+tolerance)
        rangeBlue   = range( blue-tolerance,  blue+tolerance)

        xSum  = 0
        ySum  = 0
        total = 0

        yCurr = 0
        self.array = self.array[:, :-1]
        for row in self.array:
            xCurr = 0
            for p in row:
                p = p.tobytes()
                if len(p) != 3:
                    p = b'\x00\x00\x00'
                r = p[2]
                g = p[1]
                b = p[0]
                print(r, g, b)
                if r in rangeRed and g in rangeGreen and b in rangeBlue:
                    xSum += xCurr
                    ySum += yCurr
                    total+=1
                xCurr+=1
            yCurr+=1

        if total > 0:
            rslt = (int(xSum/total), int(ySum/total))
        return rslt

    def drawPoint(self,
                    pos   : tuple[int, int],
                    width : int,
                    clr   : bytes = b'\x01\x01\x01'):
        """
        Draw a point of given radius at the given position.
        The point will be a filled square (not a cirle).

        Parameters:
            pos    (tuple[int, int]): Position of the point
            width  (int)            : Width of the filled square in pixel.
            clr    (bytes)          : Color of the square. Needs to be 3 Bytes in r,g,b order
        """
        x, y = pos
        xmin = max(0, min(self.bmpIns.bitmap_width, x-width))
        xmax = max(0, min(self.bmpIns.bitmap_width, x+width))
        ymin = max(0, min(abs(self.bmpIns.bitmap_height), y-width))
        ymax = max(0, min(abs(self.bmpIns.bitmap_height), y+width))
        c = np.array(clr[::-1], dtype='S3')

        for row in self.array[ymin:ymax]:
            for p in range(xmin, xmax):
                row[p] = c

    def writeToFile(self, fileName: str = "img.bmp"):
        """
        Writes the image to file.

        Parameters:
            fileName (str): The name of the file
        """

        print("Writing to file '" + fileName + "'.")
        arr = np.zeros((self.array.shape[0], self.array.shape[1], 3))
        for x in range(self.array.shape[0]):
            for y in range(self.array.shape[1]):
                p = self.array[x, y]
                if len(p) != 3:
                    p = b'\x00\x00\x00'
                arr[x, y] = np.array(
                    [p[2] / 255.,
                     p[1] / 255.,
                     p[0] /
                     255.])
        print(arr.shape)
        #plt.imshow(arr)
        plt.imsave(fileName, arr)

        #bm = Bitmap(self.bmpIns.bitmap_width,
        #            self.bmpIns.bitmap_height,
        #            self.array)
        #bm.save(fileName)
