#include "cstring.h"

int cstringLength(char* str){
  int l = 0;
  char c1 = str[l];
  while(c1 != '\0'){
    l++;
    c1 = str[l];
  }
  return l;
}

void cstringAppend(char* str, char c){
	int l = cstringLength(str);
    str[l] = c;
    str[l+1] = '\0';
}

int cstring2int(char *p){
    int k = 0;
    while (*p) {
        k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
     }
     return k;
}