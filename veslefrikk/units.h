#ifndef UNITS_H
#define UNITS_H

#include "pump.h"
#include "battery.h"
#include "temperature1wire.h"

typedef struct{
  uint8_t typeID;
  char name[12];
  int alarmCode;
} unitInto_t, *pUnitInfo;

#define ALARM_OFF 0

#define PUMP 0
#define BATTERY 1
#define TEMPERATURE 2

#endif
