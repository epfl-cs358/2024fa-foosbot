import numpy as np
from bmp_inspector import BMP_inspector

class BmpArray():

    """ BMP image represented by a `numpy.ndarray`. """

    def __init__(self, img: bytes):
        """
        Initialise a `numpy.ndarray` from the pixel array of a BMP
        file.

        Parameters:
            img (bytes): The binary file representing the BMP image
        """

        offset    = int(img[10] | img[11] | img[12] | img[13])
        width     = int(img[18] | img[19] | img[20] | img[21])
        height    = int(img[22] | img[23] | img[24] | img[25])
        bitPerPix = int(img[24] | img[25])
        padding   = 3*width % 4

        print(height)
        print(width)

        try:
            self.array = np.frombuffer(img, offset=offset, dtype='S3')
            print(self.array.size)
        # The code stops while running this.
            self.array = np.split(
                self.array,
                int(height)
            )[:][:-padding]
        except Exception as e:
            print(e)

    def getBallPosition(self,
                        clr:       bytes = b'ff7f00',
                        tolerance: int   = 20):
        """
        Get the position of the center of the ball.

        Parameters:
            clr       (bytes): The colour of the ball to look for
            tolerance (int)  : The tolerance for the colour
        Returns:
            The position of the center of the ball.
        """

        print("Getting ball position.")
        r = range(int(clr)-tolerance, int(clr)+tolerance)

        xSum  = 0
        ySum  = 0
        total = 0

        yCurr = 0
        for a in self.array:
            xCurr = 0
            for p in a:
                if int(p) in r:
                    xSum += xCurr
                    ySum += yCurr
                    ++total
                ++xCurr
            ++yCurr

        return (int(xSum/total), int(ySum/total))

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
        out = open(fileName, "a")
        for a in self.array:
            for p in a:
                out.write(p)
        out.close()
