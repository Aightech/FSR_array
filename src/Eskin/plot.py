import sys
import time
import numpy as np
import serial
from PyQt5 import QtCore, QtWidgets
import pyqtgraph.opengl as gl

class SerialReader(QtCore.QThread):
    data_received = QtCore.pyqtSignal(np.ndarray, np.ndarray)
    
    def __init__(self, port, parent=None):
        super(SerialReader, self).__init__(parent)
        self.port = port
        self.running = True
        try:
            self.ser = serial.Serial(port, 500000, timeout=3)
            time.sleep(1)
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
        except Exception as e:
            print("Serial connection error:", e)
            self.running = False

    def run(self):
        mode = 0b11
        size = 16
        cmd = (mode << 6) | size
        cmd_bytes = cmd.to_bytes(1, 'big')
        data_len = 8 + 96 + 1024  # Total data length (in bytes)
        while self.running:
            try:
                self.ser.write(cmd_bytes)
                d = self.ser.read(data_len)
                if len(d) != data_len:
                    continue
                # dt values (unused here)
                _ = int.from_bytes(d[0:4], "little")
                _ = int.from_bytes(d[4:8], "little")
                # 16 accelerometer readings, each with 3 values (int16)
                acc = np.frombuffer(d[8:8 + 16*6], dtype=np.int16).reshape(16, 3)
                # FSR data: 16 readings, 2 layers, each with 16 values (uint16)
                fsr = np.frombuffer(d[8 + 16*6:], dtype=np.uint16).reshape(16, 2, 16)
                self.data_received.emit(acc, fsr)
            except Exception as e:
                print("Serial read error:", e)
                self.running = False

    def stop(self):
        self.running = False
        self.wait()
        if self.ser.is_open:
            self.ser.close()

class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, serial_port, parent=None):
        super(MainWindow, self).__init__(parent)
        self.setWindowTitle("Real-time 3D Plot")
        self.widget = gl.GLViewWidget()
        self.setCentralWidget(self.widget)
        self.widget.opts['distance'] = 40
        
        # 1D arrays for FSR surfaces (required by GLSurfacePlotItem).
        self.xgrid = np.linspace(0, 15, 16)
        self.ygrid = np.linspace(0, 15, 16)
        
        # Create FSR surfaces for two layers (16x16 grid each).
        self.surface_fsr0 = gl.GLSurfacePlotItem(x=self.xgrid, y=self.ygrid, z=np.zeros((16, 16)),
                                                 shader='shaded', color=(1, 0, 0, 0.5))
        self.surface_fsr0.translate(-8, -8, 0)
        self.widget.addItem(self.surface_fsr0)
        
        self.surface_fsr1 = gl.GLSurfacePlotItem(x=self.xgrid, y=self.ygrid, z=np.zeros((16, 16)),
                                                 shader='shaded', color=(0, 1, 0, 0.5))
        self.surface_fsr1.translate(-8, -8, 0)
        self.widget.addItem(self.surface_fsr1)
        
        # Create a scatter plot for the accelerometer data (16 points).
        # Each dot's position (x, y, z) will represent its acceleration.
        self.scatter_acc = gl.GLScatterPlotItem(pos=np.zeros((16, 3)), size=10, color=(0, 0, 1, 1))
        self.widget.addItem(self.scatter_acc)
        
        # Start the serial reading thread.
        self.serial_thread = SerialReader(serial_port)
        self.serial_thread.data_received.connect(self.update_data)
        self.serial_thread.start()
        
    def update_data(self, acc, fsr):
        # Update FSR surfaces.
        # Assume fsr[:, 0, :] corresponds to layer 0 and fsr[:, 1, :] to layer 1.
        fsr0 = fsr[:, 0, :].astype(np.float32)/100.+4
        fsr1 = fsr[:, 1, :].astype(np.float32)/100.
        self.surface_fsr0.setData(z=fsr0)
        self.surface_fsr1.setData(z=fsr1)
        
        # Update the accelerometer scatter plot.
        # Use the accelerometer readings directly as (x, y, z) positions.
        acc_float = acc.astype(np.float32)/16384.

        lookupArr = [0, 7, 8, 9, 5,  6, 15, 10, 4, 2, 12, 11, 3, 1, 14, 13]
        acc_float = acc_float[lookupArr]
        # add grid offset to the x and y axis
        for ix in range(4):
            for iy in range(4):
                acc_float[ix*4+iy, 0] = acc_float[ix*4+iy, 0] + ix*4 -8
                acc_float[ix*4+iy, 1] = acc_float[ix*4+iy, 1] + iy*4 -8
                acc_float[ix*4+iy, 2] = acc_float[ix*4+iy, 2] + 6

        self.scatter_acc.setData(pos=acc_float)
        
    def closeEvent(self, event):
        self.serial_thread.stop()
        event.accept()

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage:")
        print("\t{} arduino_port n_array".format(sys.argv[0]))
        print("Ex:")
        print("\t{} COM5 16".format(sys.argv[0]))
        sys.exit(1)
        
    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow(sys.argv[1])
    window.show()
    sys.exit(app.exec_())
