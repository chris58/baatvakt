# SMS commands to send to 'Båtvakta'
Send the commmand and eventual additional parameters to the mobile telephone number of 'Båtvakta'.
Commands can be devided into 4 groups:

1. Requesting status messages
2. Controll and aknowledge of alarm messages
3. Configuration commands
4.p Help

In any case an answer will be sent to confirm the request or a short help message in case the command wasn't understood.



## Requesting status messages

Request pump status
Command: P

Example answer:

Pump Engineroom OFF for 3621 sec
 last on 12 sec
 last off 4001 sec
Pump Stern OFF for 1200 sec
 last on 5 sec
 last off 900 sec


Request temperatures
Command: T
Example: T
Response:
Temperatures:
 Cabin: 20
 Engine: 40
 Stern: 18
 Outside: 17

Request battery status
Command: B
Example: B
Response:
Batteries\n"
 12V Battery: 13.6
 24V Battery: 26.2

Request list of active alarms
Command: A
Example: A
Response:
Temperature Engine Room, ID=1
Aknowledged: NO

2) Controll and aknowledge of alarm messages

Acknowledge alarm
Command: ACK id   (id = alarm index)
Example: ACK 1
Response:
Alarm ID=1 acknowledged

Switch sms alarm on/off
Command: SMS n     (n =0 for OFF or 1 for ON)
Example: SMS 0
Response:
Alarm SMS disabled


3) Configuration commands

Add my phone number to numbers of notified users
Command: ADD 'my name'     
Example: ADD James Bond
Response:
James Bond added to phonebook of notifed users

Add a phone number to numbers of notified users (only ADMIN can do this)
Command: ADD 'number' 'my name'     
Example: ADD 90079007 James Bond
Response:
James Bond added to phonebook of notifed users

Remove phone number of calling user from list of notified users
In case the current ADMIN sends this message an error message will be send back
Command: REM 
Example: REM
Response:
90079007 (James Bond) removed from phonebook

Set low voltage alarm for 12V
Command: BAT12 xx     (xx = minum voltage)
Example:  BAT12 12.1
Response:
12V low voltage alarm = 12.1

Set low voltage alarm for 24V
Command: BAT24 xx     (xx = minum voltage)
Example:  BAT24 24.8
Response:
24V low voltage alarm = 24.8

Set duration for pump alarms (on, low duration off, high duration off)
on               : number of seconds a pump is allowed to be ON before an alarm is issued
low duration off : If the pump is OFF less than 'low duration off' between pumping an alarm is sent
high duration off: If the pump is OFF more than 'high duration off' an alarm is sent
Command: "DONOFF don dLowOff doff
Example: DONOFF 60 120 3600   (where on=1minute, low duration off = 2 minutes, high duration off=1 hour 
Response:
Setting
 Duration ON Alarm=60 sec
 Duration Off Alarm (low)=120 sec
 Duration Off Alarm (high)=3600 sec


4) Help

		 "T for temps\n"
		 "P for pumps\n"
		 "B for batteries\n"
		 "A for active alarms\n"
		 "ACK n where n=alarm idx\n"
		 "SMS 0/1 to turn off/on SMS\n"
		 "DONOFF on-duration off-low-duration off-duration"
