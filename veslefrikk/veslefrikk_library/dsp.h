#ifndef DSP_H
#define DSP_H

#define TEMP_1_MIN	-20
#define TEMP_2_MIN	-20
#define TEMP_3_MIN	-20
#define TEMP_4_MIN	-20

#define TEMP_1_MAX	40
#define TEMP_2_MAX	40
#define TEMP_3_MAX	40
#define TEMP_4_MAX	40

#define BATTERY_1_MIN 12
#define BATTERY_2_MIN 12

#define LEVEL_1_MAX 600
#define LEVEL_2_MAX 600

//CST
#define PUMPON 1
#define PUMPOFF 0

#include <Arduino.h>
#include <math.h>

int takeMeanValue(int sensorVec[], const int length);
void swap(int *sensorVec, int p1, int p2);
void sort(int *sensorVec, const int length);
double takeMedian(int *sensorVec, const int length);
void tempWarning(int indoorTemp, int outdoorTemp);
float average(float numbers[], int size);
const char *byte_to_binary(int x);
void samplePump(int *pumpBuff, int sample);
void analyzePump(int *pumpBuff, float *waterLevelBuff);

// Counters og orden til buffere. P står for pump og WL står for water level.
extern const int ORDER_P;
extern const int ORDER_WL;
extern int countP;
extern int countWL;

extern int critPumpOn1;
extern int critPumpOn2;
extern int critPumpOff1;
extern int critPumpOff2;
extern int critCount;
extern int riseCount;
// Alarm er et flagg som settes dersom noe er galt. En feilmelding blir lagret i msg. 
//Når denne feilmeldingen er sendt settes flagget msgSent slik at meldingen ikke blir sent flere ganger på rad.
extern bool alarm;
extern bool msgSent;

extern int tempMotor;
extern int tempDass;
extern int tempStyr;
extern int tempOut;

extern int pumpBuff_1[10];
extern int pumpBuff_2[10];

extern int waterlevelBuff_1[64];
extern int waterlevelBuff_2[64];

#endif 
