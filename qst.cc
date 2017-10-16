/*
  qst.cc
  Jeremy Craner, 1999
  Code for loading '.qst' files in zelda.cc and zquest.cc
*/


#include "zdefs.h"

enum { qe_OK, qe_notfound, qe_invalid, qe_version, qe_obsolete,
       qe_missing, qe_pwd, qe_match };

char *qst_error[] = {"OK","File not found","Invalid quest file",
                     "Version not supported","Obsolete version",
                     "Missing new data" /* but let it pass in ZQuest */ ,
                     "Protected by password", "Doesn't match saved game" };

extern mapscr *TheMaps;
extern MsgStr *MsgStrings;
extern dmap   *DMaps;
extern combo  *combobuf;
extern byte   *colordata;
extern byte   *tilebuf;
extern DATAFILE *data;


char ver_str[12],ord_str[8];


char *VerStr(int version)
{
  sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
  return ver_str;
}


char *ordinal(int num)
{
  static char *ending[4] = {"st","nd","rd","th"};

  char *end;
  int t=(num%100)/10;
  int n=num%10;

  if(n>=1 && n<4 && t!=1)
    end = ending[n-1];
  else
    end = ending[3];

  sprintf(ord_str,"%d%s",num%10000,end);
  return ord_str;
}



bool get_qst_buffers()
{
 if(!(TheMaps=(mapscr*)malloc(sizeof(mapscr)*(130*MAXMAPS+1))))
   return false;
 if(!(MsgStrings=(MsgStr*)malloc(sizeof(MsgStr)*MAXMSGS)))
   return false;
 if(!(DMaps=(dmap*)malloc(sizeof(dmap)*MAXDMAPS)))
   return false;
 if(!(combobuf=(combo*)malloc(sizeof(combo)*512)))
   return false;
 if(!(colordata=(byte*)malloc(psTOTAL)))
   return false;
 if(!(tilebuf=(byte*)malloc(TILEBUF_SIZE)))
   return false;
 return true;
}



inline bool pfwrite(void *p,long n,PACKFILE *f)
{ return pack_fwrite(p,n,f)==n; }


inline bool pfread(void *p,long n,PACKFILE *f)
{ return pack_fread(p,n,f)==n; }


void *read_block(PACKFILE *f, int size, int alloc_size)
{
   void *p;

   p = malloc(MAX(size, alloc_size));
   if (!p)
      return NULL;

   pack_fread(p, size, f);

   if (pack_ferror(f)) {
      free(p);
      return NULL;
   }

   return p;
}



/* read_midi:
 *  Reads MIDI data from a datafile (this is not the same thing as the 
 *  standard midi file format).
 */
MIDI *read_midi(PACKFILE *f)
{
   MIDI *m;
   int c;

   m = (MIDI*)malloc(sizeof(MIDI));
   if (!m)
     return NULL;

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = 0;
      m->track[c].data = NULL;
   }

   m->divisions = pack_mgetw(f);

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = pack_mgetl(f);
      if (m->track[c].len > 0) {
	 m->track[c].data = (byte*)read_block(f, m->track[c].len, 0);
	 if (!m->track[c].data) {
	    destroy_midi(m);
	    return NULL;
	 }
      }
   }

   LOCK_DATA(m, sizeof(MIDI));
   for (c=0; c<MIDI_TRACKS; c++)
      if (m->track[c].data)
	 LOCK_DATA(m->track[c].data, m->track[c].len);

   return m;
}



void reset_midi(music *m)
{
  m->title[0]=0;
  m->loop=1;
  m->volume=144;
  m->start=0;
  m->loop_start=-1;
  m->loop_end=-1;
  if(m->midi)
    destroy_midi(m->midi);
  m->midi=NULL;
}



int loadquest(char *filename, zquestheader *header,
              miscQdata *misc, music *midis)
{
 PACKFILE *f = pack_fopen(filename,F_READ_PACKED);
 if(!f)
   return qe_notfound;

 if(!pfread(header,sizeof(zquestheader),f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread(MsgStrings,sizeof(MsgStr)*MAXMSGS,f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread(DMaps,sizeof(dmap)*MAXDMAPS,f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread(misc,sizeof(miscQdata),f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 for(int i=0; i<header->map_count; i++) {
   if(!pfread(TheMaps+(i*130),sizeof(mapscr)*130,f)) {
     pack_fclose(f);
     return qe_invalid;
     }
   }
 if(!pfread(TheMaps+(TEMPLATE),sizeof(mapscr),f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread(combobuf,sizeof(combo)*512,f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread(colordata,psTOTAL,f)) {
   pack_fclose(f);
   return qe_invalid;
   }

 if(header->data_flags[0])
 {
   if(!pfread(tilebuf,TILEBUF_SIZE,f))
   {
     pack_fclose(f);
     return qe_invalid;
   }
 }
 else
 {
   for(int i=0; i<TILEBUF_SIZE/4; i++)
     ((long*)tilebuf)[i]=((long*)data[TIL_NES].dat)[i];
 }

 for(int i=0; i<MAXMIDIS; i++)
 {
   reset_midi(midis+i);
   if(get_bit(header->data_flags+ZQ_MIDIS,i))
   {
     if(!pfread(midis+i,sizeof(music),f) ||
        !(midis[i].midi=read_midi(f)) )
     {
       pack_fclose(f);
       return qe_invalid;
     }
   }
 }

 pack_fclose(f);

 if(header->zelda_version > ZELDA_VERSION)
   return qe_version;

 // minimum zquest version allowed for any quest file
 if(header->zelda_version < MIN_VERSION)
   return qe_obsolete;

 #ifndef _ZQUEST_
 // individual minimum version for the specific quest (optional)
 int qnum = header->quest_number;
 if(qnum < QUEST_COUNT && header->zelda_version < min_version[qnum])
   return qe_obsolete;
 #endif

 if(strcmp(header->id_str,QH_IDSTR))
   return qe_invalid;

 return qe_OK;
}


/*** end of qst.cc ***/


