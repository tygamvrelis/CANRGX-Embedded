# -*- coding: utf-8 -*-
"""
Created on Fri Oct 27 10:34:19 2017

@author: linhz

Edited by Tyler
"""

import serial
import serial.tools.list_ports
import time
import os
import numpy as np
import struct
from datetime import datetime

def decode_func(raw_bytes):
    header=struct.unpack('<H',raw_bytes[0:2])[0]
    time_stamp=struct.unpack('<I',raw_bytes[2:6])[0]
    acc_x=struct.unpack('<f',raw_bytes[6:10])[0]
    acc_y=struct.unpack('<f',raw_bytes[10:14])[0]
    acc_z=struct.unpack('<f',raw_bytes[14:18])[0]
    mag_x=struct.unpack('<f',raw_bytes[18:22])[0]
    mag_y=struct.unpack('<f',raw_bytes[22:26])[0]
    mag_z=struct.unpack('<f',raw_bytes[26:30])[0]
    mag_1_duty_cycle=struct.unpack('<H',raw_bytes[30:32])[0]
    mag_2_duty_cycle=struct.unpack('<H',raw_bytes[32:34])[0]
    tec_1_duty_cycle=struct.unpack('<H',raw_bytes[34:36])[0]
    tec_2_duty_cycle=struct.unpack('<H',raw_bytes[36:38])[0]
    temperature_1=struct.unpack('<H',raw_bytes[38:40])[0]
    temperature_2=struct.unpack('<H',raw_bytes[40:42])[0]
    temperature_3=struct.unpack('<H',raw_bytes[42:44])[0]
    temperature_4=struct.unpack('<H',raw_bytes[44:46])[0]
    temperature_5=struct.unpack('<H',raw_bytes[46:48])[0]
    temperature_6=struct.unpack('<H',raw_bytes[48:50])[0]
    
    return list([header, time_stamp, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, \
                mag_1_duty_cycle, mag_2_duty_cycle, tec_1_duty_cycle, \
                tec_2_duty_cycle, temperature_1, temperature_2, temperature_3, \
                temperature_4, temperature_5, temperature_6])
    
def receive_stream_loop():
    num_frame_shifts = 0
    num_receptions = 0

    while(ser.isOpen()):
        try:
            while(ser.in_waiting<50):
                pass
                # time.sleep(0.001)
            raw_data=ser.read(50)
            
            # Log raw data
            # f.write(str(raw_data))
            # f.write("\n")
            
            # Log unpacked data
            l = decode_func(raw_data)
            f.write(datetime.now().strftime('%H.%M.%S.%f') + " ")
            for item in l:
                f.write(str(item) + "\t")
            f.write("\n")
            
            # Every so often, we want to make sure our data is written to disk
            num_receptions = num_receptions + 1
            if(num_receptions % 1000 == 0):
                f.flush() # flushes writes to the disk buffer
                #os.fsync() # if we really wanna make sure the writes persist, this will flush the disk buffer. Slow though
            
            # Every so often, write results to terminal as sanity check, etc
            if(num_receptions % 100 == 0):
                print("%d, %f, %f" % (l[1], l[4], l[7]))
                
            header=struct.unpack('<H',raw_data[0:2])[0]
            if(header != 65535):
                # Did not receive expected header "0xFF 0xFF"
                num_frame_shifts = num_frame_shifts + 1
        except:
            ser.close()
    return num_frame_shifts
            
def printAndLogStringFromSerial(userMsg=""):
    msg = ser.readline().decode("utf-8")[:-1] # Don't log the newline
    theString = datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg + msg
    print(theString)
    f.write(theString)
    
def logString(userMsg):
    print(datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg)

def sendToMCU(msg):
    ser.write(bytes(msg.encode()))

if __name__ == "__main__":
    logString("Starting PC-side application")
    
    data_root = 'CANRGX_data\\' + time.strftime('%Y_%m_%d_%H_%M_%S')+'\\'
    if not os.path.exists(data_root):
        os.makedirs(data_root)
    logString("Log created at " + str(os.getcwd()) + '\\' + data_root)
    
    
    with serial.Serial('COM3',230400,timeout=100) as ser:
        logString("Opened port " + ser.name)
        with open(data_root + time.strftime('%Y_%m_%d_%H_%M_%S') + ".txt", 'w') as f:
            # Wait for microcontroller to come on and send its startup message
            printAndLogStringFromSerial("MCU sent: ")
            sendToMCU('A') # ACK
            
            # Wait for microcontroller to send its MPU9250 initialization status
            printAndLogStringFromSerial("MCU sent: ")
            sendToMCU('A') # ACK
            
            # Write current time to microcontroller and wait for it to send the
            # time back after a short delay. Log this time, as it is the time
            # the microcontroller is starting the scheduler.
            # One experiment showed there is a (310434-309425)/2 = 504.5 
            # microsecond delay when sending the time. This may vary a bit, and
            # so should ideally be done on-the-fly.
            sendToMCU(datetime.now().strftime('%H.%M.%S.%f'))
            printAndLogStringFromSerial("MCU starting scheduler. Echoed: ")
            
            # Log data in a loop
            num_frame_shifts = receive_stream_loop()
            
            # Once MCU is unplugged, we write out all remaining data and close the file
            f.write("Data collection terminated. Number of frame shifts: %d" % num_frame_shifts)
            f.close()
    
# f = open("C:\\Users\\Admin\\CANRGX_data\\2018_06_04_00_51_54\\2018_06_04_00_51_54.txt", 'r')
# f.seek(0)
# for line in f:
#     if "nan" in line:
#         print(line)