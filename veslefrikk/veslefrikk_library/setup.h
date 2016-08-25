#ifndef SETUP_H
#define SETUP_H

#define LED0 23 //GRØNN
#define LED1 24	//RØD

#include "../HardwareLink3.h"
#include "Arduino.h"
#include "sensor.h"

void initSensors();
void initSystem();
void initTimer();
void disableTimer();
void enableTimer();
void initModem();

extern uint16_t bilge_1_raw;
extern uint16_t bilge_2_raw; 

extern uint8_t bilge_state_1;
extern uint8_t bilge_state_2;



#endif