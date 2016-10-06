#include <Time.h>
#include <avr/wdt.h>
#include <dsp.h>

#include "HardwareLink3.h"
#include "alarm.h"
#include "sensor.h"
#include "setup.h"
#include "waterlevel.h"

#include "pump.h"
#include "battery.h"
#include "temperature1wire.h"

///////////////////////
#include "SIM900.h"
#include <sms.h>


// Modem stuff
SMSGSM sms;

int numdata;
boolean started=false;
char smsbuffer[160];
char n[20];

char sms_nr;
char phone_number[20]; // array for the phone number string
char sms_text[160];
int i;
// end Modem stuff

// Battery stuff
#define BATTERY12V_PIN 8 //8
#define BATTERY24V_PIN 9 //9
pBatteryInfo battery12V;
pBatteryInfo battery24V;

// Pump stuff
#define PUMPENGINE_PIN 11
#define PUMPAFT_PIN 10
pPumpInfo pumpEngine;
pPumpInfo pumpAft;

// Temperature stuff
pTemperatureInfo pTIbow;
pTemperatureInfo pTIengine;
pTemperatureInfo pTIaft;
pTemperatureInfo pTIout;
DeviceAddress daBow    = { 0x28, 0x0A, 0xC5, 0x4F, 0x07, 0x00, 0x00, 0x21 }; 
DeviceAddress daEngine = { 0x28, 0x57, 0x5A, 0x50, 0x07, 0x00, 0x00, 0x3F };
DeviceAddress daAft    = { 0x28, 0xA2, 0x2B, 0x4B, 0x07, 0x00, 0x00, 0xAA }; 
DeviceAddress daOut    = { 0x28, 0x91, 0xCF, 0x50, 0x07, 0x00, 0x00, 0x77 };

// Boat owner: 
bool new_temp = false;
bool new_power = false;
bool new_battery = false;
bool new_bilge = false;
bool new_level = false; 
bool send_data = false; 

unsigned int data_counter = 19;  
unsigned int temp_counter = 0;
unsigned int temp_mean_counter = 0;
unsigned int battery_counter = 0;
unsigned int bilge_counter_1 = 0;
unsigned int bilge_counter_2 = 0;
unsigned int bilge_array_counter = 0;
unsigned int level_counter = 0;
unsigned int level_mean_counter = 0;
unsigned int power_counter = 0;
uint16_t temp_sec = 0;
volatile unsigned long seconds = 0;

uint8_t bilge_buffer_1 = 0;
uint8_t bilge_buffer_2 = 0;
uint8_t data[1024] = {
}; 

uint16_t ts_bilge_1[128] = {};
uint16_t ts_bilge_2[128] = {};
uint16_t ts_power[64] = {};

int16_t temp1_raw[30] = {};
int16_t temp2_raw[30] = {};
int16_t temp3_raw[30] = {};
int16_t temp4_raw[30] = {};

uint8_t temp1_mean[3] = {};
uint8_t temp2_mean[3] = {};
uint8_t temp3_mean[3] = {};
uint8_t temp4_mean[3] = {};

uint8_t shorepower_raw = 0;
uint8_t shorepower[256] = {};

uint16_t battery_1[3] = {};
uint16_t battery_2[3] = {};

uint8_t bilge_1[128] = {};
uint8_t bilge_2[128] = {};

float level_1[31] = {};
float level_2[31] = {};
float level_1_mean[3] = {};
float level_2_mean[3] = {};

typedef union{
  float fl;		
  uint8_t bytes[4];
} FLOATUNION_t;

typedef union{
  uint16_t uint;
  uint8_t bytes[2];
} INTUNION_t;


void setup(){ 
  //initSystem();
  Serial.begin(57600);
  Serial.println("********************");
  Serial.println("* Booting Båtvakta *");
  Serial.println("********************");

  pinMode(8, OUTPUT); //Pin 8 - PWRKEY
  pinMode(9, OUTPUT); //Pin 9 - RESTART
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED0, HIGH); // red on
  digitalWrite(LED1, LOW); // green off
  //  Serial.begin(57600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  
  //For http uses is reccomanded to use 4800 or slower.
  if (gsm.begin(4800)){
    Serial.println("\nstatus=READY");
    started=true;
  } else 
    Serial.println("\nstatus=IDLE");

  ///////////////////////////////////

  //Serial.println("initSensors()");
  //delay(500);
  //initSensors();
  //delay(500);

  /***************** conversion latin-1 to gsm *************
  //   http://www.developershome.com/sms/gsmAlphabet.asp
  ********************************************************/

  // temperatures
  pTIbow = temperatureAddTemperatureProbe(daBow, "Lugar", 0, 40, TEMP_9_BIT);
  pTIengine = temperatureAddTemperatureProbe(daEngine, "Maskinrom", 0, 50, TEMP_9_BIT);
  pTIaft = temperatureAddTemperatureProbe(daAft, "Akterlugar", 0, 40, TEMP_9_BIT);
  pTIout = temperatureAddTemperatureProbe(daOut, "Ute", 0, 40, TEMP_9_BIT);

  // the pumps
  // More than 5 minutes ON and more than 30 minutes OFF will give an alarm
  pumpEngine = pumpInit(NULL, "Maskinrom", PUMPENGINE_PIN, 5*60*1000, 30*60*1000);
  pumpAft = pumpInit(NULL, "Akterlugar", PUMPAFT_PIN, 5*60*1000, 30*60*1000);

  // batteries
  battery12V = batteryInit(NULL, "12V Batteri", BATTERY12V_PIN, (13.12/844.0), 12);
  battery24V = batteryInit(NULL, "24V Batteri", BATTERY24V_PIN, (27.51/932.0), 24);

  //  Serial.println("initTimer()");
  //  delay(500);
  //  initTimer();
  //  delay(500);
  //  Serial.println("initTimer() done");
  for(int i = 0; i < 15; i++){                   
    data[i] = IMEI[i];
  }
  
  if(started) {
    if (sms.SendSMS("93636390", "Baatvakta SMS version on Veslefrikk started")) 
      Serial.println("\nSMS sent OK"); 
  }
  
  digitalWrite(LED0, LOW); // red led off
  digitalWrite(LED1, HIGH); // green led on
}

uint32_t transmit_counter = 0;
uint32_t reboot_limit = 100;
void loop(){ 
  char msg[160];
  char voltage12S[7] = "";
  char voltage24S[7] = "";

  pumpUpdate(pumpEngine);
  pumpUpdate(pumpAft);

  batteryUpdate(battery12V);
  batteryUpdate(battery24V);

  temperaturesUpdate();

  readLevel();
  
  if(started){
    //Read if there are unread messages on the SIM card
    sms_nr=sms.IsSMSPresent(SMS_UNREAD);
    if (sms_nr){
      // read new SMS
      Serial.print("SMS msg number: ");
      Serial.println(sms_nr,DEC);
      // parse the sms
      sms.GetSMS(sms_nr, phone_number, sms_text, 100);
      
      Serial.println(phone_number);
      Serial.println(sms_text);

      if (sms_text[0] == 'P' || sms_text[0] == 'p'){
	snprintf(msg, sizeof(msg), 
		 "Pumper\n"
		 "%s %s for %d sec\n" 
		 " last on %d sec\n"  
		 " last off %d sec\n"
		 "%s %s for %d sec \n"
		 " last on %d sec\n"
		 " last off %d sec\n",
		 pumpEngine->name,
		 ((pumpEngine->status == PUMPON)? "ON" : "OFF"),
		 (int) (pumpGetCurrentStateDuration(pumpEngine) / 1000.0),
		 (int) (pumpEngine->durationON/1000),
		 (int) (pumpEngine->durationOFF/1000),
		 pumpAft->name,
		 ((pumpAft->status == PUMPON)? "ON" : "OFF"),
		 (int) (pumpGetCurrentStateDuration(pumpAft) / 1000.0),
		 (int) (pumpAft->durationON/1000.0),
		 (int) (pumpAft->durationOFF/1000.0)
		 );
      }else if (sms_text[0] == 'T' || sms_text[0] == 't'){
	snprintf(msg, sizeof(msg), 
		 "Temperaturer:\n"
		 " %s: %+3d\n"
		 " %s: %+3d\n"
		 " %s: %+3d\n"
		 " %s: %+3d\n",
		 pTIbow->name, (int) temperatureGetTempC(pTIbow),
		 pTIengine->name, (int) temperatureGetTempC(pTIengine),
		 pTIaft->name, (int) temperatureGetTempC(pTIaft),
		 pTIout->name, (int) temperatureGetTempC(pTIout)
		 );
      }else if (sms_text[0] == 'B' || sms_text[0] == 'b'){
	snprintf(msg, sizeof(msg),
		 "Batterier\n"
		 " %s: %s\n"
		 " %s: %s\n",
		 battery12V->name, batteryGetVoltageAsString(battery12V, voltage12S),
		 battery24V->name, batteryGetVoltageAsString(battery24V, voltage24S)
		 );
      }else{
	snprintf(msg, sizeof(msg), "Nice try ... ;-)\n"
		 "Send\nT for temperatures\n"
		 "P for pump info or\n"
		 "B for battery info\n\nRegards\n  Baatvakta Veslefrikk");
      }
#ifdef DEBUG
      Serial.println(msg);
#endif
      if (sms.SendSMS(phone_number, msg)) 
       	Serial.println("\nSMS sent OK"); 
      
      // Delete the incoming sms
      sms.DeleteSMS(sms_nr);
      
    } else {
      Serial.println("Having a break");
    }     
  }
  delay(1000);
}

// Called every second. Timer is set up in setup.cpp, initTimer()
ISR(TIMER1_COMPA_vect){
  seconds++;
  if(seconds%TEMP_INTERVAL == 0){
    new_temp = true;
  }
  if(seconds%POWER_INTERVAL == 0){
    new_power = true;
  }
  if(seconds%BATTERY_INTERVAL == 0){
    new_battery = true;
  }
  if(seconds%BILGE_INTERVAL == 0){
    new_bilge = true;
  }
  if(seconds%LEVEL_INTERVAL == 0){
    new_level = true;
  }
  if(seconds > SEND_INTERVAL){
    send_data = true;
  }
}

void sampleTemperatures()
{
  sensors.requestTemperatures(); 
  temp1_raw[temp_counter] = sensors.getTempC(Probe1);
  temp2_raw[temp_counter] = sensors.getTempC(Probe2);
  temp3_raw[temp_counter] = sensors.getTempC(Probe3);
  temp4_raw[temp_counter] = sensors.getTempC(Probe4);

  Serial.println("");
  Serial.print("Temperature 1: ");
  Serial.println(temp1_raw[temp_counter]);
  Serial.print("Temperature 2: ");
  Serial.println(temp2_raw[temp_counter]); 
  Serial.print("Temperature 3: ");
  Serial.println(temp3_raw[temp_counter]);
  Serial.print("Temperature 4: ");
  Serial.println(temp4_raw[temp_counter]);  
  temp_counter++;

  if(temp_counter >= TEMP_MEAN_COUNT){
    Serial.println("Calculating mean temperatures... ");
    temp1_mean[temp_mean_counter] = takeMeanValue(temp1_raw, temp_counter);
    temp2_mean[temp_mean_counter] = takeMeanValue(temp2_raw, temp_counter);
    temp3_mean[temp_mean_counter] = takeMeanValue(temp3_raw, temp_counter);
    temp4_mean[temp_mean_counter] = takeMeanValue(temp4_raw, temp_counter);
    Serial.print("Mean value of Temperature 1: ");
    Serial.println(temp1_mean[temp_mean_counter]);
    Serial.print("Mean value of Temperature 2: ");
    Serial.println(temp2_mean[temp_mean_counter]);
    Serial.print("Mean value of Temperature 3: ");
    Serial.println(temp3_mean[temp_mean_counter]);
    Serial.print("Mean value of Temperature 4: ");
    Serial.println(temp4_mean[temp_mean_counter]);

    //checkTemp();

    temp_mean_counter++;
    temp_counter = 0;
  }
}

void readShorePower()
{
  if(battery_1[battery_counter] > 14){
    shorepower_raw = 1;
  }
  if(battery_1[battery_counter] < 14){
    shorepower_raw = 0;
  }

  if(shorepower_raw == 0){
    ts_power[power_counter] = seconds;
    power_counter++;
  }
}

void readBattery()
{ 
  battery_1[battery_counter] = analogRead(BATTERY_1);
  battery_2[battery_counter] = analogRead(BATTERY_2);
  Serial.print("Battery 1: ");
  Serial.println(battery_1[battery_counter]);
  Serial.print("Battery 2: ");
  Serial.println(battery_2[battery_counter]);

  checkBattery();

  battery_counter++;
}

void readBilge()
{
  bilge_1_raw = analogRead(BILGE_1);
  bilge_2_raw = analogRead(BILGE_2);

  bilge_buffer_1 = (bilge_1_raw > 512)? PUMPON : PUMPOFF;

  samplePump(pumpBuff_1, bilge_buffer_1);

  if(bilge_state_1 != bilge_buffer_1){

    bilge_state_1 = bilge_buffer_1;
    if(bilge_state_1){
      temp_sec = seconds;
      temp_sec |= ( 1 << 15);
    }
    else{
      temp_sec = seconds;
      temp_sec &= ~(1 << 15);
    }
    ts_bilge_1[bilge_counter_1] = temp_sec;
    if(bilge_buffer_1 == PUMPON){
      Serial.print("Bilge 1 turned on after: ");
      Serial.println(seconds); 
    }
    if(bilge_buffer_1 == PUMPOFF){
      Serial.print("Bilge 1 turned off after: ");
      Serial.println(seconds); 
    }
    bilge_counter_1++;
  }

  bilge_buffer_2 = (bilge_2_raw > 512)? PUMPON : PUMPOFF;

  //samplePump(pumpBuff_2, bilge_buffer_2);

  if(bilge_state_2 != bilge_buffer_2){
    bilge_state_2 = bilge_buffer_2;

    if(bilge_state_2){
      temp_sec = seconds;
      temp_sec |= ( 1 << 15);
    }
    else{
      temp_sec = seconds;
      temp_sec &= ~(1 << 15);  
    }
    ts_bilge_2[bilge_counter_2] = temp_sec;
    if(bilge_buffer_2 == 1){
      Serial.print("Bilge 2 turned on after: ");
      Serial.println(seconds); 
    }
    if(bilge_buffer_2 == 0){
      Serial.print("Bilge 2 turned off after: ");
      Serial.println(seconds); 
    }
    bilge_counter_2++;
  }
}

void readLevel()
{
  level_1[level_counter] = readLevel_1(); //* 0.2874; // - 0.5977;
  Serial.print("Water Level 1: ");
  Serial.println(level_1[level_counter]);

  level_2[level_counter] = readLevel_2(); //* 0.6214; //- 0.5513;
  Serial.print("Water Level 2: ");
  Serial.println(level_2[level_counter]);

  //analyzePump(pumpBuff_1, level_1);
  //analyzePump(pumpBuff_2, level_2);

  level_counter++;

  if(level_counter >= LEVEL_MEAN_COUNT){
    level_1_mean[level_mean_counter] = average(level_1, level_counter);
    Serial.print("Water Level 1 mean: ");
    Serial.println(level_1_mean[level_mean_counter]);

    level_2_mean[level_mean_counter] = average(level_2, level_counter);
    Serial.print("Water Level 2 mean: ");
    Serial.println(level_2_mean[level_mean_counter]);

    checkLevel();

    level_mean_counter++;
    level_counter=0;
  }  
}

void packData()
{
  Serial.println("Packing Data... ");

  for(int i=0; i<temp_mean_counter; i++)
    {
      data[data_counter] = TEMP_1_CODE;
      data_counter++;
      data[data_counter] = temp1_mean[i];
      data_counter++;

      data[data_counter] = TEMP_2_CODE;
      data_counter++;
      data[data_counter] = temp2_mean[i];
      data_counter++;

      data[data_counter] = TEMP_3_CODE;
      data_counter++;
      data[data_counter] = temp3_mean[i];
      data_counter++;

      data[data_counter] = TEMP_4_CODE;
      data_counter++;
      data[data_counter] = temp4_mean[i];
      data_counter++;
    }

  for(int i=0; i<battery_counter; i++)
    {
      data[data_counter] = BATTERY_1_CODE;
      data_counter++;
      data[data_counter] = (battery_1[i] >> 8) & 0xFF;
      data_counter++;
      data[data_counter] = battery_1[i] & 0xFF;
      data_counter++;

      data[data_counter] = BATTERY_2_CODE;
      data_counter++;
      data[data_counter] = (battery_2[i] >> 8) & 0xFF;
      data_counter++;
      data[data_counter] = battery_2[i] & 0xFF;
      data_counter++;
    }

  INTUNION_t bilge_1;
  INTUNION_t bilge_2;

  for(int i=0; i<bilge_counter_1; i++)
    {
      data[data_counter] = BILGE_1_CODE;
      data_counter++;

      bilge_1.uint = ts_bilge_1[i];

      data[data_counter] = bilge_1.bytes[1];
      data_counter++;

      data[data_counter] = bilge_1.bytes[0];
      data_counter++;
    }

  for(int i=0; i<bilge_counter_2; i++)
    {
      data[data_counter] = BILGE_2_CODE;
      data_counter++;

      bilge_2.uint = ts_bilge_2[i];

      data[data_counter] = bilge_2.bytes[1];
      data_counter++;

      data[data_counter] = bilge_2.bytes[0];
      data_counter++;
    }

  FLOATUNION_t waterlevel_1;
  FLOATUNION_t waterlevel_2;

  for(int i=0; i<level_mean_counter; i++) {
    data[data_counter] = LEVEL_1_CODE;
    data_counter++;
    waterlevel_1.fl = level_1_mean[i]; 
    data[data_counter] = waterlevel_1.bytes[0];
    data_counter++;
    data[data_counter] = waterlevel_1.bytes[1];
    data_counter++;
    data[data_counter] = waterlevel_1.bytes[2];
    data_counter++;
    data[data_counter] = waterlevel_1.bytes[3];
    data_counter++;
    
    data[data_counter] = LEVEL_2_CODE;
    data_counter++;
    waterlevel_2.fl = level_2_mean[i]; 
    data[data_counter] = waterlevel_2.bytes[0];
    data_counter++;
    data[data_counter] = waterlevel_2.bytes[1];
    data_counter++;
    data[data_counter] = waterlevel_2.bytes[2];
    data_counter++;
    data[data_counter] = waterlevel_2.bytes[3];
    data_counter++;
  }

  INTUNION_t shorepower;

  for(int i=0; i<power_counter; i++){
    shorepower.uint = ts_power[i];
    data[data_counter] = POWER_CODE;
    data_counter++;
    data[data_counter] = shorepower.bytes[1];
    data_counter++;
    data[data_counter] = shorepower.bytes[0];
    data_counter++;
  }
}

void checkTemp()
{
  if(temp1_mean[temp_mean_counter] > TEMP_1_MAX)
    {
      alert_TEMP_1_HIGH = true;
      send_SMS(alarm_TEMP_1_HIGH, temp1_mean[temp_mean_counter]);
    }
  else
    {
      alert_TEMP_1_HIGH = false;
    }
  if(temp2_mean[temp_mean_counter] > TEMP_2_MAX)
    {
      alert_TEMP_2_HIGH = true;
      send_SMS(alarm_TEMP_2_HIGH, temp2_mean[temp_mean_counter]);
    }
  else
    {
      alert_TEMP_2_HIGH = false;
    }

  if(temp3_mean[temp_mean_counter] > TEMP_3_MAX)
    {
      alert_TEMP_3_HIGH = true;
      send_SMS(alarm_TEMP_3_HIGH, temp3_mean[temp_mean_counter]);
    }
  else
    {
      alert_TEMP_3_HIGH = false;
    }

  if(temp4_mean[temp_mean_counter] > TEMP_4_MAX)
    {
      alert_TEMP_4_HIGH = true;
      send_SMS(alarm_TEMP_4_HIGH, temp4_mean[temp_mean_counter]);
    }
  else
    {
      alert_TEMP_4_HIGH = false;
    }
  if(temp1_mean[temp_mean_counter] < TEMP_1_MIN)
    {
      alert_TEMP_1_LOW = true;
      send_SMS(alarm_TEMP_1_LOW, temp1_mean[temp_mean_counter]);
    }
  else
    {
      alert_TEMP_1_LOW = false;
    }
  if(temp2_mean[temp_mean_counter] < TEMP_2_MIN)
    {
      alert_TEMP_2_LOW = true;
      send_SMS(alarm_TEMP_2_LOW, temp2_mean[temp_mean_counter]);
    }
  {
    alert_TEMP_2_LOW = false;
  }
  if(temp3_mean[temp_mean_counter] < TEMP_3_MIN)
    {
      alert_TEMP_3_LOW = true;
      send_SMS(alarm_TEMP_3_LOW, temp3_mean[temp_mean_counter]);
    }
  {
    alert_TEMP_3_LOW = false;
  }
  if(temp4_mean[temp_mean_counter] < TEMP_4_MIN)
    {
      alert_TEMP_4_LOW = true;
      send_SMS(alarm_TEMP_4_LOW, temp4_mean[temp_mean_counter]);
    }
  {
    alert_TEMP_4_LOW = false;
  }
}

void checkBattery()
{
  if(battery_1[battery_counter] < BATTERY_1_MIN)
    {
      alert_BATTERY_1 = true;
      send_SMS(alarm_BATTERY_1, battery_1[battery_counter]);
    }
  else
    {
      alert_BATTERY_1 = false;
    }

  if(battery_2[battery_counter] < BATTERY_2_MIN)
    {
      alert_BATTERY_1 = false;
      send_SMS(alarm_BATTERY_2, battery_2[battery_counter]);
    }
  else
    {
      alert_BATTERY_2 = false;
    }
}

void checkLevel()
{
  if(level_1_mean[level_mean_counter] > LEVEL_1_MAX)
    {
      alert_LEVEL_1 = true;
      send_SMS(alarm_LEVEL_1, level_1[level_counter]);
    }
  else
    {
      alert_LEVEL_1 = false;
    }

  if(level_2_mean[level_mean_counter] > LEVEL_2_MAX)
    {
      alert_LEVEL_2 = true;
      send_SMS(alarm_LEVEL_2, level_2[level_counter]);
    }
  else
    {
      alert_LEVEL_2 = false;
    }
}

void resetTimer(){
  data_counter = 19;
  temp_counter=0;
  temp_mean_counter = 0;
  battery_counter = 0;
  bilge_counter_1 = 0;
  bilge_counter_2 = 0;
  bilge_array_counter = 0;
  level_counter = 0;
  level_mean_counter = 0;
  power_counter = 0;
  seconds=0;
}

// Reboots the system using the watchdog
void reboot() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (true) {
  }
  wdt_disable();
}

