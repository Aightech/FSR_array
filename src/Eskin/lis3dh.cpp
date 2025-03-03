/*
 * A library for Grove - 3-Axis Digital Accelerometer ±2g to 16g Ultra-low Power(LIS3DH)
 *
 * Copyright (c) 2019 seeed technology co., ltd.
 * Author      : Hongtai Liu (lht856@foxmail.com)
 * Create Time : July 2019
 * Change Log  :
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software istm
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS INcommInterface
 * THE SOFTWARE.
 */

#include "lis3dh.h"

LIS3DH::LIS3DH() {}

void
LIS3DH::begin(SPIClass &comm, uint8_t CS_add)
{
    CS_address = CS_add;
    _spi_com = &comm;
    pinMode(23, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(20, OUTPUT);

    _settings = SPISettings(10000000, MSBFIRST, SPI_MODE0);

    // Serial.print("LIS3DH::begin n: ");
    // Serial.println(CS_address);
    // //test if available
    // bool isAvailable = this->checkID();
    // if(!isAvailable)
    // {
    //     Serial.println("LIS3DH is not available");
    // }

    uint8_t config5 =
        LIS3DH_REG_TEMP_ADC_PD_ENABLED | LIS3DH_REG_TEMP_TEMP_EN_DISABLED;
    writeRegister(LIS3DH_REG_TEMP_CFG, config5);
    delay(LIS3DH_CONVERSIONDELAY);

    uint8_t config1 =
        LIS3DH_REG_ACCEL_CTRL_REG1_LPEN_NORMAL | // Normal Mode
        LIS3DH_REG_ACCEL_CTRL_REG1_AZEN_ENABLE | // Acceleration Z-Axis Enabled
        LIS3DH_REG_ACCEL_CTRL_REG1_AYEN_ENABLE | // Acceleration Y-Axis Enabled
        LIS3DH_REG_ACCEL_CTRL_REG1_AXEN_ENABLE;

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG1, config1);
    delay(LIS3DH_CONVERSIONDELAY);
}

bool
LIS3DH::available()
{
    uint8_t status = 0;
    status = readRegister(LIS3DH_REG_ACCEL_STATUS2);
    status &= LIS3DH_REG_ACCEL_STATUS2_UPDATE_MASK;
    return status;
}

void
LIS3DH::openTemp()
{
    uint8_t config5 =
        LIS3DH_REG_TEMP_ADC_PD_ENABLED | LIS3DH_REG_TEMP_TEMP_EN_ENABLED;

    writeRegister(LIS3DH_REG_TEMP_CFG, config5);
    delay(LIS3DH_CONVERSIONDELAY);
}

void
LIS3DH::closeTemp()
{
    uint8_t config5 =
        LIS3DH_REG_TEMP_ADC_PD_ENABLED | LIS3DH_REG_TEMP_TEMP_EN_DISABLED;

    writeRegister(LIS3DH_REG_TEMP_CFG, config5);
    delay(LIS3DH_CONVERSIONDELAY);
}

int16_t
LIS3DH::getTemperature(void)
{
    int16_t result = ((int16_t)readRegisterInt16(0x0c)) / 256;
    result += 25;
    return result;
}

uint8_t
LIS3DH::getDeviceID(void)
{
    return readRegister(LIS3DH_REG_ACCEL_WHO_AM_I);
}

void
LIS3DH::setPowerMode(power_type_t mode)
{
    uint8_t data = 0;

    data = readRegister(LIS3DH_REG_ACCEL_CTRL_REG1);

    data &= ~LIS3DH_REG_ACCEL_CTRL_REG1_LPEN_MASK;
    data |= mode;

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG1, data);
    delay(LIS3DH_CONVERSIONDELAY);
}

void
LIS3DH::setFullScaleRange(scale_type_t range)
{
    uint8_t data = 0;

    data = readRegister(LIS3DH_REG_ACCEL_CTRL_REG4);

    data &= ~LIS3DH_REG_ACCEL_CTRL_REG4_FS_MASK;
    data |= range;

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG4, data);
    delay(LIS3DH_CONVERSIONDELAY);

    switch(range)
    {
    case LIS3DH_REG_ACCEL_CTRL_REG4_FS_16G:
        accRange = 1280;
        break;
    case LIS3DH_REG_ACCEL_CTRL_REG4_FS_8G:
        accRange = 3968;
        break;
    case LIS3DH_REG_ACCEL_CTRL_REG4_FS_4G:
        accRange = 7282;
        break;
    case LIS3DH_REG_ACCEL_CTRL_REG4_FS_2G:
        accRange = 16000;
        break;
    default:
        break;
    }
}

void
LIS3DH::setOutputDataRate(odr_type_t odr)
{
    uint8_t data = 0;

    data = readRegister(LIS3DH_REG_ACCEL_CTRL_REG1);

    data &= ~LIS3DH_REG_ACCEL_CTRL_REG1_AODR_MASK;
    data |= odr;

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG1, data);
    delay(LIS3DH_CONVERSIONDELAY);
}

void
LIS3DH::getAcceleration(float *x, float *y, float *z)
{
    // Read the Accelerometer
    uint8_t buf[8] = {0};

    // Read the Data
    readRegisterRegion(buf, LIS3DH_REG_ACCEL_OUT_X_L, 6);

    // Conversion of the result
    // 16-bit signed result for X-Axis Acceleration Data of LIS3DH
    *x = (float)((int16_t *)buf)[0] / accRange;
    // 16-bit signed result for Y-Axis Acceleration Data of LIS3DH
    *y = (float)((int16_t *)buf)[1] / accRange;
    // 16-bit signed result for Z-Axis Acceleration Data of LIS3DH
    *z = (float)((int16_t *)buf)[2] / accRange;
}

void
LIS3DH::getAccelerationRaw(int16_t *x, int16_t *y, int16_t *z)
{
    // Read the Accelerometer
    uint8_t buf[8] = {0};

    // Read the Data
    readRegisterRegion(buf, LIS3DH_REG_ACCEL_OUT_X_L, 6);

    // Conversion of the result
    // 16-bit signed result for X-Axis Acceleration Data of LIS3DH
    *x = ((int16_t *)buf)[0];
    // 16-bit signed result for Y-Axis Acceleration Data of LIS3DH
    *y = ((int16_t *)buf)[1];
    // 16-bit signed result for Z-Axis Acceleration Data of LIS3DH
    *z = ((int16_t *)buf)[2];
}

void
LIS3DH::getAccelerationRaw(uint8_t *buf)
{
    // Read the Accelerometer
    readRegisterRegion(buf, LIS3DH_REG_ACCEL_OUT_X_L, 6);
}

float
LIS3DH::getAccelerationX(void)
{
    // Read the Accelerometer
    uint8_t xAccelLo, xAccelHi;
    int16_t x;

    // Read the Data
    // Reading the Low X-Axis Acceleration Data Register
    xAccelLo = readRegister(LIS3DH_REG_ACCEL_OUT_X_L);
    // Reading the High X-Axis Acceleration Data Register
    xAccelHi = readRegister(LIS3DH_REG_ACCEL_OUT_X_H);
    // Conversion of the result
    // 16-bit signed result for X-Axis Acceleration Data of LIS3DH
    x = (int16_t)((xAccelHi << 8) | xAccelLo);

    return (float)x / accRange;
}

float
LIS3DH::getAccelerationY(void)
{
    // Read the Accelerometer
    uint8_t yAccelLo, yAccelHi;
    int16_t y;

    // Reading the Low Y-Axis Acceleration Data Register
    yAccelLo = readRegister(LIS3DH_REG_ACCEL_OUT_Y_L);
    // Reading the High Y-Axis Acceleration Data Register
    yAccelHi = readRegister(LIS3DH_REG_ACCEL_OUT_Y_H);
    // Conversion of the result
    // 16-bit signed result for Y-Axis Acceleration Data of LIS3DH
    y = (int16_t)((yAccelHi << 8) | yAccelLo);

    return (float)y / accRange;
}

float
LIS3DH::getAccelerationZ(void)
{
    // Read the Accelerometer
    uint8_t zAccelLo, zAccelHi;
    int16_t z;

    // Reading the Low Z-Axis Acceleration Data Register
    zAccelLo = readRegister(LIS3DH_REG_ACCEL_OUT_Z_L);
    // Reading the High Z-Axis Acceleration Data Register
    zAccelHi = readRegister(LIS3DH_REG_ACCEL_OUT_Z_H);
    // Conversion of the result
    // 16-bit signed result for Z-Axis Acceleration Data of LIS3DH
    z = (int16_t)((zAccelHi << 8) | zAccelLo);

    return (float)z / accRange;
}

void
LIS3DH::setHighSolution(bool enable)
{
    uint8_t data = 0;
    data = readRegister(LIS3DH_REG_ACCEL_CTRL_REG4);

    data = enable ? data | LIS3DH_REG_ACCEL_CTRL_REG4_HS_ENABLE
                  : data & ~LIS3DH_REG_ACCEL_CTRL_REG4_HS_ENABLE;

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG4, data);
    return;
}

uint16_t
LIS3DH::readbitADC1(void)
{
    uint8_t adc1_l, adc1_h;
    int16_t intTemp;
    uint16_t uintTemp;
    adc1_l = readRegister(0x08);
    adc1_h = readRegister(0x09);

    intTemp = (int16_t)(adc1_h << 8) | adc1_l;
    intTemp = 0 - intTemp;
    uintTemp = intTemp + 32768;
    return uintTemp >> 6;
}

uint16_t
LIS3DH::readbitADC2(void)
{
    uint8_t adc2_l, adc2_h;
    int16_t intTemp;
    uint16_t uintTemp;
    adc2_l = readRegister(0x0A);
    adc2_h = readRegister(0x0B);
    intTemp = (int16_t)(adc2_h << 8) | adc2_l;
    intTemp = 0 - intTemp;
    uintTemp = intTemp + 32768;
    return uintTemp >> 6;
}

uint16_t
LIS3DH::readbitADC3(void)
{
    uint8_t adc3_l, adc3_h;
    int16_t intTemp;
    uint16_t uintTemp;
    adc3_l = readRegister(0x0C);
    adc3_h = readRegister(0x0D);

    intTemp = (int16_t)(adc3_h << 8) | adc3_l;
    intTemp = 0 - intTemp;
    uintTemp = intTemp + 32768;
    return uintTemp >> 6;
}

void
LIS3DH::writeRegister(uint8_t reg, uint8_t val)
{
    _spi_com->beginTransaction(_settings);
    _CS(true);
    _spi_com->transfer(reg);
    _spi_com->transfer(val);
    _CS(false);
    _spi_com->endTransaction();
}

void
LIS3DH::_CS(bool active)
{
    if(active)
        _select_addr(CS_address);
    else
        _select_addr(CS_address - 2);
}

void
LIS3DH::readRegisterRegion(uint8_t *outputPointer, uint8_t reg, uint8_t length)
{

    // define pointer that will point to the external space
    uint8_t i = 0;
    uint8_t c = 0;
    _spi_com->beginTransaction(_settings);
    _CS(true);
    _spi_com->transfer(
        reg | 0x80 |
        0x40); // Ored with "read request" bit and "auto increment" bit
    //delayMicroseconds(10); // delay for at least 60us
    while(i < length) // slave may send less than requested
    {
        c = _spi_com->transfer(0x00); // receive a byte as character
        *outputPointer = c;
        outputPointer++;
        i++;
    }
    _CS(false);
    _spi_com->endTransaction();
}

void
LIS3DH::writeRegisterRegion(uint8_t *data, uint8_t reg, uint8_t length)
{
    _spi_com->beginTransaction(_settings);
    _CS(true);
    _spi_com->transfer(reg | 0x40); // Ored with "auto increment" bit
    for(uint8_t i = 0; i < length; i++) { _spi_com->transfer(data[i]); }
    _CS(false);
    _spi_com->endTransaction();
}

uint16_t
LIS3DH::readRegisterInt16(uint8_t reg)
{

    uint8_t myBuffer[2];
    readRegisterRegion(myBuffer, reg, 2);
    uint16_t output = myBuffer[0] | uint16_t(myBuffer[1] << 8);

    return output;
}

uint8_t
LIS3DH::readRegister(uint8_t reg)
{
    uint8_t data;

    readRegisterRegion(&data, reg, 1);

    return data;
}

void
LIS3DH::click(uint8_t c,
              uint8_t click_thresh,
              uint8_t limit,
              uint8_t latency,
              uint8_t window)
{
    if(!c)
    {
        uint8_t r = readRegister(LIS3DH_REG_ACCEL_CTRL_REG3);
        r &= ~(0x80); // turn off I1_CLICK
        writeRegister(LIS3DH_REG_ACCEL_CTRL_REG3, r);
        writeRegister(LIS3DH_REG_ACCEL_CLICK_CFG, 0);
        return;
    }
    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG3, 0x80);
    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG5, 0x08);

    if(c == 1)
    {
        writeRegister(LIS3DH_REG_ACCEL_CLICK_CFG, 0x15);
    }
    if(c == 2)
    {
        writeRegister(LIS3DH_REG_ACCEL_CLICK_CFG, 0x2A);
    }

    writeRegister(LIS3DH_REG_ACCEL_CLICK_THS, click_thresh);
    writeRegister(LIS3DH_REG_ACCEL_TIME_LIMIT, limit);
    writeRegister(LIS3DH_REG_ACCEL_TIME_LATENCY, latency);
    writeRegister(LIS3DH_REG_ACCEL_TIME_WINDOW, window);
}

void
LIS3DH::getIntStatus(uint8_t *flag)
{
    *flag = readRegister(LIS3DH_REG_ACCEL_INT1_SRC);
}

void
LIS3DH::setInterrupt(void)
{
    uint8_t data = 0;
    uint8_t config1 =
        LIS3DH_REG_ACCEL_CTRL_REG1_AODR_50 |
        LIS3DH_REG_ACCEL_CTRL_REG1_LPEN_NORMAL | // Normal Mode
        LIS3DH_REG_ACCEL_CTRL_REG1_AZEN_ENABLE | // Acceleration Z-Axis Enabled
        LIS3DH_REG_ACCEL_CTRL_REG1_AYEN_ENABLE | // Acceleration Y-Axis Enabled
        LIS3DH_REG_ACCEL_CTRL_REG1_AXEN_ENABLE;
    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG1,
                  config1); // (50 Hz),  X/Y/Z-axis enable

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG2, 0x00); //

    uint8_t config3 = LIS3DH_CTRL_REG3_IA1_ENABLE;
    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG3, config3); // IA1 interrupt

    setFullScaleRange(LIS3DH_RANGE_8G);

    writeRegister(LIS3DH_REG_ACCEL_CTRL_REG5, 0x00); // Latch interrupt request

    writeRegister(
        LIS3DH_REG_ACCEL_CTRL_REG6,
        0x42); // IA1, active-low  Enable interrupt 1 function on INT2 pin

    writeRegister(
        LIS3DH_REG_ACCEL_INT1_THS,
        0x50); // set Threshold，2g =>16mg/LSB，4g => 32mg/LSB，8g => 62mg/LSB，16g => 186mg/LSB

    writeRegister(LIS3DH_REG_ACCEL_INT1_DURATION, 0);

    data = readRegister(LIS3DH_REG_ACCEL_INT1_SRC); // clear interrupt flag
    (void)data;                                     // UNUSED

    writeRegister(LIS3DH_REG_ACCEL_INT1_CFG,
                  0x2a); // trigger when ZHIE/YHIE/XHIE
}

LIS3DH::operator bool() { return isConnection(); }
