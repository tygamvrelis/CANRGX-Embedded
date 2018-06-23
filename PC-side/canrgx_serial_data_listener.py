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
import sys
from datetime import datetime
from canrgx_data_log_file import canrgx_log_files
from PyQt5 import QtCore
from PyQt5.QtCore import QObject, QTimer, QThread


class CANRGXSerialDataListener(QtCore.QObject):

    def __init__(self, parent=None):
        super(CANRGXSerialDataListener, self).__init__(parent)
        # self.initialize()
        self.update_slot=[]

    issue_encountered = QtCore.pyqtSignal()
    request_close = QtCore.pyqtSignal()
    initialized = QtCore.pyqtSignal()

    def initialize(self):

        self.issue_encountered.connect(self.cleanup)
        # First, make sure when any issue is encountered, go cleanup.

        print("Starting PC-side application")
        print("Worker Thread ID:", int(QThread.currentThreadId()))

        data_root = 'CANRGX_data\\' + time.strftime('%Y_%m_%d_%H_%M_%S') + '\\'
        if not os.path.exists(data_root):
            os.makedirs(data_root)

        self.file = open(data_root + time.strftime('%Y_%m_%d_%H_%M_%S') + ".txt", 'w')
        self.logString("Log created at " + str(os.getcwd()) + '\\' + data_root)
        self.canrgx_log = canrgx_log_files(data_root)

        self.ser = serial.Serial('COM3', 230400, timeout=100)
        self.logString("Opened port " + self.ser.name)
        # Wait for microcontroller to come on and send its startup message
        self.ser.flushOutput()
        self.ser.flushInput()
        try:
            self.printAndLogStringFromSerial("MCU sent: ")
        except UnicodeDecodeError as decode_err:
            print('Not valid MCU response, quit')
            return
        self.sendToMCU('A')  # ACK

        # Wait for microcontroller to send its MPU9250 initialization status
        self.printAndLogStringFromSerial("MCU sent: ")
        self.sendToMCU('A')  # ACK

        # Write current time to microcontroller and wait for it to send the
        # time back after a short delay. Log this time, as it is the time
        # the microcontroller is starting the scheduler.
        # One experiment showed there is a (310434-309425)/2 = 504.5
        # microsecond delay when sending the time. This may vary a bit, and
        # so should ideally be done on-the-fly.
        self.sendToMCU(datetime.now().strftime('%H.%M.%S.%f'))
        self.printAndLogStringFromSerial("MCU starting scheduler. Echoed: ")
        self.sendToMCU('A')  # ACK

        self.num_frame_shifts = 0
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.check_serial_buffer)
        # Create the timer that fires every 2ms to check the serial buffer.
        self.initialized.emit()

        print("Start Listening to MCU Data")

        self.timer.start(2)


    def close(self):
        self.cleanup()

    def cleanup(self):
        self.timer.stop()
        # Stop any further processing by disable the timer.
        # Once MCU is unplugged, we write out all remaining data and close the file
        self.file.write(
            "Data collection terminated. Number of frame shifts: %d" % self.num_frame_shifts)
        self.file.close()
        self.ser.close()
        self.canrgx_log.close()

        print("Resource for MCU data listener properly closed. Request closing of thread.")
        self.request_close.emit()

    def check_serial_buffer(self):
        try:
            if self.ser.in_waiting < 50:
                return
            raw_data = self.ser.read(50)
            # Log unpacked data
            header = self.canrgx_log.decode_data(raw_data)
            #f.write(datetime.now().strftime('%H.%M.%S.%f') + " ")
            # for item in l:
            #    f.write(str(item) + "\t")
            # f.write("\n")

            # header=struct.unpack('<H',raw_data[0:2])[0]
            if(header != 65535):
                # Did not receive expected header "0xFF 0xFF"
                self.num_frame_shifts += 1
                print('HERR')
        except Exception as e:
            print(e)
            # self.ser.close()
            print('Exception encounterred')
            self.issue_encountered.emit()

    def printAndLogStringFromSerial(self, userMsg=""):
        # Don't log the \0x00 out front
        msg = str(self.ser.readline().decode('ascii')[1:])
        theString = datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg + msg
        print(theString)
        self.file.write(theString)
        self.file.flush()

    def logString(self, userMsg):
        theString = datetime.now().strftime('%H.%M.%S.%f') + " " + userMsg
        print(theString)
        self.file.write(theString)
        self.file.flush()

    def sendToMCU(self, msg):
        self.ser.write(bytes(msg.encode()))


def show_initialization():
    print("Initialized!!!!")

if __name__ == '__main__':
    qApp = QtCore.QCoreApplication(sys.argv)
    thread = QThread()

    listener = CANRGXSerialDataListener()
    listener.moveToThread(thread)

    listener.request_close.connect(thread.quit)
    thread.started.connect(listener.initialize)
    thread.finished.connect(qApp.quit)
    listener.initialized.connect(show_initialization)

    thread.start()
    
    #stream = QtCore.QTextStream(sys.stdin, QtCore.QIODevice.ReadOnly)

    sys.exit(qApp.exec_())
