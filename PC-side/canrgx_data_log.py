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
from canrgx_data_log_file import canrgx_log_files
    
def receive_stream_loop(canrgx_log):
    num_frame_shifts = 0
    num_receptions = 0
    print('Enter Stream Loop')
    while(ser.isOpen()):
        try:
            while(ser.in_waiting<50):
                time.sleep(0.002)
            #Do sleep so that we don't keep pulling and saturate CPU
            #The serial port has buffer underneath so it should not 
            # affect data integrity.
            raw_data=ser.read(50)
            
            # Log unpacked data
            header = canrgx_log.decode_data(raw_data)
            #f.write(datetime.now().strftime('%H.%M.%S.%f') + " ")
            #for item in l:
            #    f.write(str(item) + "\t")
            #f.write("\n")
            
            #header=struct.unpack('<H',raw_data[0:2])[0]
            if(header != 65535):
                # Did not receive expected header "0xFF 0xFF"
                num_frame_shifts = num_frame_shifts + 1
                print('HERR')
        except Exception as e:
            print(e)
            ser.close()
            print('SER Properly Closed')
            break
    return num_frame_shifts
    
def printAndLogStringFromSerial(file, userMsg=""):
    msg = str(ser.readline().decode('ascii')[1:]) # Don't log the \0x00 out front
    theString = datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg + msg
    print(theString)
    file.write(theString)
    file.flush()
    
def logString(userMsg, file):
    theString = datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg
    print(theString)
    file.write(theString)
    file.flush()

def sendToMCU(msg):
    ser.write(bytes(msg.encode()))

if __name__ == "__main__":
    print("Starting PC-side application")
    
    data_root = 'CANRGX_data\\' + time.strftime('%Y_%m_%d_%H_%M_%S')+'\\'
    if not os.path.exists(data_root):
        os.makedirs(data_root)

    with open(data_root + time.strftime('%Y_%m_%d_%H_%M_%S') + ".txt", 'w') as f:
        logString("Log created at " + str(os.getcwd()) + '\\' + data_root, f)
        with canrgx_log_files(data_root) as canrgx_log:
            with serial.Serial('COM3',230400,timeout=100) as ser:
                logString("Opened port " + ser.name, f)
                # Wait for microcontroller to come on and send its startup message
                ser.flushOutput()
                ser.flushInput()
                printAndLogStringFromSerial(f, "MCU sent: ")
                sendToMCU('A') # ACK
                
                # Wait for microcontroller to send its MPU9250 initialization status
                printAndLogStringFromSerial(f, "MCU sent: ")
                sendToMCU('A') # ACK
            
                
                # Write current time to microcontroller and wait for it to send the
                # time back after a short delay. Log this time, as it is the time
                # the microcontroller is starting the scheduler.
                # One experiment showed there is a (310434-309425)/2 = 504.5 
                # microsecond delay when sending the time. This may vary a bit, and
                # so should ideally be done on-the-fly.
                sendToMCU(datetime.now().strftime('%H.%M.%S.%f'))
                printAndLogStringFromSerial(f, "MCU starting scheduler. Echoed: ")
                sendToMCU('A') # ACK
                
                # Log data in a loop
                num_frame_shifts = receive_stream_loop(canrgx_log)
                
                # Once MCU is unplugged, we write out all remaining data and close the file
                f.write("Data collection terminated. Number of frame shifts: %d" % num_frame_shifts)
                f.close()