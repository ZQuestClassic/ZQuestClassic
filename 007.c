/*
    007.c
    Encryption utility by Jeremy Craner.
    This format is used for Zelda Classic quest and save files.
*/


#include <stdio.h>
#include <allegro.h>

#define MASK 0x4C358938

static int seed = 0;

int rand_007()
{
  short BX = seed >> 8;
  short CX = (seed&0xff) << 8;
  char AL = seed >> 24;
  char C = AL >> 7;
  char D = BX >> 15;
  AL <<= 1;
  BX = (BX << 1) | C;
  CX = (CX << 1) | D;
  CX += seed&0xffff;
  BX += (seed>>16) + C;
  CX += 0x62E9;
  BX += 0x3619 + D;
  seed = (BX << 16) + CX;
  return (CX << 16) + BX;
}


//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//
int encode_file_007(char *srcfile, char *destfile, int key, char *header)
{
  FILE *src, *dest;
  int tog = 0, c, r;
  short c1 = 0, c2 = 0;

  seed = key;
  src = fopen(srcfile, "rb");
  if(!src)
    return 1;

  dest = fopen(destfile, "wb");
  if(!dest)
  {
    fclose(src);
    return 2;
  }

  // write the header
  if(header)
  {
    for(c=0; header[c]; c++)
      fputc(header[c], dest);
  }

  // write the key, XORed with MASK
  key ^= MASK;
  fputc(key>>24, dest);
  fputc((key>>16)&255, dest);
  fputc((key>>8)&255, dest);
  fputc(key&255, dest);

  // encode the data
  while((c=fgetc(src)) != EOF)
  {
    c1 += c;
    c2 = (c2 << 4) + (c2 >> 12) + c;
    if(tog)
      c += r;
    else
    {
      r = rand_007();
      c ^= r;
    }
    tog ^= 1;

    fputc(c, dest);
  }

  // write the checksums
  r = rand_007();
  c1 ^= r;
  c2 += r;
  fputc(c1>>8, dest);
  fputc(c1&255, dest);
  fputc(c2>>8, dest);
  fputc(c2&255, dest);

  fclose(src);
  fclose(dest);
  return 0;
}


//
// RETURNS:
//   0 - OK
//   1 - srcfile not opened
//   2 - destfile not opened
//   3 - scrfile too small
//   4 - srcfile EOF
//   5 - checksum mismatch
//   6 - header mismatch
//
int decode_file_007(char *srcfile, char *destfile, char *header)
{
  FILE *src, *dest;
  int tog = 0, c, r, err;
  long size, i;
  short c1 = 0, c2 = 0, check1, check2;

  // open files
  size = file_size(srcfile);
  if(size < 1)
    return 1;
  size -= 8;  // get actual data size, minus key and checksums
  if(size < 1)
    return 3;

  src = fopen(srcfile, "rb");
  if(!src)
    return 1;

  dest = fopen(destfile, "wb");
  if(!dest)
  {
    fclose(src);
    return 2;
  }

  // read the header
  err = 4;
  if(header)
  {
    for(i=0; header[i]; i++)
    {
      if((c=fgetc(src)) == EOF)
        goto error;
      if((c&255) != header[i])
      {
        err = 6;
        goto error;
      }
      size--;
    }
  }

  // read the key
  if((c=fgetc(src)) == EOF)
    goto error;
  seed = c << 24;
  if((c=fgetc(src)) == EOF)
    goto error;
  seed += (c & 255) << 16;
  if((c=fgetc(src)) == EOF)
    goto error;
  seed += (c & 255) << 8;
  if((c=fgetc(src)) == EOF)
    goto error;
  seed += c & 255;
  seed ^= MASK;

  // decode the data
  for(i=0; i<size; i++)
  {
    if((c=fgetc(src)) == EOF)
      goto error;

    if(tog)
      c -= r;
    else
    {
      r = rand_007();
      c ^= r;
    }
    tog ^= 1;

    c &= 255;
    c1 += c;
    c2 = (c2 << 4) + (c2 >> 12) + c;

    fputc(c, dest);
  }

  // read checksums
  if((c=fgetc(src)) == EOF)
    goto error;
  check1 = c << 8;
  if((c=fgetc(src)) == EOF)
    goto error;
  check1 += c & 255;
  if((c=fgetc(src)) == EOF)
    goto error;
  check2 = c << 8;
  if((c=fgetc(src)) == EOF)
    goto error;
  check2 += c & 255;

  // verify checksums
  r = rand_007();
  check1 ^= r;
  check2 -= r;
  check1 &= 0xFFFF;
  check2 &= 0xFFFF;
  if(check1 != c1 || check2 != c2)
  {
    err = 5;
    goto error;
  }

  fclose(src);
  fclose(dest);
  return 0;

error:
  fclose(src);
  fclose(dest);
  delete_file(destfile);
  return err;
}


void copy_file(char *src, char *dest)
{
  int c;
  FILE *fin, *fout;
  fin = fopen(src, "rb");
  fout = fopen(dest, "wb");
  while((c=fgetc(fin)) != EOF)
    fputc(c, fout);
  fclose(fin);
  fclose(fout);
}


const char *usage = "Usage: %s <mode> srcfile [destfile] [key] [header]
Modes:
 -e : encode srcfile to destfile using [key [header]]
 -d : decode srcfile to destfile using [header]
 -E : encode srcfile using [key [header]]
 -D : decode srcfile using [header]\n";

int main(int argc, char **argv)
{
  char tmpbuf[L_tmpnam];
  char *tmp = tmpnam(tmpbuf);

  if(argc < 3)
  {
    printf(usage, get_filename(argv[0]));
    return 0;
  }

  if(!strcmp(argv[1],"-e"))
  {
    int key = 0;
    if(argc < 4)
    {
      printf(usage, get_filename(argv[0]));
      return 0;
    }
    if(argc > 4)
    {
      if(!strcmp(argv[4],"zc"))
        key = 0x413F0000 + argv[2][0] + (argv[2][1] << 8);
      else
        key = atol(argv[4]);
    }
    switch(encode_file_007(argv[2], argv[3], key, argc>5 ? argv[5] : NULL))
    {
    case 0: printf("Encoded %s to %s\n", argv[2], argv[3]); break;
    case 1: printf("Error opening %s\n", argv[2]); break;
    case 2: printf("Error opening %s\n", argv[3]); break;
    }
  }
  else if(!strcmp(argv[1],"-E"))
  {
    int key = 0;
    if(argc > 3)
    {
      if(!strcmp(argv[3],"zc"))
        key = 0x413F0000 + argv[2][0] + (argv[2][1] << 8);
      else
        key = atol(argv[3]);
    }

    switch(encode_file_007(argv[2], tmp, key, argc>4 ? argv[4] : NULL))
    {
    case 0: printf("Encoded %s\n", argv[2]);
            copy_file(tmp, argv[2]);
            break;
    case 1: printf("Error opening %s\n", argv[2]); break;
    case 2: printf("Error opening temp file\n"); break;
    }

    delete_file(tmp);
  }
  else if(!strcmp(argv[1],"-d"))
  {
    if(argc < 4)
    {
      printf(usage, get_filename(argv[0]));
      return 0;
    }
    switch(decode_file_007(argv[2], argv[3], argc>4 ? argv[4] : NULL))
    {
    case 0: printf("Decoded %s to %s\n", argv[2], argv[3]); break;
    case 1: printf("Error opening %s\n", argv[2]); break;
    case 2: printf("Error opening %s\n", argv[3]); break;
    case 3: printf("%s too small\n", argv[2]); break;
    case 4: printf("Unexpected EOF in %s\n", argv[2]); break;
    case 5: printf("Checksum error\n"); break;
    case 6: printf("Header error\n"); break;
    }
  }
  else if(!strcmp(argv[1],"-D"))
  {
    switch(decode_file_007(argv[2], tmp, argc>3 ? argv[3] : NULL))
    {
    case 0: printf("Decoded %s\n", argv[2]);
            copy_file(tmp, argv[2]);
            break;
    case 1: printf("Error opening %s\n", argv[2]); break;
    case 2: printf("Error opening temp file\n"); break;
    case 3: printf("%s too small\n", argv[2]); break;
    case 4: printf("Unexpected EOF in %s\n", argv[2]); break;
    case 5: printf("Checksum error\n"); break;
    case 6: printf("Header error\n"); break;
    delete_file(tmp);
    }
  }
  else
    printf(usage, get_filename(argv[0]));

  return 0;
}
