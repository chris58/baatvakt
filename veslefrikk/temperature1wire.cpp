#include "temperature1wire.h"

#define ONEWIRE_PIN 13
#define PROBE_MAX 4 // number of temperature probes. In our case there are 4.

static OneWire oneWire1(ONEWIRE_PIN);
static DallasTemperature sensors1(&oneWire1);
static int _isInit = 0;
static pTemperatureInfo pti[PROBE_MAX];
static int probeCount = 0;


static void initDallas(){
  sensors1.begin();
  memset(pti, NULL, sizeof(pti));
  _isInit = 1;
}

pTemperatureInfo temperatureAddTemperatureProbe(DeviceAddress da, char *name, uint8_t alarmLow, uint8_t alarmHigh, uint8_t resolution){
  if (!_isInit){
    initDallas();
  }
  if (probeCount >= PROBE_MAX)
    return NULL;

  pTemperatureInfo ti;
  if ((ti = (pTemperatureInfo) calloc(1, sizeof(temperatureInfo_t))) != NULL){
      sensors1.setLowAlarmTemp(da, alarmLow);
      sensors1.setHighAlarmTemp(da, alarmHigh);
      sensors1.setResolution(da, resolution);
      
      //  memset(ti->name, 0, sizeof(ti->name));
      strncpy(ti->name, name, sizeof(ti->name)-1);
      memcpy(ti->da, da, sizeof(ti->da));
      
      pti[probeCount++] = ti;
    }

  return ti; 
}


void temperaturesUpdate(){
  sensors1.requestTemperatures(); 
  for (int i=0; i<PROBE_MAX; i++){
    if (pti[i] != NULL){
      pti[i]->value = sensors1.getTempC(pti[i]->da);
    }
  } 
}

float temperatureGetTempC(pTemperatureInfo pi){
  return pi->value;
}

