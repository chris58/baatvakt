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
  Serial.print("Trying to remove alarm for ");
  Serial.println(((pUnitInfo) unit)->name);
  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] != NULL){
      if (alarmList[i]->unit == unit){ //&& alarmList[i]->alarmCode == alarmCode){
	free(alarmList[i]);
	alarmList[i] = NULL;
	Serial.println("Did it");
	return 1;
      }
    }
  }
  // not found :-(
  Serial.print("Did not find any alarm of ");
  Serial.println(((pUnitInfo) unit)->name);
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
 * Used when SMS with "ACK id" recieved, where 0<=id<MAX_ALARMS
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
	snprintf(ptr, len, "%s id: %d\nAcknowledged: %s\n",
		 pumpGetAlarmMsg(pump, txt, sizeof(txt)),
		 i,
		 (alarmList[i]->acknowledged ? "YES" : "NO")
		 );
	Serial.println(buf);
	break;
      case BATTERY:
        bat = (pBatteryInfo) ui;
	snprintf(ptr, len, "%s id: %d\nAcknowledged: %s\n",
		 batteryGetAlarmMsg(bat, txt, sizeof(txt)),
		 i,
		 (alarmList[i]->acknowledged ? "YES" : "NO")
		 );
	Serial.println(buf);
	break;
      case TEMPERATURE:
	snprintf(ptr, len, "Battery %s alarm code: %d id: %d\nAcknowledged: %s\n",
		 ((pTemperatureInfo) ui)->name,
		 ((pTemperatureInfo) ui)->alarmCode,
		 i
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
  return buf;
}
