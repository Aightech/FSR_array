#ifndef FSR_HPP
#define FSR_HPP
#include <SPI.h>

// interface ADC MAXMAX11632EEG+

class FSR
{
    enum SCAN_MODE
    {
        SCAN_0_TO_N = 0,
        SCAN_N_TO_F = 1,
        SCAN_REPT_N = 2,
        NO_SCAN = 3
    };

    enum CLK_MODE
    {
        INT_CLK_CNVST =
            0b00, // the wake-up, acquisition, conversion, and shutdown sequences are initiated through CNVST and performed automatically using the internal oscillator.
        EXT_CLK_CNVST = 0b01,
        INT_CLK = 0b10,
        SCLK = 0b11 // use SCLK to drive the conversion
    };

    enum REF_MODE
    {
        REF_MODE_0 = 0b00, // internal reference, off after conversion
        REF_MODE_1 = 0b01, // external reference, no wake-up delay
        REF_MODE_2 = 0b10, // internal reference, no wake-up delay
    };

    enum AVG_MODE
    {
        AVG_ON = 0,
        AVG_OFF = 1
    };

    enum AVG_NB
    {
        AVG_4 = 0,
        AVG_8 = 1,
        AVG_16 = 2,
        AVG_32 = 3
    };

    enum SCAN_NB
    {
        SCAN_4 = 0,
        SCAN_8 = 1,
        SCAN_16 = 2,
        SCAN_32 = 3
    };

    enum RST_MODE
    {
        RST_ALL = 0,
        RST_FIFO = 1
    };

public:
    FSR() {};
    void
    begin()
    {
        SPI.begin();
        SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

        pinMode(CS_PIN, OUTPUT);
        digitalWrite(CS_PIN, HIGH);
        pinMode(EOC_PIN, INPUT);
        pinMode(EN_PIN, OUTPUT);
        digitalWrite(EN_PIN, HIGH);

        for (int i = 0; i < 4; i++)
        {
            pinMode(ADD_PIN[i], OUTPUT);
            digitalWrite(ADD_PIN[i], LOW);
        }

        digitalWrite(EN_PIN, LOW);
        // int row = 15;//5
        // while(1)
        // {
        //     if(row == 16)
        //         row = 0;
        //     for(int i = 0; i < 4; i++) digitalWrite(ADD_PIN[i], (m_grey_code[row] >> i) & 1);
        //     delay(1);
        //     row++;
        // }

        // while(1);

        conf_setup(INT_CLK, REF_MODE_2);
        conf_averaging(SCAN_8, AVG_OFF);
    };

    void
    conf_conversion(byte CHSEL, SCAN_MODE SCAN)
    {
        digitalWrite(CS_PIN, LOW);
        SPI.transfer(0b10000000 | (CHSEL << 3) | (SCAN << 1));
        digitalWrite(CS_PIN, HIGH);
    }

    void
    conf_setup(CLK_MODE CKSEL, REF_MODE REFSEL)
    {
        digitalWrite(CS_PIN, LOW);
        SPI.transfer(0b01000000 | (CKSEL << 4) |
                     (REFSEL << 2)); // 0b00000001 << 4 = 0b00010000
        digitalWrite(CS_PIN, HIGH);
        if (CKSEL == SCLK)
            m_fast_read_mode = true;
        else
            m_fast_read_mode = false;
    }

    void
    conf_averaging(SCAN_NB SCAN, AVG_MODE AVGON, AVG_NB NAVG = AVG_4)
    {
        digitalWrite(CS_PIN, LOW);
        SPI.transfer(0b00100000 | (AVGON << 4) | (NAVG << 2) | SCAN);
        digitalWrite(CS_PIN, HIGH);
    }

    void
    conf_reset(RST_MODE RESET)
    {
        digitalWrite(CS_PIN, LOW);
        SPI.transfer(0b00010000 | (RESET << 3));
        digitalWrite(CS_PIN, HIGH);
    }

    void
    fast_read_value(uint16_t *val, int n)
    {

        byte CHSEL = 0;           // start from channel 0
        SCAN_MODE SCAN = NO_SCAN; // no scan in fast read
        uint8_t *buff = (uint8_t *)val;
        // uint32_t t = micros();
        digitalWrite(CS_PIN, LOW);
        SPI.transfer(0b10000000 | (CHSEL << 3) |
                     (SCAN << 1));      // start the first conversion
        for (int i = 0; i < 2 * n; i++) // 2 bytes per channel
        {
            if (i % 2 == 0 || i == 2 * n - 1) // get the MSbyte
                *(buff + i + (1 - 2 * (i % 2))) =
                    SPI.transfer(0x00); // big endian
            else                        // get the remaining 8LSB while sending request for the next conv (except for the last)
            {
                CHSEL++; // request conv for the next channel
                *(buff + i + (1 - 2 * (i % 2))) =
                    SPI.transfer(0b10000000 | (CHSEL << 3) | (SCAN << 1));
            }
        }
        digitalWrite(CS_PIN, HIGH);
        // Serial.println(micros() - t);
    }


    void
    read_value(uint16_t *val, int n)
    {
        if (m_fast_read_mode)
            fast_read_value(val, n);
        else
        {
            conf_conversion(n - 1, SCAN_0_TO_N);
            while (digitalRead(EOC_PIN) == HIGH)
            {
            };

            uint8_t *buff = (uint8_t *)val;
            digitalWrite(CS_PIN, LOW);
            for (int i = 0; i < 2 * n; i++)
            {
                *(buff + i + (1 - 2 * (i % 2))) = 0;
                *(buff + i + (1 - 2 * (i % 2))) = SPI.transfer(0x00);
            }
            digitalWrite(CS_PIN, HIGH);
        }
    }

    void
    select_row(uint8_t row)
    {
        digitalWrite(EN_PIN, LOW);
        for (int i = 0; i < 4; i++)
            digitalWrite(ADD_PIN[i], (row >> i) & 1);
        delay(1);
    }

    void
    scan_array(uint16_t *val)
    {
        for (int i = 0; i < 16; i++)
        {
            int r = m_grey_code[i];
            select_row(r);
            read_value(val + r * 16, 16);
        }
    }

private:
    int CS_PIN = 9;
    int EOC_PIN = 8;
    int ADD_PIN[4] = {7, 6, 5, 4};
    int EN_PIN = 3;
    bool m_fast_read_mode = false;
    const uint8_t m_grey_code[16] = {0, 1, 3, 2, 6, 7, 5, 4, 12, 13, 15, 14, 10, 11, 9, 8};
    //0000 0
    //0001 1
    //0011 3
    //0010 2
    //0110 6
    //0111 7
    //0101 5
    //0100 4
    //1100 12
    //1101 13
    //1111 15
    //1110 14
    //1010 10
    //1011 11
    //1001 9
    //1000 8

};

#endif
