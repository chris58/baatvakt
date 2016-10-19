#include <stdlib.h>
#include "alarmhandler.h"

static pAlarmInfo alarmList[MAX_ALARMS];// = (pAlarmInfo *) calloc(MAX_ALARMS, sizeof(pAlarmInfo *));

int isAcknowledgedAlarm(void *unit, short alarmCode){
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

int removeAlarm(void *unit, short alarmCode){
  int i;
  for (i=0; i<MAX_ALARMS; i++){
    if (alarmList[i] != NULL){
      if (alarmList[i]->unit == unit && alarmList[i]->alarmCode == alarmCode){
	free(alarmList[i]);
	alarmList[i] = NULL;
	return 1;
      }
    }
  }
  // not found :-(
  return ALARM_ERROR;
}

int addAlarm(void *unit, short alarmCode){
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
void acknowledgeByIdAlarm(int id){
  if (id >=0 && id < MAX_ALARMS && alarmList[id] != NULL){
    alarmList[id]->acknowledged = 1;
    return id;
  }
  // problem
  return ALARM_ERROR;
}
