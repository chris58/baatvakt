#include <stdlib.h>
#include "units.h"

//#define DEBUG_BATTERY

static char nmea[48];

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
  bat->typeID = BATTERY;
  bat->alarmCode = ALARM_OFF;

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
    bat->alarmCode = ALARM_VOLTAGE_LOW;
  }else if (!batteryIsCharging(bat)){
    bat->alarmCode = ALARM_NOT_CHARGING;
  }else {
    bat->alarmCode = ALARM_OFF;
  }
  return bat->alarmCode;
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

/*
 * Return voltage (float) as string
 */
char* batteryGetVoltageAsString(pBatteryInfo bat, char *voltageS){
  return dtostrf(batteryGetVoltage(bat), 5, 2, voltageS);
}

/*
 * Quick and dirty guess...
 */
int batteryIsCharging(pBatteryInfo bat){
  return (batteryGetVoltage(bat) >= (bat->lowAlarmVoltage + 1.5));
}

/*
 * Return a string containing the alarm message, if any.
 */
char *batteryGetAlarmMsg(pBatteryInfo bat, char *msg, size_t len){
  char voltageS[20];
#ifdef DEBUG_BATTERY
  Serial.print("batteryGetAlarmmsg "); Serial.println(bat->alarmCode);
#endif
  if (bat->alarmCode == ALARM_VOLTAGE_LOW){
    snprintf(msg, len, 
	     "Low Voltage '%s': %s[V]",
	     bat->name,
	     batteryGetVoltageAsString(bat, voltageS)
	     );
  }else if (bat->alarmCode == ALARM_NOT_CHARGING){
    snprintf(msg, len, 
	     "Not charging '%s'",
	     bat->name
	     );
  }else{
    snprintf(msg, len, 
	     "Battery '%s' ok",
	     bat->name
	     );
  }

  return msg;
}

/*
 * construct NMEA message for <n> pBatteryInfo s.
 * Returns a local string nmea. Don't mess with it outside this scope.
 */
char *batteryGetNMEA(int n, ...){
  va_list arguments;                     
  pBatteryInfo bat;
  int i;
  int len = sizeof(nmea);
  char *ptr = nmea;
  char voltageS[7] = "";
  
  va_start(arguments, n);
  snprintf(ptr, len, "$PBAT");
  ptr += strlen(ptr);
  len -= strlen(ptr);
  
  for (i=0; i<n; i++){
    bat = va_arg (arguments, pBatteryInfo);
    snprintf(ptr, len, ",%s,%d", batteryGetVoltageAsString(bat, voltageS),
	     batteryGetAlarmCode(bat));
    ptr += strlen(ptr);
    len -= strlen(ptr);
  }
  va_end(arguments);

  snprintf(ptr, len, "*%02X", nmeaCheckSum(nmea, strlen(nmea)));

  return nmea;
}

