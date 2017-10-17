//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  midi.cc
//
//  save_midi() and code for midi info.
//
//--------------------------------------------------------

/*  Allegro MIDI struct

  typedef struct MIDI                    // a midi file
  {
  int divisions;                      // number of ticks per quarter note
  struct {
  unsigned char *data;             // MIDI message stream
  int len;                         // length of the track data
  } track[MIDI_TRACKS];
  } MIDI;

  */

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <string.h>
#include <stdio.h>


#include "midi.h"
#include "zsys.h"
/* save_midi:
  *  Saves a standard MIDI file, returning 0 on success,
  *  or non-zero on error.
  */

// snprintf is defined as _snprintf in windows stdio.h
#ifdef _MSC_VER
	#define snprintf _snprintf
#endif

int save_midi(char *filename, MIDI *midi)
{
  int c;
  long len;
  PACKFILE *fp;
  int num_tracks = 0;

  if(!midi)
    return 1;

  fp = pack_fopen_password(filename, F_WRITE,"");                       /* open the file */
  if (!fp)
    return 2;

  for (c=0; c<MIDI_TRACKS; c++)
    if(midi->track[c].len > 0)
      num_tracks++;

    pack_fwrite((void *) "MThd", 4, fp);                      /* write midi header */

  pack_mputl(6, fp);                                        /* header chunk length = 6 */

  pack_mputw((num_tracks==1) ? 0 : 1, fp);                  /* MIDI file type */

  pack_mputw(num_tracks, fp);                               /* number of tracks */

  pack_mputw(midi->divisions, fp);                          /* beat divisions (negatives?) */

  for (c=0; c<num_tracks; c++)                              /* write each track */
  {
    pack_fwrite((void *) "MTrk", 4, fp);                    /* write track header */

    len = midi->track[c].len;
    pack_mputl(len, fp);                                    /* length of track chunk */

    if (pack_fwrite(midi->track[c].data, len, fp) != len)
      goto err;
  }

  pack_fclose(fp);
  return 0;

  /* oh dear... */
err:
  pack_fclose(fp);
  delete_file(filename);
  return 3;
}

/* ---  All this code just to calculate the length of a MIDI song.  --- */

/*
  #define MAX_TEMPO_CHANGES 512

  typedef struct midi_info
  {
  // midi info
  int format;
  int num_tracks;
  int divisions;
  int len_beats;
  double len_sec;
  int tempo_changes;
  double tempo[MAX_TEMPO_CHANGES];         // tempo can change during song
  dword  tempo_c[MAX_TEMPO_CHANGES];       // store the total delta time before each change in tempo

  // MTrk event info (used by parse_mtrk())
  dword dt,nbytes;
  byte event,running_status,type,byte2;
  byte *buf;

  } midi_info;

  typedef struct mtrkevent
  // sort of... it's actually not a true MTrk event because it can be a
  // combination of events, such as a "note on" event plus a "note off" event
  {
  // basic mtrk event but with total time instead of delta-time
  byte *buf;       // buffer for extra data
  dword t;         // total time from start in delta-time units
  word dur;        // duration of note
  word event;      // event + extra byte of info
  word nbytes;     // number of bytes in "buf" or note off velocity
  byte type;       // meta type, note number, etc.
  byte byte2;      // byte 2 or note on velocity
  } mtrkevent;

  */

dword getval(byte *buf,int nbytes)
{
  register dword value;
  value=0;

  for(int i=0; i<nbytes; i++)
  {
    value<<=8;
    value+=buf[i];
  }
  return value;
}

dword parse_var_len(byte **data)
{
  dword val = **data & 0x7F;

  while (**data & 0x80)
  {
    (*data)++;
    val <<= 7;
    val += (**data & 0x7F);
  }

  (*data)++;
  return val;
}

dword parse_mtrk(byte **data, midi_info *mi)
  // returns length of the <MTrk event>
{
  byte *start = *data;

  mi->dt = parse_var_len(data);
  mi->event = **data; (*data)++;
  mi->type = 0;
  mi->byte2 = 0;
  mi->nbytes = 0;
  mi->buf = NULL;

  if(mi->event<0x80 && mi->running_status>0)
  {
    mi->event = mi->running_status;
    (*data)--;
  }

  if(mi->event!=0xFF && mi->event!=0xF0 && mi->event!=0xF7)
    mi->running_status = mi->event;

  switch(mi->event)
  {
    case 0xFF:                                              // <meta-event>
    mi->type = **data; (*data)++;
    case 0xF0:
    case 0xF7:                                              // <sysex events>
    mi->nbytes = parse_var_len(data);
    mi->buf = *data;
    (*data) += mi->nbytes;
    break;

    case 0xF2:  (*data)+=2; break;
    case 0xF3:  (*data)++; break;

    default:
    switch(mi->event&0xF0)
    {
      case 0x80:
      case 0x90:
      case 0xA0:
      case 0xB0:
      case 0xE0:  mi->type  = **data; (*data)++;
      mi->byte2 = **data; (*data)++;
      break;
      case 0xC0:
      case 0xD0:  mi->type = **data; (*data)++; break;
    }
  }

  return (*data) - start;
}

bool eot(midi_info *mi)
{
  return (mi->event==0xFF && mi->type==0x2F);
}

int beats(dword dt,int divs)
{
  if(divs<=0)
    return 1;
  return dt/divs + ((dt%divs)?1:0);
}

double tempo(byte *buf)
{
  double t = (double)getval(buf,3);
  t /= 60e6;
  if(t==0)
    return 1;
  return 1/t;
}

double _runtime(int beats,double tempo)
{
  if(tempo==0)
    return 0;
  return beats/tempo*60.0;
}

double runtime(int beats,midi_info *mi)
{
  double t=0;

  int c=0;
  while(c<mi->tempo_changes)
  {
    if(beats < (int)mi->tempo_c[c])
      break;
    c++;
  }

  for(int i=1; i<c; i++)
    t += _runtime(mi->tempo_c[i] - mi->tempo_c[i-1],mi->tempo[i-1]);

  t += _runtime(beats - mi->tempo_c[c-1],mi->tempo[c-1]);

  return t;
}

void get_midi_info(MIDI *midi, midi_info *mi)
{
  dword max_dt = 0;
  mi->tempo_changes = 0;
  bool gottempo = false;

  if(midi==NULL)
    goto done;

  for(int i=0; midi->track[i].len>0; i++)
  {
    byte *data = midi->track[i].data;
    if(data==NULL)
      break;

    dword total_dt=0;
    mi->event=0;
    mi->running_status = 0;
    // tempo info should only be in first track, but sometimes it isn't
    bool gettempo = (i==0)||(!gottempo);

    while(!eot(mi) && data - midi->track[i].data < midi->track[i].len)
    {
      parse_mtrk(&data,mi);
      total_dt += mi->dt;

      if(gettempo && mi->event==0xFF && mi->type==0x51 && mi->tempo_changes<MAX_TEMPO_CHANGES)
      {
        mi->tempo[mi->tempo_changes] = tempo(mi->buf);
        int tempo_c = mi->tempo_c[mi->tempo_changes] = beats(total_dt,midi->divisions);
        if(mi->tempo_changes==0 && tempo_c!=0)              // make sure there is a tempo at beat 0
        {
          mi->tempo_c[0] = 0;
          mi->tempo_c[1] = tempo_c;
          mi->tempo[1] = mi->tempo[0];
          mi->tempo_changes++;
        }
        mi->tempo_changes++;
        gottempo=true;
      }
    }
    max_dt = zc_max(max_dt,total_dt);
  }

done:

  if(mi->tempo_changes==0)                                  // then guess
  {
    mi->tempo_changes=1;
    mi->tempo[0]=120.0;
    mi->tempo_c[0]=0;
  }

  mi->len_beats = (midi==NULL) ? 0 : beats(max_dt,midi->divisions);
  mi->len_sec = (midi==NULL) ? 0 : runtime(mi->len_beats,mi);
}

char *timestr(double sec)
{
  static char buf[16];
  int min = (int)(sec/60);
  sec -= min*60;
  if(sec>=59.5)
  {
    min++;
    sec=0;
  }
  sprintf(buf,"%02d:%02.0f",min,sec);
  return buf;
}

bool decode_text_event(char *s,int length, byte type,byte *buf)
{
  switch(type)
  {
    case 0x01: snprintf(s,length,"T: %s\n",buf); break;
    case 0x02: snprintf(s,length,"C: %s\n",buf); break;
    case 0x03: snprintf(s,length,"N: %s\n",buf); break;
    case 0x04: snprintf(s,length,"I: %s\n",buf); break;
    default:
    return false;
  }
  return true;
}

void get_midi_text(MIDI *midi, midi_info *mi, char *text)
  // must have called get_midi_info() first
{
  byte buf[1024];
  char *out = text;
  int length=4096;
  dword nbytes;

  text[0] = 0;

  for(int i=0; midi->track[i].len>0; i++)
  {
    byte *data = midi->track[i].data;
    if(data==NULL)
      break;

    mi->event=0;
    mi->running_status = 0;

    while(!eot(mi) && data - midi->track[i].data < midi->track[i].len)
    {
      parse_mtrk(&data,mi);

      if(mi->event==0xFF)
      {
        nbytes=zc_min(mi->nbytes, 1023);
        memcpy(buf,mi->buf,nbytes);
        buf[nbytes]=0;
        if(decode_text_event(out,length,mi->type,buf))
        {
          length-=strlen(out);
          out+=strlen(out);
        }
      }
    }
  }
}

