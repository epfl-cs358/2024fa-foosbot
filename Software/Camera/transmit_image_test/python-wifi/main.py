#!/usr/bin/env python3

import sys
import serial
import cv2
import urllib.request as urllr
import numpy as np

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\r\n'

def main():
    """TODO: Docstring for main.
    :returns: TODO

    """

    # For colour detection
    # #ae5757 is also a good fit
    # clr = [136, 152, 235] # BGR
    # tlr = 50
    # clr_range = [
    #     (clr[0]-tlr, clr[1]-tlr, clr[2]-tlr),
    #     (clr[0]+tlr, clr[1]+tlr, clr[2]+tlr)
    # ]

    # For getting data
    url = 'http://192.168.7.15'

    user_in = input("Stream address ['" + url + "']:")
    url     = url if not user_in else user_in

    sent = urllr.urlopen(url + '/bmp')
    print("URL opened.")

    # For sending data
    port = '/dev/ttyUSB0'

    user_in = input("Device ['" + port + "']:")

    ser = serial.Serial(
        port = port if not user_in else user_in
    )

    ser.open()
    print("Serial opened.")

    try:
        while True:
            buf  = np.array(bytearray(sent.read()), dtype=np.uint8)
            img  = cv2.imdecode(buf, -1)

            grey = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            grey = cv2.medianBlur(grey, 5)

            rows = grey.shape[0]

            circles = cv2.HoughCircles(grey, cv2.HOUGH_GRADIENT, 1, rows / 8,
                                       param1=100, param2=30,
                                       minRadius=1, maxRadius=100)

            if circles is not None:
                circles = np.uint16(np.around(circles))
                for i in circles[0, :]:
                    center = (i[0], i[1])
                    # circle center
                    cv2.circle(img, center, 1, (0, 100, 100), 3)
                    # circle outline
                    radius = i[2]
                    cv2.circle(img, center, radius, (255, 0, 0), 3)

            # mask = cv2.inRange(img, clr_range[0], clr_range[1])
            # out = cv2.bitwise_and(img, img, mask=mask)
            # cv2.imshow("img", np.hstack([img, out]))

            pos      = (circles[0, 0][0], circles[0, 0][1])
            timestmp = 0 # TODO

            ser.write(MSG_START + str(pos) + MSG_SEP + str(timestmp) + MSG_END)

            cv2.imshow("Output", img)
            cv2.waitKey(1)
    except:
        ser.close()
        sys.exit(0)


if __name__ == "__main__":
    main()
