#ifndef MIDI_H_
#define MIDI_H_

#include "base/zdefs.h"

int32_t save_midi(const char *filename, MIDI *midi);

/* ---  All this code just to calculate the length of a MIDI song.  --- */

#define MAX_TEMPO_CHANGES 512
typedef struct midi_info
{
    // midi info
    int32_t format;
    int32_t num_tracks;
    int32_t divisions;
    int32_t len_beats;
    double len_sec;
    int32_t tempo_changes;
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

dword getval(byte *buf,int32_t nbytes);
dword parse_var_len(byte **data);
// returns length of the <MTrk event>
dword parse_mtrk(byte **data, midi_info *mi);
bool eot(midi_info *mi);
int32_t beats(dword dt,int32_t divs);
double tempo(byte *buf);
double _runtime(int32_t beats,double tempo);
double runtime(int32_t beats,midi_info *mi);
void get_midi_info(MIDI *midi, midi_info *mi);
char *timestr(double sec);
bool decode_text_event(char *s,byte type,byte *buf);
void get_midi_text(MIDI *midi, midi_info *mi, char *text);
#endif
