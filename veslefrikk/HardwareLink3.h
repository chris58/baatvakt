  #include <Arduino.h>
  #include "Time.h"
  #include "cstring.h"
  #include "CRC32.h"
  #define SMS_MAX 120
  
  extern const char phone_1[9];
  extern const uint8_t IMEI[15];
  
  //Confirmation from modem checks
  bool cmdOK(char* str);					//Checks for 'OK(submit)(newline)'.
  bool rdy2write(char* str);				//Checks for '>'.
  bool rdy4pin(char* str);					//Checks for 'PIN'.
  bool bootFinished(char* str);				//Checks for 'DST'.
  bool connFailed(char* str);				//Checks for 'FAIL' .
  bool cmdError(char* adr);					//Checks for 'ERROR'.

 //GPRS / GSM communication
  void modemStart(long int pin);		//Writes pin code to modem.
  void modemStart_simple(long int pin); //TEST VERSION FOR WHEN GSM SHIELD DOES NOT ASK FOR PIN CODE
  bool GPRS_setup();					//Writes AT commands needed to configure GPRS.
  
  bool sendSMS(const char* num, char* msg);		//Sends msg as SMS to num.
  int getSignalStrength();					//Returns the modems signal strength.
  bool GPRS_send(byte* data, int len);		//Sends data (array of bytes with length len) to the server.
  byte* get_IMEI_nr();						//Returns IMEI number of modem as array of bytes.
  
  void send_Package(byte* data, int len);
  
  bool GPRS_ping(char* str);				//Ping given address and return true if pinged successfully.
  char* get_IP();							//Returns the IP address assigned to the modem.
  
  //TIMESTAMP
  long int get_unix_ts();					//Returns the local time in the modem.
  bool NTP_sync();							//Synchronizes clock with the NTP server we connected to in GPRS_setup()
	

  
  //FLUSH REGISTER
  void flushReg();							//Flushed receive buffer
  
  //SUBMIT
  void submit(uint16_t time);				//Submits (writes ASCII values 13 and 10) and delays 'time' milliseconds.
