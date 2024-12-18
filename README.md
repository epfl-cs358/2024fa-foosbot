# Foosbot

https://github.com/user-attachments/assets/93106235-c6af-46eb-8334-a48d785348d0

![table](https://github.com/user-attachments/assets/d53f9f6b-330a-4f3f-b5df-cbbba390c06f)

![circuitry](https://github.com/user-attachments/assets/fd3dcc90-f1f6-4451-a09b-e1358a30fe21)

## Overview
Table football, also known as foosball or table soccer, is a game based on as- sociation football. The objective is to move the ball into the opponent’s goal by manipulating rods with figures resembling football players of two opposing teams. This fast-paced game is typically played by 2-4 players, but it is not designed for solo play.
Our project proposes the creation of a Foosball table that can be played by a single player, with the help of an Arduino Uno board. Our vision involves a program executed from a computer connected to the microcontroller, controlling one side of the game, while a human player controls the other side. This system should at least be able to rotate the handles and displace them, considering two axes of movement per pole.
A traditional foosball table is as large as a standard table and has four handles per side. However, for the sake of our project’s budget and complexity, the prototype will be based on a smaller version with only two handles per side. This approach will reduce system complexity and improve motor reaction times.

## Project Structure
- `Admin` : Contains images used in the README
- `CAD` : Contains all the CAD files as well as the dxf files for lasercutting
- `Hardware` : Contains the electronic circuit diagram and a document with hardware component specifications
- `Software` : Contains all the code
  - `Software/Algorithms` : Contains the Arduino code for controlling the motors and handling the game logic
  - `Software/Camera/cv/python-webcam` : Contains the python code for the computer vision as well as the QR Markers to printout

## List of Materials
- 4 x 17HS4401 Stepper Motors
- 4 x A4988 Motor Drivers
- 4 x End stop Sensors
- 1 x Arduino Uno
- 1 x USB cable to connect to the Arduino
- 1 x CNC Shield
- 1 x USB Isolator
- 1 x Logitech Webcam C270
- 2 x Aluminium Profile 20 x 20 mm of length 41 cm
- 1 x 12V Power Supply
- 1 x Power Supply Plug
- 2 x ~1 meter GT2 6mm belt
- 1 x 1 meter WS2813 LED Strips
- 1 x Bread board
- 2 x Metal Bars with diameter 15 mm
- 2 x Metal Bars with diameter 8 mm
- 4 x Ball Bearings that allow for both rotational and linear motion
- 8 x Wheels (Something like [this](https://www.zyltech.com/zyltech-20-series-wheel-pulley-bearing-for-2020-aluminum-extrusion-v-slot-compatible/))
- 2 x Pully
- Jumper cables
- A ball

## Do it yourself
### General Overview of tasks
- Lasercut
- 3D Print
- Assemble the Table
- Wire up the motors to the Arduino
- Set up the Camera Vision

### Building the table

### Wiring of the Motors
#### Electronics Chart
![photo_2024-12-17_18-52-56](https://github.com/user-attachments/assets/1b046676-7fc6-4c36-af1d-b620efba9c2d)


CNC shield mounted on top of the Arduino Uno , reset buttons should be aligned 
CNC shield-Arduino Uno pin correspondance
![cf988326f7e0baaa42da84e9b4440201e97f966a](https://github.com/user-attachments/assets/9fa05bec-4b6c-4132-ae6e-10df3555fa08)

#### Motor Labeling
On the CNC Shield there are 3 slots for motor drivers labelled X, Y, Z, A. In the picture below you see to which motor they should be connected to.

The End stop sensors have three cables that need to be connected, one to the ground, one to the 5V Power and one to the correct pins on the CNC Shield. The mapping of the position of the end stop sensors to the pins on the CNC Shield are shown in green in the picture below.

![MotorLabels](https://github.com/user-attachments/assets/7c3adf2b-dc6a-4176-9014-4f16ec0fd249)

#### Protecting the motors and your laptop
Always connect the Arduino via a USB-Isolator to your laptop and ideally don't charge your laptop while it is connected to the Arduino.
It is also important that you unplug the power and the Arduino before changing anything on the wiring or manually moving the motors.
Generally it is also advised to disconnect the power and the arduino whenever you're not using them.
Also note that the motor drivers on the CNC shield might head up a bit if used for a long period of time.

### Uploading code to the Arduino
#### Dependencies
You need to have [Arduino IDE](https://www.arduino.cc/en/software) installed, as well as the libraries
#### Uploading Code
1. Open the code for the Arduino in `/Software/Algorithms/beginner/beginner.ino`
2. Connect the Arduino via the USB Isolator to your laptop
3. Press the upload button in the top left of the Arduino IDE

#### Troubleshooting
- If you get a compilation error, check whether you have all libraries installed.
- If you get an error while uploading, check whether you have selected the right board and port and try again.
- If the motors don't move check your wiring. If that doesn't help check the voltage of the motor drivers using a multimeter. Also the [documentation](https://docs.cirkitdesigner.com/component/2c4686b3-94a4-4df1-aee0-7a9f67fd5f2e/cnc-shield-v3) of the CNC shield might help.

### Camera Vision
#### QR Code Markers
You need to print out the 4 QR Code Markers stored in the file
[`/Software/Camera/cv/python-webcam/ArUco markers.pdf`](https://github.com/epfl-cs358/2024fa-foosbot/blob/main/Software/Camera/cv/python-webcam/ArUco%20markers.pdf)
and cut them out. 

You then have to tape the QR Codes into the Right corners. Note that the top left corner, marked red in images below, should always point towards the corner. The QR Codes are numbered (see below) and they need to be in the right corners, according to the image below.

<img width="359" alt="QRCodeMarkers" src="https://github.com/user-attachments/assets/70fe4499-404d-41a1-beef-c1cddc03354b" />

![QROrientation](https://github.com/user-attachments/assets/a2bb6433-0d0d-45ec-a09f-859c6c584163)

![TopView](https://github.com/user-attachments/assets/339f815d-c2ab-453e-b471-d48fd438579c)


__It is important that the Codes are at in the right corners and oriented the right way!__
Otherwise the camera vision will not detect the playing field correctly and do a weird transformation or cut of some part.

#### Code
The code for the Camera Vision is located in the folder
[`/Software/Camera/cv/python-webcam/main.py`](https://github.com/epfl-cs358/2024fa-foosbot/blob/main/Software/Camera/cv/python-webcam/main.py)
#### Installing Dependencies
You need to have python installed on your computer (See [here](https://realpython.com/installing-python/) an article how to install python).
You als need the following libraries:
- [PySerial](https://pypi.org/project/pyserial/)
- [opencv](https://opencv.org/)

If you are on linux or macOS and have pip installed you can simply run 

```commandline
pip install pyserial
pip install opencv
```
If you're having trouble, check with your package manager whether it accepts pip installs.

#### Starting camera vision
Note that you have to close all Serial monitors in the ArduinoIDE since it prevents the computer vision from opening the port. You can open it again when the computer vision is running.
2. Plug in the Logitech webcam,
1. Plug in the Arduino UNO,
3. Navigate to folder ` Software/Camera/cv/python-webcam `,
4. Run ` ./main.py `.

If you want to know how to use different flags, type `./main.py --help`.

#### Output

![ExampleCV](https://github.com/user-attachments/assets/33007776-5cc6-4fb6-ae6e-93c52b24a5ed)


By default the camera should open a window called Output which displays the area in which the ball can be detected. If  the ball is detected, there should be a blue circle around it.

#### Troubleshooting
- If the camera is not activating properly, try changing the parameter of the call to the ` cv2.VideoCapture ` function to another number (usually 0 or 2). You can also use the `-i` flag for that.
- If the camera vision does a weird transformation then the QR Codes are not placed in the right corners.
- If the output doesn't show the entire field, then the QR Codes might not be oriented correctly (Top left corner ins't pointing towards the corner).
- If the ball isn't detected then you might need to change the allowed range for the radius (line 37 in [`main.py`](https://github.com/epfl-cs358/2024fa-foosbot/blob/main/Software/Camera/cv/python-webcam/main.py)). By default it is set to accept radii between 25 and 31. This works well for us on Macbooks but we observed that on Linux a range of 10 to 15 is appropriate.

Remember that you can use different flags to debug the computer vision. For example you can display views of different stages of computer vision (Marker detection, image transformation, etc.) using the --w or --windows flag or you can disable the Serial output if no Arduino is connected by  using the -n or --no-ser-out flag.
