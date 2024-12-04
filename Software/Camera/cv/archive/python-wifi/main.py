#!/usr/bin/env python3

import sys
import serial
import cv2
from multiprocessing import Pool
import urllib.request as urllr
import numpy as np

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\r\n'

def get_img():
    """
    Gets the image from the URL.
    :returns: BMP image.

    """
    sent = urllr.urlopen(url + "/cam-hi.jpg")
    buf  = np.array(bytearray(sent.read()), dtype=np.uint8)
    return buf

def process_img(buf: np.ndarray):
    """
    Processes the image for ball detection.

    :buf: The buffer containing the image.
    :returns: The position of the ball and the time stamp.

    """
    if buf.size == 0:
        return (-1, (-1, -1), -1)
    else:
        print(buf)
        img  = cv2.imdecode(buf, -1)

        grey = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        grey = cv2.medianBlur(grey, 5)

        rows = grey.shape[0]

        circles = cv2.HoughCircles(grey, cv2.HOUGH_GRADIENT, 1, rows / 8,
                                   param1=100, param2=30,
                                   minRadius=1, maxRadius=100)

        pos = (-1, -1)

        if circles is not None:
            circles = np.uint16(np.around(circles))
            for i in circles[0, :]:
                center = (i[0], i[1])
                # circle center
                cv2.circle(img, center, 1, (0, 100, 100), 3)
                # circle outline
                radius = i[2]
                print("Radius: " + str(radius))
                cv2.circle(img, center, radius, (255, 0, 0), 3)

        # mask = cv2.inRange(img, clr_range[0], clr_range[1])
        # out = cv2.bitwise_and(img, img, mask=mask)
        # cv2.imshow("img", np.hstack([img, out]))

            pos = (circles[0, 0][0], circles[0, 0][1])
        timestmp = 0 # TODO
        return (pos, timestmp, img)

def main(url = 'http://192.168.52.15', urlMan=False, debug=False):
    """
    Connects to the livestream of the given URL, gets the image of the `/bmp`
    URI, detects the ball and sends over serial the position of the ball.
    Displays the ball detection on screen.

    :param url: The url that provides images in bmp format on page `/bmp`
    :type url: str
    :param urlMan: Asks the user to input a url manually
    :type urlMan: bool
    :param debug: Disables serial ports (Use if serial port is disconnected)
    :type debug: bool
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
    if urlMan:
        user_in = input("Stream address ['" + url + "']:")
        url     = url if not user_in else user_in

    # For sending data
    if not debug:
        port = '/dev/ttyUSB0'

        user_in = input("Device ['" + port + "']:")

        ser = serial.Serial(
            port = port if not user_in else user_in
        )

        ser.open()
        print("Serial opened.")

    pool = Pool()

    # Main Loop for image processing
    try:
        while True:
            # buf = pool.apply_async(get_img, [])
            # (pos, timestmp, img) = pool.apply_async(process_img, [buf])


            sent = urllr.urlopen(url + "/cam-hi.jpg")
            buf  = np.array(bytearray(sent.read()), dtype=np.uint8)
            img  = cv2.imdecode(buf, -1)

            # if debug:
            #     print(pos)
            # else:
            #     if pos[0] != -1 and pos[1] != -1:
            #         ser.write(MSG_START + str(pos) + MSG_SEP + str(timestmp) + MSG_END)

            cv2.imshow("Output", img)
            cv2.waitKey(1)
    except Exception as e:
        if not debug:
            ser.close()
        raise e
        #sys.exit(0)


if __name__ == "__main__":
    """
    Argument Parser when called from command line.
    Allowed Format:
        <cmd> [<ipaddress>] [-d | -debug]     (If no arguments are given, the user is asked for the url later)
    """
    url = 'http://192.168.7.15'
    urlManual = False
    debug = False
    if len(sys.argv) > 1:
        inp = sys.argv[1]
        url = inp if 'http://' in sys.argv[1] else "http://" + sys.argv[1]
    else:
        urlManual = True
    if len(sys.argv) > 1 and (sys.argv[1] in ['-d', '-debug'] or sys.argv[2] in ['-d', '-debug']):
        debug = True
    main(url, urlManual, debug)
