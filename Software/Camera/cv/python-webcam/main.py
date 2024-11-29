#!/usr/bin/env python3

import sys
import serial
import cv2
import numpy as np
from cv2 import aruco

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\r\n'

def process_img(img):
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
            center = (i[0], i[1])
            # circle center
            cv2.circle(img, center, 1, (0, 100, 100), 3)
            # circle outline
            radius = i[2]
            # print("Radius: " + str(radius))
            cv2.circle(img, center, radius, (255, 0, 0), 3)

    # mask = cv2.inRange(img, clr_range[0], clr_range[1])
    # out = cv2.bitwise_and(img, img, mask=mask)
    # cv2.imshow("img", np.hstack([img, out]))
        # Center Position of the first circle
        pos = (circles[0, 0][0], circles[0, 0][1])
    return pos

def main(noSerOut=False, noQR=False, detailed=False, windowScale=1, windows=None):
    """
    Gets the live image of the camera, transforms it such that it only contains the playing field, detects the ball and
    sends the position of the ball over a serial output to the Arduino.
    Displays the ball detection on screen.

    :param noSerOut: Disables serial output (Use if serial port is disconnected)
    :type noSerOut: bool
    :param noQR: If this is set to False, this function will first detect QR markers and transform the image such that
                 it only contains the playing area, before doing ball detection.
    :type noQR: bool
    :param detailed: If this is set to True, this function will open multiple windows showing different stages of the
                     image processing
    :type detailed: bool
    :param windowScale: Defines scaling of the displayed windows compared to original capture height and width
    :type windowScale: int
    :param windows: Defines which windows are being displayed. Should have one entry for each possible window
    :type windows: list(bool)
    """

    # For colour detection
    # #ae5757 is also a good fit
    # clr = [136, 152, 235] # BGR
    # tlr = 50
    # clr_range = [
    #     (clr[0]-tlr, clr[1]-tlr, clr[2]-tlr),
    #     (clr[0]+tlr, clr[1]+tlr, clr[2]+tlr)
    # ]

    if windows is None:
        windows = [True, False, False]
    showOut, showMarkers, showTransformed = windows

    # For getting data
    # Open the default camera
    cap = cv2.VideoCapture(0)

    frameWidth  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH ))
    frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print(f"Width: %d\n", frameWidth)
    print(f"Height: %d\n", frameHeight)

    # Setting i to the minimum possible value
    time = -sys.maxsize - 1
    
    # Defines a serial port for the output via user input
    if not noSerOut:
        port = '/dev/ttyUSB0'

        user_in = input("Device ['" + port + "']:")

        # Creates a Serial Port object with the port from the user input or the default value otherwise
        ser = serial.Serial(
            port = port if not user_in else user_in
        )
        ser.open()
        print("Serial opened.")

    if not noQR:
        # Define destination points (corners of the image)
        dst_points = np.float32([
            [0, 0],  # Upper-left corner
            [frameWidth - 1, 0],  # Upper-right corner
            [frameWidth - 1, frameHeight - 1],  # Lower-right corner
            [0, frameHeight - 1]  # Lower-left corner
        ])

        # Define Dictionary for keeping the last known positions of the markers
        last_known_positions = {1: None, 2: None, 3: None, 4: None}

        # We use the markers at indexes 1, 2, 3, 4 of this pre-defined dictionary to mark the playing area
        aruco_dict = aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)

        if detailed or showMarkers:
            cv2.namedWindow("Marker Detection", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Marker Detection", int(frameWidth / windowScale), int(frameHeight / windowScale))
        if detailed or showTransformed:
            cv2.namedWindow("Transformed Frame", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Transformed Frame", int(frameWidth / windowScale), int(frameHeight / windowScale))
    if detailed or showOut:
        cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Output", int(frameWidth / windowScale), int(frameHeight / windowScale))

    # Main Loop for image processing
    try:
        while True:
            # Get an image from the camera
            ret, frame = cap.read()

            if not noQR:
                # Detect markers in image
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                corners, ids, _ = aruco.detectMarkers(gray, aruco_dict)

                if detailed or showMarkers:
                    markers = np.copy(frame)
                    aruco.drawDetectedMarkers(markers, corners, ids)
                    cv2.imshow("Marker Detection", markers)
                    cv2.waitKey(1)

                # Updates the dictionary of last known positions if the right markers were detected
                if ids is not None:
                    for indices, corner in zip(ids, corners):
                        index = indices[0]
                        if index in last_known_positions:
                            last_known_positions[index] = corner[0][0]

                # Transforms the image if all 4 markers were detected (at some point)
                if all(last_known_positions[i] is not None for i in last_known_positions.keys()):
                    src_points = np.float32([
                        last_known_positions[1],
                        last_known_positions[2],
                        last_known_positions[3],
                        last_known_positions[4]
                    ])
                    transformation_matrix = cv2.getPerspectiveTransform(src_points, dst_points)
                    transformed_frame = cv2.warpPerspective(frame, transformation_matrix, (frameWidth, frameHeight))
                    if np.array(transformed_frame).size != 0:
                        frame = transformed_frame
                if detailed or showTransformed:
                    cv2.imshow("Transformed Frame", frame)
                    cv2.waitKey(1)

            timeStmp = time + 2**32; # Converting time to unsigned
            pos = process_img(frame)

            # Sends the position to the Serial Port
            if not noSerOut and (pos[0] != -1 and pos[1] != -1):
                ser.write(MSG_START + str(pos) + MSG_SEP + str(timeStmp) + MSG_END)
            if showOut:
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
        <cmd> [-noSerOut] [-noQR] [-detailed | -windows <window 1> ...] [-wScale <factor>]
            Flags :
            - [-noSerOut]                   : Disables Serial output
            - [-noQR]                       : Disables use of QR Markers for image transformation
            - [-detailed]                     : If this option is present, the program will open multiple windows with 
                                              views at different stages of the image processing
            - [-windows <window 1> ...]     : Specify the windows that should be displayed. Choose from 
                                              ["out", "markers", "transformed"]. So for example if you want to display 
                                              the output and the marker detection, you should use -window out markers
                                              Passing -window without specified windows will disable all windows
            - [-wScale <factor>]            : Scales Displayed window(s) to size 
                                              1/factor * (original width) x 1/factor * (original height)
    """
    windowScale = 1
    noSerOut = False
    noQR = False
    detailed = False
    wOut = True
    wMark = False
    wTransf = False
    for arg in sys.argv:
        if arg == '-noSerOut':
            noSerOut = True
        if arg == '-noQR':
            noQR = True
        if arg == '-detailed':
            detailed = True
        if arg == '-windows':
            i = sys.argv.index(arg) + 1
            wOut = False
            while i < len(sys.argv) and not sys.argv[i].startswith('-'):
                window = sys.argv[i]
                if window == 'out':
                    wOut = True
                if window == 'markers':
                    wMark = True
                if window == 'transformed':
                    wTransf = True
                i += 1
        if arg == '-wScale':
            i = sys.argv.index(arg) + 1
            if i < len(sys.argv):
                try:
                    windowScale = int(sys.argv[i])
                except:
                    print("Invalid Factor")
    print([wOut, wMark, wTransf])
    main(noSerOut, noQR, detailed, windowScale, [wOut, wMark, wTransf])
