//Calculate crc...
char nmeaCheckSum(char *buf, int len){
  char crc=0; 
  int i;

  for(i=1; i<len; i++)
    crc ^= buf[i];

  return crc;
}

