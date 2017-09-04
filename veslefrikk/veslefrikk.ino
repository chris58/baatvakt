#include <Time.h>
#include <avr/wdt.h>
#include <GSMmega.h>
#include <sms.h>

#include "units.h" // includes pumps, temperatures and batteries
#include "alarmhandler.h"

//#define DEBUG_VESLEFRIKK
///////////////////////
/* Telia earlier known as "NetCom" supports GSM, GPRS, EDGE, UMTS, HSDPA and LTE technologies. */
/*     APN name: telia  */

/* Telenor supports GSM, GPRS, EDGE, UMTS and HSDPA technologies. */
/*     APN name: Telenor  */

/* Network Norway supports GSM, GPRS and EDGE technologies. */
/*     username: (blank) (or) wap */
/*     password: (blank) (or) wap */
/*     APN name: internet  */

/* My Call It is a Service Provider, it operates in Mobile Norway/Telenor network and supports GSM and UMTS technologies. */
/*     APN name: internet  */

/* One Call It is a Service Provider, it operates in Mobile Norway(Tele2)/Telenor network and supports GSM and UMTS technologies. */
/*     username: nwn (or) (blank) */
/*     password: nwn (or) (blank) */
/*     APN name: internet  */

/* Chess It is an MVNO, it operates in NetCom network and supports GSM and UMTS technologies. */
/*     username: chess */
/*     password: chess */
/*     APN name: netcom  */

/* djuice It is an MVNO operating in Telenor Mobil network and supporting GSM and UMTS technologies. */
/*     username: dj */
/*     password: dj */
/*     APN name: wap  */

/* Tele2 It is an MNO, it operates in Mobile Norway(Tele2 owned) network and has NRA(National roaming agreement) with the NetCom network, supports GSM and UMTS technologies. */
/*     APN name: internet.tele2.no (or) mobileinternet.tele2.no */
/*     MMS Settings: APN internet.tele2.no, MMSC: http://mmsc.tele2.no, Proxy: 193.12.40.14, Proxy port: 8080  */

/* Telio It is an MVNO operating in Telenor Mobil network and supporting GSM and UMTS technologies. */
/*     APN name: internet.ventelo.no  */

/* TalkMore It is an MVNO, it operates in Telenor network, supports GSM and UMTS technologies. */
/*     username: (blank) (or) telenor */
/*     password: (blank) (or) telenor */
/*     APN name: telenor  */

/* MobiTalk It is an MVNO, it operates in Telenor Mobil network, supports GSM and UMTS technologies. */
/*     username: DJ */
/*     password: DJ */
/*     APN name: telenor  */

/* Ventelo It is an MVNO, it operates in Telenor Mobil network, supports GSM and UMTS technologies. */
/*     APN name: internet.ventelo.no  */

/**
 * The block of data which is transfered to the server
 */
typedef struct{
  unsigned long time; /**< a timestamp in Unix format */
  // temperatures
  float tCabin;       /**< temperature in the cabin */
  float tEngine;      /**< temperature in the engine room */
  float tStern;       /**< stern cabin temperature */
  float tOutside;     /**< outside temperature */
  // battery voltages
  float voltage12;    /**< actual voltage of 12V batteries */
  float voltage24;    /**< actual voltage of 24V batteries */
  // pumping durations
  unsigned long pumpEngineDuration;
  unsigned long pumpSternDuration;
  // current alarm codes
  unsigned char alarmTCabin;
  unsigned char alarmTEngine;
  unsigned char alarmTStern;
  unsigned char alarmTOutside;
  unsigned char alarmVoltage12;
  unsigned char alarmVoltage24;
  unsigned char alarmPumpEngine;
  unsigned char alarmPumpStern;
} baatvaktData_t, *pBaatvaktData;

typedef union {
  baatvaktData_t baatvaktData;
  unsigned char bytes[sizeof(baatvaktData_t)];
} data_union;
 
// Info on how to send multipart sms messages...
// http://mobiletidings.com/2009/02/18/combining-sms-messages/
//

// Modem stuff
const uint8_t IMEI[15] = {48,49,51,57,53,48,48,48,55,50,54,49,52,50,52};
// SIM900A IMEI=013950007261424          TAC: 013950 FAC: 00 SNR: 726142 CD: 4
char msg[160];
int alarmSMSenabled = 1;
unsigned long alarmRepeatInterval = 3600UL; // seconds = 1 hour
SMSGSM sms;

// Green and red LED
#define LED_RED 23
#define LED_GREEN 24

// Battery stuff
#define BATTERY12V_PIN 8 //8
#define BATTERY24V_PIN 9 //9
pBatteryInfo pBattery12V;
pBatteryInfo pBattery24V;

// Pump stuff
#define PUMPENGINE_PIN 11
#define PUMPSTERN_PIN 10
pPumpInfo pPumpEngine;
pPumpInfo pPumpStern;

// Temperature stuff
pTemperatureInfo pTIcabin;
pTemperatureInfo pTIengine;
pTemperatureInfo pTIstern;
pTemperatureInfo pTIout;
DeviceAddress daCabin    = { 0x28, 0x0A, 0xC5, 0x4F, 0x07, 0x00, 0x00, 0x21 }; 
DeviceAddress daEngine = { 0x28, 0x57, 0x5A, 0x50, 0x07, 0x00, 0x00, 0x3F };
DeviceAddress daStern    = { 0x28, 0xA2, 0x2B, 0x4B, 0x07, 0x00, 0x00, 0xAA }; 
DeviceAddress daOut    = { 0x28, 0x91, 0xCF, 0x50, 0x07, 0x00, 0x00, 0x77 };

//////////
static volatile unsigned long seconds = 0;
static volatile bool checkSMS = false;
static volatile bool doUpdate = false;
static volatile bool sendData = false;
static volatile bool sendNMEA = false;

#define SMS_INTERVAL    5 //seconds
#define UPDATE_INTERVAL 2 //seconds
#define SEND_INTERVAL 600 //seconds = 10 minutes
#define NMEA_INTERVAL   2 //seconds

void setup(){ 
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

  // depth communication. Not used/working so far :-(
  Serial1.begin(9600);
  Serial2.begin(9600);
  
  //For http uses it's reccomanded to use 4800 or slower.
  gsm.begin(4800); // Serial3 in our case

  /* **************** conversion latin-1 to gsm *************
   *   http://www.developershome.com/sms/gsmAlphabet.asp
   ********************************************************/

  // temperatures
  temperatureInit(4); // init 
  pTIcabin = temperatureAddTemperatureProbe(daCabin, "Cabin", 0, 40, TEMP_9_BIT);
  pTIengine = temperatureAddTemperatureProbe(daEngine, "Engine", 0, 50, TEMP_9_BIT);
  pTIstern = temperatureAddTemperatureProbe(daStern, "Stern", 0, 40, TEMP_9_BIT);
  pTIout = temperatureAddTemperatureProbe(daOut, "Outside", 0, 40, TEMP_9_BIT);

  // the pumps
  // More than 5 minutes ON, less than 1 minute OFF and more than 60 minutes OFF will give an alarm
  unsigned long aOn = 5L*60L;
  unsigned long aLowOff = 60L;
  unsigned long aHighOff = 60L*60L;
 
  pPumpEngine = pumpInit(NULL, "Engine", PUMPENGINE_PIN, aOn, aLowOff, aHighOff);
  pPumpStern = pumpInit(NULL, "Stern", PUMPSTERN_PIN, aOn, aLowOff, aHighOff);

  // batteries
  pBattery12V = batteryInit(NULL, "12V Bat", BATTERY12V_PIN, (13.12/844.0), 12);
  pBattery24V = batteryInit(NULL, "24V Bat", BATTERY24V_PIN, (27.51/932.0), 24);

  initTimer();
  delay(500);
  
  sms.SendSMS("93636390", "Baatvakta on Veslefrikk started");

  digitalWrite(LED_RED, LOW); // red led off
  digitalWrite(LED_GREEN, HIGH); // green led on
}

// Last time an alarm message has been send
static unsigned long lastTimeSMSsendt = 0;
// has an alarm been send?
static int alarmSend = 0;
 
/**
 * Convert a string to upper case
 */
char *toUpper(char *s){
  int l = strlen(s);
  int i;
  for (i=0; i<l; i++)
    s[i] = toupper(s[i]);
  return s;
}

/**
 * The main loop
 */
void loop(){ 
  char voltage12S[7] = "";
  char voltage24S[7] = "";
  unsigned int don;
  unsigned int doff;
  unsigned int dLowOff;
  // modem
  char sms_text[160];
  char phoneNumber[20]; // array for the phone number string
  char phoneNumber2[20]; // array for another phone number string
  char sms_nr;
  float v;
  char vs[12];
  char *s;
  int alarmCode = ALARM_OFF;
  int alarmID;
  int onoff;

  data_union bdUnion;

  if (doUpdate){
    doUpdate = false;
    // pump in engine room
    alarmCode = pumpUpdate(pPumpEngine);
    if ((s = handleAlarm((pUnitInfo) pPumpEngine, alarmCode, msg, sizeof(msg))) != NULL)
      sendAlarmMsg(s);
    // pump in the stern
    alarmCode = pumpUpdate(pPumpStern);
    if ((s = handleAlarm((pUnitInfo) pPumpStern, alarmCode, msg, sizeof(msg))) != NULL)
      sendAlarmMsg(s);

    // Battery 12V
    alarmCode = batteryUpdate(pBattery12V);
    if ((s = handleAlarm((pUnitInfo) pBattery12V, alarmCode, msg, sizeof(msg))) != NULL){
      sendAlarmMsg(s);
    }
    // Battery 24V
    alarmCode = batteryUpdate(pBattery24V);
    if ((s = handleAlarm((pUnitInfo) pBattery24V, alarmCode, msg, sizeof(msg))) != NULL)
      sendAlarmMsg(s);

    // Temperatures
    temperaturesUpdate();
    pTemperatureInfo ti = NULL;
    while((ti = temperatureGetNextTempInfo(ti)) != NULL){ 
      if ((s = handleAlarm((pUnitInfo) ti, ti->alarmCode, msg, sizeof(msg))) != NULL)
	sendAlarmMsg(s);
    }
    if (alarmSend){
      lastTimeSMSsendt = getSeconds();
      alarmSend = 0;
    }
  }
  
  // NMEA
  if (sendNMEA){
    sendNMEA = false;
    Serial.println(pumpGetNMEA(2, pPumpEngine, pPumpStern));
    Serial.println(batteryGetNMEA(2, pBattery12V, pBattery24V));
    Serial.println(temperatureGetNMEA());
  }
  
  // send data to server
  if (sendData){
    sendData = false;
    // get timestamp
    bdUnion.baatvaktData.time = getUnixTime();

    // values
    bdUnion.baatvaktData.tCabin = temperatureGetTempC(pTIcabin);
    bdUnion.baatvaktData.tEngine = temperatureGetTempC(pTIengine);
    bdUnion.baatvaktData.tStern = temperatureGetTempC(pTIstern);
    bdUnion.baatvaktData.tOutside = temperatureGetTempC(pTIout);

    bdUnion.baatvaktData.voltage12 = batteryGetVoltage(pBattery12V);
    bdUnion.baatvaktData.voltage24 = batteryGetVoltage(pBattery24V);

    bdUnion.baatvaktData.pumpEngineDuration = pumpResetPeriod(pPumpEngine);
    bdUnion.baatvaktData.pumpSternDuration = pumpResetPeriod(pPumpStern);

    // Alarms
    bdUnion.baatvaktData.alarmTCabin = temperatureGetAlarmCode(pTIcabin);
    bdUnion.baatvaktData.alarmTEngine = temperatureGetAlarmCode(pTIengine);
    bdUnion.baatvaktData.alarmTStern = temperatureGetAlarmCode(pTIstern);
    bdUnion.baatvaktData.alarmTOutside = temperatureGetAlarmCode(pTIout);

    bdUnion.baatvaktData.alarmVoltage12 = batteryGetAlarmCode(pBattery12V);
    bdUnion.baatvaktData.alarmVoltage24 = batteryGetAlarmCode(pBattery24V);

    bdUnion.baatvaktData.alarmPumpEngine = pumpGetAlarm(pPumpEngine);
    bdUnion.baatvaktData.alarmPumpStern = pumpGetAlarm(pPumpStern);

    // send the data... ;-)
  }
  
  //Check whether there are unread messages on the SIM card
  if (checkSMS){
    checkSMS = false;
    sms_nr=sms.IsSMSPresent(SMS_UNREAD);
    if (sms_nr){
      // read new SMS
#ifdef DEBUG_VESLEFRIKK
      Serial.print("SMS msg number: ");
      Serial.println(sms_nr, DEC);
#endif
      // parse the sms
      sms.GetSMS(sms_nr, phoneNumber, sms_text, sizeof(sms_text));
#ifdef DEBUG_VESLEFRIKK
      Serial.println(phoneNumber);
      Serial.println(sms_text);
#endif
      toUpper(sms_text);
#ifdef DEBUG_VESLEFRIKK
      Serial.println(sms_text);
#endif
      
      s = msg;
      int len = sizeof(msg);
      // Request pump status
      if (sms_text[0] == 'P'){
	strcpy(s, "Pumps\n");
	s += strlen(s);
	len -= strlen(s);
	pumpGetStatusMsg(pPumpEngine, s, len);
	s += strlen(s);
	len -= strlen(s);
	pumpGetStatusMsg(pPumpStern, s, len);

	// Request temperatures
      }else if (sms_text[0] == 'T'){
	snprintf(msg, sizeof(msg), 
		 "Temperatures:\n"
		 " %s: %+3d\n"
		 " %s: %+3d\n"
		 " %s: %+3d\n"
		 " %s: %+3d\n",
		 pTIcabin->name, (int) temperatureGetTempC(pTIcabin),
		 pTIengine->name, (int) temperatureGetTempC(pTIengine),
		 pTIstern->name, (int) temperatureGetTempC(pTIstern),
		 pTIout->name, (int) temperatureGetTempC(pTIout)
		 );

	// Set low voltage alarm for 12V
      }else if (sscanf(sms_text, "BAT12 %s", vs) == 1){ 
	char *x = "";
	v = strtod(vs, &x);
      	snprintf(msg, sizeof(msg), "Setting\n" 
      		 "12V low voltage alarm = %sV\n",
      		 vs); 
       	batterySetLowVoltageAlarm(pBattery12V, v); 

	// Set low voltage alarm for 24V
      }else if (sscanf(sms_text, "BAT24 %s", vs) == 1){ 
	char *x = "";
	v = strtod(vs, &x);
      	snprintf(msg, sizeof(msg), "Setting\n" 
      		 "24V low voltage alarm = %sV\n",
      		 vs); 
       	batterySetLowVoltageAlarm(pBattery24V, v); 

	// Request battery status
      }else if (sms_text[0] == 'B'){
	snprintf(msg, sizeof(msg),
		 "Batteries\n"
		 " %s: %s\n"
		 " %s: %s\n",
		 pBattery12V->name, batteryGetVoltageAsString(pBattery12V, voltage12S),
		 pBattery24V->name, batteryGetVoltageAsString(pBattery24V, voltage24S)
		 );

	// Add phone number 
     }else if (sscanf(sms_text, "ADD %s", phoneNumber2) == 1){
	if (strcmp(phoneNumber, "93636390") == 0){
	  snprintf(msg, sizeof(msg), 
		   "Adding phone %s to list\n",
		   phoneNumber2);
	}else{
	  snprintf(msg, sizeof(msg), 
		   "Only root can do this\n");
	}
	
	// Remove phone number
      }else if (sscanf(sms_text, "REM %", phoneNumber2) == 1){
	if (strcmp(phoneNumber, "93636390") == 0){
	  snprintf(msg, sizeof(msg), 
		   "Removing phone %s from list\n",
		   phoneNumber2);
	}else{
	  snprintf(msg, sizeof(msg), 
		   "Only root can do this\n");
	}

	// Acknowledge alarm
      }else if (sscanf(sms_text, "ACK %d", &alarmID) == 1){
	alarmAcknowledgeById(alarmID);
	snprintf(msg, sizeof(msg), 
		 "Alarm %d acknowledged\n",
		 alarmID);

	// Set auto-acknowledge 
      }else if (sscanf(sms_text, "AUTOACK %d", &onoff) == 1){
	alarmSetAutoAcknowledge(onoff);
	snprintf(msg, sizeof(msg), 
		 "Auto-Acknowledge is now %s\n",
		 ((onoff)? "ON" : "OFF"));

	// Request alarm list
      }else if (sms_text[0] == 'A'){
	alarmGetActiveAlarmsAsString(msg, sizeof(msg));

	// Switch sms alarm on/off
      }else if (sscanf(sms_text, "SMS %d", &alarmSMSenabled) == 1){
	snprintf(msg, sizeof(msg), "Sending of alarm SMS %s\n",
		 (alarmSMSenabled > 0) ? "enabled" : "disabled");

	// Set duration for pump alarm (on and off)
      }else if (sscanf(sms_text, "DONOFF %d %d %d", &don, &dLowOff, &doff) == 3){
	snprintf(msg, sizeof(msg), "Setting\n"
		 "Duration ON Alarm=%d sec\n"
		 "Duration Off Alarm (low)=%d sec\n",
		 "Duration Off Alarm (high)=%d sec\n",
		 don, dLowOff, doff);
	pumpSetAlarmDurations(pPumpEngine, don, dLowOff, doff);
	pumpSetAlarmDurations(pPumpStern, don, dLowOff, doff);

	// Send 'help' sms
      }else{
	snprintf(msg, sizeof(msg), "Send\n"
		 "T for temps\n"
		 "P for pumps\n"
		 "B for batteries\n"
		 "A for active alarms\n"
		 "ACK n where n=alarm idx\n"
		 "SMS 0/1 to turn off/on SMS\n"
		 "DONOFF on-duration off-low-duration off-duration"
		 );
      }
#ifdef DEBUG_VESLEFRIKK
      Serial.println(msg);
#endif
      // Return an SMS
      sms.SendSMS(phoneNumber, msg);
      
      // Delete the incoming sms
      sms.DeleteSMS(sms_nr);
      
    }
#ifdef DEBUG_VESLEFRIKK
    else {
      Serial.println("Having a break");
    }     
#endif
  }
}

/*
 * Send an alarm message if alarmRepeatInterval seconds have past
 */
void sendAlarmMsg(char *msg){
  if (alarmSMSenabled <= 0)
    return;
#ifdef DEBUG_VESLEFRIKK  
  Serial.print("lastTimeSMSsendt=");
  Serial.println(lastTimeSMSsendt);
  Serial.print("getSeconds()="); Serial.println(getSeconds());
#endif
  if ((getSeconds() - lastTimeSMSsendt) > alarmRepeatInterval || lastTimeSMSsendt > getSeconds() || lastTimeSMSsendt == 0){
    sms.SendSMS("93636390", msg);
    alarmSend = 1;
  }
}

/*
 * seconds since start up
 */
unsigned long getSeconds(){
  return seconds;
}

// Reboots the system using the watchdog
void reboot() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (true) {
  }
  wdt_disable();
}

/* 
 * Start timer interrupts
 * 
 * http://www.instructables.com/id/Arduino-Timer-Interrupts/
 * see http://www.instructables.com/id/Arduino-Timer-Interrupts/step2/Structuring-Timer-Interrupts
 */
void initTimer(){
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

/*
 * This is the function which is called every second by the timer
 */
ISR(TIMER1_COMPA_vect){
  seconds++;
  if(seconds%SMS_INTERVAL == 0){
    checkSMS = true;
  }
  if(seconds%UPDATE_INTERVAL == 0){
    doUpdate = true;
  }
  if(seconds%NMEA_INTERVAL == 0){
    sendNMEA = true;
  }
  if(seconds > SEND_INTERVAL){
    sendData = true;
  }
}


/*
 * Returns Unix time from SIM900
 * Should probably be moved to GSM library
 * 
 * For info about what's going on:
 * 1. AT+CLTS=1 ----> This is the AT command to enable the gsm module to get the time from the network, once the gsm module is powered on.
 * 2. AT+CCLK? -----> Once the first AT command is executed, this command can be executed to get the network time.
 * http://www.edaboard.com/thread306862.html
 *
 * Code works although "AT+CLTS=1" not sendt...
 */
unsigned long getUnixTime(){
  byte status;
  tmElements_t tm;

  gsm.SimpleWriteln(F("AT+CCLK?"));
  // 5 sec. for initial comm tmout
  // and max. 1500 msec. for inter character timeout
  gsm.RxInit(5000, 1500);
  // wait response is finished
  do {
    if (gsm.IsStringReceived("OK")) {
      // perfect - we have some response, but what:
      status = RX_FINISHED;
      break; // so finish receiving immediately and check response
    }
    status = gsm.IsRxFinished();
  } while (status == RX_NOT_FINISHED);

  char *s = (char *)gsm.comm_buf;
	
  //...\n+CCLK: "16/10/30,11:48:06+04\nOK\n..."
  int ret = sscanf(s, "%*[^\"]\"%d/%d/%d,%d:%d:%d%*s",
		   &tm.Year,
		   &tm.Month,		
		   &tm.Day,
		   &tm.Hour,
		   &tm.Minute,
		   &tm.Second);

  tm.Year += (2000 -1970); // Unix time started 1.1.1970..
  //Using Time.h library to convert from date time to Unix time
  return makeTime(tm); //Return Unix time as unsigned long
}

/*
 * Execute an AT command and return whatever it returns
 * Used to execute a command by SMS. Might leave SIM900 in an undefined state
 * if not used carefully.
 */
char *executeAT(char *cmd){
  byte status;

  gsm.SimpleWriteln(cmd);
  // 5 sec. for initial comm tmout
  // and max. 1500 msec. for inter character timeout
  gsm.RxInit(5000, 1500);
  // wait response is finished
  do {
    if (gsm.IsStringReceived("OK")) {
      status = RX_FINISHED;
      break; // finish receiving
    }
    status = gsm.IsRxFinished();
  } while (status == RX_NOT_FINISHED);

  // return what we received. Don't mess with it outside
  return (char *)gsm.comm_buf;
}


