# -*- coding: utf-8 -*-
"""
Created on Fri Oct 27 10:34:19 2017

@author: linhz
"""

import serial
import time
import os
import numpy as np
import struct

data_root = 'data/' + time.strftime('%Y_%m_%d_%H_%M_%S')+'/'
if not os.path.exists(data_root):
            os.makedirs(data_root)
global_start_time=time.time()
i=0
n=0
N=1000
decay_time=np.lib.format.open_memmap(data_root + time.strftime('%Y_%m_%d_%H_%M_%S')+"_decay_t.npy",
                                     dtype=np.int32, mode='w+', shape=(2, N))

def decode_func(raw_bytes):
    time_stamp=struct.unpack('<I',raw_bytes[2:6])
    acc_x=struct.unpack('<f',raw_bytes[6:10])
    acc_y=struct.unpack('<f',raw_bytes[10:14])
    acc_z=struct.unpack('<f',raw_bytes[14:18])
    
    mag_x=struct.unpack('<f',raw_bytes[18:22])
    mag_y=struct.unpack('<f',raw_bytes[22:26])
    mag_z=struct.unpack('<f',raw_bytes[26:30])

    print(time_stamp, acc_z, mag_z)

with serial.Serial('COM3',230400,timeout=10) as ser:
    print(ser.name)
    while(True):
        while(ser.in_waiting<50):
            #pass
            time.sleep(0.001)
        raw_data=ser.read(50)
        decode_func(raw_data)
