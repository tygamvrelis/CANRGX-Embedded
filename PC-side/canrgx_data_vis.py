# -*- coding: utf-8 -*-
"""
CANRAGX Data Visualization
Clean up the stored data and plot them

Created on Sat Jun  9 15:54:56 2018

@author: linhz
"""

import numpy as np
import matplotlib.pyplot as plt
import h5py
import os

class canrgx_data_visualizer():
    """ A class supporting the visualization of data"""
    def __init__(self,data_root):
        self.data_root = data_root
        self.load_data()
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


    def load_data(self):
        self.h5_file = h5py.File(self.data_root + '.hdf5', 'r')
        self.tic_record = self.h5_file["tic"]
        self.imu_record = self.h5_file["imu"]
        self.pwr_record = self.h5_file["pwr"]
        self.tmp_record = (np.array(self.h5_file["tmp"]).astype(np.float32) / 4096.0 * 3.3 - 0.4) / 0.0195
        self.syt_record = self.h5_file["syt"]

        
    def process_data(self):
        self.syt_record-=self.syt_record[0] #Shift time to 0
        #self.data_time=(self.tic_record[:,1]-self.tic_record[0,1])*0.001
        self.data_time=self.tic_record[:,1]
        #self.valid_idx=(self.tic_record[])
        self.acc_vec=self.imu_record[:,0:3]
        self.acc_nrm=np.linalg.norm(self.acc_vec,ord=2,axis=1)
        self.mag_vec=self.imu_record[:,3:6]
        self.mag_nrm=np.linalg.norm(self.mag_vec,ord=2,axis=1)


    def save_vis(self, name):
        plt.savefig(os.path.join(data_root, name + '.png'))
        plt.close()
        

    def visualize_data(self):
        s = 3  
        m = -1000
        time = self.data_time[s:m]

        fig1 = plt.figure()
        fig1.set_size_inches(16, 9)
        plt.plot(time, self.acc_nrm[s:m],'k',label='norm')
        plt.plot(time, self.acc_vec[s:m],label='vec')
        plt.ylabel('Accelerometer reading (m/s^2)')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(['norm','x','y','z'],loc='best')
        self.save_vis('Accelerometer')

        fig2 = plt.figure()
        fig2.set_size_inches(16, 9)
        plt.plot(time, self.mag_nrm[s:m],'k',label='norm')
        plt.plot(time, self.mag_vec[s:m,0],'b',label='X')
        plt.plot(time, self.mag_vec[s:m,1],'g',label='Y')
        plt.plot(time, self.mag_vec[s:m,2],'r',label='Z')
        plt.ylabel('Magnetometer reading (G)')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(loc='best')
        self.save_vis('Magnetometer')

        fig3 = plt.figure()
        fig3.set_size_inches(16, 9)
        plt.plot(time, self.pwr_record[s:m,0],label='Mag A')
        plt.plot(time, self.pwr_record[s:m,1],label='Mag B')
        plt.plot(time, self.pwr_record[s:m,2],label='TEC A')
        plt.plot(time, self.pwr_record[s:m,3],label='TEC B')
        plt.ylabel('Power Setting')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(loc='best')
        self.save_vis('Power')

        fig4 = plt.figure()
        fig4.set_size_inches(16, 9)
        plt.plot(time, self.tmp_record[s:m,0],label='Temp 1A')
        plt.plot(time, self.tmp_record[s:m,1],label='Temp 1B')
        plt.plot(time, self.tmp_record[s:m,2],label='Temp 2A')
        plt.plot(time, self.tmp_record[s:m,3],label='Temp 2B')
        plt.plot(time, self.tmp_record[s:m,4],label='Temp 3A')
        plt.plot(time, self.tmp_record[s:m,5],label='Temp 3B')
        plt.ylabel('Temperature (deg Celsius)')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(loc='best')
        self.save_vis('Temperature')


if __name__ == '__main__':
    data_root=input("supply data root:")
    #data_root = 'D:\\Users\\Tyler\\Documents\\tyler\\School\\University of Toronto\\CAN-RGX\CANRGX-Embedded\\PastFlightData\\Flight_1_August_30_2018\\2018_08_30_14_20_18\\'

    vis=canrgx_data_visualizer(data_root)
