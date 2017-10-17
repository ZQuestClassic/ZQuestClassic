typedef struct textline
{
  textline *prev;
  char     *text;
  textline *next;
  void     *extra;
//  int       maxlen;
} textline;

typedef struct textline_extra
{
  int maxlen;
} textline_extra;

#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

int main()
{
  char src_text[65535];
  char *str_ptr=src_text;

  //read the file into src_text
  FILE *f=fopen("textline.cpp", "rb");
  while (!feof(f))
  {
    *(str_ptr++)=fgetc(f); 
  }
  *str_ptr=0;
  fclose(f);
  
  textline *start=(textline *)malloc(sizeof(textline));
  start->prev=NULL;
  start->next=NULL;
  textline *current=start;


  //find each newline character(s) and split the text into lines
  int pointer=0, old_pointer=0;
  while (src_text[pointer]!=0)
  {
    while (src_text[pointer]!=10&&src_text[pointer]!=13)
    {
      if (src_text[pointer]==0)
      {
        break;
      }
      ++pointer;
    }
    if (src_text[pointer]==0)
    {
      break;
    }
    switch (src_text[pointer])
    {
      case 10:
        if (src_text[++pointer]==13)
        {
          ++pointer;
        }
        break;
      case 13:
        if (src_text[++pointer]==10)
        {
          ++pointer;
        }
        break;
    }

    current->text=(char *)malloc((pointer-old_pointer)*2);
    memcpy(current->text, src_text+old_pointer, pointer-old_pointer);
    current->text[pointer-old_pointer]=0;
    current->extra=(textline_extra *)malloc(sizeof(textline_extra));
    ((textline_extra *)(current->extra))->maxlen=((pointer-old_pointer)*2)-1;
    current->next=(textline *)malloc(sizeof(textline));
    current=current->next;

    old_pointer=pointer;
  }
  if (pointer!=old_pointer)
  {
    current->text=(char *)malloc((pointer-old_pointer)*2);
    memcpy(current->text, src_text+old_pointer, pointer-old_pointer);
    current->text[pointer-old_pointer]=0;
    current->extra=(textline_extra *)malloc(sizeof(textline_extra));
    ((textline_extra *)(current->extra))->maxlen=((pointer-old_pointer)*2)-1;
    current->next=(textline *)malloc(sizeof(textline));
    current=current->next;

    old_pointer=pointer;
  }
  current=start;
  while (current->next!=NULL)
  {
    printf("%3d:  %s",((textline_extra *)(current->extra))->maxlen,current->text);
    current=current->next;
  }
  return 0;
}
