#!/usr/bin/env python3

# Source : https://stackoverflow.com/questions/62031686/reading-a-serial-signal-from-a-usb-port-with-python
import serial;

class SerialImg(serial.Serial):
    def __init__(self,
                 port:     str = "/dev/ttyUSB0",
                 baudrate: int = 115200,
                 timeout:  int = 1):
        serial.Serial.__init__(self,
                               port=port,
                               baudrate=baudrate,
                               timeout=timeout,
                               xonxoff=False,
                               rtscts=False,
                               dsrdtr=True)

    def getImg(self):
        """
        Returns the current image sent by the device on the serial
        connection
        """

        img = b''

        end   = False
        inImg = False
        try:
            while not end:
                line = self.readline()
                print(line)

                if line[:4] == b'img:':
                    line  = line[4:]
                    inImg = True
                elif line[:3] == b'end':
                    line = b''
                    end  = True

                if inImg:
                    img += line
        except:
            img = None
        finally:
            return img
