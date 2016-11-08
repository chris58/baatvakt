#include "units.h"

static char nmea[48];

//#define DEBUGPUMP

/*
 * update pump, i.e. check whether ON or OFF and calculate corresponding durations.
 * In case an alarm duration is exceeded return the alarm code,
 * otherwise return ALARM_OFF
 */
int pumpUpdate(pPumpInfo pump){
  unsigned long now = getSeconds();
#ifdef DEBUGPUMP
  Serial.print(pump->name);
  Serial.print(" raw ");
  Serial.println(analogRead(pump->pin));
  Serial.print("Now=");
  Serial.println(now);
#endif

  if (analogRead(pump->pin) > 512){ // at least half the voltage, i.e. PUMPON
    if (pump->status == PUMPOFF){ // switched from off to on
      //if (now-pump->last > 5000)
      pump->durationOFF = (now - pump->last);
      pump->last = now;
      pump->status = PUMPON;
    }else{ // is still on
      if ((now - pump->last) > pump->alarmDurationOn){
	pump->alarmCode = ALARM_DURATION_ON;
      }else{
	pump->alarmCode = ALARM_OFF;
      }
    }
  }else{ // PUMPOFF
    if (pump->status == PUMPON){ // switched from on to off
      //      if (now-pump->last > 5000)
      pump->durationON =  (now - pump->last);

      if (pump->last > pump->lastReset)
	pump->durationThisPeriod += (now - pump->last);
      else
	pump->durationThisPeriod += (now - pump->lastReset);

      pump->last = now;
      pump->status = PUMPOFF;
    }else{ // is still off
#ifdef DEBUGPUMP
      Serial.print("Pump off time: ");
      Serial.print(now - pump->last);
      Serial.print(", alarm duration: ");
      Serial.println(pump->alarmDurationOff);
#endif
      if ((now - pump->last) > pump->alarmDurationOff){
#ifdef DEBUGPUMP
	Serial.println("!!!!!!!!! Alarm !!!!!!!!!!!!");
#endif
	pump->alarmCode = ALARM_DURATION_OFF;
      }else{
	pump->alarmCode = ALARM_OFF;
      }
    }
  }
#ifdef DEBUGPUMP
  Serial.print("Alarm as return value is " );
  Serial.println(pump->alarmCode);
#endif
  return pump->alarmCode;
}

/*
 * Set maximum time for pump durations ON/OFF in seconds
 */
void pumpSetAlarmDurations(pPumpInfo pump, unsigned long alarmDurationOn, unsigned long alarmDurationOff){
  pump->alarmDurationOn = alarmDurationOn; // * 1000L;
  pump->alarmDurationOff = alarmDurationOff; // * 1000L;
}

/*
 * Updates the pump, resets time counting for a period.
 * Sets duration of THIS period where pump was on to zero.
 */
unsigned long pumpResetPeriod(pPumpInfo pump){
  //  pumpUpdate(pump);
  pump->durationLastPeriod = pump->durationThisPeriod;
  pump->lastReset = getSeconds();
  pump->durationThisPeriod = 0;
  return pump->durationLastPeriod;
}

/*
 * Initialize a new pump. If pPumpInfo is null allocate memory for it.
 * Return initialized pPumpInfo (pointer to pumpInfo_t).
 */
pPumpInfo pumpInit(pPumpInfo pi, char *name, uint8_t pin, unsigned int alarmDurationOn, unsigned int alarmDurationOff){
  pPumpInfo pump;
  
  if (pi != NULL){
    pump = pi;
    memset(pump->name, 0, sizeof(pump->name));
  }else{
    if ((pump = (pPumpInfo) calloc(1, sizeof(pumpInfo_t))) == NULL)
      return NULL;
  }

  strncpy(pump->name,name, sizeof(pump->name)-1);
  pump->pin = pin;
  pump->durationON = 0;
  pump->durationOFF = 0;
  pump->last = getSeconds();
  pump->lastReset = getSeconds();
  pump->durationLastPeriod = 0;
  pump->durationThisPeriod = 0;
  pump->status = (analogRead(pump->pin) > 512) ? PUMPON : PUMPOFF;
  pump->alarmDurationOn = alarmDurationOn;// * 1000L;
  pump->alarmDurationOff = alarmDurationOff;// * 1000L;
  pump->typeID = PUMP;
  pump->alarmCode = ALARM_OFF;
 
  return pump;
}

/* 
 * A duration, i.e. how long the pump has
 * either been OFF (if it's OFF now) or ON (in case it's ON now)
*/
long pumpGetCurrentStateDuration(pPumpInfo pump){
  return (getSeconds() - pump->last);
}

/*
 * Return a string containing the alarm message, if any.
 */
char *pumpGetAlarmMsg(pPumpInfo pump, char *msg, size_t len){
#ifdef DEBUGPUMP
  Serial.print("pumpGetAlarmmsg "); Serial.println(pump->alarmCode);
#endif
  if (pump->alarmCode != ALARM_OFF){
    snprintf(msg, len, 
	     "ALARM:\nPump %s has been %s for %d sec\n",
	     pump->name,
	     ((pump->status == PUMPON)? "ON" : "OFF"),
	     (int) (pumpGetCurrentStateDuration(pump)) // / 1000.0)
	     );
  }else{
    snprintf(msg, len, 
	     "Status for pump %s is ok\n",
	     pump->name
	     );
  }

#ifdef DEBUGPUMP
  Serial.println(msg);
#endif
  return msg;
}

/*
 * Build the status message which is returned when pump status is required
 */
char *pumpGetStatusMsg(pPumpInfo pump, char *msg, size_t len){
  snprintf(msg, len, 
	   "%s has been %s for %d sec\n" 
	   " last on for %d sec\n"  
	   " last off for %d sec\n",
	   pump->name,
	   ((pump->status == PUMPON)? "ON" : "OFF"),
	   (int) (pumpGetCurrentStateDuration(pump)), // / 1000.0),
	   (int) (pump->durationON), // /1000),
	   (int) (pump->durationOFF) // /1000)
	   );
  return msg;
}


/*
 * Builds a string with to send as NMEA sentence
 * Don't modify the string outside this file.
 */
char *pumpGetNMEA(int n, ...){
  va_list arguments;                     
  pPumpInfo p;
  int i;
  int len = sizeof(nmea);
  char *ptr = nmea;
  
  va_start(arguments, n);
  snprintf(ptr, len, "$PPMP");
  ptr += strlen(ptr);
  len -= strlen(ptr);
  
  for (i=0; i<n; i++){
    p = va_arg ( arguments, pPumpInfo);
#ifdef DEBUGPUMP
    Serial.print("pumpnmea "); Serial.print(p->name); Serial.println(p->status);
#endif
    snprintf(ptr, len, ",%d", p->status);
    ptr += strlen(ptr);
    len -= strlen(ptr);
  }
  va_end(arguments);

  uint32_t checksum = CRC32::checksum(nmea, strlen(nmea));
  snprintf(ptr, len, "*%d", checksum);

  return nmea;
}

