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

class canrgx_log_files(QtCore.QObject):


    update_data = QtCore.pyqtSignal(np.ndarray, np.ndarray, np.ndarray, np.ndarray)
    def __init__(self,data_root,max_n=500000,parent=None):
        super(canrgx_log_files,self).__init__(parent)

        self.data_root=data_root
        self.max_n=max_n

        self.tic_record = np.lib.format.open_memmap(self.data_root + "tic.npy", dtype=np.uint32,  mode='w+', shape=(self.max_n  ,2))
        #MCU time, PC time
        self.imu_record = np.lib.format.open_memmap(self.data_root + "imu.npy", dtype=np.float32, mode='w+', shape=(self.max_n  ,6))
        #Accelerometer and Magnometer data, 
        self.pwr_record = np.lib.format.open_memmap(self.data_root + "pwr.npy", dtype=np.float32, mode='w+', shape=(self.max_n  ,4))
        #Power of Magnets and TECs
        self.tmp_record = np.lib.format.open_memmap(self.data_root + "tmp.npy", dtype=np.float32, mode='w+', shape=(self.max_n  ,6))
        #Temperature Sensor
        self.syt_record = np.lib.format.open_memmap(self.data_root + "syt.npy", dtype=np.float64, mode='w+', shape=(self.max_n  ,1))

        self.syt_record [:]=-1 #Initialize to -1, so we know what are the valid data.
        self.i=0

        self.half_type = np.dtype('<H')
        self.int_type = np.dtype('<I')
        self.float_type = np.dtype('<f')

    
    def __enter__(self):
        return self

    def decode_data(self, raw_bytes):
        #self.tic_record [self.i,0] = struct.unpack('<H',raw_bytes[ 0: 2])[0] # header
        #self.tic_record [self.i,1] = struct.unpack('<I',raw_bytes[ 2: 6])[0] # mcu time tic
        #
        #self.imu_record [self.i,0] = struct.unpack('<f',raw_bytes[ 6:10])[0] # acc_x
        #self.imu_record [self.i,1] = struct.unpack('<f',raw_bytes[10:14])[0] # acc_y
        #self.imu_record [self.i,2] = struct.unpack('<f',raw_bytes[14:18])[0] # acc_z
        #self.imu_record [self.i,3] = struct.unpack('<f',raw_bytes[18:22])[0] # mag_x
        #self.imu_record [self.i,4] = struct.unpack('<f',raw_bytes[22:26])[0] # mag_y
        #self.imu_record [self.i,5] = struct.unpack('<f',raw_bytes[26:30])[0] # mag_z
        #
        #self.pwr_record [self.i,0] = struct.unpack('<H',raw_bytes[30:32])[0] # mag_1_duty_cycle
        #self.pwr_record [self.i,1] = struct.unpack('<H',raw_bytes[32:34])[0] # mag_2_duty_cycle
        #self.pwr_record [self.i,2] = struct.unpack('<H',raw_bytes[34:36])[0] # tec_1_duty_cycle
        #self.pwr_record [self.i,3] = struct.unpack('<H',raw_bytes[36:38])[0] # tec_2_duty_cycle
        #
        #self.tmp_record [self.i,0] = struct.unpack('<H',raw_bytes[38:40])[0] # temperature_1
        #self.tmp_record [self.i,1] = struct.unpack('<H',raw_bytes[40:42])[0] # temperature_2
        #self.tmp_record [self.i,2] = struct.unpack('<H',raw_bytes[42:44])[0] # temperature_3
        #self.tmp_record [self.i,3] = struct.unpack('<H',raw_bytes[44:46])[0] # temperature_4
        #self.tmp_record [self.i,4] = struct.unpack('<H',raw_bytes[46:48])[0] # temperature_5
        #self.tmp_record [self.i,5] = struct.unpack('<H',raw_bytes[48:50])[0] # temperature_6
        
        self.syt_record [self.i,0] = time.time() #System time stamp
        
        #Or, use np.frombuffer()
        self.tic_record [self.i,0] = np.frombuffer(raw_bytes,self.half_type,1,0)
        self.tic_record [self.i,1] = np.frombuffer(raw_bytes,self.int_type,1,2)
        self.imu_record [self.i,:] = np.frombuffer(raw_bytes,self.float_type,6,6)
        self.pwr_record [self.i,:] = np.frombuffer(raw_bytes,self.half_type,4,30)
        self.tmp_record [self.i,:] = np.frombuffer(raw_bytes,self.half_type,6,38)
        
        
        header=self.tic_record [self.i,0] # Return header
        
        self.i+=1 # increment count
        
        # Every so often, write results to terminal as sanity check, etc
        if self.i%100==0:
            self.sanity_check()
        
        # Every so often, we want to make sure our data is written to disk
        if self.i%1000==0:
            self.flush()
        # Close to full, create warning, should start the process
        if self.i>self.max_n*0.8 and self.i%1000==0:
            print("OVF WARNING")
        #print('*')
        
        return header

    def sanity_check(self):
        #Pring data as sanity check
        print("%d, %f, %f" % (self.tic_record [self.i-1,1], self.imu_record [self.i-1,2], self.pwr_record [self.i-1,0]))
        tic_tmp=np.copy(self.tic_record[self.i-100:self.i,1])
        imu_tmp=np.copy(self.imu_record[self.i-100:self.i,:])
        pwr_tmp=np.copy(self.pwr_record[self.i-100:self.i,:])
        tmp_tmp=np.copy(self.tmp_record[self.i-100:self.i,:])
        #print(np.shape(tic_tmp))
        #print(np.shape(imu_tmp))
        #print(np.shape(pwr_tmp))
        #print(np.shape(tmp_tmp))
        
        self.update_data.emit(tic_tmp,imu_tmp,pwr_tmp,tmp_tmp)

    def flush(self):
        #Flush data to disk
        self.tic_record.flush()
        self.imu_record.flush()
        self.pwr_record.flush()
        self.tmp_record.flush()
        self.syt_record.flush()
        #os.fsync() # if we really wanna make sure the writes persist, this will flush the disk buffer. Slow though
    def close(self):
        del self.tic_record
        del self.imu_record
        del self.pwr_record
        del self.tmp_record
        del self.syt_record
        print("Log %s properly closed"%(self.data_root))

    def __exit__(self, type, value, tb):
        self.close()

