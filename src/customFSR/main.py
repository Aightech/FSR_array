import serial
import struct
import time
import numpy as np

# Set the correct serial port and baud rate
SERIAL_PORT = '/dev/ttyACM0'  # Change to your Arduino port
BAUD_RATE = 115200

# Define the size of the data array (2 bytes * 16 * 16)
DATA_SIZE = 2 * 16 * 16

def read_fsr_data(serial_connection):
    # Send a command to Arduino to start the FSR scan
    serial_connection.write(b'\x01')  # This sends a single byte command

    # Wait a little to let Arduino process the data
    time.sleep(0.05)

    # Read the FSR data from the Arduino
    fsr_data_bytes = serial_connection.read(DATA_SIZE)

    # Unpack the data into an array of integers (unsigned short = 2 bytes)
    fsr_data = struct.unpack('H' * (DATA_SIZE // 2), fsr_data_bytes)

    return fsr_data

def main():
    # Open serial connection to Arduino
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        time.sleep(2)  # Wait for the serial connection to initialize

        while True:
            # Read FSR data from Arduino
            fsr_data = read_fsr_data(ser)
            arr = np.array(fsr_data)
            arr = arr.reshape((16, 16))
            
            print(arr)

            # Add a small delay between each request
            time.sleep(0.1)

if __name__ == "__main__":
    main()

