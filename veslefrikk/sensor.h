#ifndef SENSOR_H
#define SENSOR_H

#define TEMP_1_CODE 2
#define TEMP_2_CODE 3
#define TEMP_3_CODE 4
#define TEMP_4_CODE 5
#define POWER_CODE 6
#define BATTERY_1_CODE 7
#define BATTERY_2_CODE 8
#define BILGE_1_CODE 9
#define BILGE_2_CODE 10
#define LEVEL_1_CODE 11
#define LEVEL_2_CODE 12

//SENSOR PINOUT
#define TEMP_BUS 13
#define BATTERY_1 8
#define BATTERY_2 9
#define BILGE_1 12
#define BILGE_2 13
#define POWER 14

//Sampling period defined in seconds
#define TEMP_INTERVAL 	10
#define POWER_INTERVAL	10000
#define BATTERY_INTERVAL 60
//#define BILGE_INTERVAL 300 CST commented out
#define BILGE_INTERVAL 30
#define LEVEL_INTERVAL 10

#define TEMP_MEAN_COUNT 30
#define LEVEL_MEAN_COUNT 30

//Define how often we will send data to server in seconds
#define SEND_INTERVAL 900

#include "OneWire.h"
#include "DallasTemperature.h"

int ipow(int base, int exp);

extern DeviceAddress Probe1; 
extern DeviceAddress Probe2;
extern DeviceAddress Probe3; 
extern DeviceAddress Probe4;

extern OneWire oneWire;
extern DallasTemperature sensors;

#endif
