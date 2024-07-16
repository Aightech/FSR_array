#include <SPI.h>
#include "fsr.hpp"


uint8_t buff[4 * 2 + 1024 * 2];
uint16_t *val = (uint16_t*)(buff + 4 * 2);
uint32_t *t = (uint32_t*)buff;

FSR fsr;

void setup()
{
  Serial.begin(500000);
  SPI.begin();
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  fsr.set_cs_pins(9, 10);        //set chip select pins
  fsr.set_eoc_pins(8, 25);       //set end of cmd pins
  fsr.set_add_pins(2, 3, 4, 5);  //set address pins
  fsr.set_en_pins(6, 7);         //set enable pins
  fsr.begin();                   //set up the inpout/output pins of the FSR


}


void loop()
{
  if (Serial.available() >= 1)
  {
    char n = Serial.read();//read the nb of col of the squre of value to read
    //n = 16;
    t[0] = micros();
    fsr.scan_array(val,n);
    t[1] = micros();
    Serial.write(buff, 8 + n * n * 2); //[ t1(4o) | t2(4o) | val(2*n*n o) ]
  }
}
