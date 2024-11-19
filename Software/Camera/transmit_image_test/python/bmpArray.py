import numpy as np

from bmp_inspector import BMP_inspector
from bitmap.bitmap import Bitmap

class BmpArray():

    """ BMP image represented by a `numpy.ndarray`. """

    def __init__(self, img: bytes):
        """
        Initialise a `numpy.ndarray` from the pixel array of a BMP
        file.

        Parameters:
            img (bytes): The binary file representing the BMP image
        """

        self.bmpIns = BMP_inspector("", img, True, True)
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
                        red_b:     bytes = b'\xff',
                        green_b:   bytes = b'\x00',
                        blue_b:    bytes = b'\x7f',
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
        for row in self.array:
            xCurr = 0
            for p in row:
                p = p.tobytes()
                r = p[2]
                g = p[1]
                b = p[0]
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

    def writeToFile(self, fileName: str = "img.bmp"):
        """
        Writes the image to file.

        Parameters:
            fileName (str): The name of the file
        """

        print("Writing to file '" + fileName + "'.")

        bm = Bitmap(self.bmpIns.bitmap_width,
                    self.bmpIns.bitmap_height,
                    self.bmpIns.pixel_array)
        bm.save(fileName)
