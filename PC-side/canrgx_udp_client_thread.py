import sys

import asyncio,socket
import numpy as np

from PyQt5 import QtCore

class EchoClientProtocol(asyncio.DatagramProtocol):
    def __init__(self, loop):
        self.loop = loop
        self.transport = None
        self.count=0

    def connection_made(self, transport):
        self.transport = transport
        self.count = 0
        print('Connection Made')

    def datagram_received(self, data, addr):
        print("Received from: ", addr)

        self.count += 1
        
        double_type = np.dtype('>d')
        float_type = np.dtype('>f')
        int_type = np.dtype('>i')

        GPSTime = np.frombuffer(data, double_type, 1, 0)
        ModeInfo = np.frombuffer(data, int_type, 2, 8)
        AttitudeInfo = np.frombuffer(data, float_type, 6, 16)
        EarthInfo = np.frombuffer(data, double_type, 3, 40)
        LinearMotion = np.frombuffer(data, float_type, 6, 64)

        print("GPSTime", GPSTime, "count", self.count)
        #print("ModeInfo",ModeInfo)
        #print("AttitudeInfo",AttitudeInfo)
        #print("EarthInfo", EarthInfo)
        #print("LinearMotion", LinearMotion)

        

        #print("Close the socket")
        #self.transport.close()

    def error_received(self, exc):
        print('Error received:', exc)

    def connection_lost(self, exc):
        print("Socket closed, stop the event loop")
        #loop = asyncio.get_event_loop()
        if exc is None:
            print('Normal Termination')
        else:
            print('Error receivd:', exc)
        self.loop.stop()


class CANRGXUDPClientThread(QtCore.QThread):

    #def __init__(self, parent=None, graphic_slot=None):
    #    super().__init__(parent)

    def hint_quit(self):
        print('Hinted quit')
        if self.loop is None or self.transport is None:
            print("Event loop or UDP transport unavailable yet.")
            return False
        else:
            self.loop.call_soon_threadsafe(self.transport.abort)
    def run(self):

        print("Starting UDP Receiving Client")

        bind_ip = '127.0.0.1' # All addresses
        bind_port = 5124

        self.loop = asyncio.new_event_loop()
        connect = self.loop.create_datagram_endpoint(lambda: EchoClientProtocol(self.loop),  local_addr=(bind_ip,bind_port))
        self.transport, self.protocol = self.loop.run_until_complete(connect)
        self.loop.run_forever()
        self.transport.close()
        self.loop.close()


if __name__ == '__main__':
    qApp = QtCore.QCoreApplication(sys.argv)
    thread = CANRGXUDPClientThread()
    QtCore.QTimer.singleShot(0, thread.start)
    # QtCore.QTimer.singleShot(1, ccCamThread.timer.start)
    
    QtCore.QTimer.singleShot(2000, thread.hint_quit)

    thread.finished.connect(qApp.quit)
    sys.exit(qApp.exec_())
