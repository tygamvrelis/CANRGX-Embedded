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
        load_data(data_root)
        process_data()
        visualize_data()

    def load_data(data_root):
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

            np.save(data_root + "tic.npy", self.tic_record)
            np.save(data_root + "imu.npy", self.imu_record)
            np.save(data_root + "pwr.npy", self.pwr_record)
            np.save(data_root + "tmp.npy", self.tmp_record)
            np.save(data_root + "syt.npy", self.syt_record)

        
        
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
    
        axs[0].plot(self.data_time, self.acc_nrm,fmt='k',label='norm')
        axs[0].plot(self.data_time, self.acc_vec[:,0],fmt='b',label='X')
        axs[0].plot(self.data_time, self.acc_vec[:,1],fmt='g',label='Y')
        axs[0].plot(self.data_time, self.acc_vec[:,2],fmt='r',label='Z')
        axs[0].set_ylabel('Accelerometer reading (m/s^2)')
        axs[0].set_xlabel('MCU Tick Time (millisec)')
        axs[0].legend(loc='best')

        axs[1].plot(self.data_time, self.mag_nrm,fmt='k',label='norm')
        axs[1].plot(self.data_time, self.mag_vec[:,0],fmt='b',label='X')
        axs[1].plot(self.data_time, self.mag_vec[:,1],fmt='g',label='Y')
        axs[1].plot(self.data_time, self.mag_vec[:,2],fmt='r',label='Z')
        axs[1].set_ylabel('Magnetometer reading (m/s^2)')
        axs[1].set_xlabel('MCU Tick Time (millisec)')
        axs[1].legend(loc='best')


        axs[2].plot(self.data_time, self.pwr_record[:,0],label='Mag A')
        axs[2].plot(self.data_time, self.pwr_record[:,1],label='Mag B')
        axs[2].plot(self.data_time, self.pwr_record[:,2],label='TEC A')
        axs[2].plot(self.data_time, self.pwr_record[:,3],label='TEC B')
        axs[2].set_ylabel('Power Setting')
        axs[2].set_xlabel('MCU Tick Time (millisec)')
        axs[2].legend(loc='best')

        axs[3].plot(self.data_time, self.tmp_record[:,0],label='Temp A')
        axs[3].plot(self.data_time, self.tmp_record[:,1],label='Temp B')
        axs[3].plot(self.data_time, self.tmp_record[:,2],label='Temp C')
        axs[3].plot(self.data_time, self.tmp_record[:,3],label='Temp D')
        axs[3].plot(self.data_time, self.tmp_record[:,4],label='Temp E')
        axs[3].plot(self.data_time, self.tmp_record[:,5],label='Temp F')
        axs[3].set_ylabel('Power Setting')
        axs[3].set_xlabel('MCU Tick Time (millisec)')
        axs[3].legend(loc='best')


if __name__ == '__main__':
    data_root=input("supply data root:")

    vis=canrgx_data_visualizer(data_root)