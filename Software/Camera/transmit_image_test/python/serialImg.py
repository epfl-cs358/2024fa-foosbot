#!/usr/bin/env python3

# Source : https://stackoverflow.com/questions/62031686/reading-a-serial-signal-from-a-usb-port-with-python
import serial;

class SerialImg(serial.Serial):
    xonxoff = False
    rtscts  = False
    dsrdtr  = True

    def __init__(self,
                 port:     str = '/dev/ttyUSB0',
                 baudrate: int = 115200,
                 timeout:  int = 1):
        self.port     = port,
        self.baudrate = baudrate
        self.timeout  = timeout

    def getImg(self):
        """
        Returns the current image sent by the device on the serial
        connection
        """

        img = b''

        end   = False
        inImg = False
        while not end:
            try:
                line = self.readline()

                if line[:4] == 'img:':
                    line  = line[4:]
                    inImg = True
                elif line[:2] == 'end':
                    line = b''
                    end  = True

                if inImg:
                    img += line
            except:
                img = None
            finally:
                return img
