#!/usr/bin/env python3

import sys

from serialImg  import SerialImg

def main():
    port = input("Port ['/dev/ttyUSB0']: ")
    s = SerialImg() if port == '\n' else SerialImg(port=port)

    end = False
    try:
        while not end:
            data = s.readline()
            print(data)

            f = open("img.jpg", "w")
            f.write(data)
            f.close()
    except:
        pass
    finally:
        print("Closing connection...\n")
        s.close()
        sys.exit(0)





if __name__ == '__main__':
    main()
