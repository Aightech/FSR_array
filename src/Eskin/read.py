import numpy as np
import serial
import sys
import time 
import numpy as np

#connect to the arduino
if len(sys.argv) < 3:
    print("Usage:")
    print("\t" + sys.argv[0] + " arduino_port n_array")
    print("Ex:")
    print("\t" + sys.argv[0] + " COM5 16")
    exit()

#connect to the arduino
with serial.Serial(sys.argv[1], 500000, timeout=3) as arduino:
    time.sleep(1)
    #flush the serial port
    arduino.reset_input_buffer()
    arduino.reset_output_buffer()
    
    mode = 0b11
    size = 16
    cmd = (mode << 6) | size
    cmd_bytes = cmd.to_bytes(1, 'big')
    len = 2*4 + (4*4*3+16*16*2)*2
    dt_acc=0
    dt_fsr=0
    acc = np.zeros((16, 3))
    fsr = np.zeros((16, 16, 2))
    while(True):
        arduino.write(cmd_bytes)
        d = arduino.read(len)
        prev = dt_acc
        dt_acc = int.from_bytes(d[0:4], "little")
        dt_fsr = int.from_bytes(d[4:8], "little")
        acc = np.frombuffer(d[8:8 + 16*6], dtype=np.int16).reshape(16, 3)
        for i in range(16):
            print(f"{acc[i, 2]}\t", end="")
        print()
        fsr = np.frombuffer(d[8 + 16*6:], dtype=np.uint16).reshape(16, 2, 16)
        
        
        
        