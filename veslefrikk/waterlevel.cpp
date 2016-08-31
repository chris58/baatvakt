#include "waterlevel.h"

char waterlevel_2[8] = {};
float level_2_float = 0;
char waterlevel_1[8] = {};
float level_1_float = 0;

void setBaseCapacitance(){
  Serial1.write(1);
  Serial2.write(1);
}

float readLevel_1(){
  Serial1.write(2);
  if(Serial1.available() > 0)	{
    int tmp = Serial1.available()-1;
    for(int i=0; i<=tmp; i++){
      waterlevel_1[i] = Serial1.read();
    }
  }
	
  level_1_float = atof(waterlevel_1)*40;
		
  return level_1_float;
}

float readLevel_2()
{
  Serial2.write(2);
  if(Serial2.available() > 0){
    int tmp = Serial2.available()-1;
    for(int i=0; i<=tmp; i++){
      waterlevel_2[i] = Serial2.read();
    }
  }
  level_2_float = atof(waterlevel_2)*40;
	
  return level_2_float;
}
