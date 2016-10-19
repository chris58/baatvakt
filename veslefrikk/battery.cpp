#include "battery.h"

//#define DEBUG_BATTERY

/*
 * Initialize a new battery structure
 * Return pointer to it
 */
pBatteryInfo batteryInit(pBatteryInfo bi, char *name, uint8_t pin, float conversionFactor, float lowAlarmVoltage){
  pBatteryInfo bat = bi;
  
  if (bat == NULL){
    if ((bat = (pBatteryInfo) calloc(1, sizeof(batteryInfo_t))) == NULL)
      return NULL;
  }

  memset(bat->name, 0, sizeof(bat->name));
  strncpy(bat->name,name, sizeof(bat->name)-1);
  bat->pin = pin;
  bat->bit2voltConversion = conversionFactor;
  bat->lowAlarmVoltage = lowAlarmVoltage;

  return bat;
}

/*
 * Read new voltage and check whether there is an alarm
 * return the current alarm status
 */
int batteryUpdate(pBatteryInfo bat){
  bat->raw = analogRead(bat->pin);
#ifdef DEBUG_BATTERY
  Serial.print(bat->name);
  Serial.print("!!!!!!!!!!!!bat->raw=");
  Serial.println(bat->raw);
#endif

  if (batteryGetVoltage(bat) < bat->lowAlarmVoltage){
    bat->alarm = ALARM_VOLTAGE_LOW;
  }else if (!batteryIsCharging(bat)){
    bat->alarm = ALARM_NOT_CHARGING;
  }else {
    bat->alarm = ALARM_OFF;
  }
  return bat->alarm;
}

/*
 * Apply conversion factor to raw bit value and 
 * return voltage as float
 */
float batteryGetVoltage(pBatteryInfo bat){
  // Voltage devider:
  // float r1 = 100;
  // float r2 = 47;
  // return ((float) raw * (5.0 * (r1+r2)/r2) / 1023.0);
  return bat->raw * bat->bit2voltConversion;
}

char* batteryGetVoltageAsString(pBatteryInfo bat, char *voltageS){
  return dtostrf(batteryGetVoltage(bat), 5, 2, voltageS);
}

/*
 * Quick and dirty guess...
 */
int batteryIsCharging(pBatteryInfo bat){
  return (batteryGetVoltage(bat) >= (bat->lowAlarmVoltage + 2));
}

/*
 * utility to prepare a status message
 */
char *batteryGetAlarmMsg(pBatteryInfo bat, char *msg, size_t len){
  char voltageS[20];
  if (bat->alarm == ALARM_VOLTAGE_LOW){
    snprintf(msg, len, 
	     "ALARM:\nLow Voltage '%s': %s[V]\n",
	     bat->name,
	     batteryGetVoltageAsString(bat, voltageS)
	     );
  }else if (bat->alarm == ALARM_NOT_CHARGING){
    snprintf(msg, len, 
	     "ALARM:\nBattery not charging '%s'\n",
	     bat->name
	     );
  }else{
    snprintf(msg, len, 
	     "Status for battery '%s' is ok\n",
	     bat->name
	     );
  }
  return msg;
}
