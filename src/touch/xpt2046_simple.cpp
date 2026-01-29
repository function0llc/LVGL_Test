#include "touch/xpt2046_simple.h"

#ifndef TOUCH_SPI_FREQUENCY
#define TOUCH_SPI_FREQUENCY 2000000
#endif

constexpr uint16_t Z_THRESHOLD = 120;

SimpleXPT2046::SimpleXPT2046(int csPin, int irqPin) : cs(csPin), irq(irqPin) {}

bool SimpleXPT2046::begin(SPIClass &spiRef)
{
    spi = &spiRef;

    if (cs < 0)
    {
        return false;
    }

    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);

    if (irq >= 0)
    {
        pinMode(irq, INPUT_PULLUP);
    }

    initialized = true;
    return true;
}

void SimpleXPT2046::setRotation(uint8_t value)
{
    rotation = value & 0x03;
}

bool SimpleXPT2046::touched()
{
    if (!initialized)
    {
        return false;
    }

    if (irq >= 0)
    {
        return digitalRead(irq) == LOW;
    }

    return lastZ > Z_THRESHOLD;
}

TS_Point SimpleXPT2046::getPoint()
{
    TS_Point point;

    if (!initialized || !spi)
    {
        return point;
    }

    // When IRQ is available, only read if touch detected
    if (irq >= 0 && digitalRead(irq) != LOW)
    {
        lastZ = 0;
        return point;
    }

    // For polling mode (no IRQ), always attempt to read SPI data
    // The Z threshold check below will filter out non-touches

    SPISettings settings(TOUCH_SPI_FREQUENCY, MSBFIRST, SPI_MODE0);
    spi->beginTransaction(settings);
    digitalWrite(cs, LOW);

    // throw away first conversion after chip select
    readData(0x90);

    uint16_t y = readData(0x90);
    uint16_t x = readData(0xD0);
    uint16_t z1 = readData(0xB0);
    uint16_t z2 = readData(0xC0);

    spi->transfer(0x00);

    digitalWrite(cs, HIGH);
    spi->endTransaction();

    int16_t z = z1 + (4095 - z2);
    lastZ = z;

    if (z < Z_THRESHOLD)
    {
        return TS_Point();
    }

    point.x = x;
    point.y = y;
    point.z = z;
    return point;
}

uint16_t SimpleXPT2046::readData(uint8_t command)
{
    spi->transfer(command);
    uint16_t high = spi->transfer(0x00);
    uint16_t low = spi->transfer(0x00);
    return ((high << 8) | low) >> 3;
}
