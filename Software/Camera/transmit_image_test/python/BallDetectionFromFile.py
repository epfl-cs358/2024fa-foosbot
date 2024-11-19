#!/usr/bin/env python3

import sys

from bmpArray       import BmpArray

"""
Used for Debugging.
Does the image processing on a bmp file instead of from the Serial port.
Stores the image with a square around the estimated midpoint of the ball.
"""

def main(fnIn=None, fnOut=None):
    print("Input File : ", fnIn)
    print("Output File : ", fnOut)

    f = open(fnIn, 'rb')

    img = f.read()
    arr = BmpArray(img)
    pos = arr.getBallPosition(
        b'\xff',
        b'\xff',
        b'\x84',
        100
    )
    if pos == (-1, -1):
        print("Ball not found.")
    else:
        print("Ball is at position: " + str(pos))
        #arr.drawSquare(pos, 200, 25)
        arr.drawPoint(pos, 10)
        arr.writeToFile(fnOut)

if __name__ == '__main__':

    fnIn  = "img.bmp"
    fnOut = "img_out.bmp"

    if len(sys.argv) >= 2:
        fnIn = sys.argv[1]
    if len(sys.argv) >= 3:
        fnOut = sys.argv[2]
    main(fnIn, fnOut)
