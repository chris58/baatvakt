#ifndef BATTERY_H
#define BATTERY_H
#include <Arduino.h>

typedef struct{
  uint8_t pin;
  char name[12];
  float lowAlarmVoltage;
  uint16_t raw;
  float bit2voltConversion;
} batteryInfo_t, *pBatteryInfo;


pBatteryInfo initBattery(pBatteryInfo bat, char *name, uint8_t pin, float conversionFactor, float lowAlarmVoltage);

void updateBattery(pBatteryInfo bat);
float getVoltage(pBatteryInfo bat);
char* getVoltageAsString(pBatteryInfo bat, char *voltageS);
#endif
