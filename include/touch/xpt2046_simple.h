#pragma once

#include <Arduino.h>
#include <SPI.h>

struct TS_Point
{
    int16_t x;
    int16_t y;
    int16_t z;

    TS_Point() : x(0), y(0), z(0) {}
    TS_Point(int16_t x_, int16_t y_, int16_t z_) : x(x_), y(y_), z(z_) {}
};

class SimpleXPT2046
{
public:
    SimpleXPT2046(int csPin, int irqPin = -1);

    bool begin(SPIClass &spiRef);
    bool touched();
    TS_Point getPoint();

    void setRotation(uint8_t rotation);

private:
    uint16_t readData(uint8_t command);

    SPIClass *spi{nullptr};
    int cs;
    int irq;
    uint8_t rotation{0};
    bool initialized{false};
    int16_t lastZ{0};
};
