//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  midi.cc
//
//  save_midi() and code for midi info.
//
//--------------------------------------------------------

#ifndef _MIDI_H_
#define _MIDI_H_
#include "zdefs.h"
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

int save_midi(char *filename, MIDI *midi);

/* ---  All this code just to calculate the length of a MIDI song.  --- */

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
  double tempo[MAX_TEMPO_CHANGES];                          // tempo can change during song
  dword  tempo_c[MAX_TEMPO_CHANGES];                        // store the total delta time before each change in tempo

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
  byte *buf;                                                // buffer for extra data
  dword t;                                                  // total time from start in delta-time units
  word dur;                                                 // duration of note
  word event;                                               // event + extra byte of info
  word nbytes;                                              // number of bytes in "buf" or note off velocity
  byte type;                                                // meta type, note number, etc.
  byte byte2;                                               // byte 2 or note on velocity
} mtrkevent;

dword getval(byte *buf,int nbytes);
dword parse_var_len(byte **data);
// returns length of the <MTrk event>
dword parse_mtrk(byte **data, midi_info *mi);
bool eot(midi_info *mi);
int beats(dword dt,int divs);
double tempo(byte *buf);
double _runtime(int beats,double tempo);
double runtime(int beats,midi_info *mi);
void get_midi_info(MIDI *midi, midi_info *mi);
char *timestr(double sec);
bool decode_text_event(char *s,byte type,byte *buf);
void get_midi_text(MIDI *midi, midi_info *mi, char *text);
#endif
 
