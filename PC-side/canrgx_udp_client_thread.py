import sys,time

import asyncio,socket
import numpy as np
import h5py

from PyQt5 import QtCore

class CANRGXUDPProtocol(asyncio.DatagramProtocol):

    def __init__(self, loop, data_fn=None):
        
        self.initialize_data_storage(data_fn)

        self.loop = loop
        self.transport = None
        self.index=0

    def initialize_data_storage(self, data_fn):
        if data_fn is None:
            data_fn = 'CANRGX_data\\udp_data_' + \
                time.strftime('%Y_%m_%d_%H_%M_%S') + '.hdf5'
        self.data_fn = data_fn
        self.h5_file = h5py.File(self.data_fn, 'w')

        self.gps_time_ds=self.h5_file.create_dataset(
            'GPS_time',(1000,1),'>i',chunks=True,maxshape=(None,1))
        self.mode_info_ds=self.h5_file.create_dataset(
            'mode_info', (1000, 2), '>i', chunks=True, maxshape=(None, 2))
        self.attitude_info_ds = self.h5_file.create_dataset(
            'attitude_info', (1000, 6), '>f', chunks=True, maxshape=(None, 6))
        self.earth_info_ds = self.h5_file.create_dataset(
            'earth_info', (1000, 3), '>d', chunks=True, maxshape=(None, 3))
        self.linear_motion_ds = self.h5_file.create_dataset(
            'linear_motion', (1000, 6), '>f', chunks=True, maxshape=(None, 6))


    def connection_made(self, transport):
        self.transport = transport
        print('Connection Made')

    def check_ds_size(self):
        for ds in [self.gps_time_ds, self.mode_info_ds, self.attitude_info_ds, self.earth_info_ds, self.linear_motion_ds]:
            if self.index +250 >= ds.shape[0]:
                ds.resize(self.index+1000,0)
                print('Resized')
        

    def datagram_received(self, data, addr):

        self.gps_time_ds[self.index,:] = np.frombuffer(data, '>d', 1, 0)
        self.mode_info_ds[self.index, :] = np.frombuffer(data, '>i', 2, 8)
        self.attitude_info_ds[self.index, :] = np.frombuffer(data, '>f', 6, 16)
        self.earth_info_ds[self.index, :] = np.frombuffer(data, '>d', 3, 40)
        self.linear_motion_ds[self.index, :] = np.frombuffer(data, '>f', 6, 64)

        self.index += 1
        if(self.index%200==0):
            self.check_ds_size()
            print("Received from: ", addr, "GPSTime",
                  self.gps_time_ds[self.index - 1, 0], "index", self.index)
        
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
        connect = self.loop.create_datagram_endpoint(lambda: CANRGXUDPProtocol(self.loop),  local_addr=(bind_ip,bind_port))
        self.transport, self.protocol = self.loop.run_until_complete(connect)
        self.loop.run_forever()
        self.transport.close()
        self.loop.close()


if __name__ == '__main__':
    qApp = QtCore.QCoreApplication(sys.argv)
    thread = CANRGXUDPClientThread()
    QtCore.QTimer.singleShot(0, thread.start)
    # QtCore.QTimer.singleShot(1, ccCamThread.timer.start)
    
    QtCore.QTimer.singleShot(100000, thread.hint_quit)

    thread.finished.connect(qApp.quit)
    sys.exit(qApp.exec_())
