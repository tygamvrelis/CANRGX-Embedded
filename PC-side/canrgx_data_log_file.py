# -*- coding: utf-8 -*-
"""
CANRAGX Data binary utility.
Flush data received from serial to a binary file.

Created on Sat Jun  9 15:54:56 2018

@author: linhz
"""
import numpy as np
import struct
import time
from PyQt5 import QtCore
import h5py

class canrgx_log_files(QtCore.QObject):

    update_data = QtCore.pyqtSignal(
        np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    update_status = QtCore.pyqtSignal(np.uint8)
    # for ds in [self.gps_time_ds, self.mode_info_ds, self.attitude_info_ds, self.earth_info_ds, self.linear_motion_ds, self.sys_time_ds]:
    #         if self.index + 250 >= ds.shape[0]:
    #             ds.resize(self.index + 1000, 0)
    #             print('Resized')

    def __init__(self, data_root, max_n=500000, parent=None):
        super(canrgx_log_files, self).__init__(parent)

        self.data_root = data_root
        self.max_n = max_n

        '''
        self.tic_record = np.lib.format.open_memmap(
            self.data_root + "tic.npy", dtype=np.uint32,  mode='w+', shape=(self.max_n, 2))
        # MCU time, PC time
        self.imu_record = np.lib.format.open_memmap(
            self.data_root + "imu.npy", dtype=np.float32, mode='w+', shape=(self.max_n, 6))
        # Accelerometer and Magnometer data,
        self.pwr_record = np.lib.format.open_memmap(
            self.data_root + "pwr.npy", dtype=np.float32, mode='w+', shape=(self.max_n, 4))
        # Power of Magnets and TECs
        self.tmp_record = np.lib.format.open_memmap(
            self.data_root + "tmp.npy", dtype=np.float32, mode='w+', shape=(self.max_n, 6))
        # Temperature Sensor
        self.syt_record = np.lib.format.open_memmap(
            self.data_root + "syt.npy", dtype=np.float64, mode='w+', shape=(self.max_n, 1))
        '''

        self.h5_file = h5py.File(data_root + '.hdf5', 'w')

        self.tic_record = self.h5_file.create_dataset('tic', 
            (2000, 2), dtype=np.uint32, chunks=True, maxshape=(None, 2))
        # Header, MCU time
        self.imu_record = self.h5_file.create_dataset('imu',
            (2000, 6), dtype=np.float32, chunks=True, maxshape=(None, 6))
        # Accelerometer and Magnometer data,
        self.pwr_record = self.h5_file.create_dataset('pwr',
            (2000, 4), dtype=np.float32, chunks=True, maxshape=(None, 4))
        # Power of Magnets and TECs
        self.tmp_record = self.h5_file.create_dataset('tmp',
            (2000, 6), dtype=np.float32, chunks=True, maxshape=(None, 6))
        # Temperature Sensor
        self.sts_record = self.h5_file.create_dataset('sts',
            (2000, 1), dtype=np.uint8, chunks=True, maxshape=(None, 1))
        # Experiment status register
        self.syt_record = self.h5_file.create_dataset('syt',
            (2000, 1), dtype=np.float64, chunks=True, maxshape=(None, 1))
        # System time record

        self.h5_records=[self.tic_record,self.imu_record,self.pwr_record,self.tmp_record,self.sts_record,self.syt_record]

        # Initialize to -1, so we know what are the valid data.
        self.syt_record[:] = -1
        self.i = 0

        self.half_type = np.dtype('<H')
        self.signed_half_type = np.dtype('<h')
        self.int_type = np.dtype('<i')
        self.Uint_type = np.dtype('<I')
        self.float_type = np.dtype('<f')

    def __enter__(self):
        return self

    def decode_data(self, raw_bytes):
        self.syt_record[self.i, 0] = time.time()  # System time stamp
        
        # Resize buffer as needed
        if self.i % 500 == 25:
            self.check_ds_size()

        # Or, use np.frombuffer()
        self.tic_record[self.i, 0] = np.frombuffer(
            raw_bytes, self.half_type, 1, 0)
        self.tic_record[self.i, 1] = np.frombuffer(
            raw_bytes, self.Uint_type, 1, 2)
        self.imu_record[self.i, :] = np.frombuffer(
            raw_bytes, self.float_type, 6, 6)
        self.pwr_record[self.i, :] = np.frombuffer(
            raw_bytes, self.signed_half_type, 4, 30) / 100.0
        self.tmp_record[self.i, :] = np.frombuffer(
            raw_bytes, self.half_type, 6, 38)
        self.sts_record[self.i, 0] = np.frombuffer(
            raw_bytes, np.uint8, 1, 50)

        header = self.tic_record[self.i, 0]  # Return header

        self.i += 1  # increment count

        # Every so often, write results to terminal as sanity check, etc
        if self.i % 100 == 0:
            self.sanity_check()
        
        # Every so often, we want to make sure our data is written to disk
        if self.i % 200 == 10:    
            self.h5_file.flush()
        
        # Update the status every so often
        if self.i % 50 == 0:
            self.update_status.emit(self.sts_record[self.i - 1, 0])
            
        # Close to full, create warning, should start the process
        #if self.i > self.max_n * 0.8 and self.i % 1000 == 0:
        #    print("OVF WARNING")
        # print('*')

        return header

    def sanity_check(self):
        # Pring data as sanity check
        print("%d, %f, %f" % (
            self.tic_record[self.i - 1, 1], self.imu_record[self.i - 1, 2], self.pwr_record[self.i - 1, 0]))
        print ("Log Thread ID: ", int(QtCore.QThread.currentThreadId()))
        tic_tmp = np.copy(self.tic_record[self.i - 100:self.i, 1])
        imu_tmp = np.copy(self.imu_record[self.i - 100:self.i, :])
        pwr_tmp = np.copy(self.pwr_record[self.i - 100:self.i, :])
        tmp_tmp = np.copy(self.tmp_record[self.i - 100:self.i, :])
        # print(np.shape(tic_tmp))
        # print(np.shape(imu_tmp))
        # print(np.shape(pwr_tmp))
        # print(np.shape(tmp_tmp))

        self.update_data.emit(tic_tmp, imu_tmp, pwr_tmp, tmp_tmp)

    def check_ds_size(self):
        for ds in self.h5_records:
            if self.i + 500 >= ds.shape[0]:
                ds.resize(self.i+1000,0)
        print('Resized data buffer')

    def close(self):
        del self.tic_record
        del self.imu_record
        del self.pwr_record
        del self.tmp_record
        del self.syt_record
        print("Log %s properly closed" % (self.data_root))

    def __exit__(self, type, value, tb):
        self.close()
