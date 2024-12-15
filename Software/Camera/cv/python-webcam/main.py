#!/usr/bin/env python3

import sys
import serial
import cv2
import math
import numpy as np
from cv2 import aruco

FIELD_WIDTH  = 690
FIELD_HEIGHT = 615

QR_SIZE = 47

MSG_START = ':'
MSG_SEP   = ';'
MSG_END   = '\n'

def get_ball_pos(img, clrRange, clrDet):
    """
    Detects the ball.

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
                               minRadius=10, maxRadius=15)

    pos = (-1, -1)

    if circles is not None:
        circles = np.uint16(np.around(circles))
        # Draws Circles and centers
        for i in circles[0, :]:
            # Circle center
            center = (i[0], i[1])
            # Circle outline
            radius = i[2]
            if clrDet:
                # Inner square length divided by two
                length = math.floor(math.sqrt(2) * radius / 2)
                mean = cv2.mean(
                    img[i[0]-length:i[0]+length, i[1]-length:i[1]+length]
                )
                if verbose:
                    print(clrRange)
                    print("\nB: ", mean[0], "\nG: ", mean[1], "\nR: ", mean[2])
                    print("Found: ", mean)

                # if (clrRange[0][0] <= mean[0] <= clrRange[0][1] and
                #     clrRange[1][0] <= mean[1] <= clrRange[1][1] and
                #     clrRange[2][0] <= mean[2] <= clrRange[2][1]):

                pos = center
                cv2.circle(img, center, radius, (255, 0, 0), 3)
            else:
                cv2.circle(img, center, radius, (255, 0, 0), 3)
            cv2.circle(img, center, 1, (0, 100, 100), 3)

    return pos

def main(noSerOut,
         noQR,
         clrDet,
         verbose,
         windowScale,
         windows,
         inp,
         port):
    """
    Gets the live image of the camera, transforms it such that it only contains
    the playing field, detects the ball and sends the position of the ball over
    a serial output to the Arduino. Displays the ball detection on screen.

    :param noSerOut: Disables serial output (Use if serial port is disconnected)

    :type noSerOut:  bool

    :param noQR: If this is set to False, this function will first detect QR
                 markers and transform the image such that it only contains the
                 playing area, before doing ball detection.
    :type noQR:  bool

    :param clrDet: Activates Color Detection to complement Circle Detection.

    :type clrDet: bool

    :param verbose: If this is set to True, this function will open multiple
                    windows showing different stages of the image processing
    :type verbose:  bool

    :param windowScale: Defines scaling of the displayed windows compared to
                        original capture height and width
    :type windowScale:  int

    :param windows: Defines which windows are being displayed. Should have
                    one entry for each possible window
    :type windows:  list(bool)

    :param inp:     Defines what is passed to the VideoCapture function.
                    Passing 0 generally opens the default camera.

    :type inp:      int

    :param port:    The Serial port to which the position should be sent

    :type port:     str
    """

    # Decides what windows will be displayed
    showOut, showMarkers, showTransformed, showOrigin = windows

    # Might be good fits (RGB):
    #  - #ae5757
    #  - #ff7f41
    #  - #eb9888
    #  - #eb5230
    #  - #ffa894
    #  - #70282a
    #  - #7d342f
    clr = [47, 52, 125] # BGR
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

    # Defines a serial port for the output via user input
    if not noSerOut:
        if port is not None:
            port = '/dev/ttyUSB0'
            user_in = input("Device ['" + port + "']:")
            if user_in:
                port = user_in
        try:
            # Creates a Serial Port object with the port from the user input or the
            # default value otherwise
            ser = serial.Serial(port = port)
            if ser.isOpen() == False:
                ser.open()
            print("Serial opened.")
        except serial.SerialException:
            noSerOut = False
            print("Invalid Port : "+str(port)+" !\nLaunching the program with Serial output disabled.\n")

    # Opens the camera for video capturing
    cap = cv2.VideoCapture(inp)

    frameWidth  = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH ))
    frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    if not noQR:
        # We use the markers at indices 1, 2, 3, 4 of this pre-defined
        # dictionary to mark the playing area
        aruco_dict = aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
        if verbose or showMarkers:
            cv2.namedWindow("Marker Detection", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Marker Detection",
                             int(frameWidth / windowScale),
                             int(frameHeight / windowScale))
        # Define Dictionary for keeping the last known positions of the markers
        lastKnownPos = {1: None, 2: None, 3: None, 4: None}

        if verbose or showTransformed: # Create window for the transformed frame if necessary
            cv2.namedWindow("Transformed Frame", cv2.WINDOW_NORMAL)
            cv2.resizeWindow("Transformed Frame",
                             int(frameWidth / windowScale),
                             int(frameHeight / windowScale))

    if verbose or showOrigin: # Create window for original frame if necessary
        cv2.namedWindow("Origin", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Origin",
                         int(frameWidth / windowScale),
                         int(frameHeight / windowScale))
    if verbose or showOut: # Create window for output (after processing) if necessary
        cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Output",
                         int(frameWidth / windowScale),
                         int(frameHeight / windowScale))

    # Main Loop for image processing
    try:
        while True:
            # Get an image from the camera
            _, frame = cap.read()

            if verbose or showOrigin:
                cv2.imshow("Origin", frame)

            if not noQR: # Do QR Marker Detection and Transformation if necessary
                # Detect markers in image
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                corners, ids, _ = aruco.detectMarkers(gray, aruco_dict)

                if verbose or showMarkers:
                    markers = np.copy(frame)
                    aruco.drawDetectedMarkers(markers, corners, ids)
                    cv2.imshow("Marker Detection", markers)
                    cv2.waitKey(1)

                # Updates the dictionary of last known positions if the right
                # markers were detected
                if ids is not None:
                    for indices, corner in zip(ids, corners):
                        index = indices[0]
                        if index in lastKnownPos:
                            lastKnownPos[index] = corner[0][0]

                # Transforms the image if all 4 markers were detected (at some
                                                                       # point)
                if all(lastKnownPos[i] is not None
                       for i in lastKnownPos.keys()):
                    src_points = np.float32([
                        lastKnownPos[1],
                        lastKnownPos[2],
                        lastKnownPos[3],
                        lastKnownPos[4]
                    ])
                    fieldWidth = lastKnownPos[2][0] - lastKnownPos[1][0]
                    fieldHeight = lastKnownPos[3][1] - lastKnownPos[2][1]
                    dst_points = np.float32([
                        [0, 0],  # Upper-left corner
                        [fieldWidth, 0],  # Upper-right corner
                        [fieldWidth, fieldHeight],  # Lower-right corner
                        [0, fieldHeight]  # Lower-left corner
                    ])
                    transformation_matrix = cv2.getPerspectiveTransform(
                        src_points, dst_points
                    )
                    transformed_frame = cv2.warpPerspective(
                        frame, transformation_matrix, (int(fieldWidth), int(fieldHeight))
                    )
                    if np.array(transformed_frame).size != 0:
                        frame = transformed_frame
                if verbose or showTransformed:
                    cv2.imshow("Transformed Frame", frame)
                    cv2.waitKey(1)

            pos = get_ball_pos(frame, clrRange, clrDet)

            # Sends the position to the Serial Port
            if not noSerOut and (pos[0] != -1 and pos[1] != -1):
                ser.write((
                    MSG_START + str(pos[0])   +
                    MSG_SEP   + str(pos[1])   +
                    MSG_END).encode())

            if verbose or showOut: # Show Output Window
                cv2.imshow("Output", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    except KeyboardInterrupt: # Terminates the program when pressing 'q'
        if not noSerOut:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()
        print("\nTerminating...")
    except Exception as e: # Exception handling
        if not noSerOut:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()
        raise e


if __name__ == "__main__":
    """
    Argument Parser when called from command line.
    Allowed Format:
        <cmd> [-n | --no-ser-out] [-t | --no-transf] [-c | -clr-det] [-v | --verbose] [[-w | --windows] <window 1> ...] 
                [[-s | --scale] <factor>] [[-i | --input] <cam id>] [[-p | --port] <port>]
            Flags :
            - [-n | --no-ser-out]               : Disables Serial output
            - [-t | --no-transf]                : Disables use of QR Markers for image transformation
            - [-c | --clr-det]                  : Activates filtering by Color Detection
            - [-v | --verbose]                  : If this option is present, the program will open multiple windows with
                                                  views at different stages of the image processing
            - [[-w | --windows] <window 1> ...] : Specify the windows that should be displayed. Choose from
                                                  ["out", "markers", "transformed", "origin"]. So for example if you
                                                  want to display the output and the marker detection, you should use
                                                  -window out markers Passing -w without specified windows will
                                                  disable all windows
            - [[-s | --scale] <factor>]         : Scales Displayed window(s) to size
                                                  1/factor * (original width) x 1/factor * (original height)
            - [[-i | --input] <cam id>]         : Sets the video capture input¨
            - [-p || -port <port>]              : Defines a port for the serial output
    """
    windowScale = 1

    noSerOut    = False
    noQR        = False
    clrDet      = False
    verbose     = False
    wOut        = True
    wMark       = False
    wTransf     = False
    wOrig       = False
    helpMode    = False
    inp         = 0
    port        = "/dev/ttyUSB0"
    for arg in sys.argv:
        if arg == '--no-ser-out' or arg == '-n':
            noSerOut = True
        if arg == '--no-transf' or arg == '-t':
            noQR = True
        if arg == '--c' or arg == '--clr-det':
            clrDet = True
        if arg == '--verbose' or arg == '-v':
            verbose = True
        if arg == '--windows' or arg == '-w':
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
                if window == 'origin':
                    wOrig = True
                i += 1
        if arg == '--scale' or arg == '-s':
            i = sys.argv.index(arg) + 1
            if i < len(sys.argv):
                try:
                    windowScale = int(sys.argv[i])
                except:
                    print("Invalid Factor")
        if arg == '--input' or arg == '-i':
            i = sys.argv.index(arg) + 1
            if i < len(sys.argv):
                try:
                    inp = int(sys.argv[i])
                except:
                    print("Invalid Input")
        if arg == '--port' or arg == '-p':
            i = sys.argv.index(arg) + 1
            if i < len(sys.argv) and not sys.argv[i].startswith('-'):
                inp = sys.argv[i]
        if arg == '--help' or arg == '-h':
            helpMode = True
            print("""
    Argument Parser when called from command line.
    Allowed Format:
        <cmd> [-n | --no-ser-out] [-t | --no-transf] [-c | -clr-det] [-v | --verbose] [[-w | --windows] <window 1> ...] 
                [[-s | --scale] <factor>] [[-i | --input] <cam id>] [[-p | --port] <port>]
            Flags :
            - [-n | --no-ser-out]               : Disables Serial output
            - [-t | --no-transf]                : Disables use of QR Markers for image transformation
            - [-c | --clr-det]                  : Activates filtering by Color Detection
            - [-v | --verbose]                  : If this option is present, the program will open multiple windows with
                                                  views at different stages of the image processing
            - [[-w | --windows] <window 1> ...] : Specify the windows that should be displayed. Choose from
                                                  ["out", "markers", "transformed", "origin"]. So for example if you
                                                  want to display the output and the marker detection, you should use
                                                  -window out markers Passing -w without specified windows will
                                                  disable all windows
            - [[-s | --scale] <factor>]         : Scales Displayed window(s) to size
                                                  1/factor * (original width) x 1/factor * (original height)
            - [[-i | --input] <cam id>]         : Sets the video capture input¨
            - [-p || -port <port>]              : Defines a port for the serial output
    """)
    if not helpMode:
        main(
            noSerOut,
            noQR,
            clrDet,
            verbose,
            windowScale,
            [wOut, wMark, wTransf, wOrig],
            inp,
            port
        )
