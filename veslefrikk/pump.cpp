#include "pump.h"


void updatePump(pPumpInfo pump){
  uint32_t now = millis();
  if (analogRead(pump->pin) > 512){ // PUMPON
    if (pump->status == PUMPOFF){ // switched from off to on
      #ifdef DEBUG
      Serial.print("Pump ");
      #endif
      pump->durationOFF = now - pump->last;
      pump->last = now;
      pump->status = PUMPON;
    }else{ // is still on
      pump->durationON = now - pump->last;
    }
  }else{ // PUMPOFF
    if (pump->status == PUMPON){ // switched from on to off
      pump->durationON = now - pump->last;
      pump->last = now;
      pump->status = PUMPOFF;
    }else{ // is still off
      pump->durationOFF = now - pump->last;
    }
  }
}







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
