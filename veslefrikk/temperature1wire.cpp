#include "units.h"

#define ONEWIRE_PIN 13

static OneWire oneWire1(ONEWIRE_PIN);
static DallasTemperature sensors1(&oneWire1);
static int _isInit = 0;
static pTemperatureInfo *pti; //pti[PROBE_MAX];
static int probeCount = 0;

static int maxProbe; // maximum number of temperature probes.
static char nmea[128];

/*
 * Allocate memory for an array of 'n' temperature devices. Init the DallasTemperature library
 */
void temperatureInit(int n){
  pti = (pTemperatureInfo *) calloc(n, sizeof(pTemperatureInfo));
  sensors1.begin();
  _isInit = 1;
  maxProbe = n;
}

/*
 * Allocate a new structure, initialize it and return a pointer to it
 */
pTemperatureInfo temperatureAddTemperatureProbe(DeviceAddress da, char *name, uint8_t alarmLow, uint8_t alarmHigh, uint8_t resolution){
  if (!_isInit){
    return NULL;
    //    initDallas();
  }
  if (probeCount >= maxProbe)
    return NULL;

  pTemperatureInfo ti;
  if ((ti = (pTemperatureInfo) calloc(1, sizeof(temperatureInfo_t))) != NULL){
      sensors1.setLowAlarmTemp(da, alarmLow);
      sensors1.setHighAlarmTemp(da, alarmHigh);
      sensors1.setResolution(da, resolution);
      
      strncpy(ti->name, name, sizeof(ti->name)-1);
      memcpy(ti->da, da, sizeof(ti->da));
      ti->typeID = TEMPERATURE;
      ti->alarmCode = ALARM_OFF;
      
      pti[probeCount++] = ti;
    }

  return ti; 
}

/*
 * Update temperatureInfo structures with current values
 */
void temperaturesUpdate(){
  sensors1.requestTemperatures(); 
  for (int i=0; i<maxProbe; i++){
    if (pti[i] != NULL){
      pti[i]->value = sensors1.getTempC(pti[i]->da);
      if (pti[i]->value > sensors1.getHighAlarmTemp(pti[i]->da)){
	pti[i]->alarmCode = ALARM_TEMPERATURE_HIGH;
      }else if (pti[i]->value < sensors1.getLowAlarmTemp(pti[i]->da)){
	pti[i]->alarmCode = ALARM_TEMPERATURE_LOW;
      }else
	pti[i]->alarmCode = ALARM_OFF;
    }
  } 
}

/*
 * Return a string containing the alarm message, if any.
 */
char *temperatureGetAlarmMsg(pTemperatureInfo temp, char *msg, size_t len){
  if (temp->alarmCode == ALARM_TEMPERATURE_LOW){
    snprintf(msg, len, 
	     "Temp %s too low: %d\n",
	     temp->name,
	     (int) temp->value
	     );
  }else if (temp->alarmCode == ALARM_TEMPERATURE_HIGH){
    snprintf(msg, len, 
	     "Temp %s too high: %d\n",
	     temp->name,
	     (int) temp->value
	     );
  }else{
    snprintf(msg, len, 
	     "Temperature %s is ok\n",
	     temp->name
	     );
  }
  return msg;
}

/*
 * Returns in turn all pTemperatureInfos known
 * Call the first time with NULL
 * Next time with last returned pTemperatureInfo
 * Returns NULL if last pTemperatureInfo has been returned
 * otherwise the next
 */
pTemperatureInfo temperatureGetNextTempInfo(pTemperatureInfo last){
  int foundLast = 0;

  for (int i=0; i<maxProbe; i++){
    if (pti[i] != NULL){
      if (last == NULL || foundLast){
	return pti[i];
      }else if (last != NULL && !foundLast && pti[i] == last){
	foundLast = 1;
      }
    }
  }
  return NULL;
}

/*
 * builds a string with all temperatures to send as NMEA sentence
 * Don't modify the string outside this file.
 */
char *temperatureGetNMEA(){
  int i;
  int len = sizeof(nmea);
  char *ptr = nmea;

  snprintf(ptr, len, "$PTMP");
  
  ptr += strlen(ptr);
  len -= strlen(ptr);
  
  for (i=0; i<maxProbe; i++){
    if (pti[i] != NULL){
      snprintf(ptr, len, ",%d,%d", (int) temperatureGetTempC(pti[i]), temperatureGetAlarmCode(pti[i]));
      ptr += strlen(ptr);
      len -= strlen(ptr);
    }
  }

  snprintf(ptr, len, "*%02X", nmeaCheckSum(nmea, strlen(nmea)));

  return nmea;
}
