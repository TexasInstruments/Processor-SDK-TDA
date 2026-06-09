import os
import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

col0 = "green"
col1 = "blue"
col2 = "red"
col3 = "orange"
col4 = "purple"

pBinFileName = "binaries/test_case_%d.bin"
pGoldenBinFileName = "binaries/golden_points%d.bin"
testcase_num = int(sys.argv[1])
f_name = pBinFileName % (testcase_num)
f_size = os.path.getsize(f_name)
print(f_size)
data = np.fromfile(f_name, dtype = np.int16)
print(data.size)
data = data.reshape(128,128)

f1_name = pGoldenBinFileName % (testcase_num)
f1_size = os.path.getsize(f1_name)
print(f1_size)
data1 = np.fromfile(f1_name, dtype = np.int32)
print(data1.size)
if data1.size == 0:
    print("No data in {}".format(f1_name))
else:
    data1 = data1.reshape((data1.size//2,2))
x0 = []
y0 = []

x1 = []
y1 = []

x2 = []
y2 = []

x3 = []
y3 = []

x4 = []
y4 = []

for tx in range(0, 128):
    for rx in range(0, 128):
        if(int(data[rx][tx]) == 0):
            x0.append(tx)
            y0.append(rx)
        if(int(data[rx][tx]) == 1):
            x1.append(tx)
            y1.append(rx)
        if(int(data[rx][tx]) == 2):
            x2.append(tx)
            y2.append(rx)
        if(int(data[rx][tx]) == 3):
            x3.append(tx)
            y3.append(rx)
        if(int(data[rx][tx]) == 4):
            x4.append(tx)
            y4.append(rx)


plt.xlim(0,128)
plt.ylim(0,128)

plt.scatter(x0,y0,c=col0)
plt.scatter(x1,y1,c=col1)
plt.scatter(x2,y2,c=col2)
plt.scatter(x3,y3,c=col3)

if data1.size:
    plt.scatter(data1[:, 1], data1[:, 0], color='#ffd700')
try:
    rxDll = int(sys.argv[2]) if len(sys.argv) > 2 else 127
    txDll = int(sys.argv[3]) if len(sys.argv) > 3 else 127
    rxDll1 = int(sys.argv[4]) if len(sys.argv) > 4 else 127
    txDll1 = int(sys.argv[5]) if len(sys.argv) > 5 else 127
except (ValueError, IndexError):
    print("Invalid or missing arguments. Please provide rxDLL and txDLL as command line arguments.")
    sys.exit(1)

plt.plot(txDll,rxDll,color='black', marker='o', linestyle='dashed', label='Old Tuning Algorithm Point')
plt.plot(txDll1,rxDll1,color='magenta', marker='*', linestyle='dashed', label='New Tuning Algorithm Point')
plt.legend(loc='upper right')
plt.savefig('Images/test_case_%d.png' % (testcase_num))

plt.show()