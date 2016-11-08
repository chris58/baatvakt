#include <stdlib.h>
#include "units.h"
#include "battery.h"
#include "alarmhandler.h"

//#define DEBUG_ALARM

/*
 * Array of all active alarms, max MA_ALARMS long
 */
static pAlarmInfo alarmList[MAX_ALARMS];// = (pAlarmInfo *) calloc(MAX_ALARMS, sizeof(pAlarmInfo *));

/*
 * find out whether an alarm has been acknowledged (by SMS)
 */
int alarmIsAcknowledged(void *unit, short alarmCode){
  int i;
  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] != NULL){
      if (alarmList[i]->unit == unit && alarmList[i]->alarmCode == alarmCode)
	return alarmList[i]->acknowledged;
    }
  }
  // not found... :-( At least not acknowleged...
  return 0;

}

/*
 * Find and remove a specific alarm
 */
int alarmRemove(void *unit, short alarmCode){
  int i;
#ifdef DEBUG_ALARM
  Serial.print("Trying to remove alarm for ");
  Serial.println(((pUnitInfo) unit)->name);
#endif
  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] != NULL){
      if (alarmList[i]->unit == unit){ //&& alarmList[i]->alarmCode == alarmCode){
	free(alarmList[i]);
	alarmList[i] = NULL;
	return 1;
      }
    }
  }
  // not found :-(
#ifdef DEBUG_ALARM
  Serial.print("Did not find any alarm of ");
  Serial.println(((pUnitInfo) unit)->name);
#endif
  return ALARM_ERROR;
}

/*
 * Find empty spot in alarm list and insert a new alarm
 * Check first whether it's there already
 */
int alarmAdd(void *unit, short alarmCode){
  int i;
  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] != NULL){
      if (alarmList[i]->unit == unit && alarmList[i]->alarmCode == alarmCode){
	// already there;
	return i;
      }
    }
  }

  // insert new
  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] == NULL){
      pAlarmInfo ai = (pAlarmInfo) calloc(1, sizeof(alarmInfo_t));
      ai->unit = unit;
      ai->alarmCode = alarmCode;
      alarmList[i] = ai;
      return i;
    }
  }
  // no more space
  return ALARM_ERROR;
}

/*
 * Acknowledge an alarm at inidex id. If acknowledged no more further SMS will be send.
 * Called when SMS with "ACK id" recieved, where 0<=id<MAX_ALARMS
 */
void alarmAcknowledgeById(int id){
  if (id >=0 && id < MAX_ALARMS && alarmList[id] != NULL){
    alarmList[id]->acknowledged = 1;
    return id;
  }
  // not found
  return ALARM_ERROR;
}

/*
 * Build a string of all active alarms
 */
char *alarmGetActiveAlarmsAsString(char *buf, int buflen){
  int i;
  char txt[64];
  char *ptr = buf;
  int len = buflen;
  pBatteryInfo bat;
  pPumpInfo pump;

  buf[0] = '\0';
  strcpy(buf, "No alarms");

  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] != NULL){
      pUnitInfo ui = (pUnitInfo) alarmList[i]->unit;
      switch(ui->typeID){
      case PUMP:
	pump = (pPumpInfo) ui;
	snprintf(ptr, len, "- %s id: %d\nAck: %s\n",
		 pumpGetAlarmMsg(pump, txt, sizeof(txt)),
		 i,
		 (alarmList[i]->acknowledged ? "YES" : "NO")
		 );
	break;
      case BATTERY:
        bat = (pBatteryInfo) ui;
	snprintf(ptr, len, "- %s id: %d\nAck: %s\n",
		 batteryGetAlarmMsg(bat, txt, sizeof(txt)),
		 i,
		 (alarmList[i]->acknowledged ? "YES" : "NO")
		 );
	break;
      case TEMPERATURE:
	snprintf(ptr, len, "Temp %s alarm: %d id: %d\nAck: %s\n",
		 ((pTemperatureInfo) ui)->name,
		 ((pTemperatureInfo) ui)->alarmCode,
		 i,
		 (alarmList[i]->acknowledged ? "YES" : "NO")
		 );
#ifdef DEBUG_ALARM
	Serial.println(buf);
#endif
	break;
      default:
	break;
      }
      ptr += strlen(ptr);
      len -= strlen(ptr);
    }
  }
  buf[buflen-1] = '\0';
  return buf;
}

/*
 * If no alarm remove eventual entry in alarm list
 * Otherwise get a string from the appropriate unit and return it.
 */
char *handleAlarm(pUnitInfo unit, short alarmCode, char *msg, size_t len){
  int id;

  if (alarmCode == ALARM_OFF){
    // make sure that there is no alarm left in the alarm list
    alarmRemove(unit, alarmCode);
    return NULL;
  }

  msg[0] = '\0';
  if (!alarmIsAcknowledged(unit, alarmCode)){
    id = alarmAdd(unit, alarmCode);
    switch(unit->typeID){
    case PUMP:
      pumpGetAlarmMsg((pPumpInfo) unit, msg, len);
      break;
    case BATTERY:
      batteryGetAlarmMsg((pBatteryInfo) unit, msg, len);
      break;
    case TEMPERATURE:
      temperatureGetAlarmMsg((pTemperatureInfo) unit, msg, len);
      break;
    }
  }

  if (strlen(msg) > 0){
    snprintf(msg, len, "%s\n To acknowledge reply\nACK %d\n", msg, id);
    //sendAlarmMsg(msg);
#ifdef DEBUG_ALARM
    Serial.println("handleAlarm, msg=");
    Serial.println(msg);
#endif
    return msg;
  }else{
    return NULL;
  }

}
