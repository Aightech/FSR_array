import numpy as np
import serial
import sys
from scipy.io import savemat
import keyboard


#connect to the arduino
if len(sys.argv) < 2:
    print("Usage:")
    print("\t" + sys.argv[0] + " arduino_port")
    print("Ex:")
    print("\t" + sys.argv[0] + " COM5")
    exit()

#connect to the arduino
arduino = serial.Serial(sys.argv[1], 500000, timeout=1)

d=b'\x00'
while( int.from_bytes(d,"big") != 0xaa):
    d = arduino.read()
    print(int.from_bytes(d,"big"))

arduino.write(b'\xbb')
print("Press 'a' to quit ")


arr = np.zeros(16*16)
data = np.array(arr)
while(True):
    if keyboard.is_pressed('a'):
        break
    arduino.write(b'\xbb')
    #Send position to the arduino and print in the terminal
    for i in range(256):
        d = arduino.read(2)
        arr[i] = int.from_bytes(d, "little")
    data = np.vstack ((data, arr))
        #print(d)

dat = {'data':data}
savemat('data.mat', dat)
