#ifndef TEMPERATURE1WIRE_H
#define TEMPERATURE1WIRE_H

#include <DallasTemperature.h>
#include <OneWire.h>

typedef struct{
  uint8_t typeID;
  char name[12];
  int alarmCode;
  DeviceAddress da;
  float value;
} temperatureInfo_t, *pTemperatureInfo;

void temperaturesUpdate();
//pTemperatureInfo initTemperatureProbe(pTemperatureInfo ti, DeviceAddress da, char *name, uint8_t alarmLow, uint8_t alarmHigh, uint8_t resolution);
pTemperatureInfo temperatureAddTemperatureProbe(DeviceAddress da, char *name, uint8_t alarmLow, uint8_t alarmHigh, uint8_t resolution);
float temperatureGetTempC(pTemperatureInfo pi);

#endif
