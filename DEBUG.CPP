//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  debug.cc
//
//  Misc debugging tools.
//
//--------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include "zc_alleg.h"

static int trace_y=0;

void trace_reset()
{
  trace_y=0;
}

void trace(char *format,...)
{
  char buf[256];

  va_list ap;
  va_start(ap, format);
  vsprintf(buf, format, ap);
  va_end(ap);

  textout_ex(screen,font,buf,0,trace_y,1,0);
  trace_y+=8;
  if(trace_y>=200)
    trace_y=0;
  readkey();
}
 
