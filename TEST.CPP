#include <stdlib.h>
#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

typedef struct stuff
{
  byte   data1;
  word   data2;
  char  *data3;
  dword  data4;
} stuff;

stuff tstuff;

void changestuff2(stuff *pstuff)
{
  char *q=pstuff->data3;
  free(pstuff->data3);
  pstuff->data3=(char *)malloc(50);
  sprintf(pstuff->data3,"Hello!");
  free(q);
  q=(char *)malloc(90);
  sprintf(q,"DIE!");
}

void changestuff(stuff *pstuff)
{
  pstuff->data1=0;
  changestuff2(pstuff);
}

int main()
{
  tstuff.data1=1;
  char *p=tstuff.data3;
  tstuff.data3=(char *)malloc(10);
  sprintf(tstuff.data3,"Goodbye!");
  changestuff(&tstuff);

  return 0;
}
