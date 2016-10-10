#include "pump.h"

//#define DEBUGPUMP

void pumpUpdate(pPumpInfo pump){
  unsigned long now = millis();
#ifdef DEBUGPUMP
  Serial.print(pump->name);
  Serial.print(" raw ");
  Serial.println(analogRead(pump->pin));
  Serial.print("Now=");
  Serial.println(now);
#endif

  if (analogRead(pump->pin) > 512){ // PUMPON
    if (pump->status == PUMPOFF){ // switched from off to on
      if (now-pump->last > 5000)
	pump->durationOFF = (now - pump->last);
      pump->last = now;
      pump->status = PUMPON;
    }else{ // is still on
      if ((now - pump->last) > pump->alarmDurationOn){
	pump->alarm = ALARM_DURATION_ON;
      }
    }
  }else{ // PUMPOFF
    if (pump->status == PUMPON){ // switched from on to off
      if (now-pump->last > 5000)
	pump->durationON =  (now - pump->last);

      if (pump->last > pump->lastReset)
	pump->durationThisPeriod += (now - pump->last);
      else
	pump->durationThisPeriod += (now - pump->lastReset);

      pump->last = now;
      pump->status = PUMPOFF;
    }else{ // is still off
      if ((now - pump->last) > pump->alarmDurationOff){
	pump->alarm = ALARM_DURATION_OFF;
      }
    }
  }
}

void pumpSetAlarmDurations(pPumpInfo pump, long alarmDurationOn, long alarmDurationOff){
  pump->alarmDurationOn = alarmDurationOn;
  pump->alarmDurationOff = alarmDurationOff;
}

/*
 * updates the pump, resets time counting for a period
 * sets duration of THIS period where pump was on to zero.
 */
void pumpResetPeriod(pPumpInfo pump){
  pumpUpdate(pump);
  pump->durationLastPeriod = pump->durationThisPeriod;
  pump->lastReset = millis();
  pump->durationThisPeriod = 0;
}

/*
 * Initialize a new pump. If pPumpInfo is null allocate memory for it.
 * Return initialized pPumpInfo (pointer to pumpInfo_t).
 */
pPumpInfo pumpInit(pPumpInfo pi, char *name, uint8_t pin, long alarmDurationOn, long alarmDurationOff){
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
  pump->last = millis();
  pump->lastReset = millis();
  pump->durationLastPeriod = 0;
  pump->durationThisPeriod = 0;
  pump->status = (analogRead(pump->pin) > 512) ? PUMPON : PUMPOFF;
  pump->alarmDurationOn = alarmDurationOn;
  pump->alarmDurationOff = alarmDurationOff;
 
  return pump;
}


/* 
   A duration, i.e. how long has the pump
   either been OFF (if it's OFF now) or ON (in case it's ON now)
*/
long pumpGetCurrentStateDuration(pPumpInfo pump){
  return (millis() - pump->last);
}

char *pumpGetStatusMsg(pPumpInfo pump, char *msg, size_t len){
  snprintf(msg, len, 
	   //	   "Lensepumper:\n"
	   "%s has been %s for %d sec\n" 
	   " last on for %d sec\n"  
	   " last off for %d sec\n",
	   pump->name,
	   ((pump->status == PUMPON)? "ON" : "OFF"),
	   (int) (pumpGetCurrentStateDuration(pump) / 1000.0),
	   (int) (pump->durationON/1000),
	   (int) (pump->durationOFF/1000)
	   );
  return msg;
}
