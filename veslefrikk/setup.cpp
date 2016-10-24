#include "setup.h"
#include "waterlevel.h"

uint16_t bilge_1_raw;
uint16_t bilge_2_raw; 

uint8_t bilge_state_1;
uint8_t bilge_state_2;

void initTimerXX()
{
  //Serial.println("Startup Complete. Starting timer...");
   //delay(1000);
  cli();            			
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  sei();  
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, HIGH);
}

// http://www.instructables.com/id/Arduino-Timer-Interrupts/
// see http://www.instructables.com/id/Arduino-Timer-Interrupts/step2/Structuring-Timer-Interrupts/
void initTimerChrisXX(){
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  // "compare match register" = [ 16,000,000Hz/ (prescaler * "desired interrupt frequency") ] - 1
  OCR1A = 15624;// = (16*10^6) / (1024*1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
}

void initSystem()
{
  pinMode(8, OUTPUT); //Pin 8 - PWRKEY
  pinMode(9, OUTPUT); //Pin 9 - RESTART
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED0, HIGH);
  Serial.begin(57600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(4800);
  Serial.println("Starting Veslefrikk 1.0... ");
}

void enableTimer()
{
  TIMSK1 |= (1 << OCIE1A);
}

void disableTimer()
{
  TIMSK1 &= ~(1 << OCIE1A);
}

void initModem()
{		
  Serial.println("***************");
  Serial.println("*Booting Modem*");
  Serial.println("***************");
  digitalWrite(8, HIGH);
  delay(1000);
  digitalWrite(8, LOW);
  delay(20000);

  Serial.println("	-	Modem boot completed.");

  // CST
  // Serial.print("Entering modem setup...");
  	
  // if(GPRS_setup())
  //   {                              
  //     Serial.println("	-	Modem setup completed");   
  //   }
  // else
  //   {
  //     Serial.println(F("Modem setup failed"));
  //   }
  	
  Serial.print("Signal strength: ");
  Serial.println(getSignalStrength());  	 
}

void initSensors()
{
  sensors.begin();
  sensors.setResolution(Probe1, 9);
  sensors.setResolution(Probe2, 9);
  sensors.setResolution(Probe3, 9);
  sensors.setResolution(Probe4, 9);
  	
  bilge_1_raw = analogRead(BILGE_1);
  bilge_2_raw = analogRead(BILGE_2);
    
  if(bilge_1_raw > 512)
    {
      bilge_state_1 = 1;
    }
  else
    {
      bilge_state_1 = 0;
    }
    
  if(bilge_2_raw > 512)
    {
      bilge_state_2 = 1;
    }
  else
    {
      bilge_state_2 = 0;
    }
  //	setBaseCapacitance();
}

  	
  	
	
