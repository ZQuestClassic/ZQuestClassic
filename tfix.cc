#include <stdio.h>
#include <stdlib.h>

#define SIZE 153600

int readfile(char *path,void *buf,int count)
{
 FILE *f=fopen(path,"rb");
 if(!f)
  return 0;
 int r=fread(buf,1,count,f);
 fclose(f);
 return r;
}

int writefile(char *path,void *buf,int count)
{
 FILE *f=fopen(path,"wb");
 if(!f)
  return 0;
 int r=fwrite(buf,1,count,f);
 fclose(f);
 return r;
}


int main(int argc,char **argv)
{
 char *tbuf = (char*)malloc(SIZE);
 char *temp = (char*)malloc(64*80);

 if(readfile("data\\nes16.til",tbuf,SIZE)!=SIZE)
   return -1;

 for(int i=0; i<64*80; i++)
   temp[i] = tbuf[i];

 for(int i=0; i<20; i++) {
   int si = i*2;
   int di = i*4;
   for(int j=0; j<64; j++)
     tbuf[di*64+j]=temp[si*64+j];

   si=i*2+40;
   di++;
   for(int j=0; j<64; j++)
     tbuf[di*64+j]=temp[si*64+j];

   si=i*2+1;
   di++;
   for(int j=0; j<64; j++)
     tbuf[di*64+j]=temp[si*64+j];

   si=i*2+41;
   di++;
   for(int j=0; j<64; j++)
     tbuf[di*64+j]=temp[si*64+j];
   }

 writefile("new.til",tbuf,SIZE);
 return 0;
}
