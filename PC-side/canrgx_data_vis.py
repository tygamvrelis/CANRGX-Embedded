# -*- coding: utf-8 -*-
"""
CANRAGX Data Visualization
Clean up the stored data and plot them

Created on Sat Jun  9 15:54:56 2018

@author: linhz
"""

import numpy as np
import matplotlib.pyplot as plt

class canrgx_data_visualizer():
    """ A class supporting the visualization of data"""
    def __init__(self,data_root):
        self.load_data(data_root)
        self.process_data()
        self.visualize_data()

    def load_data_legacy(self,data_root):
        # Load the data, and then erase empty data
        self.tic_record = np.load(data_root + "/tic.npy")
        self.imu_record = np.load(data_root + "/imu.npy")
        self.pwr_record = np.load(data_root + "/pwr.npy")
        self.tmp_record = np.load(data_root + "/tmp.npy")
        self.syt_record = np.load(data_root + "/syt.npy")

        if np.sum(self.syt_record == -1) > 0:
            n = np.sum(self.syt_record != -1)
            self.tic_record = self.tic_record[:n]
            self.imu_record = self.imu_record[:n]
            self.pwr_record = self.pwr_record[:n]
            self.tmp_record = self.tmp_record[:n]
            self.syt_record = self.syt_record[:n]

            np.save(data_root + "/tic.npy", self.tic_record)
            np.save(data_root + "/imu.npy", self.imu_record)
            np.save(data_root + "/pwr.npy", self.pwr_record)
            np.save(data_root + "/tmp.npy", self.tmp_record)
            np.save(data_root + "/syt.npy", self.syt_record)

    def load_data(self,data_file):

        self.h5_file = h5py.File(data_file + '.hdf5', 'r')
        self.tic_record = self.h5_file["tic"]
        self.imu_record = self.h5_file["imu"]
        self.pwr_record = self.h5_file["pwr"]
        self.tmp_record = self.h5_file["tmp"]
        self.syt_record = self.h5_file["syt"]

        
    def process_data(self):
        self.syt_record-=self.syt_record[0] #Shift time to 0
        self.data_time=(self.tic_record[:,1]-self.tic_record[0,1])*0.001
        #self.valid_idx=(self.tic_record[])
        self.acc_vec=self.imu_record[:,0:3]
        self.acc_nrm=np.linalg.norm(self.acc_vec,ord=2,axis=1)
        self.mag_vec=self.imu_record[:,3:6]
        self.mag_nrm=np.linalg.norm(self.mag_vec,ord=2,axis=1)




    def visualize_data(self):
        fig, axs = plt.subplots(2, 2)
        print(axs)
        axs[0][0].plot(self.data_time, self.acc_nrm,'k',label='norm')
        #axs[0][0].plot(self.data_time, self.acc_vec[:,0],'b',label='X')
        #axs[0][0].plot(self.data_time, self.acc_vec[:,1],'g',label='Y')
        #axs[0][0].plot(self.data_time, self.acc_vec[:,2],'r',label='Z')
        axs[0][0].plot(self.data_time, self.acc_vec,label='vec')
        axs[0][0].set_ylabel('Accelerometer reading (m/s^2)')
        axs[0][0].set_xlabel('MCU Tick Time (millisec)')
        axs[0][0].legend(['norm','x','y','z'],loc='best')

        axs[0][1].plot(self.data_time, self.mag_nrm,'k',label='norm')
        axs[0][1].plot(self.data_time, self.mag_vec[:,0],'b',label='X')
        axs[0][1].plot(self.data_time, self.mag_vec[:,1],'g',label='Y')
        axs[0][1].plot(self.data_time, self.mag_vec[:,2],'r',label='Z')
        axs[0][1].set_ylabel('Magnetometer reading (G)')
        axs[0][1].set_xlabel('MCU Tick Time (millisec)')
        axs[0][1].legend(loc='best')


        axs[1][0].plot(self.data_time, self.pwr_record[:,0],label='Mag A')
        axs[1][0].plot(self.data_time, self.pwr_record[:,1],label='Mag B')
        axs[1][0].plot(self.data_time, self.pwr_record[:,2],label='TEC A')
        axs[1][0].plot(self.data_time, self.pwr_record[:,3],label='TEC B')
        axs[1][0].set_ylabel('Power Setting')
        axs[1][0].set_xlabel('MCU Tick Time (millisec)')
        axs[1][0].legend(loc='best')

        axs[1][1].plot(self.data_time, self.tmp_record[:,0],label='Temp 1A')
        axs[1][1].plot(self.data_time, self.tmp_record[:,1],label='Temp 1B')
        axs[1][1].plot(self.data_time, self.tmp_record[:,2],label='Temp 2A')
        axs[1][1].plot(self.data_time, self.tmp_record[:,3],label='Temp 2B')
        axs[1][1].plot(self.data_time, self.tmp_record[:,4],label='Temp 3A')
        axs[1][1].plot(self.data_time, self.tmp_record[:,5],label='Temp 3B')
        axs[1][1].set_ylabel('Power Setting')
        axs[1][1].set_xlabel('MCU Tick Time (millisec)')
        axs[1][1].legend(loc='best')


if __name__ == '__main__':
    data_root=input("supply data root:")

    vis=canrgx_data_visualizer(data_root)
