#ifndef PUMP
#define PUMP

#include <Arduino.h>

#define PUMPON 1
#define PUMPOFF 0

//SENSOR PINOUT
#define PUMPENGINE_PIN 12
#define PUMPAFT_PIN 13

typedef struct{
  uint8_t pin;
  char *name;
  uint8_t status;
  uint32_t last;
  uint32_t durationON;
  uint32_t durationOFF;
} pumpInfo_t, *pPumpInfo;

#define isRunning(pump) (pump->status)
#define getName(pump) (pump->name)

void updatePump(pPumpInfo pump);

#endif
