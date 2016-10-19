#ifndef PUMP
#define PUMP
#include <Arduino.h>

#define PUMPON 1
#define PUMPOFF 0
#define ALARM_OFF 0
#define ALARM_DURATION_ON 1
#define ALARM_DURATION_OFF 2

typedef struct{
  uint8_t pin;
  char name[12];
  uint8_t status;
  unsigned long last;
  long durationON;
  long durationOFF;
  unsigned long lastReset;
  long durationThisPeriod;
  long durationLastPeriod;
  int alarm;
  unsigned int alarmDurationOn;
  unsigned int alarmDurationOff;
} pumpInfo_t, *pPumpInfo;

#define pumpIsRunning(pump) (pump->status)
#define pumpGetName(pump) (pump->name)

pPumpInfo pumpInit(pPumpInfo p, char *name, uint8_t pin, unsigned int alarmDurationOn, unsigned int alarmDurationOff);

int pumpUpdate(pPumpInfo pump);
long pumpGetCurrentStateDuration(pPumpInfo pump);
void pumpSetAlarmDurations(pPumpInfo pump, long alarmDurationOn, long alarmDurationOff);
void pumpResetPeriod(pPumpInfo pump);
char *pumpGetAlarmMsg(pPumpInfo pump, char *msg, size_t len);


#endif