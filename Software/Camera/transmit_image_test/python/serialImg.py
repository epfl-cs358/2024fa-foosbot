import serial

class SerialImg(serial.Serial):

    """ Serial connection that reads an image. """

    def __init__(self,
                 port:     str = "/dev/ttyUSB0",
                 baudrate: int = 115200,
                 timeout:  int = 1):
        """
        Initialise a serial connection through `serial.Serial` with
        predefined arguments.

        Parameters:
            port     (str): The port to use for the connection
            baudrate (int): The baudrate to use for the connection
            tiemout  (int): The time to wait before ending the read
        """
        serial.Serial.__init__(self,
                               port     = port,
                               baudrate = baudrate,
                               timeout  = timeout,
                               xonxoff  = False,
                               rtscts   = False,
                               dsrdtr   = True)

    def getImg(self):
        """
        Returns the current image sent by the device on the serial
        connection

        Returns:
            Image received on serial port in Bytes.
        """

        img = b''

        end   = False
        inImg = False
        try:
            while not end:
                line = self.readline()

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
