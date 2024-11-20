#!/usr/bin/env python3

import urllib.request
import cv2
import numpy as np

def main():
    """TODO: Docstring for main.
    :returns: TODO

    """

    cv2.namedWindow("Detection", cv2.WINDOW_AUTOSIZE)

    url = 'http://192.168.7.15'
    # #ae5757 is also a good fit
    clr = [136, 152, 235] # BGR
    tlr = 50

    clr_range = [
        (clr[0]-tlr, clr[1]-tlr, clr[2]-tlr),
        (clr[0]+tlr, clr[1]+tlr, clr[2]+tlr)
    ]

    user_in = input("Stream address ['" + url + "']:")
    url     = url if not user_in else user_in

    while True:
        with urllib.request.urlopen(url + '/bmp') as response:
            buf  = np.array(bytearray(response.read()), dtype=np.uint8)
            img  = cv2.imdecode(buf, -1)
            mask = cv2.inRange(img, clr_range[0], clr_range[1])

            out = cv2.bitwise_and(img, img, mask=mask)
            cv2.imshow("img", np.hstack([img, out]))
            cv2.waitKey(0)


if __name__ == "__main__":
    main()
