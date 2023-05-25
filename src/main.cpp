#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <com_client.hpp>
#include <lsl_cpp.h>
#include <unistd.h>

int
main(int argc, char **argv)
{

    std::string path = "/dev/ttyACM0";
    Communication::Client device(true);
    try
    {
        std::cout << "Com Interface:" << std::endl;
        device.open_connection(Communication::Client::SERIAL, path.c_str());
    }
    catch(std::string msg)
    {
        std::cout << "ERROR: " << msg << "\n";
        return 0;
    }

    usleep(2000000);

    int nb_channels = 16 * 16;
    lsl::stream_info info_sample("FSR", "sample", nb_channels, 0,
                                 lsl::cf_int16);
    lsl::stream_outlet outlet_sample(info_sample);
    std::vector<int16_t> sample(nb_channels);
    std::cout << "[INFOS] Now sending data... " << std::endl;

    int dj = 10;
    double v = 0;

    uint8_t buff[1000] = {};
    for(int j = 0;; j++)
    {
        char n_ch = 16;
        int a = device.writeS(&n_ch, 1);
        //std::cout << a << std::flush;
	uint32_t timestamps[2];
        int n = device.readS((uint8_t *)&timestamps, 2*4, false);
	//std::cout << "nb data read: " << n<< std::endl;
	//std::cout << "timestamps[0]: " << timestamps[0] << " " << timestamps[1] << std::endl;
	uint16_t data[256];
        n = device.readS((uint8_t *)&data, 256*2, false);
	//std::cout << "nb data read: " << n<< std::endl;
	std::cout << "nb data read: " << data[0] << " " << data[1] << std::endl;
	
        for(int i = 0; i < 256; i++)
        {
            sample[i] = data[i];
        }
        outlet_sample.push_sample(sample);
    }

    return 0;
}
