# Foosbot

## Overview
Table football, also known as foosball or table soccer, is a game based on as- sociation football. The objective is to move the ball into the opponent’s goal by manipulating rods with figures resembling football players of two opposing teams. This fast-paced game is typically played by 2-4 players, but it is not designed for solo play.
Our project proposes the creation of a Foosball table that can be played by a single player, with the help of an Arduino Uno board. Our vision involves a program executed from a computer connected to the microcontroller, controlling one side of the game, while a human player controls the other side. This system should at least be able to rotate the handles and displace them, considering two axes of movement per pole.
A traditional foosball table is as large as a standard table and has four handles per side. However, for the sake of our project’s budget and complexity, the prototype will be based on a smaller version with only two handles per side. This approach will reduce system complexity and improve motor reaction times.

## Project Structure

## General Overview of tasks
- Lasercut
- 3D Print
- Assemble the Table
- Wire up the motors to the Arduino
- Connect and mount the LED Strips
- Set up the Camera Vision

## List of Materials

## Building the table

## Wiring of the Motors

## Camera Vision
### QR Code Markers
You need to print out the 4 QR Code Markers stored in the file
[`/Software/Camera/cv/python-webcam/ArUco markers.pdf`](https://github.com/epfl-cs358/2024fa-foosbot/blob/main/Software/Camera/cv/python-webcam/ArUco%20markers.pdf)
and cut them out. 

![QRCodeMarkers.png]()

You then have to tape the QR Codes into the Right corners. 
Here is a picture from below the table where the motors are to the left:

![table.png]()

__It is important that the Codes are oriented the right way!__
Otherwise the camera won't detect the full playing field.

### Code
The code for the Camera Vision is located in the folder
[`/Software/Camera/cv/python-webcam/main.py`](https://github.com/epfl-cs358/2024fa-foosbot/blob/main/Software/Camera/cv/python-webcam/main.py)
### Installing Dependencies
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

### Starting camera vision

1. Plug in the Arduino UNO,
2. Plug in the Logitech webcam,
3. Navigate to folder ` Software/Camera/cv/python-webcam `,
4. Run ` ./main.py `.

If you want to know how to use different flags, type `./main.py --help`.

### Troubleshooting

If the camera is not activating properly, try changing the parameter of the
call to the ` cv2.VideoCapture ` function to another number (usually 0 or 2).
You can also use the `-i` flag for that.

## Uploading code to the Arduino