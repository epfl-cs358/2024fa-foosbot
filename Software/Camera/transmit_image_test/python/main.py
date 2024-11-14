#!/usr/bin/env python3

import sys

from serialImg  import SerialImg

def main():
    port = input("Port ['/dev/ttyUSB0']: ")
    s = SerialImg() if not port else SerialImg(port=port)

    print("Starting serial reception service...\n")
    print("To kill the service, press <CTRL-C>.")
    try:
        while True:
            data = s.getImg()

            if data is None:
                print("Image could not be read.")
            elif not data:
                print("Image produced seems to be empty, " +
                      "I will not write to the file.")
            else:
                # TODO: Remove writing to file
                # f = open("img.bmp", "wb")
                # f.write(data)
                # f.close()

                # TODO: Send image to processing for ball detection
                pass
    except:
        pass
    finally:
        print("Closing connection...\n")
        s.close()
        sys.exit(0)


if __name__ == '__main__':
    main()
