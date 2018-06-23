# -*- coding: utf-8 -*-
"""
Created on Wed Jul  5 16:18:28 2017

@author: linhz
"""


from __future__ import unicode_literals
import sys
import os
import argparse
# Make sure that we are using QT5
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtCore import QObject, QTimer, QThread

import numpy as np
from numpy import arange, sin, pi, random
import peakutils
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolBar
from canrgx_data_widget import CANRGXPlotCanvas
from canrgx_serial_data_listener import CANRGXSerialDataListener
progname = os.path.basename(sys.argv[0])
progversion = "0.1"


class CANRGXMainWindow(QtWidgets.QMainWindow):

    closing=QtCore.pyqtSignal()

    def __init__(self):

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

        main_layout = QtWidgets.QHBoxLayout(self.main_widget)

        self.cameraImageLayout = QtWidgets.QVBoxLayout()
        self.cameraImageCanvas = CANRGXPlotCanvas()
        self.cameraImageLayout.addWidget(self.cameraImageCanvas)
        self.cameraImageToolBar = NavigationToolBar(
            self.cameraImageCanvas, self.main_widget)
        self.cameraImageLayout.addWidget(self.cameraImageToolBar)

        main_layout.addLayout(self.cameraImageLayout)
        # main_layout.addLayout(self.rowAvgDataLayout)
        print("Setup Graphic")
        print("Main Thread ID:", int( QThread.currentThreadId()))

        # Qt timer and the working thread started by the main thread.

        self.log_thread = QtCore.QThread()
        #print("Log Thread", self.log_thread)
        self.listener = CANRGXSerialDataListener()
        
        self.listener.moveToThread(self.log_thread)

        self.listener.initialized.connect(self.hook_update)
        self.listener.request_close.connect(self.log_thread.quit)
        self.log_thread.started.connect(self.listener.initialize)
        self.closing.connect(self.listener.close)
        #self.log_thread.finished.connect(qApp.quit)
        self.log_thread.start()
        
        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        self.statusBar().showMessage("All hail matplotlib!", 2000)
        
    
    def hook_update(self):
        print("Hook Properly Connected")
        self.listener.canrgx_log.update_data.connect(
            self.cameraImageCanvas.new_data_slot
        )
        
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
        self.cameraImageCanvas.close()
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


# Use python click
if __name__ == '__main__':
    # Main program runs from here. Standard Qt start procedure.
    qApp = QtWidgets.QApplication(sys.argv)
    aw = CANRGXMainWindow()
    aw.show()
    sys.exit(qApp.exec_())
    # qApp.exec_()
'''
TODO:
More convenient setting of parameters - Moreorless done for commandline.
Graphic interface can be implemented (but involves more event handling and cleaning-up of existing code)
Potentially add interaction to some server through internet. 
'''
