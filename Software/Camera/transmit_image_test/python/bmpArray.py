import numpy as np

class BmpArray(np.ndarray):

    """ BMP image represented by a `numpy.ndarray`. """

    def __init__(self, img: bytes):
        """
        Initialise a `numpy.ndarray` from the pixel array of a BMP
        file.

        Parameters:
            img (bytes): The binary file representing the BMP image
        """

        offset  = int(img[10])
        width   = int(img[18])
        height  = int(img[20])
        padding = 3*width % 4

        self = np.split(
            np.frombuffer(img, offset=offset),
            int(height)
        )[:][:-padding]

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

        r = range(int(clr)-tolerance, int(clr)+tolerance)

        xSum  = 0
        ySum  = 0
        total = 0

        yCurr = 0
        for a in self:
            xCurr = 0
            for p in a:
                if int(p) in r:
                    xSum += xCurr
                    ySum += yCurr
                    ++total
                ++xCurr
            ++yCurr

        return (xSum/total, ySum/total)

    def drawSquare(self, pos: tuple[int, int], length: int):
        """
        Draw a square around the given position.

        Parameters:
            pos    (tuple[int, int]): Position to draw the square around
            length (int)            : Length of the square edges
        """
        pass
