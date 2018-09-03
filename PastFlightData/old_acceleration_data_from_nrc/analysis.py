
import os
import numpy as np
from matplotlib import pyplot as plt

fs = 100 # 100 Hz sample rate assumed

imgRoot = 'plots'
if not os.path.exists(imgRoot):
    os.mkdir(imgRoot)

files = os.listdir()
files.remove('analysis.py')
files.remove(imgRoot)

print("There are {0} files".format(len(files)))

for idx in range(len(files)):
    filename = files[idx]
    data = np.loadtxt(fname=filename, skiprows=1)
    data = data * 1 / 100 * 2.54 * 12 # Convert feet to metres
    time = np.arange(0, data.shape[0]) / 100
    
    plt.plot(time, data)
    plt.title('Data for {0}'.format(filename))
    plt.xlabel('Time (s)')
    plt.ylabel('Acceleration (m/s^2)')
    plt.savefig(imgRoot + '/' + filename + '.png')
    plt.close()
    
print("Their plots have been places in the {0} directory".format(imgRoot))