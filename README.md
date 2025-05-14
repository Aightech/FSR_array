# FSR_array

This project implements a electronic skin using a FSR accelerometers arrays. 

The electronic board uses two multiplexers and two 16-channel ADCs to read the FSRs. The multiplexers are used to select the row or column of the FSRs array. The ADCs are used to read the FSRs values.
A teensy 4.1 is used to control the multiplexers and ADCs and to send the data to the computer.
The teensy 4.1 is connected to the computer using a USB cable or ethernet cable.
There are two modes of operation:
- **Single mode**: In this mode, you can connect a FSR array with up to 32x32 FSRs.
- **Dual mode**: In this mode, you can connect two FSR arrays with up to 16x16 FSRs each.

The sampling is around 400KHz/channel. So for a 16x16 FSR array, the sampling rate is around 400KHz/256 = 1562.5Hz.


The project contains a C++/matlab/python code demonstrating how to communicate with the teensy 4.1 and read the FSRs values.

### Submodules dependencies
#### libraries
- [built_lsl](lib/built_lsl/README.md)
- [com_client](lib/com_client/README.md)



# Building source code

To build the project run:
```bash
cd FSR_array
mkdir build && cd build
cmake .. && make
```

# Demonstration app

When the project have been built, you can run:
```bash
./FSR_array -h
```
to get the demonstration app usage.

# Example
Open the ![main.cpp](cpp:src/main.cpp) file to get an example how to use the lib.

# Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.