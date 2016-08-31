#include "HardwareLink3.h"

uint32_t unix_time = 0;
uint32_t checksum = 0;
uint8_t checksum_index = 0;

//const char phone_1[9] = "94788247";
const char phone_1[9] = "93636390";
const uint8_t IMEI[15] = {48,49,51,57,53,48,48,48,55,50,54,49,52,50,52};

bool strEndsWith(char *str, char *endStr){
  int l = strlen(endStr);

  if (strlen(str) >= l){
    char *s = str+strlen(str)-l;
    if (strncmp(s, endStr, l) == 0)
      return true;
  }
  return false;
}


char *readLine(char *buf, int maxLen, HardwareSerial *port){ 
  int cnt=0;
  while (port->available()){
    char c = port->read();
    if (c != '\r') // don't need that
      buf[cnt++] = c;
    else if (c == '\n' || cnt == maxLen-1){
      break;
    }
  }
  // In case we did not read anything there will still be 
  // returned a valid null terminated string
  buf[cnt] = '\0';
  Serial.print("readLine: ");
  Serial.print(buf);
  Serial.print("\n");
  return buf;
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

//Run this in setup() to configure GPRS communication.
bool GPRS_setup(){
  int strMax = 128;
  char str[strMax] = "";
	
  flushReg();
  Serial3.print(F("AT+CLTS=1"));		//Enable time update
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';	
	
  flushReg();
  Serial3.print(F("AT+CGATT=1"));		//Attach to GPRS service
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  flushReg();
  Serial3.print(F("AT+CIPMUX=0"));	//Configure single-IP connection
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();	
  Serial3.print(F("AT+CSTT=\"telia\",\"\",\"\"")); 	//Start task, set APN, username and password

  //With some telecom operators it may take some time to get a valid IP address.
  //If an invalid IP address is assigned, add a delay here.
	
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CIICR"));		//Bring up wireless connection with GPRS.
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
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
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"APN\",\"telia\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';	
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"USER\",\"\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=3,1,\"PWD\",\"\""));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+SAPBR=1,1"));	//Activate bearer profile
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CNTP=\"no.pool.ntp.org\",1,1,0"));	//Connect to NTP server
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  flushReg();
  Serial3.print(F("AT+CNTP"));	//Get network time
  Serial3.print("\r\n");
  while(!cmdOK(str)){
    str = readLine(str, strMax, &Serial3);
  }
  str[0] = '\0';
	
  delay(2000); //Need a small delay here before we flush the receive register to sync time.
	
  flushReg();
	
  return true;
}

//Sends SMS and returns true if successful and false if not.
bool sendSMS(const char* num, char* msg)
{
  if(strlen(msg) > 160){			//Maximum size for an SMS is 160 letters.
    return false;
  }
  flushReg();
  Serial3.print(F("AT+CMGF=1\r\n"));			//Configures SMS format to text mode
  delay(500);
  flushReg();
  Serial3.print(F("AT+CSCS=\"HEX\"\r\n"));	//Configures SMS character set to HEX
  delay(500);
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
      case (-122):	//Æ.
	Serial3.print(28,HEX);
	break;
      case (-104): 	//Ø.
	Serial3.print(0);
	Serial3.print(11,HEX);
	break;
      case (-123):	//Å.
	Serial3.print(0);
	Serial3.print(14,HEX);
	break;                         
      case (-90):		//æ.
	Serial3.print(29,HEX);
	break;
      case (-72):		//ø.
	Serial3.print(0);
	Serial3.print(12,HEX);
	break;
      case (-91):		//å.
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
