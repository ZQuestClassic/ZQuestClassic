typedef struct custGuy150 {
  byte  family;
  byte  cset,spritepal;
  byte  dp,wdp;
  byte  hrate,step,weapon;
  byte  grumble,item_set,frate;
  byte  foo[5];
  short hp;
  word  f[7];
  byte  extra[8];
}

typedef struct newcombo150 {
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte attr;
  word e;
}


typedef struct dmap150 {
 byte map;
 byte level;
 char xoff;
 byte compass;
 byte color;
 byte midi;
 byte cont;
 byte type;
 byte grid[8];
// 16 bytes total
}

typedef struct shoptype150 {
 byte item[3];
 byte price[3];
 byte d1,d2;
}

typedef struct infotype150 {
 byte str[3];
 byte price[3];
 byte d1,d2;
}

typedef struct warpring150 {
 byte dmap[8];
 byte scr[8];
 byte size;
 byte d1;
}

typedef struct windwarp150 {
 byte dmap;
 byte scr;
}


typedef struct zcolors150 {
 byte text,caption;
 byte triforce,triframe;
 byte overw;
 byte map;
 byte dngn_bg, dngn_fg;
 byte cave_bg, cave_fg;
 byte bs_lt, bs_med, bs_dk, bs_goal;
// 14 bytes
}



inline bool pfwrite150(void *p,long n,PACKFILE *f)
{ return pack_fwrite(p,n,f)==n; }


inline bool pfread150(void *p,long n,PACKFILE *f)
{ return pack_fread(p,n,f)==n; }




typedef struct zquestheader150 {
 char  id_str[31];
 short zelda_version;
 short internal;
 byte  quest_number;
 byte  rules[2];
 char  map_count;
 char  str_count;
 byte  data_flags[ZQ_MAXDATA];
 char  foo[3]; // unused
 char  version[9];
 char  title[65];
 char  author[65];
 short pwdkey;
 char  password[30];
}

typedef struct MsgStr150 {
 char s[73];
 byte d1,d2,d3;
// 76 bytes total
}

typedef struct miscQdata150 {
 shoptype shop[16];
 infotype info[16];
 warpring warp[8];
 byte     foo[8*18];
 windwarp wind[9];      // destination of whirlwind for each level
 byte     triforce[8];  // positions of triforce pieces on subscreen
 zcolors  colors;
 byte     misc[32];
}


typedef struct mapscr150 {
// 48-byte header
  byte valid,guy,str,room,        item,catchall,warptype,flags;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,flags2;
  byte enemy[10],pattern, warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte unused_data[2];
// 352 bytes of screen data
  word data[16*11];
// 400 bytes total
} ;


typedef struct combo150 {
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte attr;
  word e[5];
}


typedef struct music150 {
 char title[20];
 long start;
 long loop_start;
 long loop_end;
 short loop;
 short volume;
 MIDI *midi;
}

#define QH_IDSTR150 "AG Zelda Classic Quest File\n "

#define TILEBUF_SIZE150    320*480
#define MAXMIDIS150        16      // uses bit string for midi flags, so 2 bytes

#define ZQ_MAXDATA150      20

#define ZQ_TILES150        0
#define ZQ_MIDIS150        1       // uses bit string for midi flags


#define MAXMAPS150         10
#define TEMPLATE150        MAXMAPS150*130
#define MAXMSGS150         128
#define MAXDMAPS150        32


void *read_block150(PACKFILE *f, int size, int alloc_size)
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
MIDI *read_midi150(PACKFILE *f)
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
	 m->track[c].data = (byte*)read_block150(f, m->track[c].len, 0);
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



void reset_midi150(music *m)
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

 if(!pfread150(header,sizeof(zquestheader150),f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread150(MsgStrings,sizeof(MsgStr150)*MAXMSGS150,f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread150(DMaps,sizeof(dmap)*MAXDMAPS150,f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread150(misc,sizeof(miscQdata150),f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 for(int i=0; i<header->map_count; i++) {
   if(!pfread150(TheMaps+(i*130),sizeof(mapscr150)*130,f)) {
     pack_fclose(f);
     return qe_invalid;
     }
   }
 if(!pfread150(TheMaps+(TEMPLATE150),sizeof(mapscr150),f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread150(combobuf,sizeof(combo150)*512,f)) {
   pack_fclose(f);
   return qe_invalid;
   }
 if(!pfread150(colordata,psTOTAL,f)) {
   pack_fclose(f);
   return qe_invalid;
   }

 if(header->data_flags[0])
 {
   if(!pfread150(tilebuf,TILEBUF_SIZE150,f))
   {
     pack_fclose(f);
     return qe_invalid;
   }
 }
 else
 {
   for(int i=0; i<TILEBUF_SIZE150/4; i++)
     ((long*)tilebuf)[i]=((long*)data[11].dat)[i];
 }

 for(int i=0; i<MAXMIDIS150; i++)
 {
   reset_midi150(midis+i);
   if(get_bit(header->data_flags+ZQ_MIDIS150,i))
   {
     if(!pfread150(midis+i,sizeof(music150),f) ||
        !(midis[i].midi=read_midi150(f)) )
     {
       pack_fclose(f);
       return qe_invalid;
     }
   }
 }

 pack_fclose(f);

 if(header->zelda_version > 0x184)
   return qe_version;

 // minimum zquest version allowed for any quest file
 if(header->zelda_version < 0x150)
   return qe_obsolete;

 #ifndef _ZQUEST_
 // individual minimum version for the specific quest (optional)
 int qnum = header->quest_number;
 if(qnum < 3 && header->zelda_version < min_version[qnum])
   return qe_obsolete;
 #endif

 if(strcmp(header->id_str,QH_IDSTR150))
   return qe_invalid;

 return qe_OK;
}
