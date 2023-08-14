#include "fsr.hpp"

//#define ETHERNET_MODE 1

#ifdef ETHERNET_MODE
#include <NativeEthernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 127, 254);
EthernetServer server(5000);
#endif

uint8_t buff[4 * 2 + 2048 * 2 + 1];
uint16_t *val = (uint16_t *)(buff + 4 * 2);
uint32_t *t = (uint32_t *)buff;
uint8_t end_c = 0xaa;



FSR fsr;

int process(char n) {
  int len=0;
  uint8_t conf = n & 0xc0;
  n &= 0x3f;
  t[0] = micros();
  if (conf == 0x80) {
    len = 8 + 2 * n * n * 2;
    n = min(n, 16);
    fsr.scan_2array(val, n);
  } else {
    len = 8 + n * n * 2;
    n = min(n, 16);
    fsr.scan_array(val, n);
  }
  t[1] = micros();
  // buff[len]=end_c;
  // len++;
  return len;
}

void setup() {
  Serial.begin(500000);



  fsr.set_cs_pins(9, 10);        //set chip select pins
  fsr.set_eoc_pins(8, 25);       //set end of cmd pins
  fsr.set_add_pins(2, 3, 4, 5);  //set address pins
  fsr.set_en_pins(6, 7);         //set enable pins
  fsr.begin();                   //set up the inpout/output pins of the FSR


#ifdef ETHERNET_MODE
  Ethernet.begin(mac, ip);                              // start the Ethernet connection and the server:
  if (Ethernet.hardwareStatus() == EthernetNoHardware)  // Check for Ethernet hardware present
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  if (Ethernet.linkStatus() == LinkOFF)
    Serial.println("Ethernet cable is not connected.");

  Serial.println("Ok");
  server.begin();  // start the server
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
#endif
}


void loop() {
  char n;
  int len;
#ifdef ETHERNET_MODE
  EthernetClient client = server.available();

  if (client) {
    while (client.connected()) {
      if (client.available() >= 1) {
        client.read(&n, 1);
        len = process(n);
        client.write(buff, len);  //[ t1(4o) | t2(4o) | val(2*n*n o) ]
      }
    }
    client.stop();  // close the connection
    delay(10);
  }
#else
  if (Serial.available() >= 1) {
    n = Serial.read();  //read the nb of col of the squre of value to read
    len = process(n);
    Serial.write(buff, len);  //[ t1(4o) | t2(4o) | val(2*n*n o) ]
  }
#endif
}
