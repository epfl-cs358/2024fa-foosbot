#!/usr/bin/env python3

import sys
import serial
import cv2
import math
import numpy as np
from cv2 import aruco

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\n'

def get_ball_pos(img, clrRange):
    """
    Processes the image and detect the ball.

    :img: The image to be processed.
    :type img: np.ndarray
    :returns: The position of the ball.
    """
    # Pre-Processing : convert to grayscale image and apply blur
    grey = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    grey = cv2.medianBlur(grey, 5)

    rows = grey.shape[0]

    # Detects circles
    circles = cv2.HoughCircles(grey, cv2.HOUGH_GRADIENT, 1, rows / 8,
                               param1=100, param2=30,
                               minRadius=1, maxRadius=30)

    pos = (-1, -1)

    if circles is not None:
        circles = np.uint16(np.around(circles))
        # Draws Circles and centers
        for i in circles[0, :]:
            # Circle center
            center = (i[0], i[1])
            cv2.circle(img, center, 1, (0, 100, 100), 3)
            # Circle outline
            radius = i[2]
            # Inner square length divided by two
            length = math.floor(math.sqrt(2) * radius / 2)
            mean = cv2.mean(
                img[i[0]-length:i[0]+length, i[1]-length:i[1]+length]
            )
            # Draw inner square
            # cv2.rectangle(img,
            #               (i[0]-length, i[1]-length),
            #               (i[0]+length, i[1]+length),
            #               (255, 0, 0), 3)
            print(clrRange)
            print(mean)
            print("\nB: ", mean[0], "\nG: ", mean[1], "\nR: ", mean[2])

            #if (clrRange[0][0] <= mean[0] <= clrRange[0][1] and
            #    clrRange[1][0] <= mean[1] <= clrRange[1][1] and
            #    clrRange[2][0] <= mean[2] <= clrRange[2][1]):
            print("Found: ", mean)
            pos = center
            cv2.circle(img, center, radius, (255, 0, 0), 3)

            pos = (circles[0, 0][0], circles[0, 0][1])

    return pos

def main(noSerOut, useQR, detailed):
    """
    Gets the live image of the camera, transforms it such that it only contains
    the playing field, detects the ball and sends the position of the ball over
    a serial output to the Arduino. Displays the ball detection on screen.

    :param noSerOut: Disables serial output (Use if serial port is disconnected)
    :type noSerOut: bool
    :param useQR: If this is set to True, this function will first detect QR
                  markers and transform the image such that it only contains the
                  playing area, before doing ball detection.
    :type useQR: bool
    :param detailed: If this is set to True, this function will open multiple
                     windows showing different stages of the image processing
    :type detailed: bool
    """

    # For colour detection
    # Might be good fits (RGB):
    #  - #ae5757
    #  - #ff7f41
    #  - #eb9888
    #  - #eb5230
    clr = [48, 82, 235] # BGR
    tlr = 100
    clrLo = [
        clr[0] - tlr,
        clr[1] - tlr,
        clr[2] - tlr
    ]
    clrHi = [
        clr[0] + tlr,
        clr[1] + tlr,
        clr[2] + tlr
    ]
    clrRange = [
        (clrLo[0], clrHi[0]),
        (clrLo[1], clrHi[1]),
        (clrLo[2], clrHi[2])
    ]

    # For getting data
    # Open the default camera
    cap = cv2.VideoCapture(2)

    frameWidth  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH ))
    frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print("Width: ", frameWidth)
    print("Height: ", frameHeight)

    # Setting i to the minimum possible value
    time = -sys.maxsize - 1

    # Defines a serial port for the output via user input
    if not noSerOut:
        port = '/dev/ttyUSB0'

        user_in = input("Device ['" + port + "']:")

        # Creates a Serial Port object with the port from the user input or the
        # default value otherwise
        ser = serial.Serial(
            port = port if not user_in else user_in
        )
        if ser.isOpen() == False:
            ser.open()
        print("Serial opened.")

    # Setting i to the minimum possible value
    i = -sys.maxsize - 1
    if useQR:
        # Define destination points (corners of the image)
        dst_points = np.float32([
            [0, 0],  # Upper-left corner
            [frameWidth - 1, 0],  # Upper-right corner
            [0, frameHeight - 1],  # Lower-left corner
            [frameWidth - 1, frameHeight - 1]  # Lower-right corner
        ])

        # Define Dictionary for keeping the last known positions of the markers
        last_known_positions = {1: None, 2: None, 3: None, 4: None}

        # We use the markers at indexes 1, 2, 3, 4 of this pre-defined
        # dictionary to mark the playing area
        aruco_dict = aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)

        if detailed:
            cv2.namedWindow("Marker Detection", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Marker Detection",
                             int(frameWidth / 2),
                             int(frameHeight / 2))
            cv2.namedWindow("Transformed Frame", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Transformed Frame",
                             int(frameWidth / 2),
                             int(frameHeight / 2))

    cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
    cv2.resizeWindow("Output", int(frameWidth / 2), int(frameHeight / 2))

    # Main Loop for image processing
    try:
        while True:
            # Get an image from the camera
            ret, frame = cap.read()

            if useQR:
                # Detect markers in image
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                corners, ids, _ = aruco.detectMarkers(gray, aruco_dict)

                if detailed:
                    markers = np.copy(frame)
                    aruco.drawDetectedMarkers(markers, corners, ids)
                    #cv2.imshow("Marker Detection", markers)
                    cv2.waitKey(1)

                # Updates the dictionary of last known positions if the right
                # markers were detected
                if ids is not None:
                    for indices, corner in zip(ids, corners):
                        index = indices[0]
                        if index in last_known_positions:
                            last_known_positions[index] = corner[0][0]

                # Transforms the image if all 4 markers were detected (at some
                                                                       # point)
                if all(last_known_positions[i] is not None
                       for i in last_known_positions.keys()):
                    src_points = np.float32([
                        last_known_positions[1],
                        last_known_positions[2],
                        last_known_positions[3],
                        last_known_positions[4]
                    ])
                    transformation_matrix = cv2.getPerspectiveTransform(
                        src_points, dst_points
                    )
                    transformed_frame = cv2.warpPerspective(
                        frame, transformation_matrix, (frameWidth, frameHeight)
                    )
                    if np.array(transformed_frame).size != 0:
                        frame = transformed_frame
                    if detailed:
                        #cv2.imshow("Transformed Frame", frame)
                        cv2.waitKey(1)

            timeStmp = time + 2**32; # Converting time to unsigned
            pos = get_ball_pos(frame, clrRange)

            # Sends the position to the Serial Port
            if not noSerOut and (pos[0] != -1 and pos[1] != -1):
                ser.write(str.encode(MSG_START + str(pos)      +
                          MSG_SEP   + str(timeStmp) +
                          MSG_END))

            cv2.imshow("Output", frame)
            time += 1
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    except KeyboardInterrupt:
        if not noSerOut:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()
        print("\nTerminating...")
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
        <cmd> [--noSerOut] [--useQR] [--detail]
            Flags :
            - [--noSerOut]   : Disables Serial output
            - [--useQR]      : Enables use of QR Markers for image transformation
            - [--detail]     : If this option is present, the program will open
                               multiple windows with views at different stages of
                               the image processing
    """
    noSerOut = False
    useQR    = False
    detailed = False
    for arg in sys.argv:
        if arg == '--noSerOut' or arg == '-n':
            noSerOut = True
            main(noSerOut, useQR, detailed)
        if arg == '--useQR' or arg == '-u':
            useQR = True
            main(noSerOut, useQR, detailed)
        if arg == '--detail' or arg == '-d':
            detailed = True
            main(noSerOut, useQR, detailed)
        if arg == '--help' or arg == '-h':
            print("""
Argument Parser when called from command line.
Allowed Format:
    <cmd> [--noSerOut] [--useQR] [--detail]
        Flags :
        - [--noSerOut]   : Disables Serial output
        - [--useQR]      : Enables use of QR Markers for image transformation
        - [--detail]     : If this option is present, the program will open
                           multiple windows with views at different stages of
                           the image processing
                  """)
        else:
            main(noSerOut, useQR, detailed)
