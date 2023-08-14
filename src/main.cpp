#include <iostream>
#include <lsl_cpp.h>
#include <serial_client.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp_client.hpp>
#include <unistd.h>

#include <chrono>

int
main(int argc, char **argv)
{
    Communication::Client *device;

    if(0)
    {
        std::string path = "/dev/ttyACM0";
        device = new Communication::Serial();
        try
        {
            std::cout << "Com Interface:" << std::endl;
            device->open_connection(path.c_str(), 115200, O_RDWR | O_NOCTTY);
        }
        catch(std::string msg)
        {
            std::cout << "ERROR: " << msg << "\n";
            return 0;
        }
    }
    else
    {
        std::string path = "192.168.127.254";
        device = new Communication::TCP(2);
        try
        {
            device->open_connection(path.c_str(), 5000, 2);
        }
        catch(std::string msg)
        {
            std::cout << "ERROR: " << msg << "\n";
            return 0;
        }
    }

    usleep(2000000);

    int n_width = 16;
    int n_layer = 2;
    char cmd = n_width + (n_layer - 1) * 0x80;
    int nb_channel = n_width * n_width * n_layer;
    uint16_t data[nb_channel];
    uint32_t timestamps[2];

    lsl::stream_info info_sample("FSR", "sample", nb_channel, 0, lsl::cf_int16);
    lsl::stream_outlet outlet_sample(info_sample);
    std::vector<int16_t> sample(nb_channel);
    std::cout << "[INFOS] Now sending data... " << std::endl;

    int n = 0;
    uint32_t dt = 1000;
    for(int j = 0;; j++)
    {
        //compute the time for each loop
        //auto start = std::chrono::high_resolution_clock::now();
        n = device->writeS(&cmd, 1);
        //int32_t prev_t = timestamps[0];
        n = device->readS((uint8_t *)&timestamps, 2 * 4, false);
        // std::cout << "timestamps: " << timestamps[1] - timestamps[0] << " "
        //           << timestamps[0] - prev_t << std::endl;

        n = device->readS((uint8_t *)&data, nb_channel * 2, false);
        //std::cout << "nb data read: " << n<< std::endl;
        //std::cout << "nb data read: " << data[0] << " " << data[1] << std::endl;

        for(int i = 0; i < nb_channel; i++) { sample[i] = data[i]; }
        outlet_sample.push_sample(sample);

        //auto stop = std::chrono::high_resolution_clock::now();
        // auto duration =
        //     std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        // dt = (dt * (j + 1) + duration.count()) / (j + 2);
        // std::cout << "loop time: " << dt << std::endl;
    }

    return 0;
}
