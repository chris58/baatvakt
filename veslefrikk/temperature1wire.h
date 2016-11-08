#ifndef TEMPERATURE1WIRE_H
#define TEMPERATURE1WIRE_H

#include <DallasTemperature.h>
#include <OneWire.h>

#define ALARM_TEMPERATURE_LOW 1
#define ALARM_TEMPERATURE_HIGH 2

typedef struct{
  uint8_t typeID;
  char name[12];
  int alarmCode;
  DeviceAddress da;
  float value;
} temperatureInfo_t, *pTemperatureInfo;

#define temperatureGetTempC(pi) (pi->value)
#define temperatureGetAlarmCode(pi) (pi->alarmCode)

void temperatureInit(int n);
void temperaturesUpdate();
pTemperatureInfo temperatureAddTemperatureProbe(DeviceAddress da, char *name, uint8_t alarmLow, uint8_t alarmHigh, uint8_t resolution);
char *temperatureGetAlarmMsg(pTemperatureInfo temp, char *msg, size_t len);
pTemperatureInfo temperatureGetNextTempInfo(pTemperatureInfo last);
char *temperatureGetNMEA();
#endif
