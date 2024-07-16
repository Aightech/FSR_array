#ifndef FSR_HPP
#define FSR_HPP
#include <SPI.h>

// interface ADC MAXMAX11632EEG+

class FSR {
  enum SCAN_MODE { SCAN_0_TO_N = 0,
                   SCAN_N_TO_F = 1,
                   SCAN_REPT_N = 2,
                   NO_SCAN = 3
  };


  enum CLK_MODE { INT_CLK_CNVST = 0b00,  //the wake-up, acquisition, conversion, and shutdown sequences are initiated through CNVST and performed automatically using the internal oscillator.
                  EXT_CLK_CNVST = 0b01,
                  INT_CLK = 0b10,
                  SCLK = 0b11
  };

  enum REF_MODE { REF_MODE_0 = 0b00,
                  REF_MODE_1 = 0b01,
                  REF_MODE_2 = 0b10,
                  REF_MODE_3 = 0b11
  };

  enum AVG_MODE { AVG_ON = 0,
                  AVG_OFF = 1
  };

  enum AVG_NB { AVG_4 = 0,
                AVG_8 = 1,
                AVG_16 = 2,
                AVG_32 = 3
  };

  enum SCAN_NB { SCAN_4 = 0,
                 SCAN_8 = 1,
                 SCAN_16 = 2,
                 SCAN_32 = 3
  };

  enum RST_MODE { RST_ALL = 0,
                  RST_FIFO = 1
  };

public:
  FSR(){};
  void begin() {
    SPI.begin();
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    for (int i = 0; i < 2; i++) {
      pinMode(CS_PIN[i], OUTPUT);
      digitalWrite(CS_PIN[i], HIGH);
      pinMode(EOC_PIN[i], INPUT);

      pinMode(EN_PIN[i], OUTPUT);
      digitalWrite(EN_PIN[i], HIGH);
    }

    for (int i = 0; i < 4; i++) {
      pinMode(ADD_PIN[i], OUTPUT);
      digitalWrite(ADD_PIN[i], LOW);
    }

    conf_setup(0, SCLK, REF_MODE_2);
    conf_setup(1, SCLK, REF_MODE_2);
    conf_averaging(0, SCAN_8, AVG_OFF);
    conf_averaging(1, SCAN_8, AVG_OFF);
  };

  void
  set_cs_pins(int p1, int p2) {
    CS_PIN[0] = p1;
    CS_PIN[1] = p2;
  };

  void
  set_eoc_pins(int p1, int p2) {
    EOC_PIN[0] = p1;
    EOC_PIN[1] = p2;
  };

  void
  set_add_pins(int p1, int p2, int p3, int p4) {
    ADD_PIN[0] = p1;
    ADD_PIN[1] = p2;
    ADD_PIN[2] = p3;
    ADD_PIN[3] = p4;
  };


  void
  set_en_pins(int p1, int p2) {
    EN_PIN[0] = p1;
    EN_PIN[1] = p2;
  };

  void conf_conversion(int id, byte CHSEL, SCAN_MODE SCAN) {
    digitalWrite(CS_PIN[id], LOW);
    SPI.transfer(0b10000000 | (CHSEL << 3) | (SCAN << 1));
    digitalWrite(CS_PIN[id], HIGH);
  }

  void conf_setup(int id, CLK_MODE CKSEL, REF_MODE REFSEL) {
    digitalWrite(CS_PIN[id], LOW);
    SPI.transfer(0b01000000 | (CKSEL << 4) | (REFSEL << 2));
    digitalWrite(CS_PIN[id], HIGH);
    if (CKSEL == SCLK)
      m_fast_read_mode = true;
    else
      m_fast_read_mode = false;
  }

  void conf_averaging(int id, SCAN_NB SCAN, AVG_MODE AVGON, AVG_NB NAVG = AVG_4) {
    digitalWrite(CS_PIN[id], LOW);
    SPI.transfer(0b00100000 | (AVGON << 4) | (NAVG << 2) | SCAN);
    digitalWrite(CS_PIN[id], HIGH);
  }

  void conf_reset(int id, RST_MODE RESET) {
    digitalWrite(CS_PIN[id], LOW);
    SPI.transfer(0b00010000 | (RESET << 3));
    digitalWrite(CS_PIN[id], HIGH);
  }

  void fast_read_value(int id, uint16_t *val, int n) {
    //to use when the conversion is drive by Sclk
    byte CHSEL = 0;
    SCAN_MODE SCAN = NO_SCAN;  //no scan in fast read
    uint8_t *buff = (uint8_t *)val;

    digitalWrite(CS_PIN[id], LOW);
    SPI.transfer(0b10000000 | (CHSEL << 3) | (SCAN << 1));
    for (int i = 0; i < 2 * n; i++) {
      *(buff + i + (1 - 2 * (i % 2))) = 0;
      if (i % 2 == 0 || i == 2 * n - 1)  //get the 4MSB
        *(buff + i + (1 - 2 * (i % 2))) = SPI.transfer(0x00);
      else  //get the remaining 8LSB while sending request for the next conv (except for the last)
      {
        CHSEL++;//request conv for the next channel
        *(buff + i + (1 - 2 * (i % 2))) = SPI.transfer(0b10000000 | (CHSEL << 3) | (SCAN << 1));
      }
    }
    digitalWrite(CS_PIN[id], HIGH);
  }

  void read_value(int id, uint16_t *val, int n) {
    if (m_fast_read_mode)
      fast_read_value(id, val, n);
    else {
      conf_conversion(id, n - 1, SCAN_0_TO_N);
      while (digitalRead(EOC_PIN[id]) == HIGH) {
      };

      uint8_t *buff = (uint8_t *)val;
      digitalWrite(CS_PIN[id], LOW);
      for (int i = 0; i < 2 * n; i++) {
        *(buff + i + (1 - 2 * (i % 2))) = 0;
        *(buff + i + (1 - 2 * (i % 2))) = SPI.transfer(0x00);
      }
      digitalWrite(CS_PIN[id], HIGH);
    }
  }

  void select_raw(int id, uint8_t raw) {
    digitalWrite(EN_PIN[(id + 1) % 2], HIGH);
    digitalWrite(EN_PIN[id % 2], LOW);
    for (int i = 0; i < 4; i++)
      digitalWrite(ADD_PIN[i], (raw >> i) & 1);
  }

  void select_2raw(uint8_t raw) {
    digitalWrite(EN_PIN[0], LOW);
    digitalWrite(EN_PIN[1], LOW);
    for (int i = 0; i < 4; i++)
      digitalWrite(ADD_PIN[i], (raw >> i) & 1);
  }

  void scan_array(uint16_t *val, int n) {
    for (int i = 0; i < n; i++) {
      select_raw(i / 16, i);
      if (n <= 16)
        read_value(0, val + i * n, n);
      else {
        read_value(0, val + i * n, 16);
        read_value(1, val + i * n + 16, n - 16);
      }
    }
  }

  void scan_2array(uint16_t *val, int n) {
    for (int i = 0; i < n; i++) {
      select_2raw(i);
      read_value(0, val + 2 * i * n, n);
      read_value(1, val + 2 * i * n + n, n);
    }
  }


private:
  int CS_PIN[2] = { 9, 10 };
  int EOC_PIN[2] = { 14, 15 };
  int ADD_PIN[4] = { 2, 3, 4, 5 };
  int EN_PIN[2] = { 6, 7 };
  bool m_fast_read_mode = false;
};

#endif
