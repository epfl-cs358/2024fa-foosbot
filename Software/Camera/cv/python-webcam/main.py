#!/usr/bin/env python3

import sys
import serial
import cv2
import math
import numpy as np

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\r\n'

def get_ball_pos(img, clrRange):
    """
    Processes the image for ball detection.

    :buf: The buffer containing the image.
    :clr_range: The colour range to detect the ball.
    :returns: The position of the ball.

    """
    grey = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    grey = cv2.medianBlur(grey, 5)

    rows = grey.shape[0]

    circles = cv2.HoughCircles(grey, cv2.HOUGH_GRADIENT, 1, rows / 8,
                               param1=100, param2=30,
                               minRadius=1, maxRadius=25)

    pos = (-1, -1)

    if circles is not None:
        circles = np.uint16(np.around(circles))
        for i in circles[0, :]:
            # Circle center
            center = (i[0], i[1])
            cv2.circle(img, center, 1, (0, 100, 100), 3)
            # Circle outline
            radius = i[2]
            # Inner square length divided by two
            length = math.floor(math.sqrt(2) * radius / 2)
            sums = np.array(cv2.sumElems(
                img[i[0]-length:i[0]+length, i[1]-length:i[1]+length]
            ))
            for i in range(sums.size):
                if sums[i] > 255:
                    sums[i] = 255
            mean = sums / (length*2)**2
            # Draw inner square
            # cv2.rectangle(img,
            #               (i[0]-length, i[1]-length),
            #               (i[0]+length, i[1]+length),
            #               (255, 0, 0), 3)
            print(clrRange)
            print(mean)

            if (mean[0] in clrRange[0] and
                mean[1] in clrRange[1] and
                mean[2] in clrRange[2]):
                print("Found: ", mean)
                pos = center
                # print("Radius: " + str(radius))
                cv2.circle(img, center, radius, (255, 0, 0), 3)

        pos = (circles[0, 0][0], circles[0, 0][1])

    return pos

def main(debug=False):
    """
    Connects to the livestream of the given URL, gets the image of the `/bmp`
    URI, detects the ball and sends over serial the position of the ball.
    Displays the ball detection on screen.

    :param debug: Disables serial ports (Use if serial port is disconnected)
    :type debug: bool
    """

    # For colour detection
    # Might be good fits (RGB):
    #  - #ae5757
    #  - #ff7f41
    #  - #eb9888
    clr = np.array([65, 127, 255]) # BGR
    tlr = 50
    clrLo = clr - tlr
    clrHi = clr + tlr
    clrRange = [
        range(clrLo[0], clrHi[0]),
        range(clrLo[1], clrHi[1]),
        range(clrLo[2], clrHi[2])
    ]

    # For getting data
    # Open the default camera
    cap = cv2.VideoCapture(2)

    frameWidth  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH ))
    frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print("Width: ", frameWidth)
    print("Height: ", frameHeight)

    # For sending data
    if not debug:
        port = '/dev/ttyUSB0'

        user_in = input("Device ['" + port + "']:")

        ser = serial.Serial(
            port = port if not user_in else user_in
        )

        ser.open()
        print("Serial opened.")

    # Setting i to the minimum possible value
    i = -sys.maxsize - 1
    # Main Loop for image processing
    try:
        while True:

            ret, frame = cap.read()
            timeStmp = i + 2**32; # Converting i to unsigned
            pos = get_ball_pos(frame, clrRange)

            # if debug:
            #     print(pos)
            # else:
            #     if pos[0] != -1 and pos[1] != -1:
            #         ser.write(MSG_START + str(pos) + MSG_SEP + str(timestmp) + MSG_END)

            cv2.imshow("Output", frame)
            cv2.waitKey(1)

            i += 1
    except KeyboardInterrupt:
        print("\nTerminating...")
        if not debug:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()
    except Exception as e:
        if not debug:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()
        raise e


if __name__ == "__main__":
    """
    Argument Parser when called from command line.
    Allowed Format:
        <cmd> [-d | -debug]     (If no arguments are given, the user is asked for the url later)
    """
    debug = False
    if len(sys.argv) > 1 and (sys.argv[1] in ['-d', '-debug'] or sys.argv[2] in ['-d', '-debug']):
        debug = True
    main(debug)
