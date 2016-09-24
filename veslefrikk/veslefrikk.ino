#include <CRC32.h>
#include <DallasTemperature.h>
#include <FDC1004_differential.h>
#include <OneWire.h>
#include <Time.h>
#include <Wire.h>
#include <avr/wdt.h>
#include <dsp.h>
#include <string.h>
#include <stdlib.h>

#include "HardwareLink3.h"
#include "alarm.h"
#include "sensor.h"
#include "setup.h"
#include "waterlevel.h"
#include "pump.h"


///////////////////////
#include "SIM900.h"
#include <SoftwareSerial.h>
#include <sms.h>


// Modem stuff
SMSGSM sms;

int numdata;
boolean started=false;
char smsbuffer[160];
char n[20];

char sms_position;
char phone_number[20]; // array for the phone number string
char sms_text[100];
int i;
// end Modem stuff
// Pump stuff

pumpInfo_t pumpEngine_t;
pumpInfo_t pumpAft_t;
pPumpInfo pumpEngine = &pumpEngine_t;
pPumpInfo pumpAft = &pumpAft_t;


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

uint16_t ts_bilge_1[128] = {
};
uint16_t ts_bilge_2[128] = {
};
uint16_t ts_power[64] = {
};

int16_t temp1_raw[30] = {
};
int16_t temp2_raw[30] = {
};
int16_t temp3_raw[30] = {
};
int16_t temp4_raw[30] = {
};

uint8_t temp1_mean[3] = {
};
uint8_t temp2_mean[3] = {
};
uint8_t temp3_mean[3] = {
};
uint8_t temp4_mean[3] = {
};
uint8_t shorepower_raw = 0;
uint8_t shorepower[256] = {
};
uint16_t battery_1[3] = {
};
uint16_t battery_2[3] = {
};
uint8_t bilge_1[128] = {
};
uint8_t bilge_2[128] = {
};
float level_1[31] = {
};
float level_2[31] = {
};
float level_1_mean[3] = {
};
float level_2_mean[3] = {
};

typedef union{
  float fl;		
  uint8_t bytes[4];
} 
  FLOATUNION_t;

typedef union
{
  uint16_t uint;
  uint8_t bytes[2];
} 
  INTUNION_t;

void setup()
{ 

  initSystem();
  // CST removed tcp/ip stuff  
  // initModem();

  //// instead we use this
  //SIM900Power();
//  sendSMS("93636390", "Båtvakt possibly starting");
  //setupSMSreception();
//  sendSMS("93636390", "Båtvakt started");
    //Serial connection.
    Serial.begin(9600);
    Serial.println("GSM Shield testing.");
    //Start configuration of shield with baudrate.
    //For http uses is raccomanded to use 4800 or slower.
    if (gsm.begin(4800)) 
    {
        Serial.println("\nstatus=READY");
        started=true;
    } 
    else 
        Serial.println("\nstatus=IDLE");




  ///////////////////////////////////

  Serial.println("initSensors()");
  delay(500);
  initSensors();
  delay(500);

  pumpEngine->name = "Maskinrom";
  pumpEngine->pin = PUMPENGINE_PIN;

  pumpAft->pin = PUMPAFT_PIN;
  pumpAft->name = "Akterlugar";

  //  Serial.println("initTimer()");
  //  delay(500);
  //  initTimer();
  //  delay(500);
  //  Serial.println("initTimer() done");
  for(int i = 0; i < 15; i++){                   
    data[i] = IMEI[i];
  }
  
  
  if(started) {
    if (sms.SendSMS("93636390", "Arduino SMS version on Veslefrikk started"))
      Serial.println("\nSMS sent OK");
    
    //if NO SPACE ,you need delete SMS  from position 1 to position 20
    //please enable this four lines
    for(i=1;i<=20;i++)
      {
	sms.DeleteSMS(i);
      }
  }
  
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, HIGH);
}



float analogRead12V(uint16_t pin){
  uint16_t raw = analogRead(pin);
  //Serial.print("analogRead12V=");
  //Serial.println(raw);
  // Voltage devider with R1=100kOhm, R2=47kOhom
  // Arduino uses 10bits (0..1023) for 5V.
  float r1 = 100;
  float r2 = 22;
  return ((float) raw * (5.0 * (r1+r2/r2)) / 1023.0);
}

float analogRead24V(uint16_t pin){
  uint16_t raw = analogRead(pin);
  float r1 = 100;
  float r2 = 22;
  //Serial.print("analogRead24V=");
  //Serial.println(raw);
  // Voltage devider with R1=100kOhm, R2=22kOhom
  // Arduino uses 10bits (0..1023) for 5V.
  return ((float) raw * (5.0 * (r1+r2)/r2) / 1023.0);
}


uint32_t transmit_counter = 0;
uint32_t reboot_limit = 100;
void loop()
{ 
  char msg[160];

  if(started){
    //Read if there are messages on SIM card and print them.
    sms_position=sms.IsSMSPresent(SMS_UNREAD);
    if (sms_position){
      // read new SMS
      Serial.print("SMS postion:");
      Serial.println(sms_position,DEC);
      sms.GetSMS(sms_position, phone_number, sms_text, 100);
      // now we have phone number string in phone_num
      Serial.println(phone_number);
      // and SMS text in sms_text
      Serial.println(sms_text);
      snprintf(msg, sizeof(msg), "Temps\n lugar: %+3d\n maskin: %+3d\n akter: %+3d\n ute: %+3d\n"
	       "Batteri\n 12V: %5.2f\n 24V: %5.2f\n" 
	       "Pump 12V is  %s\n", 
	       "  last on for %d sec\n"
	       "  last of for %d sec\n" 
	       "Pump 24V is  %s\n", 
	       "  last on for %dsec\n"
	       "  last of for %dsec\n", 
	       temp1_raw[temp_counter-1],
	       temp2_raw[temp_counter-1],
	       temp3_raw[temp_counter-1],
	       temp4_raw[temp_counter-1],
	       analogRead12V(BATTERY_1),
	       analogRead24V(BATTERY_2),
	       ((pumpEngine->status == PUMPON)? "ON" : "OFF"),
	       (int) (pumpEngine->durationON),
	       (int) (pumpEngine->durationOFF),
	       ((pumpAft->status == PUMPON)? "ON" : "OFF"),
	       (int) (pumpAft->durationON/1000.0),
	       (int) (pumpAft->durationOFF/1000.0)
	       );
      Serial.print(msg);
      //      if (sms.SendSMS(phone_number, msg))
      //	Serial.println("\nSMS sent OK");
    } else {
      Serial.println("NO NEW SMS,WAITTING");
    }     
    delay(1000);

    updatePump(pumpEngine);
    updatePump(pumpAft);
      
    sampleTemperatures();
    
    readShorePower();
    new_power = false;
    readBattery();
    
    //      readBilge();
    
    if (transmit_counter > reboot_limit) {
      reboot();
    }
  }
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

  //CST
  Serial.println("processing sms");
  processSMS(1, true);
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

  for(int i=0; i<level_mean_counter; i++)
    {
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

  for(int i=0; i<power_counter; i++)
    {
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

void resetTimer()
{
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

