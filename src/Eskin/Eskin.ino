#include "fsr.hpp"
#include "lis3dh.h"
#define ACC_SAMPLE_RATE 1000
#define FSR_SAMPLE_RATE 200

uint8_t arr0x17[16] = {0x1F, 0x23,             //21
                       0x22, 0x22,             //21
                       0x23, 0x23,             //25
                       0x1F, 0x1C,             //D
                       0x1E, 0x23, 0x23, 0x1C, //D
                       0x1F, 0x22, 0x1C, 0x1E};

FSR fsr;

uint16_t fsr_data[2 * 16 * 16];
int16_t acc_data[3 * 4 * 4];
uint32_t ts_acc = 0;
uint32_t ts_fsr = 0;
uint8_t size_acc = 16;
uint8_t size_fsr = 0;
uint8_t layers_fsr = 0;
int count = 0;

uint8_t LIS3DH::last_index = 0;

//protocol:
//read 1 byte: 2 MSB bits are mode, 6 LSB bits are size
//mode 0: request 1 FSR array of size*size elements
//mode 1: request 2 FSR array of size*size elements
//mode 2: request 1 ACC array of size*size elements
//mode 3: request 1 ACC array and 2 FSR arrays of size*size elements

//reply format:
//- 4 bytes: timestamp acc => if != 0, then the requested amount of data will be sent
//- 4 bytes: timestamp fsr => if != 0, then the requested amount of data will be sent
//- data requested if timestamps != 0

IntervalTimer timer_acc;
IntervalTimer timer_fsr;

LIS3DH lis3dh[16];

int aa = 1;

void
acc_callback()
{
    if(size_acc == 0)
        return;
    ts_acc = micros();

    for(int i = 0; i < 16; i++)
    {
        lis3dh[i].getAccelerationRaw(&acc_data[i * 3], &acc_data[i * 3 + 1],
                                     &acc_data[i * 3 + 2]);
        // Serial.print(acc_data[i * 3 + 2]);
        // Serial.print(" ");
    } //256us
    // Serial.println();
}

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
    Serial.begin(115200);
    // while(!Serial);
    fsr.begin();
    SPI1.begin();

    delay(1000);
    // initialize LIS3DH accelerometers
    for(int i = 0; i < 16; i++)
    { // pass select_address function to LIS3DH constructor
        lis3dh[i].begin(SPI1, i);
        lis3dh[i].setFullScaleRange(LIS3DH_RANGE_2G);
        lis3dh[i].setOutputDataRate(LIS3DH_DATARATE_5KHZ);
        lis3dh[i].setHighSolution(true);
        // lis3dh[i].openTemp();
        delay(1);
    }

    // timer_acc.begin(acc_callback, 1000000 / ACC_SAMPLE_RATE);
    //timer_fsr.begin(fsr_callback, 1000000 / FSR_SAMPLE_RATE);
}

void
loop()
{
    if(Serial.available() > 0)
    {
        uint8_t cmd = Serial.read(); //get the command
        uint8_t mode = cmd >> 6;     //extract the mode
        uint8_t size = cmd & 0x3F;   //extract the size

        switch(mode)
        {
        case 0:             //request 1 FSR array of size*size elements
            noInterrupts(); //disable interrupts to avoid data corruption with callbacks
            size_fsr = size; //set the size of the array to read
            layers_fsr = 1;  //set the number of layers to read
            size_acc = 0;    //reset the size of the acc array to not read it
            Serial.write(
                (uint8_t *)&ts_fsr,
                4); //send the timestamp of the last read (0 if no data available)
            if(ts_fsr != 0) //if data is available, send it
            {
                //the data is stored in fsr_data, and contains size*size elements of 2 bytes each
                Serial.write((uint8_t *)&fsr_data, 2 * size * size);
                ts_fsr =
                    0; //reset the timestamp to signal that the data has been read
            }
            interrupts();
            break;
        case 1: //request 2 FSR array of size*size elements
            noInterrupts();
            size_fsr = size;
            layers_fsr = 2;
            size_acc = 0;
            Serial.write((uint8_t *)&ts_fsr, 4);
            if(ts_fsr != 0)
            {
                Serial.write((uint8_t *)&fsr_data, 4 * size * size);
                ts_fsr = 0;
            }
            interrupts();
            break;
        case 2: //request 1 ACC array of size*size elements
            noInterrupts();
            size_acc = size;
            size_fsr = 0;
            Serial.write((uint8_t *)&ts_acc, 4);
            if(ts_acc != 0)
            {
                Serial.write((uint8_t *)&acc_data, 2 * size * size);
                ts_acc = 0;
            }
            interrupts();
            break;
        case 3: //request 1 ACC array of 4*4 elements and 2 FSR arrays of size*size elements
            //no parallelism, the data is sent in the order ACC, FSR1, FSR2
            ts_acc = micros();
            for(int i = 0; i < 16; i++) //256us
                lis3dh[i].getAccelerationRaw((uint8_t *)&acc_data[i * 3]);
            ts_fsr = micros();
            //[fsr_l1_row1, fsr_l2_row1, fsr_l1_row2, fsr_l2_row2, ...]
            fsr.scan_2array(fsr_data, size);
            Serial.write((uint8_t *)&ts_acc, 4);
            Serial.write((uint8_t *)&ts_fsr, 4);
            Serial.write((uint8_t *)&acc_data, 2 * 16 * 3);
            Serial.write((uint8_t *)&fsr_data, 2 * 2 * size * size);
            break;
        }
    }
}