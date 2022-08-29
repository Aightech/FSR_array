import numpy as np
import serial
import sys
from scipy.io import savemat
import keyboard
import time 

#connect to the arduino
if len(sys.argv) < 2:
    print("Usage:")
    print("\t" + sys.argv[0] + " arduino_port")
    print("Ex:")
    print("\t" + sys.argv[0] + " COM5")
    exit()

#connect to the arduino
arduino = serial.Serial(sys.argv[1], 500000, timeout=3)
time.sleep(1)

d=b'\x00'
print("Press 'a' to quit ")

n=32
arr = np.zeros(n*n)
data = np.array(arr)
time = np.array([0, 0])
dt = [0, 0]
while(True):
    if keyboard.is_pressed('a'):
        break
    arduino.write(bytes([n]))
    #Send position to the arduino and print in the terminal
    prev_dt=dt[0]
    for i in range(2):
        d = arduino.read(4)
        dt[i] = int.from_bytes(d, "little")
    #print(str(dt[0]-prev_dt)+ " " + str(dt[1]-dt[0]))
    
    for i in range(n*n):
        d = arduino.read(2)
        arr[i] = int.from_bytes(d, "little")
    data = np.vstack ((data, arr))
    time = np.vstack((time, dt))
    #print(d)

dat = {'data':data, 'timestamp':time}
savemat('data.mat', dat)
