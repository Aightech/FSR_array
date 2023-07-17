#include <SPI.h>
#include "fsr.hpp"


uint8_t buff[4 * 2 + 2048 * 2];
uint16_t *val = (uint16_t *)(buff + 4 * 2);
uint32_t *t = (uint32_t *)buff;

FSR fsr;

void setup() {
  Serial.begin(500000);
  SPI.begin();
  SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));

  fsr.set_cs_pins(9, 10);        //set chip select pins
  fsr.set_eoc_pins(8, 25);       //set end of cmd pins
  fsr.set_add_pins(2, 3, 4, 5);  //set address pins
  fsr.set_en_pins(6, 7);         //set enable pins
  fsr.begin();                   //set up the inpout/output pins of the FSR

  //  delay(10);
  //  Serial.write(0xaa);
  //  char c;
  //  while (c != 0xbb)
  //    c = Serial.read();
}


void loop() {
  if (Serial.available() >= 1) {
    char n = Serial.read();  //read the nb of col of the squre of value to read
    uint8_t conf = n & 0xc0;
    n &= 0x3f;
    int len;

    t[0] = micros();
    if (conf == 0x80) {
      len = 8+2*n*n*2;
      n = min(n,16);
      fsr.scan_2array(val, n);
    } else {
      len =  8 + n * n * 2;
      n = min(n,16);
      fsr.scan_array(val, n);
    }
    t[1] = micros();
    Serial.write(buff, len);  //[ t1(4o) | t2(4o) | val(2*n*n o) ]
  }
}
