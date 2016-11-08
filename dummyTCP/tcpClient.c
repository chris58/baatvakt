/****************** CLIENT CODE ****************/
/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include <time.h>

#define BUFSIZE 1024

void fillData();
unsigned int rand_interval(unsigned int min, unsigned int max);

typedef struct {
  unsigned int time;
  float tCabin;
  float tEngine;
  float tAft;
  float tOutside;
  float voltage12;
  float voltage24;
  unsigned int pumpEngineDuration;
  unsigned int pumpAftDuration;

  /* unsigned char alarmTCabin; */
  /* unsigned char alarmTEngine; */
  /* unsigned char alarmTAft; */
  /* unsigned char alarmTOutside; */
  /* unsigned char alarmVoltage12; */
  /* unsigned char alarmVoltage24; */
  /* unsigned char alarmPumpEngine; */
  /* unsigned char alarmPumpAft; */
} baatvaktData_t, *pBaatvaktData;

typedef union {
  baatvaktData_t baatvaktData;
  unsigned char bytes[sizeof(baatvaktData_t)];
} data_union;
 
data_union bdUnion;

typedef union {
  float f;
  unsigned char bytes[sizeof(float)];
} float_union;

typedef union {
  unsigned long l;
  unsigned char bytes[sizeof(unsigned long)];
} long_union;




/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd,(struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

    fillData();

    n = write(sockfd, &bdUnion.bytes, sizeof(bdUnion.bytes));
    if (n < 0) 
      error("ERROR writing to socket");

    /* print the server's reply */
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    printf("Echo from server: %s", buf);
    close(sockfd);
    return 0;
}


unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void fillData(){
  bdUnion.baatvaktData.time = time(NULL);

    // values
  bdUnion.baatvaktData.tCabin = (float)rand_interval(-50, 400)/10.0;
  bdUnion.baatvaktData.tEngine = (float)rand_interval(-50, 400)/10.0;
  bdUnion.baatvaktData.tAft = (float)rand_interval(-50, 400)/10.0;
  bdUnion.baatvaktData.tOutside = (float)rand_interval(-50, 400)/10.0;
  
  bdUnion.baatvaktData.voltage12 = (float)rand_interval(110, 150)/10.0;
  bdUnion.baatvaktData.voltage24 = (float)rand_interval(230, 280)/10.0;
  
  bdUnion.baatvaktData.pumpEngineDuration = (float)rand_interval(0, 6000)/10.0;
  bdUnion.baatvaktData.pumpAftDuration = (float)rand_interval(0, 6000)/10.0;

  printf("%d %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %d %d: size=%ld\n",
	 bdUnion.baatvaktData.time,
	 bdUnion.baatvaktData.tCabin,
	 bdUnion.baatvaktData.tEngine,
	 bdUnion.baatvaktData.tAft,
	 bdUnion.baatvaktData.tOutside,
	 bdUnion.baatvaktData.voltage12,
	 bdUnion.baatvaktData.voltage24,
	 bdUnion.baatvaktData.pumpEngineDuration,
	 bdUnion.baatvaktData.pumpAftDuration,
	 (long) sizeof(bdUnion.bytes)
	 );

  int i;
  for (i=0; i<sizeof(baatvaktData_t); i++){
    printf("%d\n", bdUnion.bytes[i]);
  }

  
    // Alarms
    /* bdUnion.baatvaktData.alarmTCabin = ALARM_OFF; */
    /* bdUnion.baatvaktData.alarmTEngine = ALARM_OFF; */
    /* bdUnion.baatvaktData.alarmTAft = ALARM_OFF; */
    /* bdUnion.baatvaktData.alarmTOutside = ALARM_OFF; */

    /* bdUnion.baatvaktData.alarmVoltage12 = batteryGetAlarmCode(pBattery12V); */
    /* bdUnion.baatvaktData.alarmVoltage24 = batteryGetAlarmCode(pBattery24V); */

    /* bdUnion.baatvaktData.alarmPumpEngine = pumpGetAlarm(pPumpEngine); */
    /* bdUnion.baatvaktData.alarmPumpAft = pumpGetAlarm(pPumpAft); */
}

