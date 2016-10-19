#ifndef BATTERY_H
#define BATTERY_H
#include <Arduino.h>

typedef struct{
  short alarm;
  uint8_t pin;
  char name[12];
  float lowAlarmVoltage;
  uint16_t raw;
  float bit2voltConversion;
} batteryInfo_t, *pBatteryInfo;

#define ALARM_VOLTAGE_LOW 1
#define ALARM_OFF 0
#define ALARM_NOT_CHARGING 2

pBatteryInfo batteryInit(pBatteryInfo bat, char *name, uint8_t pin, float conversionFactor, float lowAlarmVoltage);
int batteryUpdate(pBatteryInfo bat);
float batteryGetVoltage(pBatteryInfo bat);
char* batteryGetVoltageAsString(pBatteryInfo bat, char *voltageS);
int batteryIsCharging(pBatteryInfo bat);
char *batteryGetAlarmMsg(pBatteryInfo bat, char *msg, size_t len);
#endif
