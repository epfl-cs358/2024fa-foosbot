#!/usr/bin/env python3

import sys
import serial
import cv2
import numpy as np
from cv2 import aruco

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\r\n'

width = 0
height = 0

def process_img(img):
    """
    Processes the image for ball detection.

    :img: The image to be processed.
    :returns: The position of the ball and the time stamp.

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

def main(noSerOut, useQR, detailed):
    """
    Connects to the livestream of the given URL, gets the image of the `/bmp`
    URI, detects the ball and sends over serial the position of the ball.
    Displays the ball detection on screen.

    :param noSerOut: Disables serial output (Use if serial port is disconnected)
    :type noSerOut: bool
    :param useQR: If this is set to True, this function will first detect QR markers and transform the image such that it only contains the playing area, before doing ball detection.
    :type useQR: bool
    :param detailed: If this is set to True, this function will open multiple windows showing different stages of the image processing
    :type detailed: bool
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
    # Open the default camera
    cap = cv2.VideoCapture(0)

    frameWidth  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH ))
    frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    # For sending data
    if not noSerOut:
        port = '/dev/ttyUSB0'

        user_in = input("Device ['" + port + "']:")

        ser = serial.Serial(
            port = port if not user_in else user_in
        )

        ser.open()
        print("Serial opened.")

    if useQR:
        global width, height
        # Define destination points (corners of the image)
        dst_points = np.float32([
            [0, 0],  # Upper-left corner
            [width - 1, 0],  # Upper-right corner
            [width - 1, height - 1],  # Lower-right corner
            [0, height - 1]  # Lower-left corner
        ])

        last_known_positions = {1: None, 2: None, 3: None, 4: None}
        aruco_dict = aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)


    # Main Loop for image processing
    try:
        while True:

            ret, frame = cap.read()

            if useQR:
                height, width = frame.shape[:2]
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                corners, ids, _ = aruco.detectMarkers(gray, aruco_dict)
                if detailed:
                    aruco.drawDetectedMarkers(gray, corners, ids)
                    cv2.imshow("Marker Detection", gray)
                    cv2.waitKey(1)
                if ids is not None:
                    for id_array, corner in zip(ids, corners):
                        id = id_array[0]
                        if id in last_known_positions:
                            last_known_positions[id] = corner[0][0]

                if all(last_known_positions[id] is not None for id in last_known_positions):
                    src_points = np.float32([
                        last_known_positions[1],
                        last_known_positions[2],
                        last_known_positions[3],
                        last_known_positions[4]
                    ])
                    transformation_matrix = cv2.getPerspectiveTransform(src_points, dst_points)
                    transformed_frame = cv2.warpPerspective(frame, transformation_matrix, (width, height))
                    if detailed:
                        cv2.imshow("Transformed Frame", transformed_frame)
                        cv2.waitKey(1)

            pos, timeStmp, img = process_img(frame)

            # if debug:
            #     print(pos)
            # else:
            #     if pos[0] != -1 and pos[1] != -1:
            #         ser.write(MSG_START + str(pos) + MSG_SEP + str(timestmp) + MSG_END)

            cv2.imshow("Output", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    except Exception as e:
        if not noSerOut:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()
        raise e


if __name__ == "__main__":
    """
    Argument Parser when called from command line.
    Allowed Format:
        <cmd> [-noSerOut] [-useQR] [-detail]
            Flags :
            - [-noSerOut]   : Disables Serial output
            - [-useQR]      : Enables use of QR Markers for image transformation
            - [-detail]     : If this option is present, the program will open multiple windows 
                                with views at different stages of the image processing
    """
    noSerOut = False
    useQR = False
    detailed = False
    for arg in sys.argv:
        if arg == '-noSerOut':
            noSerOut = True
        if arg == '-useQR':
            useQR = True
        if arg == '-detail':
            detailed = True
    main(noSerOut, useQR, detailed)
