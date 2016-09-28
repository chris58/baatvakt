#include "battery.h"

//#define DEBUG_BATTERY

pBatteryInfo initBattery(pBatteryInfo bat, char *name, uint8_t pin, float conversionFactor, float lowAlarmVoltage){
  memset(bat->name, 0, sizeof(bat->name));
  strncpy(bat->name,name, sizeof(bat->name)-1);
  bat->pin = pin;
  bat->bit2voltConversion = conversionFactor;
  bat->lowAlarmVoltage = lowAlarmVoltage;

  return bat;
}

void updateBattery(pBatteryInfo bat){
  bat->raw = analogRead(bat->pin);
#ifdef DEBUG_BATTERY
  Serial.print(bat->name);
  Serial.print("!!!!!!!!!!!!bat->raw=");
  Serial.println(bat->raw);
#endif
}

float getVoltage(pBatteryInfo bat){
  // float r1 = 100;
  // float r2 = 47;
  // return ((float) raw * (5.0 * (r1+r2)/r2) / 1023.0);
  return bat->raw * bat->bit2voltConversion;
}

char* getVoltageAsString(pBatteryInfo bat, char *voltageS){
  return dtostrf(getVoltage(bat), 5, 2, voltageS);
}
