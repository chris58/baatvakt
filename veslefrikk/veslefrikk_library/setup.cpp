#include "setup.h"
#include "waterlevel.h"

uint16_t bilge_1_raw;
uint16_t bilge_2_raw; 

uint8_t bilge_state_1;
uint8_t bilge_state_2;

void initTimer()
{
	Serial.println("Startup Complete. Starting timer...");
    Serial.println("");    
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, HIGH);
	cli();            			
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A = 15624;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS10);
	TCCR1B |= (1 << CS12);
	TIMSK1 |= (1 << OCIE1A);
	sei();        				
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
	Serial.print("Booting Modem..."); 
	digitalWrite(8, HIGH);
	delay(1000);
	digitalWrite(8, LOW);
    delay(20000);

  	Serial.println("	-	Modem boot completed.");
  	Serial.print("Entering modem setup...");
  	
  	if(GPRS_setup())
  	{                              
    	Serial.println("	-	Modem setup completed");   
  	}
  	else
  	{
    	Serial.println(F("Modem setup failed"));
  	}
  	
  	Serial.print("Signal strenght: ");
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

  	
  	
	