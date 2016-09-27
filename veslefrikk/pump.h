#ifndef PUMP
#define PUMP
#include <Arduino.h>

#define PUMPON 1
#define PUMPOFF 0


typedef struct{
  uint8_t pin;
  char name[12];
  uint8_t status;
  unsigned long last;
  long durationON;
  long durationOFF;
} pumpInfo_t, *pPumpInfo;

#define isRunning(pump) (pump->status)
#define getName(pump) (pump->name)

pPumpInfo initPump(pPumpInfo p, char *name, uint8_t pin);

void updatePump(pPumpInfo pump);
#endif
