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

int isAcknowledgedAlarm(void *unit, short alarmCode);
int removeAlarm(void *unit, short alarmCode);
int addAlarm(void *unit, short alarmCode);
void acknowledgeByIdAlarm(int id);
char *getActiveAlarmsAsString(char *buf, int buflen);

#endif
