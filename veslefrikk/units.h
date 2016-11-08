#ifndef UNITS_H
#define UNITS_H

#include <CRC32.h>
#include "pump.h"
#include "battery.h"
#include "temperature1wire.h"

typedef struct{
  uint8_t typeID;
  char name[12];
  int alarmCode;
} unitInfo_t, *pUnitInfo;

#define ALARM_OFF 0

#define PUMP 0
#define BATTERY 1
#define TEMPERATURE 2


unsigned long getSeconds();
#endif
