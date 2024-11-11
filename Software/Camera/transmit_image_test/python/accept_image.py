# Source : https://stackoverflow.com/questions/62031686/reading-a-serial-signal-from-a-usb-port-with-python
import serial;
import io;
import time;
import os;

if __name__ == '__main__' :
    end = False
    while not end:
        try :
            # configure the serial connections (the parameters differs on the device you are connecting to)
            s = serial.Serial(port='/dev/cu.usbserial-A5XK3RJT Serial Port (USB)', baudrate=115200, timeout=1,
                           xonxoff=False, rtscts=False, dsrdtr=True)

            data = s.readline()
            f = open("img.jpg", "w")
            print(data)
            f.write(data)
            end = True
            f.close()
            s.close()
        except :
            pass
    pass
