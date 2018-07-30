# -*- coding: utf-8 -*-
"""
Created on Wed Jul  5 16:18:28 2017

@author: linhz
"""


from __future__ import unicode_literals
import sys
import os
import click
# Make sure that we are using QT5
from PyQt5 import QtCore, QtWidgets, QtGui
from PyQt5.QtCore import QObject, QTimer, QThread

import numpy as np
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolBar
from canrgx_data_widget import CANRGXPlotCanvas
from canrgx_serial_data_listener import CANRGXSerialDataListener
progname = os.path.basename(sys.argv[0])
progversion = "0.1"


class CANRGXMainWindow(QtWidgets.QMainWindow):

    closing=QtCore.pyqtSignal()
    request_manual_start = QtCore.pyqtSignal(int)
    request_manual_stop = QtCore.pyqtSignal()
    request_manual_reset = QtCore.pyqtSignal()


    def __init__(self,ser_port=None):

        # Initialize the main application window and put various components on it.
        # Pretty routine code. Long, tiedious but trivial, just putting components in place.
        # Check documentation of pyqt5 or try qt-designer if you want to extend it.
        QtWidgets.QMainWindow.__init__(self)

        # self.camThread = CrossCorreCamera(
        #    cali=cali, background=background, t_img=t_img, n_img=n_img)

        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.setWindowTitle("CANRGX Team FAM (UofT EngSci) Real Time Data")

        self.main_menu = QtWidgets.QMenu('&Menu', self)
        self.main_menu.addAction(
            '&Quit', self.fileQuit, QtCore.Qt.CTRL + QtCore.Qt.Key_Q)
        #self.main_menu.addAction('&Acquire Background', self.acquire_background)
        self.menuBar().addMenu(self.main_menu)

        self.main_widget = QtWidgets.QWidget(self)

        self.mainLayout = QtWidgets.QHBoxLayout(self.main_widget)
        
        
        self.graphVLayout=QtWidgets.QVBoxLayout()
        self.dataCanvas = CANRGXPlotCanvas()
        self.graphVLayout.addWidget(self.dataCanvas)


        self.plotToolBar = NavigationToolBar(
            self.dataCanvas, self.main_widget)
        
        self.bottomHLayout = QtWidgets.QHBoxLayout()
        self.bottomHLayout.addWidget(self.plotToolBar)


        self.runNumberSpinBox = QtWidgets.QSpinBox(self.main_widget)
        self.runNumberSpinBox.setObjectName("runNumberSpinBox")
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.runNumberSpinBox.sizePolicy().hasHeightForWidth())
        self.runNumberSpinBox.setSizePolicy(sizePolicy)
        self.runNumberSpinBox.setAlignment(
            QtCore.Qt.AlignRight | QtCore.Qt.AlignTrailing | QtCore.Qt.AlignVCenter)
        self.runNumberSpinBox.setMinimum(1)
        self.runNumberSpinBox.setMaximum(4)
        self.runNumberSpinBox.setPrefix("S")
        self.bottomHLayout.addWidget(self.runNumberSpinBox)


        self.manualStartButton = QtWidgets.QPushButton(self.main_widget)
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.manualStartButton.sizePolicy().hasHeightForWidth())
        self.manualStartButton.setObjectName("manualStartButton")
        self.manualStartButton.setText( "Manual Start")
        self.bottomHLayout.addWidget(self.manualStartButton)
        self.manualStartButton.clicked.connect(self.manual_start_button_callback)

        self.manualStopButton = QtWidgets.QPushButton(self.main_widget)
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.manualStopButton.sizePolicy().hasHeightForWidth())
        self.manualStopButton.setObjectName("manualStopButton")
        self.manualStopButton.setText("Manual Stop")
        self.bottomHLayout.addWidget(self.manualStopButton)
        self.manualStopButton.clicked.connect(self.manual_stop_button_callback)

        self.manualResetButton = QtWidgets.QPushButton(self.main_widget)
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.manualResetButton.sizePolicy().hasHeightForWidth())
        self.manualResetButton.setObjectName("manualResetButton")
        self.manualResetButton.setText("Manual Reset")
        self.bottomHLayout.addWidget(self.manualResetButton)
        self.manualResetButton.clicked.connect(self.manual_reset_button_callback)


        self.errLabel = QtWidgets.QLabel(self.main_widget)
        sizePolicy = QtWidgets.QSizePolicy(
            QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(
            self.errLabel.sizePolicy().hasHeightForWidth())
        self.errLabel.setSizePolicy(sizePolicy)
        self.errLabel.setObjectName("ERR label")
        self.errLabel.setText("CLEAR")
        self.bottomHLayout.addWidget(self.errLabel)
        

        self.graphVLayout.addLayout(self.bottomHLayout)
        self.mainLayout.addLayout(self.graphVLayout)

        self.setupNumDisp();
        # main_layout.addLayout(self.rowAvgDataLayout)
        print("Setup Graphic")
        print("Main Thread ID:", int( QThread.currentThreadId()))

        # Qt timer and the working thread started by the main thread.

        self.log_thread = QtCore.QThread()
        #print("Log Thread", self.log_thread)
        if ser_port is None:
            self.listener = CANRGXSerialDataListener()
        else:
            self.listener = CANRGXSerialDataListener(ser_port=ser_port)

        self.listener.moveToThread(self.log_thread)

        self.listener.initialized.connect(self.hook_update)
        self.listener.request_close.connect(self.log_thread.quit)
        self.log_thread.started.connect(self.listener.initialize)
        self.closing.connect(self.listener.close)
        #self.log_thread.finished.connect(qApp.quit)
        self.listener.frame_error.connect(self.update_error)
        self.request_manual_start.connect(self.listener.execute_manual_start)
        self.request_manual_stop.connect(self.listener.execute_manual_stop)
        self.request_manual_reset.connect(self.listener.execute_manual_reset)


        self.log_thread.start()
        
        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        self.statusBar().showMessage("All hail matplotlib!", 2000)
        
    
    def hook_update(self):
        print("Hook Properly Connected")
        self.listener.canrgx_log.update_data.connect(
            self.dataCanvas.new_data_slot
        )
        self.listener.canrgx_log.update_data.connect(self.updateNumDisp)
        
    def update_error(self, set_error):
        if set_error:
            self.errLabel.setText("ERROR")
        else:
            self.errLabel.setText("CLEAR")
    
    def manual_start_button_callback(self,checked):
        self.request_manual_start.emit(int(self.runNumberSpinBox.value()))

    def manual_reset_button_callback(self,checked):
        print("Manual Reset")
        self.request_manual_reset.emit()

    
    def manual_stop_button_callback(self, checked):
        self.request_manual_stop.emit()


    def fileQuit(self):
        # Close action for the Quit button in menubar.
        self.close()

    def closeEvent(self, ce):
        print('closeEvent')
        # self.fileQuit()
        # self.camThread.external_quit()
        #print('ExternalQuit executed')
        # self.camThread.wait()
        self.closing.emit()
        print('Closing Emitted')
        #self.log_thread.quit()
        #
        print('Thread Exited')
        self.dataCanvas.close()
        # self.rowAvgDataCanvas.close()
        # Make sure various sub-components are closed before main widget exits.
        # Especially the working thread needs to be stopped.
        #self.log_thread.wait()
        super().closeEvent(ce)

    def about(self):
        # Show explanation for the About button in menubar.
        QtWidgets.QMessageBox.about(self, "About", """This is a simple program 
            for streaming experiment data from the micro-controller for the 
            CANRGX FAM team (UofT EngSci) experiment. It is written with numpy,
            scipy, matplotlib, and PyQt5, by Hanzhen Lin. """)

    def updateNumDisp( self, new_tic, new_imu, new_pwr, new_tmp):
        acc_norm = np.linalg.norm(new_imu[:, 0:3], ord=2, axis=1)
        mag_norm = np.linalg.norm(new_imu[:, 3:6], ord=2, axis=1)
        acc_z = new_imu[:, 2]

        acc_norm = np.nanmean(acc_norm)
        mag_norm = np.nanmean(mag_norm)
        acc_z = np.nanmean(acc_z)

        tmp = (new_tmp.astype(np.float64) / 4096.0 * 3.3 - 0.4) / 0.0195
        tmp = np.mean(tmp,axis=0)

        self.accNrmNum.display(float(acc_norm))
        self.magNrmNum.display(float(mag_norm))
        self.accZaxNum.display(float(acc_z))

        self.tmpANum.display(tmp[0])
        self.tmpBNum.display(tmp[1])
        self.tmpCNum.display(tmp[2])
        self.tmpDNum.display(tmp[3])
        self.tmpENum.display(tmp[4])
        self.tmpFNum.display(tmp[5])




    def setupNumDisp(self):
        self.numDispLayout = QtWidgets.QGridLayout()
        self.numDispLayout.setObjectName("numDispLayout")
        font = QtGui.QFont()
        font.setFamily("Agency FB")
        font.setPointSize(12)

        self.accNrmLabel = QtWidgets.QLabel(self.main_widget)
        self.accNrmLabel.setObjectName("accNrmLabel")
        self.accNrmLabel.setText("Acc. Norm:")
        self.accNrmLabel.setFont(font)
        self.accNrmLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.accNrmLabel, 1, 0, 1, 1)
        self.accNrmNum = QtWidgets.QLCDNumber(self.main_widget)
        self.accNrmNum.setSmallDecimalPoint(False)
        self.accNrmNum.display(1.23)
        self.accNrmNum.setDigitCount(5)
        self.accNrmNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.accNrmNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.accNrmNum.setMinimumHeight(32)
        self.accNrmNum.setObjectName("accNrmNum")
        self.numDispLayout.addWidget(self.accNrmNum, 1, 1, 1, 1)

        self.accZaxLabel = QtWidgets.QLabel(self.main_widget)
        self.accZaxLabel.setObjectName("accZaxLabel")
        self.accZaxLabel.setText("Acc. Z-ax:")
        self.accZaxLabel.setFont(font)
        self.accZaxLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.accZaxLabel, 2, 0, 1, 1)
        self.accZaxNum = QtWidgets.QLCDNumber(self.main_widget)
        self.accZaxNum.setSmallDecimalPoint(False)
        self.accZaxNum.display(1.23)
        self.accZaxNum.setDigitCount(5)
        self.accZaxNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.accZaxNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.accZaxNum.setMinimumHeight(32)
        self.accZaxNum.setObjectName("accZaxNum")
        self.numDispLayout.addWidget(self.accZaxNum, 2, 1, 1, 1)

        self.magNrmLabel = QtWidgets.QLabel(self.main_widget)
        self.magNrmLabel.setObjectName("magNrmLabel")
        self.magNrmLabel.setText("Mag. Norm:")
        self.magNrmLabel.setFont(font)
        self.magNrmLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.magNrmLabel, 3, 0, 1, 1)
        self.magNrmNum = QtWidgets.QLCDNumber(self.main_widget)
        self.magNrmNum.setSmallDecimalPoint(False)
        self.magNrmNum.display(1.23)
        self.magNrmNum.setDigitCount(5)
        self.magNrmNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.magNrmNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.magNrmNum.setMinimumHeight(32)
        self.magNrmNum.setObjectName("magNrmNum")
        self.numDispLayout.addWidget(self.magNrmNum, 3, 1, 1, 1)

        self.tmpALabel = QtWidgets.QLabel(self.main_widget)
        self.tmpALabel.setObjectName("tmpALabel")
        self.tmpALabel.setText("Temp 1A:")
        self.tmpALabel.setFont(font)
        self.tmpALabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.tmpALabel, 4, 0, 1, 1)
        self.tmpANum = QtWidgets.QLCDNumber(self.main_widget)
        self.tmpANum.setSmallDecimalPoint(False)
        self.tmpANum.display(1.23)
        self.tmpANum.setDigitCount(5)
        self.tmpANum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.tmpANum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.tmpANum.setMinimumHeight(32)
        self.tmpANum.setObjectName("tmpANum")
        self.numDispLayout.addWidget(self.tmpANum, 4, 1, 1, 1)

        self.tmpBLabel = QtWidgets.QLabel(self.main_widget)
        self.tmpBLabel.setObjectName("tmpBLabel")
        self.tmpBLabel.setText("Temp 1B:")
        self.tmpBLabel.setFont(font)
        self.tmpBLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.tmpBLabel, 5, 0, 1, 1)
        self.tmpBNum = QtWidgets.QLCDNumber(self.main_widget)
        self.tmpBNum.setSmallDecimalPoint(False)
        self.tmpBNum.display(1.23)
        self.tmpBNum.setDigitCount(5)
        self.tmpBNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.tmpBNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.tmpBNum.setMinimumHeight(32)
        self.tmpBNum.setObjectName("tmpBNum")
        self.numDispLayout.addWidget(self.tmpBNum, 5, 1, 1, 1)

        self.tmpCLabel = QtWidgets.QLabel(self.main_widget)
        self.tmpCLabel.setObjectName("tmpCLabel")
        self.tmpCLabel.setText("Temp 2A:")
        self.tmpCLabel.setFont(font)
        self.tmpCLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.tmpCLabel, 6, 0, 1, 1)
        self.tmpCNum = QtWidgets.QLCDNumber(self.main_widget)
        self.tmpCNum.setSmallDecimalPoint(False)
        self.tmpCNum.display(1.23)
        self.tmpCNum.setDigitCount(5)
        self.tmpCNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.tmpCNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.tmpCNum.setMinimumHeight(32)
        self.tmpCNum.setObjectName("tmpCNum")
        self.numDispLayout.addWidget(self.tmpCNum, 6, 1, 1, 1)

        self.tmpDLabel = QtWidgets.QLabel(self.main_widget)
        self.tmpDLabel.setObjectName("tmpDLabel")
        self.tmpDLabel.setText("Temp 2B:")
        self.tmpDLabel.setFont(font)
        self.tmpDLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.tmpDLabel, 7, 0, 1, 1)
        self.tmpDNum = QtWidgets.QLCDNumber(self.main_widget)
        self.tmpDNum.setSmallDecimalPoint(False)
        self.tmpDNum.display(1.23)
        self.tmpDNum.setDigitCount(5)
        self.tmpDNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.tmpDNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.tmpDNum.setMinimumHeight(32)
        self.tmpDNum.setObjectName("tmpDNum")
        self.numDispLayout.addWidget(self.tmpDNum, 7, 1, 1, 1)

        self.tmpELabel = QtWidgets.QLabel(self.main_widget)
        self.tmpELabel.setObjectName("tmpELabel")
        self.tmpELabel.setText("Temp 3A:")
        self.tmpELabel.setFont(font)
        self.tmpELabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.tmpELabel, 8, 0, 1, 1)
        self.tmpENum = QtWidgets.QLCDNumber(self.main_widget)
        self.tmpENum.setSmallDecimalPoint(False)
        self.tmpENum.display(1.23)
        self.tmpENum.setDigitCount(5)
        self.tmpENum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.tmpENum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.tmpENum.setMinimumHeight(32)
        self.tmpENum.setObjectName("tmpENum")
        self.numDispLayout.addWidget(self.tmpENum, 8, 1, 1, 1)

        self.tmpFLabel = QtWidgets.QLabel(self.main_widget)
        self.tmpFLabel.setObjectName("tmpFLabel")
        self.tmpFLabel.setText("Temp 3B:")
        self.tmpFLabel.setFont(font)
        self.tmpFLabel.setScaledContents(True)
        self.numDispLayout.addWidget(self.tmpFLabel, 9, 0, 1, 1)
        self.tmpFNum = QtWidgets.QLCDNumber(self.main_widget)
        self.tmpFNum.setSmallDecimalPoint(False)
        self.tmpFNum.display(1.23)
        self.tmpFNum.setDigitCount(5)
        self.tmpFNum.setSegmentStyle(QtWidgets.QLCDNumber.Flat)
        self.tmpFNum.setFrameShape(QtWidgets.QFrame.NoFrame)
        self.tmpFNum.setMinimumHeight(32)
        self.tmpFNum.setObjectName("tmpFNum")
        self.numDispLayout.addWidget(self.tmpFNum, 9, 1, 1, 1)

        self.mainLayout.addLayout(self.numDispLayout)

# Use python click
@click.command()
@click.option('--ser_port', default='COM12', help='Serial Port Name')
def main_gui(ser_port):
    # Main program runs from here. Standard Qt start procedure.
    qApp = QtWidgets.QApplication(sys.argv)
    qApp.setWindowIcon(QtGui.QIcon("FAM.jpg"))
    aw = CANRGXMainWindow(ser_port)
    aw.show()
    sys.exit(qApp.exec_())
    # qApp.exec_()


if __name__ == '__main__':
    main_gui()
'''
TODO:
More convenient setting of parameters - Moreorless done for commandline.
Graphic interface can be implemented (but involves more event handling and cleaning-up of existing code)
Potentially add interaction to some server through internet. 
'''
