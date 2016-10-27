#ifndef ALARM_H
#define ALARM_H
#include "units.h"

typedef struct{
  void *unit;
  short alarmCode;
  int acknowledged;
} *pAlarmInfo, alarmInfo_t;

//alarmCode here again? Or remove in units.h??

#define MAX_ALARMS 5
#define ALARM_ERROR -1

int alarmIsAcknowledged(void *unit, short alarmCode);
int alarmRemove(void *unit, short alarmCode);
int alarmAdd(void *unit, short alarmCode);
void alarmAcknowledgeById(int id);
char *alarmGetActiveAlarmsAsString(char *buf, int buflen);

#endif
