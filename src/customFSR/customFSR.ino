#include "fsr.hpp"

#define FSR_SAMPLE_RATE 200

FSR fsr;

uint16_t fsr_data[2 * 16 * 16];
uint32_t ts_fsr = 0;
uint8_t size_fsr = 0;
uint8_t layers_fsr = 1;
int count = 0;

IntervalTimer timer_fsr;

void
fsr_callback()
{
    if(size_fsr != 0 && layers_fsr != 0)
    {
        ts_fsr = micros();
        if(layers_fsr == 1)
            fsr.scan_array(fsr_data, size_fsr);
        else //1053us
            fsr.scan_2array(fsr_data, size_fsr);
        //[fsr_l1_row1, fsr_l2_row1, fsr_l1_row2, fsr_l2_row2, ...]
    }
}

void
setup()
{
    Serial.begin(115200); //initialize serial communication

    fsr.begin();// initialize the FSR (ADC and SPI)

}

void
loop()
{
    if(Serial.available() > 0)
    {
        uint8_t cmd = Serial.read(); //get the command
        fsr.scan_array(fsr_data, 16);
        Serial.write((uint8_t *)fsr_data, 2 * 16 * 16);
    }
}