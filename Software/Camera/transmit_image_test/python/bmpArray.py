import numpy
import numpy as np

from bmp_inspector import BMP_inspector
from bitmap        import Bitmap

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
        self.array = np.split(
            np.array(self.bmpIns.pixel_array),
            int(
                self.bmpIns.bitmap_height if
                self.bmpIns.bitmap_height > 0
                else -self.bmpIns.bitmap_height
            )
        )[:][:-padding if padding != 0 else None]

    def getBallPosition(self,
                        red:       int = int.from_bytes(b'\xff', "little"),
                        green:     int = int.from_bytes(b'\x00', "little"),
                        blue:      int = int.from_bytes(b'\x7f', "little"),
                        tolerance: int   = 20):
        """
        Get the position of the center of the ball.

        Parameters:
            clr       (bytes): The colour of the ball to look for
            tolerance (int)  : The tolerance for the colour
        Returns:
            The position of the center of the ball.
        """

        rslt   = (-1, -1)
        #clrInt = int.from_bytes(blue+green+red, "big")

        print("Getting ball position.")
        rangeRed    = range(red-tolerance, red+tolerance)
        rangeGreen  = range(green-tolerance, green+tolerance)
        rangeBlue   = range(blue-tolerance, blue+tolerance)

        #print(rangeRed, rangeGreen, rangeBlue)

        xSum  = 0
        ySum  = 0
        total = 0

        yCurr = 0
        for row in self.array:
            xCurr = 0
            for p in row:
                #print(type(p))
                p = p.tobytes()
                r = p[2]
                g = p[1]
                b = p[0]
                #print(r, g, b)
                if r in rangeRed and g in rangeGreen and b in rangeBlue:
                    xSum += xCurr
                    ySum += yCurr
                    total+=1
                xCurr+=1
            yCurr+=1

        if total > 0:
            rslt = (int(xSum/total), int(ySum/total))
        return rslt

    def drawSquare(self,
                   pos:    tuple[int, int],
                   length: int,
                   thick:  int,
                   clr:    bytes = b'e6b400'):
        """
        Draw a square around the given position.

        Parameters:
            pos    (tuple[int, int]): Position to draw the square around
            length (int)            : Length of the square edges
            thick  (int)            : Thickness of the square
            clr    (bytes)          : Colour of the square
        """

        def drawEdge(xFrom: int, xTo: int, yFrom: int, yTo: int):
            """
            Draws an edge.

            Parameters:
                xFrom (int): x coordinate to draw from
                xTo   (int): x coordinate to draw to
                yFrom (int): y coordinate to draw from
                yTo   (int): y coordinate to draw to
            """

            for p in self.array[yFrom:yTo][xFrom:xTo]:
                p == clr

        print("Drawing the square.")

        x = pos[0]
        y = pos[1]

        xLeEdge = x - length
        xRiEdge = x + length
        yUpEdge = y - length
        yLoEdge = y + length

        # Drawing horizontal edges
        drawEdge(yUpEdge-thick,yUpEdge+thick,xLeEdge,xRiEdge)
        drawEdge(yLoEdge-thick,yLoEdge+thick,xLeEdge,xRiEdge)

        # Drawing vertical edges
        drawEdge(xLeEdge-thick, xLeEdge+thick, yUpEdge, yLoEdge)
        drawEdge(xRiEdge-thick, xRiEdge+thick, yUpEdge, yLoEdge)

    def drawPoint(self,
                    pos     : tuple[int, int],
                    width  : int,
                    clr     : bytes = b'\x01\x01\x01'):
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
        c = numpy.array(clr[::-1], dtype='S3')

        for row in self.array[ymin:ymax]:
            for p in range(xmin, xmax):
                row[p] = c
        #self.array[xmin:xmax, ymin:ymax] = numpy.bytes_(clr[2:]+clr[1:2]+clr[0:1])

    def writeToFile(self, fileName: str = "img.bmp"):
        """
        Writes the image to file.

        Parameters:
            fileName (str): The name of the file
        """

        print("Writing to file '" + fileName + "'.")
        bm = Bitmap(self.bmpIns.bitmap_width,
                    self.bmpIns.bitmap_height,
                    self.array)
        bm.save(fileName)
