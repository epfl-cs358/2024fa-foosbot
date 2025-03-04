#!/usr/bin/env python3

import sys

from serialImg import SerialImg
from bmpArray  import BmpArray

def main():
    print("To kill the service, press <CTRL-C>.")
    port = input("Port ['/dev/ttyUSB0']: ")
    s = SerialImg() if not port else SerialImg(port=port)

    print("Starting serial reception service...\n")
    try:
        # while True:

            img = s.getImg()

            if img is None:
                print("Image could not be read.")
            elif not img:
                print("Image produced seems to be empty.")
            else:
                array = BmpArray(img)
                pos   = array.getBallPosition()
                if pos == (-1, -1):
                    print("Ball not found.")
                else:
                    print("Ball is at position: " + str(pos))
                    array.drawPoint(pos, 10)
                    array.writeToFile()
                # TODO: Send position to Arduino UNO so that it can
                #       understand it
    except:
        pass
    finally:
        print("\nClosing connection...\n")
        s.close()
        sys.exit(0)


if __name__ == '__main__':
    main()
