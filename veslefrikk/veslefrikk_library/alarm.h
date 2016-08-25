#ifndef ALARM_H
#define ALARM_H
#define ALARM_REPETITION 96

#include "../HardwareLink3.h"

extern char alarm_TEMP_1_HIGH[SMS_MAX];
extern char alarm_TEMP_2_HIGH[SMS_MAX];
extern char alarm_TEMP_3_HIGH[SMS_MAX];
extern char alarm_TEMP_4_HIGH[SMS_MAX];
extern char alarm_TEMP_1_LOW[SMS_MAX];
extern char alarm_TEMP_2_LOW[SMS_MAX];
extern char alarm_TEMP_3_LOW[SMS_MAX];
extern char alarm_TEMP_4_LOW[SMS_MAX];
extern char alarm_BILGE_1[SMS_MAX];
extern char alarm_BILGE_2[SMS_MAX];
extern char alarm_BATTERY_1[SMS_MAX];
extern char alarm_BATTERY_2[SMS_MAX];
extern char alarm_LEVEL_1[SMS_MAX];
extern char alarm_LEVEL_2[SMS_MAX];

extern bool alert_TEMP_1_HIGH;
extern bool alert_TEMP_2_HIGH;
extern bool alert_TEMP_3_HIGH;
extern bool alert_TEMP_4_HIGH;
extern bool alert_TEMP_1_LOW;
extern bool alert_TEMP_2_LOW;
extern bool alert_TEMP_3_LOW;
extern bool alert_TEMP_4_LOW;
extern bool alert_BILGE_1;
extern bool alert_BILGE_2;
extern bool alert_BATTERY_1;
extern bool alert_BATTERY_2;
extern bool alert_LEVEL_1;
extern bool alert_LEVEL_2;

extern uint8_t alert_t_TEMP_1_HIGH;
extern uint8_t alert_t_TEMP_2_HIGH;
extern uint8_t alert_t_TEMP_3_HIGH;
extern uint8_t alert_t_TEMP_4_HIGH;
extern uint8_t alert_t_TEMP_1_LOW;
extern uint8_t alert_t_TEMP_2_LOW;
extern uint8_t alert_t_TEMP_3_LOW;
extern uint8_t alert_t_TEMP_4_LOW;
extern uint8_t alert_t_BILGE_1;
extern uint8_t alert_t_BILGE_2;
extern uint8_t alert_t_BATTERY_1;
extern uint8_t alert_t_BATTERY_2;
extern uint8_t alert_t_LEVEL_1;
extern uint8_t alert_t_LEVEL_2;

void alarmTimer();
void alarmReset();
void send_SMS(char *msg, uint8_t val);

#endif