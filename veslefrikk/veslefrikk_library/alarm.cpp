#include "alarm.h"

String sms_msg = "";

char alarm_TEMP_1_HIGH[SMS_MAX] = "Det er blitt registrert unormalt høy temperatur i kabinen. Følgende temperatur er målt: ";
char alarm_TEMP_2_HIGH[SMS_MAX] = "Det er blitt registrert unormalt høy temperatur i maskinrom. Følgende temperatur er målt: ";
char alarm_TEMP_3_HIGH[SMS_MAX] = "Det er blitt registrert unormalt høy temperatur i lensepumperom. Følgende temperatur er målt: ";
char alarm_TEMP_4_HIGH[SMS_MAX] = "Det er blitt registrert unormalt høy temperatur utvendig på båten. Følgende temperatur er målt: ";
char alarm_TEMP_1_LOW[SMS_MAX] = "Det er blitt registrert unormalt lav temperatur i kabinen. Følgende temperatur er målt: ";
char alarm_TEMP_2_LOW[SMS_MAX] = "Det er blitt registrert unormalt lav temperatur i maskinrom. Følgende temperatur er målt: ";
char alarm_TEMP_3_LOW[SMS_MAX] = "Det er blitt registrert unormalt lav temperatur i lensepumperom. Følgende temperatur er målt: ";
char alarm_TEMP_4_LOW[SMS_MAX] = "Det er blitt registrert unormalt lav temperatur utvendig på båten. Følgende temperatur er målt: ";
char alarm_BILGE_1[SMS_MAX] = "Det er blitt registrert unormalt høy pumpeaktivitet på lensepumpe 1";
char alarm_BILGE_2[SMS_MAX] = "Det er blitt registrert unormalt høy pumpeaktivitet på lensepumpe 2";
char alarm_BATTERY_1[SMS_MAX] = "Det er blitt registrert kritisk lav spenning av batteri 1.";
char alarm_BATTERY_2[SMS_MAX] = "Det er blitt registrert kritisk lav spenning av batteri 2.";
char alarm_LEVEL_1[SMS_MAX] = "Det er blitt registrert farlig høy vannstand. Følgende vannstand er målt: ";
char alarm_LEVEL_2[SMS_MAX] = "Det er blitt registrert farlig høy vannstand. Følgende vannstand er målt: ";

bool alert_TEMP_1_HIGH = false;
bool alert_TEMP_2_HIGH = false;
bool alert_TEMP_3_HIGH = false;
bool alert_TEMP_4_HIGH = false;
bool alert_TEMP_1_LOW = false;
bool alert_TEMP_2_LOW = false;
bool alert_TEMP_3_LOW = false;
bool alert_TEMP_4_LOW = false;
bool alert_BILGE_1 = false;
bool alert_BILGE_2 = false;
bool alert_BATTERY_1 = false;
bool alert_BATTERY_2 = false;
bool alert_LEVEL_1 = false;
bool alert_LEVEL_2 = false;

uint8_t alert_t_TEMP_1_HIGH = 0;
uint8_t alert_t_TEMP_2_HIGH = 0;
uint8_t alert_t_TEMP_3_HIGH = 0;
uint8_t alert_t_TEMP_4_HIGH = 0;
uint8_t alert_t_TEMP_1_LOW = 0;
uint8_t alert_t_TEMP_2_LOW = 0;
uint8_t alert_t_TEMP_3_LOW = 0;
uint8_t alert_t_TEMP_4_LOW = 0;
uint8_t alert_t_BILGE_1 = 0;
uint8_t alert_t_BILGE_2 = 0;
uint8_t alert_t_BATTERY_1 = 0;
uint8_t alert_t_BATTERY_2 = 0;
uint8_t alert_t_LEVEL_1 = 0;
uint8_t alert_t_LEVEL_2 = 0;

void send_SMS(char *msg, uint8_t val)
{
	sms_msg = msg + val;
    sms_msg.toCharArray(msg, SMS_MAX);
    sendSMS(phone_1, msg);
    Serial.print("Sending SMS: ");
    Serial.println(sms_msg);
    sms_msg.remove(0,160);
}

void alarmTimer()
{
	if(alert_TEMP_1_HIGH == true)
	{
		alert_t_TEMP_1_HIGH++;
	}
	if(alert_TEMP_2_HIGH == true)
	{
		alert_t_TEMP_2_HIGH++;
	}
	if(alert_TEMP_3_HIGH == true)
	{
		alert_t_TEMP_3_HIGH++;
	}
	if(alert_TEMP_4_HIGH == true)
	{
		alert_t_TEMP_4_HIGH++;
	}
	if(alert_TEMP_1_LOW == true)
	{
		alert_t_TEMP_1_LOW++;
	}
	if(alert_TEMP_2_LOW == true)
	{
		alert_t_TEMP_2_LOW++;
	}
	if(alert_TEMP_3_LOW == true)
	{
		alert_t_TEMP_3_LOW++;
	}
	if(alert_TEMP_4_LOW == true)
	{
		alert_t_TEMP_4_LOW++;
	}
	if(alert_BATTERY_1 == true)
	{
		alert_t_BATTERY_1++;
	}
	if(alert_BATTERY_2 == true)
	{
		alert_t_BATTERY_2++;
	}
	if(alert_LEVEL_1 == true)
	{
		alert_t_LEVEL_1++;
	}
	if(alert_LEVEL_2 == true)
	{
		alert_t_LEVEL_2++;
	}
}

void alarmReset()
{
	if(alert_t_TEMP_1_HIGH > ALARM_REPETITION)
	{
		alert_TEMP_1_HIGH = false;
	}
	if(alert_t_TEMP_2_HIGH > ALARM_REPETITION)
	{
		alert_TEMP_2_HIGH = false;
	}
	if(alert_t_TEMP_3_HIGH > ALARM_REPETITION)
	{
		alert_TEMP_3_HIGH = false;
	}
	if(alert_t_TEMP_4_HIGH > ALARM_REPETITION)
	{
		alert_TEMP_4_HIGH = false;
	}	
	if(alert_t_TEMP_1_LOW > ALARM_REPETITION)
	{
		alert_TEMP_1_LOW = false;
	}
	if(alert_t_TEMP_2_LOW > ALARM_REPETITION)
	{
		alert_TEMP_2_LOW = false;
	}
	if(alert_t_TEMP_3_LOW > ALARM_REPETITION)
	{
		alert_TEMP_3_LOW = false;
	}
	if(alert_t_TEMP_4_LOW > ALARM_REPETITION)
	{
		alert_TEMP_4_LOW = false;
	}
	if(alert_t_BATTERY_1 > ALARM_REPETITION)
	{
		alert_BATTERY_1 = false;
	}
	if(alert_t_BATTERY_2 > ALARM_REPETITION)
	{
		alert_BATTERY_2 = false;
	}	
	if(alert_t_LEVEL_1 > ALARM_REPETITION)
	{
		alert_LEVEL_1 = false;
	}
	if(alert_t_LEVEL_2 > ALARM_REPETITION)
	{
		alert_LEVEL_2 = false;
	}
}
