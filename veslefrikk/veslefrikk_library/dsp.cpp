#include "dsp.h"

int pumpBuff_1[10];
int pumpBuff_2[10];

// Counters og orden til buffere. P står for pump og WL står for water level. 
const int ORDER_P = 4;
int countP = 0;
int countWL = 0;

// Kritiske grenser for pumpe og vannstand
int critPumpOn1 = 60; // Kritisk verdi for hvor lenge pumpa har vært på.
int critPumpOn2 = 60; // Kritisk verdi for hvor lenge pumpa har vært på gitt at vannstanden har steget imellomtiden 
int critPumpOff1 = 86400;// Kritisk verdi for hvor lenge pumpa har vært av.
int critPumpOff2 = 86400; // Kritisk verdi for hvor lenge pumpa har vært av gitt at vannstanden hver vært over critHighWL.
int critHighWL = 25; // Kritisk vannstandsnivå.
int critCount = 0; // teller hvor mange samples som har ligget over kritisk vannstandsnivå
int riseCount = 5; // teller hvor mange samples som vannstanden har steget.

// Alarm er et flagg som settes dersom noe er galt. En feilmelding blir lagret i msg. 
//Når denne feilmeldingen er sendt settes flagget msgSent slik at meldingen ikke blir sent flere ganger på rad.
bool alarm = 0;
bool msgSent = 0;

int tempMotor = 0;
int tempDass = 0;
int tempStyr = 0;
int tempOut = 0;

int waterlevelBuff_1[64];
int waterlevelBuff_2[64];

int takeMeanValue(int sensorVec[], const int length) {
  int tempSum = 0;

  for (int i = 0; i < length; i++) {
    tempSum += sensorVec[i];
  }
  return (int)round(tempSum / (double)length);
}

float average(float numbers[], int size) {
  double sum = 0;
  for (int x = 0; x < size; x++) {
    sum += numbers[x];
  }
  return sum /(double)size;
}

const char *byte_to_binary(int x){
  static char b[9];
  b[0] = '\0';

  int z;
  for (z = 128; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }
  return b;
}

void samplePump(int *pumpBuff, int sample){

  // Dersom pumpa er av eller på.
  if (sample == 0 || sample == 1) {
    // Dersom pumpa var av.
    if (countP % 2 == 0) {
      // Dersom pumpa er på inkrementeres countP og den nye plassen i arrayet settes til 1.
      if (sample == 1) {
	countP++;
	pumpBuff[countP] = 1;
      }
      // Dersom pumpa fortsatt er av inkrementeres elementet i arrayet som countP allerede peker på. 
      else {
	pumpBuff[countP]++;
      }
    }
    // Dersom pumpa var på
    else if (countP % 2 == 1) {
      // Dersom pumpa er av og vi ikke har kommet til siste elementet i arrayet inkrementeres "pekeren"/telleren countP og neste element i arrayet settes til 1.
      if (sample == 0 && countP < (ORDER_P - 1)) {
	countP++;
	pumpBuff[countP] = 1;
      }
      // Dersom pumpa er av og vi har kommet til siste element i arrayet settes "pekeren" til å peke på første element i arrayet og elementet il 1.
      else if (sample == 0 && countP == ORDER_P - 1) {
	countP = 0;
	pumpBuff[countP] = 1;
      }
      // Dersom pumpa fortsatt er på inkrementeres det elementet i arrayet som countP peker på.
      else {
	pumpBuff[countP]++;
      }
    }
  }
}

void analyzePump(int *pumpBuff, float *waterLevelBuff){
  // Dersom vannstanden er over kritisk verdi inkrementeres critCount.
  if (waterLevelBuff[countWL] > critHighWL){
    critCount++;
  }
  // Dersom vannstanden er under kritsik verdi  nullstilles critCount.
  else{
    critCount = 0;
  }

  // Dersom vannstanden har steget fra forrige til nåværende sample inkrementeres riseCount. 
  //MERK: Dersom vannstanden har steget fra siste sample i arrayen på 30 sample til første i det nye vil ikke det ikke registreres.
  if (countWL > 0 && countWL < 30 && (waterLevelBuff[countWL] - waterLevelBuff[countWL - 1]) > 0){
    riseCount++;
  }else{
    riseCount = 0;
  }

  // Dersom lensepumpa er av
  if (countP % 2 == 0) {
    // Dersom pumpa har vært av kritisk lenge går alarmen.
    if (pumpBuff[countP] > critPumpOff1) {
      Serial.print("Lensepumpa har vært av i: ");
      Serial.println(pumpBuff[countP]);
      alarm = 1;
    }
    // Dersom lensepumpa er av og den har vært av over en tid critPumpOff2 og vannstanden har vært over kritisk verdi i tre punktprøvinger går alarmen.
    else if (pumpBuff[countP] > critPumpOff2 && critCount > 2){
      Serial.println("Vannstanden er over kritsik verdi og lensepumpa har ikke gått på. ");
      Serial.print("nåværende vannstand er: ");
      Serial.println(waterLevelBuff[countWL]);
      alarm = 1;
    }
    // Ellers er alt OK.
    else {
      Serial.print("OK");
      alarm = 0;
    }
  }
  //Dersom lensepumpa er på
  else if (countP % 2 == 1) {

    if (pumpBuff[countP] > critPumpOn2) {

      // Dersom vannstanden har steget over mer enn tre samples på rad går alarmen.
      if (riseCount > 2) {
	Serial.print("Vannstanden har steget de ");
	Serial.print(riseCount);
	Serial.println(" siste punktprøvene.");
	Serial.print("Nåværende vannstand er: ");
	Serial.println(waterLevelBuff[countWL]);
	Serial.print("Lensepumpa har vært på i: ");
	Serial.print(pumpBuff[countP]);
	Serial.println(" sekunder.");				
	alarm = 1;
      }
      // Dersom pumpa har vært på kritisk lenge går alarmen.
      else if (pumpBuff[countP] > critPumpOn1) {
	Serial.print("Lensepumpa har vaert paa i: ");
	Serial.print(pumpBuff[countP]);
	Serial.println(" sekunder");
	alarm = 1;
      }
      //Dersom vannstanden har vært over kritisk nivå i tre eller flere punktprøvinger går alarmen.
      else if (critCount > 3){
			
	Serial.println("Vannstanden er over kritisk verdi.");
	Serial.print("Nåværende vannstand er: ");
	Serial.println(waterLevelBuff[countWL]);
	alarm = 1;
      }
      else {
	Serial.println("OK");
	alarm = 0;
      }
    }

    else {
      Serial.println("OK");
      alarm = 0;
    }
  }

}



// // cst
// typedef struct{
//   int id;
//   long pumpingtime;
//   int status;
//   long startTimeMillis;
//   time_t startTime;
//   float duration;
//   int secondsSinceLastStart;
// } *pPumpInfo;


// void updatePump(pPumpInfo pump){
//   if (analogRead(pump->id) > 512){ // PUMPON
//     if (pump->status == PUMPOFF){
//       long now = millis();
//       pump->secondsSinceLastStart = (now - pump->startTimeMillis)/1000;
//       pump->startTimeMillis = now;
//       pump->status = PUMPON;
//       pump->startTime = get_unix_ts();
//     }
//   }else{ // PUMPOFF
//     if (pump->status == PUMPON){
//       pump_>pumpingtime += millis() - pump->startTimeMillis;
//     }
//     pump->status = PUMPOFF;
//   }
// }
// // cst 
// typedef struct{
//   time_t timestamp;
//   int[4] temperatures;
//   pPumpInfo[2] pumps;
//   float[2] batteries;
// } vesleInfo;

// typdef union{
//   vesleInfo vi;
//   uint8_t [sizeof(vesleInfo)] data;
// } vesleInfoData;
