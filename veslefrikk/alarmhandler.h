#ifndef ALARM_H
#define ALARM_H

typedef struct{
  void *unit;
  short alarmCode;
  int acknowledged;
} *pAlarmInfo, alarmInfo_t;

#define ALARM_OFF 0
#define MAX_ALARMS 5
#define ALARM_ERROR -1

int isAcknowledgedAlarm(void *unit, short alarmCode);
int removeAlarm(void *unit, short alarmCode);
int addAlarm(void *unit, short alarmCode);
void acknowledgeByIdAlarm(int id);

#endif
