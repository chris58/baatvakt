#include "HardwareLink3.h"

bool strEndsWith(char *str, char *endStr);
void modemTest();
void SIM900Power();
char *readLine(char *buf, int size, HardwareSerial *port);
int8_t sendATcommand(char* ATcommand, HardwareSerial *port, char* buf, size_t buflen, char* expectedAnswer1, char* expectedAnswer2, unsigned int timeout);
void setupSMSreception();

uint32_t unix_time = 0;
uint32_t checksum = 0;
uint8_t checksum_index = 0;

//const char phone_1[9] = "94788247";
const char phone_1[9] = "93636390";
const uint8_t IMEI[15] = {48,49,51,57,53,48,48,48,55,50,54,49,52,50,52};

bool strEndsWith(char *str, char *endStr){
//  Serial.print("Compare ");
//  Serial.print(str);
//  Serial.print(" to ");
//  Serial.println(endStr);
  int l = strlen(endStr);

  if (strlen(str) >= l){
    char *s = str+strlen(str)-l;
    if (strncmp(s, endStr, l) == 0)
      return true;
  }
  return false;
}


char *readLine(char *buf, int size, HardwareSerial *port){ 
  int cnt=0;
  while (port->available()){
    char c = port->read();
    buf[cnt++] = c;
    if ((c == '\n') || (cnt == size-1)){
      buf[cnt] = '\0';
      Serial.print("readLine: ");
      Serial.print(buf);
      Serial.print("\n");
      return buf;
    }
  }
}

int8_t sendATcommand(char* ATcommand, HardwareSerial *port, char* buf, size_t buflen, char* expectedAnswer1,
		     char* expectedAnswer2, unsigned int timeout){

  uint8_t answer=0;
  uint8_t cnt=0;

  unsigned long start;

  memset(buf, '\0', buflen);
  delay(100);

  while( port->available() > 0) port->read();    // Clean the input buffer

  port->println(ATcommand);    // Send the AT command
  start = millis();

  // wait for the answer
  do{
    // read and check
    if(port->available() != 0){   
      buf[cnt++] = port->read();
      buf[cnt] = '\0';
      // check if the desired answer 1  is in the response of the module
      if (expectedAnswer1 != NULL && strstr(buf, expectedAnswer1) != NULL){
	answer = 1;
      }
      // check if the desired answer 2 is in the response of the module
      else if (strstr(buf, expectedAnswer2) != NULL){
	answer = 2;
      }
    }
  }
  // Waits for the answer with time out
  while((cnt < buflen-1) && (answer == 0) && ((millis() - start) < timeout));   
  return answer;
}




//Checks if input str ends with 'OK(carriage return)(newline)'. ASCII value 13 = carriage return. ASCII value 10 = newline.
//Is used to keep the code from writing the next command until the modem has confirmed the previous command.
bool cmdOK(char* str){
  return strEndsWith(str, "OK");
}

//Checks if str ends with '>'.
bool rdy2write(char* str){
  return strEndsWith(str, ">");
}

//Checks if str ends with 'PIN'.
bool rdy4pin(char* str){
  return strEndsWith(str, "PIN");
}

//Checks if str ends with 'DST'.
bool bootFinished(char* str){
  return strEndsWith(str, "DST");
}

//Checks if str ends with 'FAIL'.
bool connFailed(char* str){
  return strEndsWith(str, "FAIL");
}

//Checks if str ends with 'ERROR'.
bool cmdError(char* str){
  strEndsWith(str, "ERROR");
}



//Writes pin code to modem, and waits until modem is finished booting.
void modemStart(long int pin)
{
  char str[128] = ""; 	//String to gather answer from modem.
	
  pinMode(8, OUTPUT);		//Pin 8 is PWRKEY pin on the GSM-shield. 
  pinMode(9, OUTPUT);		//Pin 9 is RESTART pin on the GSM-shield.
  digitalWrite(9, LOW);	//Setting both to low.
  digitalWrite(8, LOW);	
	
  long loopcounter = 0;
	
  /*while(!rdy4pin(str)){				//Waits until the modem asks for pin code.
    while(Serial3.available()){
    cstringAppend(str, (char)Serial3.read());
    }
		
    if(loopcounter > 5000){			//If enough time has passed, we presume the modem is off.
    digitalWrite(8, HIGH);		//We boot the modem.
    delay(1000);
    digitalWrite(8, LOW);
			
    loopcounter = 0;
			
    str[0] = '\0';
    }
		
    delay(1);
    loopcounter++;
		
    }
    str[0] = '\0';
	
    flushReg();
    //Serial3.print("AT+CPIN="); 		//Writes pin code to modem.
    //Serial3.print(pin);
    Serial3.print("\r\n");
	
    while(!bootFinished(str)){		//Waits until boot is finished.
    if(Serial3.available()){
    cstringAppend(str, (char)Serial3.read());
    }
    }
	
    return;*/
}

void modemStart_simple(long int pin){
  char str[128] = ""; 	//String to gather answer from modem.
	
  pinMode(8, OUTPUT);		//Pin 8 is PWRKEY pin on the GSM-shield.
  pinMode(9, OUTPUT);		//Pin 9 is RESTART pin on the GSM-shield.
  digitalWrite(9, LOW);	//Setting both to low.
  digitalWrite(8, LOW);	
	
  long loopcounter = 0;
	
  flushReg();
  //Serial3.print("AT+CPIN="); 		//Writes pin code to modem.
  //Serial3.print(pin);
  Serial3.print("\r\n");
	
  while(!cmdOK(str)){				//Waits until the modem asks for pin code.
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
    
    if(loopcounter > 10000){			//If enough time has passed, we presume the modem is off.
      digitalWrite(8, HIGH);		//We boot the modem.
      delay(1000);
      digitalWrite(8, LOW);
      delay(3000);
      
      loopcounter = 0;
      
      str[0] = '\0';
      
      flushReg();
      //Serial3.print("AT+CPIN="); 		//Writes pin code to modem.
      //Serial3.print(pin);
      Serial3.print("\r\n");
    }
		
    delay(1);
    loopcounter++;
		
  }
	
  while(!bootFinished(str)){		//Waits until boot is finished.
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  
  return;
}
/*
  Starting Veslefrikk 1.0... 
  Booting Modem...	-	Modem boot completed.
  Entering modem setup...GPRS_setup()
  sent to modem
  AT+CLTS=1
  :-):-):-):-):-):-):-):-):-):-):-):-):-):-):-)AT+CGATT=1
  ;-);-);-);-);-);-);-);-);-);-);-);-);-);-);-);-);-);-)CIPMUX: CIPMUX: 
CIPMUX: 
  CIPMUX: 
  ACIPMUX: 
  ATCIPMUX: 
  AT+CIPMUX: 
  AT+CCIPMUX: 
  AT+CICIPMUX: 
  AT+CIPCIPMUX: 
  AT+CIPMCIPMUX: 
  AT+CIPMUCIPMUX: 
  AT+CIPMUXCIPMUX: 
  AT+CIPMUX=CIPMUX: 
  AT+CIPMUX=0CIPMUX: 
  AT+CIPMUX=0
CIPMUX: 
  AT+CIPMUX=0
  CIPMUX: 
  AT+CIPMUX=0
  
CIPMUX: 
  AT+CIPMUX=0

  CIPMUX: 
  AT+CIPMUX=0

  O
*/
//Run this in setup() to configure GPRS communication.
bool GPRS_setup(){
  char str[128] = "";
  char *s;
	
  modemTest();


  Serial.println("GPRS_setup()");
	
  flushReg();
  Serial3.print("AT+CLTS=1\r\n");
  //  Serial3.print(F("AT+CLTS=1"));		//Enable time update
  //  Serial3.print("\r\n");
  Serial.println("sent to modem");
  Serial.print("AT+CLTS=1\r\n");
   
  Serial.print("step 1: ");
  while(!cmdOK(str)){
    //s = readLine(str, 128, &Serial3); 
     if(Serial3.available()){
       char c = (char)Serial3.read();
//       Serial.print("c=\"");
//       Serial.print(c);
//       Serial.println("\"");
       cstringAppend(str, c);
//       Serial.print(str);
     }
    delay(1000);       
  }
  Serial.println("step 1 ok");
  str[0] = '\0';	
	
	
	
  flushReg();
  //Serial3.print(F("AT+CGATT=1"));		//Attach to GPRS service
  //Serial3.print("\r\n");
//  Serial3.print(F("AT+CGATT=1\r\n"));		//Attach to GPRS service
  Serial3.print("AT+CGATT=1\r\n");		//Attach to GPRS service
  Serial.print("step 2: ");
  while(!cmdOK(str)) {
    if(Serial3.available()){
      Serial.print(";-)");
      char c = (char)Serial2.read();
      cstringAppend(str, c);
    }
    delay(1000);
  }
  Serial.print("step 2 ok ");
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CIPMUX=0"));	//Configure single-IP connection
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
      //      Serial.print("CIPMUX: ");
      //Serial.print(str);
    }
  }
  Serial.print("step 3: ");
  Serial.println(str);
  str[0] = '\0';
	
  flushReg();	
  Serial3.print(F("AT+CSTT=\"telia\",\"\",\"\"")); 	//Start task, set APN, username and password
	
  //With some telecom operators it may take some time to get a valid IP address.
  //If an invalid IP address is assigned, add a delay here.
	
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  Serial.print("step 4: ");
  Serial.println(str);
  
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CIICR"));		//Bring up wireless connection with GPRS.
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  flushReg();
  int fullStopCounter = 0;
  Serial3.print(F("AT+CIFSR"));		//Get local IP address.
  Serial3.print("\r\n");
  while(fullStopCounter < 3){
    if(Serial3.available()){
      if((char)Serial3.read() == '.'){
	fullStopCounter++;
      }
    }
		
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"Contype\",\"GPRS\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"APN\",\"telia\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';	
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"USER\",\"\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"PWD\",\"\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=1,1"));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CNTP=\"no.pool.ntp.org\",1,1,0"));	//Connect to NTP server
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CNTP"));	//Get network time
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  str[0] = '\0';
	
  delay(2000); //Need a small delay here before we flush the receive register to sync time.
	
  flushReg();
	
  return true;
}

int CstringLength2(const char * str)
{
  // compute number of non-null characters in a C string
  int i = 0;

  while (str[i] != '\0')
    i++;

  return i;
}


//Sends SMS and returns true if successful and false if not.
bool sendSMS(const char* num, char* msg)
{
  if(strlen(msg) > 160){			//Maximum size for an SMS is 160 letters.
    return false;
  }
  flushReg();
  Serial3.print(F("AT+CMGF=1"));			//Configures SMS format to text mode
  submit(500);
  flushReg();
  Serial3.print(F("AT+CSCS=\"HEX\""));	//Configures SMS character set to HEX
  submit(500);
  flushReg();
  Serial3.print(F("AT+CMGS=\"2b3437"));			//Sending the SMS. Adds '+47' in HEX at the start of the phone number
  for(int i = 0; i < strlen(num); i++){	//Adds phone number in HEX.
    Serial3.print((int)num[i],HEX);
  }
  Serial3.print(F("\""));
  Serial3.write(13);
  delay(500);
  for(int i = 0; i < strlen(msg); i++){	//Writes message in HEX.

				
    if((int)msg[i] == -61){			//If special letter.
      switch((int)msg[i+1]){
      case (-122):	//ï¿½.
	Serial3.print(28,HEX);
	break;
      case (-104): 	//ï¿½.
	Serial3.print(0);
	Serial3.print(11,HEX);
	break;
      case (-123):	//ï¿½.
	Serial3.print(0);
	Serial3.print(14,HEX);
	break;                         
      case (-90):		//ï¿½.
	Serial3.print(29,HEX);
	break;
      case (-72):		//ï¿½.
	Serial3.print(0);
	Serial3.print(12,HEX);
	break;
      case (-91):		//ï¿½.
	Serial3.print(0);
	Serial3.print(15,HEX);
	break; 
      }
      i++;
    }else {
      Serial3.print((int)msg[i],HEX);	
    }
                       
  }
  Serial3.write(26);
  Serial3.print("\r\n");
  char ret[160+64] = "";
  char c;
  int count = 0;
		
  while(!cmdOK(ret)){ 			//Waits for answer
    if(Serial3.available()){
      cstringAppend(ret, (char)Serial3.read());
    }
    delay(100);
    count++;
			
    if(count > 250){
      return false;
    }
  }
		
  return cmdOK(ret);
}

//Returns signal strength.
int getSignalStrength(){
  flushReg();
  Serial3.print(F("AT+CSQ"));				//Asks for signal strength
  submit(500); 							
  char ret[128] = "";						//String to gather what SIM900 returns
  char signalStrength[2] = "";		
  char c;
  while(Serial3.available()){
    cstringAppend(ret, (char)Serial3.read() );
  }
  if(cmdOK(ret)){					//If the command went through
    int l = strlen(ret);
    cstringAppend(signalStrength, ret[l-12]);		//The signal strength will appear at the 12th and 11th last position in the answer from the modem.
    cstringAppend(signalStrength, ret[l-11]);
    return cstring2int(signalStrength);
  }else{
    return -1;
  }
}

void send_Package(byte* data, int len)
{
	
  data[len] = 99; 
  unix_time = get_unix_ts();
  data[15] = (unix_time >> 24) & 0xFF;
  data[16] = (unix_time >> 16) & 0xFF;
  data[17] = (unix_time >> 8) & 0xFF;
  data[18] = unix_time & 0xFF;
    
  checksum = CRC32::checksum(data, len+1);
    
  checksum_index = len++;
  data[len] = (checksum >> 24) & 0xFF;
  len++;
  data[len] = (checksum >> 16) & 0xFF;
  len++;
  data[len] = (checksum >> 8) & 0xFF;
  len++;
  data[len] = checksum & 0xFF;
  len++;
    
  Serial.print("Sending Package: ");
  Serial.println("");
  Serial.print("IMEI: ");
  for(int i = 0; i < 15; i++)
    {
      Serial.print(data[i]);
      Serial.print(" ");
    }
  Serial.println("");
  Serial.print("Unix Time: ");
  Serial.print(" ");
    
  for(int i = 15; i < 19; i++)
    {
      Serial.print(data[i]);
      Serial.print(" ");
    }
  Serial.println("");
  Serial.print("Data: ");
  for(int i = 19; i < checksum_index; i++)
    {
      Serial.print(data[i]);
      Serial.print(" ");
    }
  Serial.println("");
  Serial.print("Checksum: ");
  for(int i = checksum_index+1; i < checksum_index+5; i++)
    {
      Serial.print(data[i]);
      Serial.print(" ");
    }
  Serial.println("");
  Serial.println("Total Size: ");
  Serial.print(len);
  Serial.println(" Bytes");
    
  if(GPRS_send(data, len))
    {                    
      Serial.println("Data was successfully sent!");
      Serial.println("");
    }
  else
    {
      Serial.println("ERROR: Failed to send data");
    }
}

// Maximum data length: 1024 bytes.
bool GPRS_send(byte* data, int len) 
{
  char str[64] = ""; //String to gather answer from modem.
  char c;
  unsigned long millis_start_of_send = millis();
  unsigned long max_wait = 15000; //Maximum wait before time out.
  flushReg();
  Serial3.print(F("AT+CIPSTART=\"TCP\",\"baatvaktserver.tele.ntnu.no\",\"8884\"")); //Connects to the server.
  Serial3.print("\r\n");
  Serial3.flush(); // hold program until TX buffer is empty
  while (!cmdOK(str)) 
    { 
      //Waits until modem has confirmed that the command went through.
      if (Serial3.available()) 
	{
	  cstringAppend(str, (char)Serial3.read());
	}
 
      if ((unsigned long)millis() - millis_start_of_send >= max_wait) 
	{
	  Serial.println("Timeout1 in GPRS_SEND");
	  return false; //Timeout
	}
    }
 	
  str[0] = '\0'; // reset cstring so we don't overflow
 	
  while (!cmdOK(str)) 
    { //Waits until we are fully connected to the modem.
      if (Serial3.available()) 
	{
	  cstringAppend(str, (char)Serial3.read());
 			
	  if (connFailed(str)) 
	    { //If the connection failed we stop the sending.
	      return false;
	    }
	}
 	
      if ((unsigned long)millis() - millis_start_of_send >= max_wait) 
	{
	  Serial.println("Timeout2 in GPRS_SEND");
	  return false; //Time out.
	}
    }
 	
  str[0] = '\0';
  flushReg();
  Serial3.print(F("AT+CIPSEND=")); // tell modem length of string. maximum length: 1024
  Serial3.print(len);
  Serial3.print("\r\n");
 	
  while (!rdy2write(str)) 
    { //Waits until the modem says its ready to be written to.
      if (Serial3.available()) 
	{
	  cstringAppend(str, (char)Serial3.read());
	}
 	
      if ((unsigned long)millis() - millis_start_of_send >= max_wait) 
	{
	  Serial.println("Timeout3 in GPRS_SEND");
	  return false; //Time out.
	}
    }
 	
  str[0] = '\0';
  flushReg();
  Serial3.write(data, len); // pushes entire data buffer to Serial3
  Serial3.write(26);
  Serial3.print("\r\n");
  Serial3.flush();
  int dataCounter = 0;
	
  while (!cmdOK(str)) 
    {
      if (Serial3.available()) 
	{
	  dataCounter++;
	  if (dataCounter >= len) 
	    { // data buffer is echoed back. wait len chars before SEND OK
	      cstringAppend(str, (char)Serial3.read());
	    }
	  else 
	    {
	      Serial3.read(); // flush one char from RX buffer
	    }
	}
      if ((unsigned long)millis() - millis_start_of_send >= max_wait) {
	Serial.println("Timeout4 in GPRS_SEND");
	return false; //Time out.
      }
    }
  str[0] = '\0';
  Serial3.print(F("AT+CIPCLOSE")); //We close the connection to the server.
  Serial3.print("\r\n");
  return true; //The data was sent, and we did not time out.
}

//We command the modem to give us its IMEI number, and returns it as a string of bytes.
//One byte for each digit because the number is longer than an unsigned long. 
//IMEI number is 15 digits long.

byte* get_IMEI_nr()
{
  flushReg();
  Serial3.print(F("AT+GSN"));		//Asks modem for IMEI number
  submit(500);
  byte ret[15] = {};
  int counter = 0;
  byte c;
  while(Serial3.available())
    {
      c = Serial3.read();
      if( ((char)c >= '0' && (char)c <= '9') && counter < 15 ){
	ret[counter] = c;
	counter++;
      }
    }
  return ret;
}

//Pinging the given address.
bool GPRS_ping(char* adr)
{
  char t;
  char str[128] = "";
  flushReg();
  Serial3.print(F("AT+CIPPING=\""));	//Command to ping address.
  Serial3.print(adr);					//Printing address.
  Serial3.print("\"");
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
			
      if(strlen(str) >= 100){
	str[0] = '\0';
      }
			
      if(cmdError(str)){
	return false;
      }
    }

  }
  return true;
}

//Returns IP address assigned to the modem.
char* get_IP(){
  char str[64] = "";
  char t;
  int newLineCount = 0;
  flushReg();
  Serial3.print("AT+CIFSR");	//Get local IP address
  Serial3.print("\r\n");
  while(newLineCount < 3){
    if(Serial3.available()){
      t = (char)Serial3.read();
      if( (t >= '0' && t <= '9') || t == '.'){
	cstringAppend(str, t);
      }
      if(t == 10){
	newLineCount++;
      }
    }
  }
  return str;
}

/*
1. AT+CLTS=1 ----> This is the AT command to enable the gsm module to get the time from the network, once the gsm module is powered on.
2. AT+CCLK? -----> Once the first AT command is executed, this command can be executed to get the network time.
http://www.edaboard.com/thread306862.html
*/

//Returns Unix time from SIM900
long int get_unix_ts(){
  char str[64] = "";
  char tmp[2] = "";
  flushReg();
  Serial3.print(F("AT+CCLK?"));	//Asks for local time
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }

  int l = strlen(str);
  long int arr[6] = {};
  int arr_count = 0;
	
  //Converting time stamp from date time string to array
  for(int i = 29; i >= 0; i -= 3){
    tmp[0] = str[l-i];
    tmp[1] = str[l-i+1];
    arr[arr_count] = atol(tmp);
    arr_count++;
    if(arr_count >= 6){
      break;
    }
  }
  arr[0] += 2000; //15 + 2000 = 2015
	
  //Using Time.h library to convert from date time to Unix time
  tmElements_t tmSet;
	
  tmSet.Year = arr[0] - 1970;
  tmSet.Month = arr[1];		
  tmSet.Day = arr[2];
  tmSet.Hour = arr[3];
  tmSet.Minute = arr[4];
  tmSet.Second = arr[5];
	
  return makeTime(tmSet);         //Return Unix time as unsigned long
}


bool NTP_sync(){
  char str[64] = "";
  flushReg();
  Serial3.print(F("AT+CNTP"));	//Synchronize with the NTP server we connected to in GPRS_setup()
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    if(Serial3.available()){
      cstringAppend(str, (char)Serial3.read());
    }
  }
  return true;
}


//Empties the receive buffer 
void flushReg(){
  while(Serial3.available() > 0){
    char t = (char)Serial3.read();
  }
  return;
}
 
//Writes ASCII signs with values 13 (submit) and 10 (newline) and creates delay. 
void submit(uint16_t time){
  if(time>=0){
    Serial3.write(13);   //This combination is the same as hitting the enter key once. 
    Serial3.write(10);
    if(time>0){
      delay(time); //In-parameter is delay. 			
    }

  }
  return;
}
 
/*
When communicating with the GSM Modem  using a microcontroller, you usually want very short responses, no 
local echo, and no startup messages. Sticking on the &W to the end of the command saves the setting into memory. 

ATV0&W\r     Enable short response    
ATE0&W\r     Disable Local Echo 
AT+CIURC=0;&W\r    Disable “CALL READY” Startup Message 

Now instead of commands returning OK or ERROR in plain text, as well as repeating all written commands, the
GSM Modem will not echo what you transmit and the GSM Modem  will return error codes in single bytes. For 
example, instead of: 
Transmit: AT\r 
Receive: \r\nOK\r\n
You’ll have: 
Transmit: AT\r 
Receive: \r\n0\r\n  

 */


// CST
void SIM900Power()
// software equivalent of pressing the GSM shield "power" button
//Pin 8 is PWRKEY pin on the GSM-shield. 
//Pin 9 is RESTART pin on the GSM-shield.
{
  // switch it off and on
  digitalWrite(8, HIGH);
  delay(1000);
  digitalWrite(8, LOW);
  delay(5000);

  // just to be sure
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(5000);
}

/*
5. Receiving SMS using AT commands

The GSM modem can be configured to response in different ways when it receives a SMS.

a) Immediate – when a SMS is received, the SMS’s details are immediately sent to the host computer (DTE) via the +CMT command
AT+CMGF=1 	To format SMS as a TEXT message
AT+CNMI=1,2,0,0,0 	Set how the modem will response when a SMS is received

When a new SMS is received by the GSM modem, the DTE will receive the following ..

+CMT :  “+61xxxxxxxx” , , “04/08/30,23:20:00+40”
This the text SMS message sent to the modem

Your computer (DTE) will have to continuously monitor the COM serial port, read and parse the message.

b) Notification – when a SMS is recieved, the host computer ( DTE ) will be notified of the new message. The computer will then have to read the message from the indicated memory location and clear the memory location.
AT+CMGF=1 	To format SMS as a TEXT message
AT+CNMI=1,1,0,0,0 	Set how the modem will response when a SMS is received

When a new SMS is received by the GSM modem, the DTE will receive the following ..
+CMTI: “SM”,3 	Notification sent to the computer. Location 3 in SIM memory
AT+CMGR=3 <Enter> 	AT command to send read the received SMS from modem

The modem will then send to the computer details of the received SMS from the specified memory location ( eg. 3 ) ..

+CMGR: “REC READ”,”+61xxxxxx”,,”04/08/28,22:26:29+40″
This is the new SMS received by the GSM modem

After reading and parsing the new SMS message, the computer (DTE) should send a AT command to clear the memory location in the GSM modem ..

AT+CMGD=3 <Enter>   To clear the SMS receive memory location in the GSM modem

If the computer tries to read a empty/cleared memory location, a +CMS ERROR : 321 will be sent to the computer.
*/

void setupSMSreception(){
  Serial3.println("AT+CMGF=1"); //format SMS as a TEXT message
  while(Serial3.available()) Serial.write((char) Serial3.read());

  //Notification – when a SMS is received, the host computer ( DTE ) will be notified of the new message. 
  //The computer will then have to read the message from the indicated memory location and clear the memory location.
  //Set how the modem will response when a SMS is received
  //When a new SMS is received by the GSM modem, the DTE will receive the following ..
  //+CMTI: “SM”,3 	Notification sent to the computer. Location 3 in SIM memory
  Serial3.println("AT+CNMI=1,1,0,0,0");
  while(Serial3.available()) Serial.write((char) Serial3.read());
}

void processSMStest(int nr, bool doDelete){
  Serial3.print("AT+CMGR="); // get message at position nr
  Serial3.println(nr);

  // process whatever there is
  while(Serial3.available()) Serial.write((char) Serial3.read());

  if (doDelete){
    Serial3.print("AT+CMGD="); // delete message at position nr
    Serial3.println(nr);
  }
  delay(100);
  while(Serial3.available()) Serial.write((char) Serial3.read());
}

void processSMS(int nr, bool doDelete){
  char command[20];
  char buf[256];
  char phonenumber[32];
  char text[180];

  sprintf(command, "AT+CMGF=1");
  if (sendATcommand(command, &Serial3, buf, sizeof(buf), "OK", "ERROR", 100) != 1){
    Serial.println("Could not switch to text mode, AT+CMGF=1");
  }

  sprintf(command, "AT+CMGR=%d", nr);
  if (sendATcommand(command, &Serial3, buf, sizeof(buf), NULL, "ERROR", 100) == 0){
    // +CMGR: “REC READ”,”+61xxxxxx”,,”04/08/28,22:26:29+40″
    // This is the new SMS received by the GSM modem

    // %*[^,]   read and discard everything up to the first ','
    
    sscanf(buf, "%*[^,],\"%[^\"]%*[\n]%s", phonenumber, text);
    Serial.print("got an sms from ");
    Serial.println(phonenumber);
    Serial.print("Text: ");
    Serial.println(text);

    sprintf(text, "Echo: %s\n", text);
    sendSMS(phonenumber, text);
    
  }else{
    Serial.print("could not process SMS: ");
    Serial.println(buf);
  }
  
  if (doDelete){
    sprintf(command, "AT+CMGD=%d", nr);
    sendATcommand(command, &Serial3, buf, sizeof(buf), "OK", "ERROR", 100);
  }
}


// more AT related http://www.electrodragon.com/w/SIM908_SIM900_Common_AT_Commands
void modemTest(){
  Serial.println("***************");
  Serial.println("*Testing Modem*");
  Serial.println("***************");

  pinMode(8, OUTPUT);		//Pin 8 is PWRKEY pin on the GSM-shield.
  pinMode(9, OUTPUT);		//Pin 9 is RESTART pin on the GSM-shield.
  digitalWrite(9, LOW);	//Setting both to low.
  digitalWrite(8, LOW);	

  SIM900Power();

  /** First test if everything is okay **/
  Serial3.println("AT");
  /* <= AT This should come back. SIM900 default is to echo back commands you enter **/
  Serial.print("<= 1");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* OK  This string should tell you all is well**/

  Serial3.println("AT+CPIN?"); /**This is to check if SIM is unlocked. This sample assumes unlocked SIMs**/
  Serial.print("<= 2");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /*  <= +CPIN: READY  If your response contains this, then it means SIM is unlocked and ready**/

  // CST
  Serial3.println("AT+CSQ"); /**This is to ask for quality, max=30 **/
  Serial.print("<= 2");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /*  <= CSQ: 20,0 something like that **/

  Serial3.println("AT+CREG?"); /**This checks if SIM is registered or not**/
  Serial.print("<= 3");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* <=+CREG: 0,1 This string in the response indicates SIM is registered**/

  Serial3.println("AT+CGATT?"); /**Check if GPRS is attached or not**/
  Serial.print("<=4 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* +CGATT: 1 A response containing this string indicates GPRS is attached**/

  Serial3.println("AT+CIPSHUT"); /**Reset the IP session if any**/
  Serial.print("<=5 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* SHUT OK This string in the response represents all IP sessions shutdown.**/

  Serial3.println("AT+CIPSTATUS"); /**Check if the IP stack is initialized**/
  Serial.print("<=6 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* STATE: IP INITIAL This string in the response indicates IP stack is initialized**/

//sendSMS("93636390", "hallo");

  Serial3.println("AT+CIPMUX=0"); /**To keep things simple, I’m setting up a single connection mode**/
  Serial.print("<=7 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* OK This string indicates single connection mode set successfully at SIM 900**/

  //Serial3.println(F("AT+CSTT=\"telia\"")); 	//Start task, set APN, username and password
  Serial3.println(F("AT+CSTT=\"telia\",\"\",\"\"")); 	//Start task, set APN, username and password
  //Serial3.println("AT+CSTT=\“telia\”, \“\”, \“\”"); /**Start the task, based on the SIM card you are using, you need to know the APN, username and password for your service provider**/
  Serial.print("<=8 ");delay(5000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* 'OK'  response indicates task started successfully**/

  Serial3.println("AT+CIICR"); /**Now bring up the wireless. Please note, the response to this might take some time**/
  Serial.print("<=9 ");delay(10000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /*OK This text in response string indicates wireless is up**/

  // Serial3.println("ATO"); /** switch to data mode */
  // Serial.print("<=8a ");delay(1000);
  // while(Serial3.available()) Serial.write((char) Serial3.read());


//  Serial3.print(F("AT+SAPBR=1,1\r\n"));	//Activate bearer profile
//  while(Serial3.available()) Serial.write((char) Serial3.read());
  Serial3.print(F("AT+CGDCONT?\r\n"));	//show list of defined cgd contexts
  while(Serial3.available()) Serial.write((char) Serial3.read());



  Serial3.println("AT+CIFSR\r"); /**Get the local IP address. Some people say that this step is not required, but if I do not issue this, it was not working for my case. So I made this mandatory, no harm.**/
  Serial.print("<=10 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* xxx.xxx.xxx.xxx If previous command is successful, you should see an IP address in the response**/

  Serial3.println("AT+CIPSTART=\“TCP\”, \“raga3.sintef.no\”, \“80\”"); /**Start the connection, TCP, domain name, port**/
  Serial.print("<=11 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* CONNECT OK This string in the response indicates TCP connection established**/

  Serial3.println("AT+CIPSEND"); /**Request initiation of data sending (the request)**/
  Serial.print("<=12 ");delay(10000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* > The response should be the string “>” to indicate, type your data to send**/

  Serial3.println("hello buddy");
  Serial3.write(26);
  Serial.print("<=13 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /* xxxxxxxxxx You should get some response back from the server…it would generally be a complain that the request string was not valid…but that is a different subject…you have established the connection**/

  Serial3.println("AT+CIPSHUT"); /**Request shutting down of the current connections**/
  Serial.print("<=14 ");delay(1000);
  while(Serial3.available()) Serial.write((char) Serial3.read());
  /*SHUT OK Indicates shutdown successful**/
}


// First you need to open a PDP context. 

// >> AT+CGATT=1   - Attach to GPRS Service 
// << OK 

// >> AT+CGDCONT=1,"IP","wap.cingular"   - Define PDP Context (cid, PDP type, APN) 
// << OK 

// >> AT+CDNSCFG="208.67.222.222","208.67.220.220" - Configure Domain Name Server (primary DNS, secondary DNS) 
// << OK 

// >> AT+CSTT="wap.cingular","wap(at)cingulargprs.com","cingular1" - Start Task & set APN, User ID, and password 
// << OK 

// >> AT+CIICR     - Bring up wireless connection with GPRS - THIS MAY TAKE A WHILE 
// << OK 

// >> AT+CIFSR      - Get Local IP address 
// << 10.190.245.172   - returns IP address assigned to your module 
// << OK 

// >> AT+CIPSTATUS      - Get Connection Status 
// << OK 
// << STATE: IP STATUS   - returns status of connection, needs to be 'IP STATUS' before you can connect to a server

// -----------------------------

// >> AT+CIPHEAD=1      - Tells module to add an 'IP Header' to receive data 
// << OK 

// >> AT+CDNSORIP=1   - Indicates whether connection request will be IP address (0), or domain name (1) 
// << OK 

// >> AT+CIPSTART="TCP","www.google.com","80" - Start up TCP connection (mode, IP address/name, port) 
// << OK 
// << CONNECT OK      - Indicates you've connected to the server - IT MAKE TAKE A WHILE FOR THIS TO BE RETURNED 

// >> AT+CIPSEND      - Issue Send Command 
// << >                   - wait for module to return'>' to indicate it's ready to receive data 
// >> GET / HTTP/1.1   - Send data - this example is an HTTP request for the default page 
// >> Host: www.google.com 
// >> Connection: Keep-Alive 
// >> Accept: */* 
// >> Accept-Language: en-us 
// >> 
// << data from server returned - Server will return data here

