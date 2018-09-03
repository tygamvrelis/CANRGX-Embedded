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
    def __init__(self,data_path):
        self.data_path = data_path
        if(self.load_data() == True):
            self.process_data()
            self.visualize_data()


    def load_data_legacy(self,data_path):
        # Load the data, and then erase empty data
        self.tic_record = np.load(data_path + "/tic.npy")
        self.imu_record = np.load(data_path + "/imu.npy")
        self.pwr_record = np.load(data_path + "/pwr.npy")
        self.tmp_record = np.load(data_path + "/tmp.npy")
        self.syt_record = np.load(data_path + "/syt.npy")

        if np.sum(self.syt_record == -1) > 0:
            n = np.sum(self.syt_record != -1)
            self.tic_record = self.tic_record[:n]
            self.imu_record = self.imu_record[:n]
            self.pwr_record = self.pwr_record[:n]
            self.tmp_record = self.tmp_record[:n]
            self.syt_record = self.syt_record[:n]

            np.save(data_path + "/tic.npy", self.tic_record)
            np.save(data_path + "/imu.npy", self.imu_record)
            np.save(data_path + "/pwr.npy", self.pwr_record)
            np.save(data_path + "/tmp.npy", self.tmp_record)
            np.save(data_path + "/syt.npy", self.syt_record)


    def load_data(self):
        try:
            self.h5_file = h5py.File(self.data_path + '.hdf5', 'r')
            self.tic_record = self.h5_file["tic"]
            self.imu_record = self.h5_file["imu"]
            self.pwr_record = (np.array(self.h5_file["pwr"]).astype(np.dtype('<h')))
            self.tmp_record = (np.array(self.h5_file["tmp"]).astype(np.float32) / 4096.0 * 3.3 - 0.4) / 0.0195
            self.syt_record = self.h5_file["syt"]
            return True
        except:
            return False

        
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
        plt.savefig(self.data_path + name + '.png', dpi=300)
        plt.close()
        

    def visualize_data(self):
        # Set image width and height
        size_x = 32
        size_y = 2
        
        # Adjust data sets so that they don't include weird values at the start
        # and end
        s = 3  
        m = -1000
        time = self.data_time[s:m]

        fig1 = plt.figure()
        fig1.set_size_inches(size_x, size_y)
        plt.plot(time, self.acc_nrm[s:m],'k',label='norm')
        plt.plot(time, self.acc_vec[s:m],label='vec')
        plt.axhline(y=0, color='k', linestyle='--', linewidth = 0.5)
        plt.axhline(y=0.981, color='k', linestyle='--', linewidth = 0.5)
        plt.axhline(y=3.13, color='k', linestyle='--', linewidth = 0.5)
        plt.ylabel('Accelerometer reading (m/s^2)')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(['norm','x','y','z'],loc='best')
        self.save_vis('Accelerometer')

        fig2 = plt.figure()
        fig2.set_size_inches(size_x, size_y)
        plt.plot(time, self.mag_nrm[s:m],'k',label='norm')
        plt.plot(time, self.mag_vec[s:m,0],'b',label='X')
        plt.plot(time, self.mag_vec[s:m,1],'g',label='Y')
        plt.plot(time, self.mag_vec[s:m,2],'r',label='Z')
        plt.axhline(y=0, color='k', linestyle='--', linewidth = 0.5)
        plt.ylabel('Magnetometer reading (G)')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(loc='best')
        self.save_vis('Magnetometer')

        fig3 = plt.figure()
        fig3.set_size_inches(size_x, size_y)
        plt.plot(time, self.pwr_record[s:m,0])
        plt.plot(time, self.pwr_record[s:m,1])
        plt.plot(time, self.pwr_record[s:m,2])
        plt.plot(time, self.pwr_record[s:m,3])
        plt.ylabel('Power Setting')
        plt.xlabel('MCU Tick Time (millisec)')
        plt.legend(['Bottom mag', 'Top mag', 'Top TEC', 'Bot TEC'], loc='best')
        self.save_vis('Power')

        fig4 = plt.figure()
        fig4.set_size_inches(size_x, size_y)
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
        
        fig5, ax5 = plt.subplots()
        ax5_control = ax5.twinx()
        fig5.set_size_inches(size_x, size_y)
        ax5.plot(time, self.acc_nrm[s:m], color='k')
        ax5_control.plot(time, self.pwr_record[s:m,0], color='r')
        ax5_control.plot(time, self.pwr_record[s:m,1], color='g')
        ax5_control.plot(time, self.pwr_record[s:m,2], color='b')
        ax5_control.plot(time, self.pwr_record[s:m,3], color='m')
        ax5.axhline(y=0, color='k', linestyle='--', linewidth = 0.5)
        ax5.axhline(y=0.981, color='k', linestyle='--', linewidth = 0.5)
        ax5.axhline(y=3.13, color='k', linestyle='--', linewidth = 0.5)
        ax5.set_ylabel('Acceleration norm (m/s^2)')
        ax5.set_xlabel('MCU Tick Time (millisec)')
        ax5_control.set_ylabel('Control signals')
        ax5.legend(['norm'], loc='best')
        ax5_control.legend(['Bottom mag', 'Top mag', 'Top TEC', 'Bot TEC'], loc='best')
        self.save_vis('accel_and_control_overlay')
        
        fig6, ax6 = plt.subplots()
        ax6_temp = ax6.twinx()
        fig6.set_size_inches(size_x, size_y)
        ax6.plot(time, self.acc_nrm[s:m],'k')
        ax6_temp.plot(time, self.tmp_record[s:m,2], color='r')
        ax6_temp.plot(time, self.tmp_record[s:m,0], color='g')
        ax6.axhline(y=0, color='k', linestyle='--', linewidth = 0.5)
        ax6.axhline(y=0.981, color='k', linestyle='--', linewidth = 0.5)
        ax6.axhline(y=3.13, color='k', linestyle='--', linewidth = 0.5)
        ax6.set_ylabel('Acceleration norm (m/s^2)')
        ax6.set_xlabel('MCU Tick Time (millisec)')
        ax6_temp.set_ylabel('Temperature (deg Celsius)')
        ax6.legend(['norm'], loc='best')
        ax6_temp.legend(['Temp 2A', 'Temp 1A'], loc='best')
        self.save_vis('accel_and_temperature_overlay')
        
        fig7, ax7 = plt.subplots()
        ax7_mag = ax7.twinx()
        fig7.set_size_inches(size_x, size_y)
        ax7.plot(time, self.acc_nrm[s:m],'k')
        ax7_mag.plot(time, self.mag_nrm[s:m],'m')
        ax7_mag.plot(time, self.mag_vec[s:m,0],'b')
        ax7_mag.plot(time, self.mag_vec[s:m,1],'g')
        ax7_mag.plot(time, self.mag_vec[s:m,2],'r')
        ax7.axhline(y=0, color='k', linestyle='--', linewidth = 0.5)
        ax7.axhline(y=0.981, color='k', linestyle='--', linewidth = 0.5)
        ax7.axhline(y=3.13, color='k', linestyle='--', linewidth = 0.5)
        ax7.set_ylabel('Acceleration norm (m/s^2)')
        ax7.set_xlabel('MCU Tick Time (millisec)')
        ax7_mag.set_ylabel('Magnetometer (G)')
        ax7.legend(['acc norm'], loc='best')
        ax7_mag.legend(['mag norm', 'mag X', 'mag Y', 'mag Z'], loc='best')
        self.save_vis('accel_and_mag_overlay')


if __name__ == '__main__':
    data_root=input("supply data root (top-level directory which contains subdirectories for each logging session):")
    #data_root = 'D:\\Users\\Tyler\\Documents\\tyler\\School\\University of Toronto\\CAN-RGX\CANRGX-Embedded\\Flight_data\\Flight_1_August_30_2018\\zero_g_only'
    
    files = os.listdir(data_root)
    
    for dir in files:
        path = data_root + 2 * os.sep + dir + 2 * os.sep
        print(path)
        vis=canrgx_data_visualizer(path)