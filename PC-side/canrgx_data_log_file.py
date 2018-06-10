# -*- coding: utf-8 -*-
"""
CANRAGX Data binary utility.
Flush data received from serial to a binary file.

Created on Sat Jun  9 15:54:56 2018

@author: linhz
"""
import numpy as np
import time

class canrgx_log_files:
    
    def __init__(self,data_root,max_n=500000):
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

    def decode_data(self, raw_bytes):
        self.tic_record [i,0] = struct.unpack('<H',raw_bytes[ 0: 2])[0] # header
        self.tic_record [i,1] = struct.unpack('<I',raw_bytes[ 2: 6])[0] # mcu time tic

        self.imu_record [i,0] = struct.unpack('<f',raw_bytes[ 6:10])[0] # acc_x
        self.imu_record [i,1] = struct.unpack('<f',raw_bytes[10:14])[0] # acc_y
        self.imu_record [i,2] = struct.unpack('<f',raw_bytes[14:18])[0] # acc_z
        self.imu_record [i,3] = struct.unpack('<f',raw_bytes[18:22])[0] # mag_x
        self.imu_record [i,4] = struct.unpack('<f',raw_bytes[22:26])[0] # mag_y
        self.imu_record [i,5] = struct.unpack('<f',raw_bytes[26:30])[0] # mag_z
        
        self.pwr_record [i,0] = struct.unpack('<H',raw_bytes[30:32])[0] # mag_1_duty_cycle
        self.pwr_record [i,1] = struct.unpack('<H',raw_bytes[32:34])[0] # mag_2_duty_cycle
        self.pwr_record [i,2] = struct.unpack('<H',raw_bytes[34:36])[0] # tec_1_duty_cycle
        self.pwr_record [i,3] = struct.unpack('<H',raw_bytes[36:38])[0] # tec_2_duty_cycle
        
        self.tmp_record [i,0] = struct.unpack('<H',raw_bytes[38:40])[0] # temperature_1
        self.tmp_record [i,1] = struct.unpack('<H',raw_bytes[40:42])[0] # temperature_2
        self.tmp_record [i,2] = struct.unpack('<H',raw_bytes[42:44])[0] # temperature_3
        self.tmp_record [i,3] = struct.unpack('<H',raw_bytes[44:46])[0] # temperature_4
        self.tmp_record [i,4] = struct.unpack('<H',raw_bytes[46:48])[0] # temperature_5
        self.tmp_record [i,5] = struct.unpack('<H',raw_bytes[48:50])[0] # temperature_6
        
        self.syt_record [i,0] = time.time() #System time stamp

        self.i+=1 # increment count

        # Every so often, write results to terminal as sanity check, etc
        if self.i%100=0:
            self.sanity_check()
        
        # Every so often, we want to make sure our data is written to disk
        if self.i%1000=0:
            self.flush()
        # Close to full, create warning, should start the process
        if self.i>self.n_max*0.8 && i%1000==0:
            print("OVF WARNING")
        return self.tic_record [i,0] # Return header

    def sanity_check(self):
        #Pring data as sanity check
        print("%d, %f, %f" % (self.imu_record [i,0], self.imu_record [i,2], self.pwr_record [i,0]))


    def flush(self):
        #Flush data to disk
        self.tic_record.flush()
        self.imu_record.flush()
        self.pwr_record.flush()
        self.tmp_record.flush()
        self.syt_record.flush()
        #os.fsync() # if we really wanna make sure the writes persist, this will flush the disk buffer. Slow though

