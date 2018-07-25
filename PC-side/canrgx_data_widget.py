# -*- coding: utf-8 -*-
"""
The canvas widgets that the data will be drawn on.
There are two types of widgets:
CameraImageCanvas shows the 2D image.
RowAvgDataCanvas shows the data of each column after averageing across rows.

@author: linhz
"""

import matplotlib
# Make sure that we are using QT5
matplotlib.use('Qt5Agg')
from PyQt5 import QtCore, QtWidgets
import numpy as np
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
# Import necessary files


class CANRGXPlotCanvas(FigureCanvas):
    """Inhereit the FigureCanvas from matplotlib (Qt5 backend version), which is ultimately a QWdiget"""

    def __init__(self, parent=None, width=16, height=12, dpi=160, data_n=300):
        # Initialize various components and flags for the widget
        
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        #fig.subplots_adjust(left = 0.0, bottom = 0.0, right = 1.0, top = 1.0, wspace = 0.0, hspace = 0.0)
        #self.fig.set_tight_layout(True)
        
        self.acc_ax = self.fig.add_subplot(2, 2, 1)
        self.mag_ax = self.fig.add_subplot(2, 2, 2)
        self.pwr_ax = self.fig.add_subplot(2, 2, 3)
        self.tmp_ax = self.fig.add_subplot(2, 2, 4)
        
        self.axes=[self.acc_ax,self.mag_ax,self.pwr_ax,self.tmp_ax]

        FigureCanvas.__init__(self, self.fig)
        self.setParent(parent)

        assert data_n>100
        self.data_n=data_n
        self.init_data_buffer()
        self.show_initial_figure()

        
        FigureCanvas.setSizePolicy(
            self, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)

    def init_data_buffer(self):
        self.data_time=np.empty(self.data_n)
        
        self.acc_vec=np.empty((self.data_n,3))
        self.acc_nrm=np.empty(self.data_n)
        self.mag_vec=np.empty((self.data_n,3))
        self.mag_nrm=np.empty(self.data_n)

        self.pwr_buf=np.empty((self.data_n,4))
        self.tmp_buf=np.empty((self.data_n,6))
    
    def show_initial_figure(self):
        ''' Wraps the update_figure function'''
        self.acc_plt_n=self.acc_ax.plot(self.data_time, self.acc_nrm,'k',label='norm')[0]
        self.acc_plt_v=self.acc_ax.plot(self.data_time, self.acc_vec, label='vec')
        self.acc_ax.set_ylabel('Accelerometer reading (m/s^2)')
        self.acc_ax.set_xlabel('MCU Tick Time (millisec)')
        self.acc_ax.set_ylim((-10,10))
        self.acc_ax.legend(['norm','x','y','z'],loc='upper right',fontsize='xx-small',frameon=False)

        self.mag_plt_n=self.mag_ax.plot(self.data_time, self.mag_nrm,'k',label='norm')[0]
        self.mag_plt_v=self.mag_ax.plot(self.data_time, self.mag_vec, label='vec')
        self.mag_ax.set_ylabel('Magnetometer reading (G)')
        self.mag_ax.set_xlabel('MCU Tick Time (millisec)')
        self.mag_ax.set_ylim((-60,60))
        self.mag_ax.legend(['norm','x','y','z'],loc='upper right',fontsize='xx-small',frameon=False)


        self.pwr_plts=self.pwr_ax.plot(self.data_time, self.pwr_buf)
        self.pwr_ax.set_ylabel('Power Setting')
        self.pwr_ax.set_xlabel('MCU Tick Time (millisec)')
        self.pwr_ax.set_ylim((-105,105))
        self.pwr_ax.legend(['Mag A' , 'Mag B' , 'TEC A', 'TEC B'],loc='upper right',fontsize='xx-small',frameon=False)

        self.tmp_plts=self.tmp_ax.plot(self.data_time, self.tmp_buf)
        self.tmp_ax.set_ylabel('Temperature Sensor')
        self.tmp_ax.set_xlabel('MCU Tick Time (millisec)')
        self.tmp_ax.set_ylim((5,70))
        self.tmp_ax.legend(['1A','1B','2A','2B','3A','3B'],loc='upper right',fontsize='xx-small',frameon=False)
        self.fig.subplots_adjust(left=0.075, bottom=0.1, right=0.975, top=0.975, wspace=0.15, hspace=0.25)

    def resizeEvent(self, event):
        #self.fig.tight_layout()
        #self.fig.tight_layout()
        #self.fig.tight_layout()
        
        # emit our resize events
        super().resizeEvent(event)

    def new_data_slot(self, new_tic, new_imu, new_pwr, new_tmp):
        roll_step=-np.shape(new_tic)[0]
        #print("roll_step:",roll_step)
        self.data_time=np.roll(self.data_time,roll_step)
        self.acc_vec=np.roll(self.acc_vec,roll_step,axis=0)
        self.acc_nrm=np.roll(self.acc_nrm,roll_step)
        self.mag_vec=np.roll(self.mag_vec,roll_step,axis=0)
        self.mag_nrm=np.roll(self.mag_nrm,roll_step)
        self.pwr_buf=np.roll(self.pwr_buf,roll_step,axis=0)
        self.tmp_buf=np.roll(self.tmp_buf,roll_step,axis=0)
        
        self.data_time[roll_step:]=new_tic
        self.acc_vec[roll_step:]=new_imu[:,0:3]
        self.acc_nrm[roll_step:]=np.linalg.norm(self.acc_vec[roll_step:],ord=2,axis=1)
        self.mag_vec[roll_step:]=new_imu[:,3:6]
        self.mag_nrm[roll_step:]=np.linalg.norm(self.mag_vec[roll_step:],ord=2,axis=1)
        self.pwr_buf[roll_step:]=new_pwr
        self.tmp_buf[roll_step:] =(new_tmp.astype(np.float32) / 4096.0 * 3.3 - 0.4) / 0.0195
        #self.tmp_buf[roll_step:] = new_tmp

        self.update_figure()

    def update_figure(self):
        self.acc_plt_n.set_data(self.data_time,self.acc_nrm)
        self.mag_plt_n.set_data(self.data_time,self.mag_nrm)
        
        for i in range(3):
            self.acc_plt_v[i].set_data(self.data_time,self.acc_vec[:,i])
            self.mag_plt_v[i].set_data(self.data_time,self.mag_vec[:,i])
        for i in range(4):
            self.pwr_plts[i].set_data(self.data_time,self.pwr_buf[:,i])
        for i in range(6):
            self.tmp_plts[i].set_data(self.data_time,self.tmp_buf[:,i])

        for ax in self.axes:
            ax.relim()
            ax.autoscale(axis='x')
        self.draw()

    def re_initialize(self):
        '''In case the entire figure needs to be rescaled, call this to re-intialize'''
        #self.graphic_initialized = False
        for ax in self.axes:
            ax.cla()
