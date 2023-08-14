#include "fsr.hpp"

#include <NativeEthernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 127, 254);
EthernetServer server(5000);


uint8_t buff[4 * 2 + 2048 * 2];
uint16_t *val = (uint16_t *)(buff + 4 * 2);
uint32_t *t = (uint32_t *)buff;
uint8_t end_c = 0xaa;

FSR fsr;

void setup() {
  Serial.begin(500000);
  fsr.set_cs_pins(9, 10);        //set chip select pins
  fsr.set_eoc_pins(8, 25);       //set end of cmd pins
  fsr.set_add_pins(2, 3, 4, 5);  //set address pins
  fsr.set_en_pins(6, 7);         //set enable pins
  fsr.begin();                   //set up the inpout/output pins of the FSR


  Ethernet.begin(mac, ip);                              // start the Ethernet connection and the server:
  if (Ethernet.hardwareStatus() == EthernetNoHardware)  // Check for Ethernet hardware present
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  if (Ethernet.linkStatus() == LinkOFF)
    Serial.println("Ethernet cable is not connected.");

  Serial.println("Ok");
  server.begin();  // start the server
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

}


void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("new client");
    while (client.connected()) {
      int len = client.available();
      if (len >= 1) {
        char n;
        client.read(&n, 1);
        uint8_t conf = n & 0xc0;
        n &= 0x3f;
        int len;

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
        client.write(buff, len);  //[ t1(4o) | t2(4o) | val(2*n*n o) ]
      }
    }
    client.stop();// close the connection
    Serial.println("client disconnected");
    delay(10);
  }
}
