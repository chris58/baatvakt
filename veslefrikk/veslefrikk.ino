#include <Time.h>
#include <avr/wdt.h>

//#include <dsp.h>
//#include "setup.h"
//#include "waterlevel.h"

#include <sms.h>
#include "pump.h"
#include "battery.h"
#include "temperature1wire.h"
#include "alarmhandler.h"

///////////////////////

// Modem stuff
const uint8_t IMEI[15] = {48,49,51,57,53,48,48,48,55,50,54,49,52,50,52};
SMSGSM sms;

int numdata;
char smsbuffer[160];
char n[20];
char msg[160];
int send_SMS = 1;

char sms_nr;
char phone_number[20]; // array for the phone number string
char sms_text[160];
int i;
// end Modem stuff

// Green and red LED
#define LED_RED 23 //
#define LED_GREEN 24	//


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
unsigned int level_counter = 0;
unsigned int level_mean_counter = 0;
uint16_t temp_sec = 0;
//////////
volatile unsigned long seconds = 0;
volatile bool checkSMS = false;
volatile bool doUpdate = false;
volatile bool sendData = false;

#define SMS_INTERVAL    5 //seconds
#define UPDATE_INTERVAL 2 //seconds
#define SEND_INTERVAL 600 //seconds


uint8_t data[1024] = {
}; 

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
  Serial.println("*********************");
  Serial.println("* Booting Baatvakta *");
  Serial.println("*********************");

  pinMode(8, OUTPUT); //Pin 8 - PWRKEY
  pinMode(9, OUTPUT); //Pin 9 - RESTART

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED, HIGH); // red on
  digitalWrite(LED_GREEN, LOW); // green off

  Serial1.begin(9600);
  Serial2.begin(9600);
  
  //For http uses it's reccomanded to use 4800 or slower.
  gsm.begin(4800); // Serial3 in our case

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
  // pumpEngine = pumpInit(NULL, "Maskinrom", PUMPENGINE_PIN, 5*60*1000, 30*60*1000);
  unsigned long aon = 5L*60L*1000L;
  unsigned long aoff = 30L*60L*1000L;
  pumpEngine = pumpInit(NULL, "Maskinrom", PUMPENGINE_PIN, aon, aoff);
  pumpAft = pumpInit(NULL, "Akterlugar", PUMPAFT_PIN, aon, aoff);

  // batteries
  battery12V = batteryInit(NULL, "12V Batteri", BATTERY12V_PIN, (13.12/844.0), 12);
  battery24V = batteryInit(NULL, "24V Batteri", BATTERY24V_PIN, (27.51/932.0), 24);

  Serial.println("initTimer()");
  delay(500);
  initTimer();
  delay(500);
  Serial.println("initTimer() done");
  for(int i = 0; i < 15; i++){                   
    data[i] = IMEI[i];
  }
  
  sms.SendSMS("93636390", "Baatvakta SMS version on Veslefrikk started");

  digitalWrite(LED_RED, LOW); // red led off
  digitalWrite(LED_GREEN, HIGH); // green led on
}

static unsigned long lastTimeSMSsendt = 0;

void loop(){ 
  char *txt;
  char voltage12S[7] = "";
  char voltage24S[7] = "";
  int alarmCode = ALARM_OFF;
  int alarmID;
  unsigned int don;
  unsigned int doff;

  if (doUpdate){
    doUpdate = false;
    // pump in engine room
    alarmCode = pumpUpdate(pumpEngine);
    handlePumpAlarm(pumpEngine, alarmCode);
    // pump in the aft
    alarmCode = pumpUpdate(pumpAft);
    handlePumpAlarm(pumpAft, alarmCode);

    // Battery 12V
    alarmCode = batteryUpdate(battery12V);
    handleBatteryAlarm(battery12V, alarmCode);
    // Battery 24V
    alarmCode = batteryUpdate(battery24V);
    handleBatteryAlarm(battery24V, alarmCode);

    temperaturesUpdate();
  }
  
  // 
  //readLevel();
  //
  if (sendData){
    pumpResetPeriod(pumpEngine);
    pumpResetPeriod(pumpAft);
    // send the data... ;-)
  }

  //Read if there are unread messages on the SIM card
  if (checkSMS){
    checkSMS = false;
    Serial.println("Checking SMS");
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
      }else if (sscanf(sms_text, "ACK %d", &alarmID) == 1){
	acknowledgeByIdAlarm(alarmID);
      }else if (sscanf(sms_text, "SMS %d", &send_SMS) == 1){
	snprintf(msg, sizeof(msg), "Sending of alarm SMS %s\n",
		 (send_SMS > 0) ? "enabled" : "disabled");
      }else if (sscanf(sms_text, "DONOFF %d %d", &don, &doff) == 2){
	snprintf(msg, sizeof(msg), "Setting\n"
		 "DurationOnAlarm=%d seconds\n"
		 "DurationOffAlarm=%d seconds\n",
		 don, doff);
	pumpSetAlarmDurations(pumpEngine, don, doff);
      }else{
	snprintf(msg, sizeof(msg), "Nice try ... ;-)\n"
		 "Send\n"
		 "T for temperatures\n"
		 "P for pump info or\n"
		 "B for battery info\n\nRegards\n  Baatvakta Veslefrikk");
      }
#ifdef DEBUG
      Serial.println(msg);
#endif
      // Return an SMS
      if (sms.SendSMS(phone_number, msg)) 
	Serial.println("\nSMS sent OK"); 
      
      // Delete the incoming sms
      sms.DeleteSMS(sms_nr);
      
    } else {
      Serial.println("Having a break");
    }     
  }
}

// Called every second 
ISR(TIMER1_COMPA_vect){
  seconds++;
  if(seconds%SMS_INTERVAL == 0){
    checkSMS = true;
  }
  if(seconds%UPDATE_INTERVAL == 0){
    doUpdate = true;
  }
  if(seconds > SEND_INTERVAL){
    sendData = true;
  }
}

void handlePumpAlarm(pPumpInfo pump, short alarmCode){
    if (alarmCode != ALARM_OFF){
      if (!isAcknowledgedAlarm(pump, alarmCode)){
	int id = addAlarm(pump, alarmCode);
	pumpGetAlarmMsg(pump, msg, sizeof(msg));
	snprintf(msg, sizeof(msg), "%s\n To acknowledge reply\nACK %d\n", msg, id);
	sendAlarmMsg(msg);
      }
    }else{
      // make sure that there is no alarm left in the alarm list
      int removeAlarm(pump, alarmCode);
    }
}

void handleBatteryAlarm(pBatteryInfo bat, short alarmCode){
    if (alarmCode != ALARM_OFF){
      if (!isAcknowledgedAlarm(bat, alarmCode)){
	int id = addAlarm(bat, alarmCode);
	batteryGetAlarmMsg(bat, msg, sizeof(msg));
	snprintf(msg, sizeof(msg), "%s\n To acknowledge reply\nACK %d\n", msg, id);
	sendAlarmMsg(msg);
      }
    }else{
      // make sure that there is no alarm left in the alarm list
      int removeAlarm(bat, alarmCode);
    }
}

void sendAlarmMsg(char *msg){
  if (send_SMS <= 0)
    return;
  if ((millis() - lastTimeSMSsendt) > 900000UL || lastTimeSMSsendt > millis()){
    sms.SendSMS("93636390", msg);
    lastTimeSMSsendt = millis();
  }
}



/* void readShorePower() */
/* { */
/*   if(battery_1[battery_counter] > 14){ */
/*     shorepower_raw = 1; */
/*   } */
/*   if(battery_1[battery_counter] < 14){ */
/*     shorepower_raw = 0; */
/*   } */

/*   if(shorepower_raw == 0){ */
/*     ts_power[power_counter] = seconds; */
/*     power_counter++; */
/*   } */
/* } */



/* void readLevel() */
/* { */
/*   level_1[level_counter] = readLevel_1(); //\* 0.2874; // - 0.5977; */
/*   Serial.print("Water Level 1: "); */
/*   Serial.println(level_1[level_counter]); */

/*   level_2[level_counter] = readLevel_2(); //\* 0.6214; //- 0.5513; */
/*   Serial.print("Water Level 2: "); */
/*   Serial.println(level_2[level_counter]); */

/*   //analyzePump(pumpBuff_1, level_1); */
/*   //analyzePump(pumpBuff_2, level_2); */

/*   level_counter++; */

/*   if(level_counter >= LEVEL_MEAN_COUNT){ */
/*     level_1_mean[level_mean_counter] = average(level_1, level_counter); */
/*     Serial.print("Water Level 1 mean: "); */
/*     Serial.println(level_1_mean[level_mean_counter]); */

/*     level_2_mean[level_mean_counter] = average(level_2, level_counter); */
/*     Serial.print("Water Level 2 mean: "); */
/*     Serial.println(level_2_mean[level_mean_counter]); */

/*     //    checkLevel(); */

/*     level_mean_counter++; */
/*     level_counter=0; */
/*   }   */
/* } */

/* void packData() */
/* { */
/*   Serial.println("Packing Data... "); */

/*   for(int i=0; i<temp_mean_counter; i++) */
/*     { */
/*       data[data_counter] = TEMP_1_CODE; */
/*       data_counter++; */
/*       data[data_counter] = temp1_mean[i]; */
/*       data_counter++; */

/*       data[data_counter] = TEMP_2_CODE; */
/*       data_counter++; */
/*       data[data_counter] = temp2_mean[i]; */
/*       data_counter++; */

/*       data[data_counter] = TEMP_3_CODE; */
/*       data_counter++; */
/*       data[data_counter] = temp3_mean[i]; */
/*       data_counter++; */

/*       data[data_counter] = TEMP_4_CODE; */
/*       data_counter++; */
/*       data[data_counter] = temp4_mean[i]; */
/*       data_counter++; */
/*     } */

/*   for(int i=0; i<battery_counter; i++) */
/*     { */
/*       data[data_counter] = BATTERY_1_CODE; */
/*       data_counter++; */
/*       data[data_counter] = (battery_1[i] >> 8) & 0xFF; */
/*       data_counter++; */
/*       data[data_counter] = battery_1[i] & 0xFF; */
/*       data_counter++; */

/*       data[data_counter] = BATTERY_2_CODE; */
/*       data_counter++; */
/*       data[data_counter] = (battery_2[i] >> 8) & 0xFF; */
/*       data_counter++; */
/*       data[data_counter] = battery_2[i] & 0xFF; */
/*       data_counter++; */
/*     } */

/*   INTUNION_t bilge_1; */
/*   INTUNION_t bilge_2; */

/*   for(int i=0; i<bilge_counter_1; i++) */
/*     { */
/*       data[data_counter] = BILGE_1_CODE; */
/*       data_counter++; */

/*       bilge_1.uint = ts_bilge_1[i]; */

/*       data[data_counter] = bilge_1.bytes[1]; */
/*       data_counter++; */

/*       data[data_counter] = bilge_1.bytes[0]; */
/*       data_counter++; */
/*     } */

/*   for(int i=0; i<bilge_counter_2; i++) */
/*     { */
/*       data[data_counter] = BILGE_2_CODE; */
/*       data_counter++; */

/*       bilge_2.uint = ts_bilge_2[i]; */

/*       data[data_counter] = bilge_2.bytes[1]; */
/*       data_counter++; */

/*       data[data_counter] = bilge_2.bytes[0]; */
/*       data_counter++; */
/*     } */

/*   FLOATUNION_t waterlevel_1; */
/*   FLOATUNION_t waterlevel_2; */

/*   for(int i=0; i<level_mean_counter; i++) { */
/*     data[data_counter] = LEVEL_1_CODE; */
/*     data_counter++; */
/*     waterlevel_1.fl = level_1_mean[i];  */
/*     data[data_counter] = waterlevel_1.bytes[0]; */
/*     data_counter++; */
/*     data[data_counter] = waterlevel_1.bytes[1]; */
/*     data_counter++; */
/*     data[data_counter] = waterlevel_1.bytes[2]; */
/*     data_counter++; */
/*     data[data_counter] = waterlevel_1.bytes[3]; */
/*     data_counter++; */
    
/*     data[data_counter] = LEVEL_2_CODE; */
/*     data_counter++; */
/*     waterlevel_2.fl = level_2_mean[i];  */
/*     data[data_counter] = waterlevel_2.bytes[0]; */
/*     data_counter++; */
/*     data[data_counter] = waterlevel_2.bytes[1]; */
/*     data_counter++; */
/*     data[data_counter] = waterlevel_2.bytes[2]; */
/*     data_counter++; */
/*     data[data_counter] = waterlevel_2.bytes[3]; */
/*     data_counter++; */
/*   } */

/*   INTUNION_t shorepower; */

/*   for(int i=0; i<power_counter; i++){ */
/*     shorepower.uint = ts_power[i]; */
/*     data[data_counter] = POWER_CODE; */
/*     data_counter++; */
/*     data[data_counter] = shorepower.bytes[1]; */
/*     data_counter++; */
/*     data[data_counter] = shorepower.bytes[0]; */
/*     data_counter++; */
/*   } */
/* } */

void resetTimer(){
  data_counter = 19;
  level_counter = 0;
  level_mean_counter = 0;
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

void initTimer()
{
  //Serial.println("Startup Complete. Starting timer...");
   //delay(1000);
  cli();            			
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  sei();  
}

// http://www.instructables.com/id/Arduino-Timer-Interrupts/
// see http://www.instructables.com/id/Arduino-Timer-Interrupts/step2/Structuring-Timer-Interrupts/
void initTimerChris(){
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  // "compare match register" = [ 16,000,000Hz/ (prescaler * "desired interrupt frequency") ] - 1
  OCR1A = 15624;// = (16*10^6) / (1024*1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}
