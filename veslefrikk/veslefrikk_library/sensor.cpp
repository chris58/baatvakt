#include "sensor.h"

DeviceAddress Probe1 = { 0x28, 0x0A, 0xC5, 0x4F, 0x07, 0x00, 0x00, 0x21 }; 
DeviceAddress Probe2 = { 0x28, 0x57, 0x5A, 0x50, 0x07, 0x00, 0x00, 0x3F };
DeviceAddress Probe3 = { 0x28, 0xA2, 0x2B, 0x4B, 0x07, 0x00, 0x00, 0xAA }; 
DeviceAddress Probe4 = { 0x28, 0x91, 0xCF, 0x50, 0x07, 0x00, 0x00, 0x77 };


OneWire oneWire(TEMP_BUS);
DallasTemperature sensors(&oneWire);

int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}	

