#!/usr/bin/env python3

import sys
import serial
import cv2
import math
import numpy as np
from cv2 import aruco

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\r\n'

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
    cap = cv2.VideoCapture(0)

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
            pos = process_img(frame)

            # Sends the position to the Serial Port
            if not noSerOut and (pos[0] != -1 and pos[1] != -1):
                ser.write(MSG_START + str(pos)      +
                          MSG_SEP   + str(timeStmp) +
                          MSG_END)

            cv2.imshow("Output", frame)
            time += 1
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
            - [-detail]     : If this option is present, the program will open
                              multiple windows with views at different stages of
                              the image processing
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
