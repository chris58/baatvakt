#include "pump.h"

//#define DEBUGPUMP

void updatePump(pPumpInfo pump){
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
      pump->durationOFF = (now - pump->last);
      pump->last = now;
      pump->status = PUMPON;
    }else{ // is still on
      pump->durationON = (now - pump->last);
    }
  }else{ // PUMPOFF
    if (pump->status == PUMPON){ // switched from on to off
      pump->durationON =  (now - pump->last);
      pump->last = now;
      pump->status = PUMPOFF;
    }else{ // is still off
      pump->durationOFF =  (now - pump->last);
    }
  }
}

pPumpInfo initPump(pPumpInfo pump, char *name, uint8_t pin){
  memset(pump->name, 0, sizeof(pump->name));
  strncpy(pump->name,name, sizeof(pump->name)-1);
  pump->pin = pin;
  pump->durationON = 0;
  pump->durationOFF = 0;
  return pump;
}


// NO NEW SMS,WAITTING
// 12V 13.85
// 24V 25.37
// Maskinrom raw 678
// Akterlugar raw 0





/* // cst  */
/* typedef struct{ */
/*   time_t timestamp; */
/*   int[4] temperatures; */
/*   pPumpInfo[2] pumps; */
/*   float[2] batteries; */
/* } vesleInfo; */

/* typdef union{ */
/*   vesleInfo vi; */
/*   uint8_t [sizeof(vesleInfo)] data; */
/* } vesleInfoData; */
