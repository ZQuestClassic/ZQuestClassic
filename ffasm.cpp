#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ffasm.h"
#include "zquest.h"
#include "zsys.h"
#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_LINUX
#define strnicmp strncasecmp
#endif

extern char *datapath, *temppath;

script_command command_list[NUMCOMMANDS]=
{
    //name                args arg1 arg2 more
  { "SETV",                2,   0,   1,   0},
  { "SETR",                2,   0,   0,   0},
  { "ADDR",                2,   0,   0,   0},
  { "ADDV",                2,   0,   1,   0},
  { "SUBR",                2,   0,   0,   0},
  { "SUBV",                2,   0,   1,   0},
  { "MULTR",               2,   0,   0,   0},
  { "MULTV",               2,   0,   1,   0},
  { "DIVR",                2,   0,   0,   0},
  { "DIVV",                2,   0,   1,   0},
  { "WAITFRAME",           0,   0,   0,   0},
  { "GOTO",                1,   1,   0,   0},
  { "CHECKTRIG",           0,   0,   0,   0},
  { "WARP",                2,   1,   1,   0},
  { "COMPARER",            2,   0,   0,   0},
  { "COMPAREV",            2,   0,   1,   0},
  { "GOTOTRUE",            2,   0,   0,   0},
  { "GOTOFALSE",           2,   0,   0,   0},
  { "GOTOLESS",            2,   0,   0,   0},
  { "GOTOMORE",            2,   0,   0,   0},
  { "LOAD1",               2,   0,   0,   0},
  { "LOAD2",               2,   0,   0,   0},
  { "SETA1",               2,   0,   0,   0},
  { "SETA2",               2,   0,   0,   0},
  { "QUIT",                0,   0,   0,   0},
  { "SINR",                2,   0,   0,   0},
  { "SINV",                2,   0,   1,   0},
  { "COSR",                2,   0,   0,   0},
  { "COSV",                2,   0,   1,   0},
  { "TANR",                2,   0,   0,   0},
  { "TANV",                2,   0,   1,   0},
  { "MODR",                2,   0,   0,   0},
  { "MODV",                2,   0,   1,   0},
  { "ABS",                 1,   0,   0,   0},
  { "MINR",                2,   0,   0,   0},
  { "MINV",                2,   0,   1,   0},
  { "MAXR",                2,   0,   0,   0},
  { "MAXV",                2,   0,   1,   0},
  { "RNDR",                2,   0,   0,   0},
  { "RNDV",                2,   0,   1,   0},
  { "FACTORIAL",           1,   0,   0,   0},
  { "POWERR",              2,   0,   0,   0},
  { "POWERV",              2,   0,   1,   0},
  { "IPOWERR",             2,   0,   0,   0},
  { "IPOWERV",             2,   0,   1,   0},
  { "ANDR",                2,   0,   0,   0},
  { "ANDV",                2,   0,   1,   0},
  { "ORR",                 2,   0,   0,   0},
  { "ORV",                 2,   0,   1,   0},
  { "XORR",                2,   0,   0,   0},
  { "XORV",                2,   0,   1,   0},
  { "NANDR",               2,   0,   0,   0},
  { "NANDV",               2,   0,   1,   0},
  { "NORR",                2,   0,   0,   0},
  { "NORV",                2,   0,   1,   0},
  { "XNORR",               2,   0,   0,   0},
  { "XNORV",               2,   0,   1,   0},
  { "NOT",                 1,   0,   0,   0},
  { "LSHIFTR",             2,   0,   0,   0},
  { "LSHIFTV",             2,   0,   1,   0},
  { "RSHIFTR",             2,   0,   0,   0},
  { "RSHIFTV",             2,   0,   1,   0},
  { "TRACER",              1,   0,   0,   0},
  { "TRACEV",              1,   1,   0,   0},
  { "TRACENL",             0,   0,   0,   0},
  { "LOOP",                2,   1,   0,   0},
  { "PUSHR",               1,   0,   0,   0},
  { "PUSHV",               1,   1,   0,   0},
  { "POP",                 1,   0,   0,   0},
  { "ENQUEUER",            2,   0,   0,   0},
  { "ENQUEUEV",            2,   0,   1,   0},
  { "DEQUEUE",             1,   0,   0,   0},
  { "PLAYSOUNDR",          1,   0,   0,   0},
  { "PLAYSOUNDV",          1,   1,   0,   0},
  { "LOADWEAPONR",         1,   0,   0,   0},
  { "LOADWEAPONV",         1,   1,   0,   0},
  { "LOADITEMR",           1,   0,   0,   0},
  { "LOADITEMV",           1,   1,   0,   0},
  { "LOADNPCR",            1,   0,   0,   0},
  { "LOADNPCV",            1,   1,   0,   0},
  { "CREATELWEAPONR",       1,   0,   0,   0},
  { "CREATELWEAPONV",       1,   1,   0,   0},
  { "CREATEITEMR",         1,   0,   0,   0},
  { "CREATEITEMV",         1,   1,   0,   0},
  { "CREATENPCR",          1,   0,   0,   0},
  { "CREATENPCV",          1,   1,   0,   0},
  { "LOADI",               2,   0,   0,   0},
  { "STOREI",              2,   0,   0,   0},
  { "GOTOR",               1,   0,   0,   0},
  { "SQROOTV",             2,   0,   1,   0},
  { "SQROOTR",             2,   0,   0,   0},
  { "CREATEEWEAPONR",       1,   0,   0,   0},
  { "CREATEEWEAPONV",       1,   1,   0,   0},
  { "PITWARP",             2,   1,   1,   0},
  { "WARPR",             2,   0,   0,   0},
  { "PITWARPR",             2,   0,   0,   0},
};


script_variable variable_list[]=
{  //name                id                maxcount       multiple
  { "D",                 D(0),                 8,             0 },
  { "A",                 A(0),                 2,             0 },
  { "DATA",              DATA,                 0,             0 },
  { "CSET",              FCSET,                0,             0 },
  { "DELAY",             DELAY,                0,             0 },
  { "X",                 FX,                   0,             0 },
  { "Y",                 FY,                   0,             0 },
  { "XD",                XD,                   0,             0 },
  { "YD",                YD,                   0,             0 },
  { "XD2",               XD2,                  0,             0 },
  { "YD2",               YD2,                  0,             0 },
  { "FLAG",              FLAG,                 0,             0 },
  { "WIDTH",             WIDTH,                0,             0 },
  { "HEIGHT",            HEIGHT,               0,             0 },
  { "LINK",              LINK,                 0,             0 },
  { "FFFLAGSD",          FFFLAGSD,             0,             0 },
  { "FFCWIDTH",          FFCWIDTH,             0,             0 },
  { "FFCHEIGHT",         FFCHEIGHT,            0,             0 },
  { "FFTWIDTH",          FFTWIDTH,             0,             0 },
  { "FFTHEIGHT",         FFTHEIGHT,            0,             0 },
  { "FFLINK",            FFLINK,               0,             0 },
  { "COMBOD",            COMBOD(0),          176,             3 },
  { "COMBOC",            COMBOC(0),          176,             3 },
  { "COMBOF",            COMBOF(0),          176,             3 },
  { "INPUTSTART",        INPUTSTART,           0,             0 },
  { "INPUTUP",           INPUTUP,              0,             0 },
  { "INPUTDOWN",         INPUTDOWN,            0,             0 },
  { "INPUTLEFT",         INPUTLEFT,            0,             0 },
  { "INPUTRIGHT",        INPUTRIGHT,           0,             0 },
  { "INPUTA",            INPUTA,               0,             0 },
  { "INPUTB",            INPUTB,               0,             0 },
  { "INPUTL",            INPUTL,               0,             0 },
  { "INPUTR",            INPUTR,               0,             0 },
  { "LINKX",             LINKX,                0,             0 },
  { "LINKY",             LINKY,                0,             0 },
  { "LINKDIR",           LINKDIR,              0,             0 },
  { "LINKHP",            LINKHP,               0,             0 },
  { "LINKMP",            LINKMP,               0,             0 },
  { "LINKMAXHP",         LINKMAXHP,            0,             0 },
  { "LINKMAXMP",         LINKMAXMP,            0,             0 },
  { "LINKACTION",        LINKACTION,           0,             0 },
  { "LINKITEMD",		 LINKITEMD,			   0,			  0 },
  /*{ "WPNX",              WPNX,                 0,             0 },
  { "WPNY",              WPNY,                 0,             0 },
  { "WPNDIR",            WPNDIR,               0,             0 },
  { "WPNSTEP",           WPNSTEP,              0,             0 },
  { "WPNANGULAR",        WPNANGULAR,           0,             0 },
  { "WPNANGLE",          WPNANGLE,             0,             0 },
  { "WPNDRAWTYPE",       WPNDRAWTYPE,          0,             0 },
  { "WPNPOWER",          WPNPOWER,             0,             0 },
  { "WPNDEAD",           WPNDEAD,              0,             0 },
  { "WPNID",             WPNID,                0,             0 },
  { "WPNTILE",           WPNTILE,              0,             0 },
  { "WPNCSET",           WPNCSET,              0,             0 },
  { "WPNFLASHCSET",      WPNFLASHCSET,         0,             0 },
  { "WPNFRAMES",         WPNFRAMES,            0,             0 },
  { "WPNFRAME",          WPNFRAME,             0,             0 },
  { "WPNASPEED",         WPNASPEED,            0,             0 },
  { "WPNFLASH",          WPNFLASH,             0,             0 },
  { "WPNFLIP",           WPNFLIP,              0,             0 },
  { "WPNCOUNT",          WPNCOUNT,             0,             0 },*/
  { "ITEMX",             ITEMX,                0,             0 },
  { "ITEMY",             ITEMY,                0,             0 },
  { "ITEMDRAWTYPE",      ITEMDRAWTYPE,         0,             0 },
  { "ITEMID",            ITEMID,               0,             0 },
  { "ITEMTILE",          ITEMTILE,             0,             0 },
  { "ITEMCSET",          ITEMCSET,             0,             0 },
  { "ITEMFLASHCSET",     ITEMFLASHCSET,        0,             0 },
  { "ITEMFRAMES",        ITEMFRAMES,           0,             0 },
  { "ITEMFRAME",         ITEMFRAME,            0,             0 },
  { "ITEMASPEED",        ITEMASPEED,           0,             0 },
  { "ITEMDELAY",         ITEMDELAY,            0,             0 },
  { "ITEMFLASH",         ITEMFLASH,            0,             0 },
  { "ITEMFLIP",          ITEMFLIP,             0,             0 },
  { "ITEMCOUNT",         ITEMCOUNT,            0,             0 },
  { "ICLASSFAMILY",        ICLASSFAMILY,           0,             0 },
  { "ICLASSFAMTYPE",       ICLASSFAMTYPE,          0,             0 },
  { "ICLASSSETGAME",       ICLASSSETGAME,          0,             0 },
  { "ICLASSAMOUNT",        ICLASSAMOUNT,           0,             0 },
  { "ICLASSSETMAX",        ICLASSSETMAX,           0,             0 },
  { "ICLASSMAX",           ICLASSMAX,              0,             0 },
  { "ICLASSCOUNTER",       ICLASSCOUNTER,          0,             0 },
  { "ITEMEXTEND",        ITEMEXTEND,           0,             0 },
  { "NPCX",              NPCX,                 0,             0 },
  { "NPCY",              NPCY,                 0,             0 },
  { "NPCDIR",            NPCDIR,               0,             0 },
  { "NPCRATE",           NPCRATE,              0,             0 },
  { "NPCFRAMERATE",      NPCFRAMERATE,         0,             0 },
  { "NPCHALTRATE",       NPCHALTRATE,          0,             0 },
  { "NPCDRAWTYPE",       NPCDRAWTYPE,          0,             0 },
  { "NPCHP",             NPCHP,                0,             0 },
  { "NPCID",             NPCID,                0,             0 },
  { "NPCDP",             NPCDP,                0,             0 },
  { "NPCWDP",            NPCWDP,               0,             0 },
  { "NPCTILE",           NPCTILE,              0,             0 },
  { "NPCENEMY",          NPCENEMY,             0,             0 },
  { "NPCWEAPON",         NPCWEAPON,            0,             0 },
  { "NPCITEMSET",        NPCITEMSET,           0,             0 },
  { "NPCCSET",           NPCCSET,              0,             0 },
  { "NPCBOSSPAL",        NPCBOSSPAL,           0,             0 },
  { "NPCBGSFX",          NPCBGSFX,             0,             0 },
  { "NPCCOUNT",          NPCCOUNT,             0,             0 },
  { "SD",                SD(0),                8,             0 },
  { "GD",                GD(0),              256,             0 },
  { "SDD",               SDD,                  0,             0 },
  { "GDD",               GDD,                  0,             0 },
  { "SDDD",              SDDD,                 0,             0 },
  { "GAMECOUNTER",       GAMECOUNTER(0),      32,             3 },
  { "GAMEMCOUNTER",      GAMEMCOUNTER(0),     32,             3 },
  { "GAMEDCOUNTER",      GAMEDCOUNTER(0),     32,             3 },
  { "GAMEGENERIC",       GAMEGENERIC(0),     256,             0 },
  { "GAMEITEMS",         GAMEITEMS(0),       256,             0 },
  { "GAMELITEMS",        GAMELITEMS(0),      256,             0 },
  { "GAMELKEYS",         GAMELKEYS(0),       256,             0 },
  { "GAMEDEATHS",        GAMEDEATHS,           0,             0 },
  { "GAMECHEAT",         GAMECHEAT,            0,             0 },
  { "GAMETIME",          GAMETIME,             0,             0 },
  { "GAMEHASPLAYED",     GAMEHASPLAYED,        0,             0 },
  { "GAMETIMEVALID",     GAMETIMEVALID,        0,             0 },
  { "GAMEGUYCOUNT",      GAMEGUYCOUNT,         0,             0 },
  { "GAMECONTSCR",       GAMECONTSCR,          0,             0 },
  { "GAMECONTDMAP",      GAMECONTDMAP,         0,             0 },
  { "GAMECOUNTERD",      GAMECOUNTERD,         0,             0 },
  { "GAMEMCOUNTERD",     GAMEMCOUNTERD,        0,             0 },
  { "GAMEDCOUNTERD",     GAMEDCOUNTERD,        0,             0 },
  { "GAMEGENERICD",      GAMEGENERICD,         0,             0 },
  { "GAMEITEMSD",        GAMEITEMSD,           0,             0 },
  { "GAMELITEMSD",       GAMELITEMSD,          0,             0 },
  { "GAMELKEYSD",        GAMELKEYSD,           0,             0 },
  { "GAMEMAPFLAG",       GAMEMAPFLAG(0),      32,             0 },
  { "GAMEMAPFLAGD",      GAMEMAPFLAGD,         0,             0 },
  { "GAMEMAPFLAGDD",     GAMEMAPFLAGDD,        0,             0 },
  { "GAMEGUYCOUNTD",     GAMEGUYCOUNTD,        0,             0 },
  { "CURMAP",            CURMAP,               0,             0 },
  { "CURSCR",            CURSCR,               0,             0 },
  { "CURDMAP",           CURDMAP,              0,             0 },
  { "COMBODD",           COMBODD,              0,             0 },
  { "COMBOCD",           COMBOCD,              0,             0 },
  { "COMBOFD",           COMBOFD,              0,             0 },
  { "COMBOTD",           COMBOTD,              0,             0 },
  { "COMBOID",           COMBOID,              0,             0 },
  { "COMBOSD",           COMBOSD,              0,             0 },
  { "REFITEMCLASS",      REFITEMCLASS,         0,             0 },
  { "REFITEM",           REFITEM,              0,             0 },
  { "REFFFC",            REFFFC,               0,             0 },
  { "REFLWPN",           REFLWPN,              0,             0 },
  { "REFEWPN",           REFEWPN,              0,             0 },
  { "REFLWPNCLASS",      REFLWPNCLASS,         0,             0 },
  { "REFEWPNCLASS",      REFEWPNCLASS,         0,             0 },
  { "REFNPC",            REFNPC,               0,             0 },
  { "REFNPCCLASS",       REFNPCCLASS,          0,             0 },
  { "LWPNX",             LWPNX,                0,             0 },
  { "LWPNY",             LWPNY,                0,             0 },
  { "LWPNDIR",           LWPNDIR,              0,             0 },
  { "LWPNSTEP",          LWPNSTEP,             0,             0 },
  { "LWPNANGULAR",       LWPNANGULAR,          0,             0 },
  { "LWPNANGLE",         LWPNANGLE,            0,             0 },
  { "LWPNDRAWTYPE",      LWPNDRAWTYPE,         0,             0 },
  { "LWPNPOWER",         LWPNPOWER,            0,             0 },
  { "LWPNDEAD",          LWPNDEAD,             0,             0 },
  { "LWPNID",            LWPNID,               0,             0 },
  { "LWPNTILE",          LWPNTILE,             0,             0 },
  { "LWPNCSET",          LWPNCSET,             0,             0 },
  { "LWPNFLASHCSET",     LWPNFLASHCSET,        0,             0 },
  { "LWPNFRAMES",        LWPNFRAMES,           0,             0 },
  { "LWPNFRAME",         LWPNFRAME,            0,             0 },
  { "LWPNASPEED",        LWPNASPEED,           0,             0 },
  { "LWPNFLASH",         LWPNFLASH,            0,             0 },
  { "LWPNFLIP",          LWPNFLIP,             0,             0 },
  { "LWPNCOUNT",         LWPNCOUNT,            0,             0 },
  { "LWPNEXTEND",        LWPNEXTEND,           0,             0 },
  { "LWPNOTILE",         LWPNOTILE,            0,             0 },
  { "LWPNOCSET",         LWPNOCSET,            0,             0 },
  { "EWPNX",             EWPNX,                0,             0 },
  { "EWPNY",             EWPNY,                0,             0 },
  { "EWPNDIR",           EWPNDIR,              0,             0 },
  { "EWPNSTEP",          EWPNSTEP,             0,             0 },
  { "EWPNANGULAR",       EWPNANGULAR,          0,             0 },
  { "EWPNANGLE",         EWPNANGLE,            0,             0 },
  { "EWPNDRAWTYPE",      EWPNDRAWTYPE,         0,             0 },
  { "EWPNPOWER",         EWPNPOWER,            0,             0 },
  { "EWPNDEAD",          EWPNDEAD,             0,             0 },
  { "EWPNID",            EWPNID,               0,             0 },
  { "EWPNTILE",          EWPNTILE,             0,             0 },
  { "EWPNCSET",          EWPNCSET,             0,             0 },
  { "EWPNFLASHCSET",     EWPNFLASHCSET,        0,             0 },
  { "EWPNFRAMES",        EWPNFRAMES,           0,             0 },
  { "EWPNFRAME",         EWPNFRAME,            0,             0 },
  { "EWPNASPEED",        EWPNASPEED,           0,             0 },
  { "EWPNFLASH",         EWPNFLASH,            0,             0 },
  { "EWPNFLIP",          EWPNFLIP,             0,             0 },
  { "EWPNCOUNT",         EWPNCOUNT,            0,             0 },
  { "EWPNEXTEND",        EWPNEXTEND,           0,             0 },
  { "EWPNOTILE",         EWPNOTILE,            0,             0 },
  { "EWPNOCSET",         EWPNOCSET,            0,             0 },
  { "NPCEXTEND",         NPCEXTEND,            0,             0 },
  { "SP",                SP,                   0,             0 },
  { " ",                -1,                    0,             0 }
};

long ffparse(char *string)
{
  //return int(atof(string)*10000);

  //this function below isn't working too well yet
  //clean_numeric_string(string);
  double negcheck = atof(string);

  //if no decimal point, ascii to int conversion
  char *ptr=strchr(string, '.');
  if (!ptr)
  {
    return atoi(string)*10000;
  }

  long ret=0;
  char *tempstring1;
  tempstring1=(char *)malloc(strlen(string)+5);
  sprintf(tempstring1, string);
  for (int i=0; i<4; ++i)
  {
    tempstring1[strlen(string)+i]='0';
  }
  ptr=strchr(tempstring1, '.');
  *ptr=0;
  ret=atoi(tempstring1)*10000;
  
  ++ptr;
  char *ptr2=ptr;
  ptr2+=4;
  *ptr2=0;
  
  if(negcheck<0)
	  ret-=atoi(ptr);
  else ret+=atoi(ptr);

  return ret;
}

bool ffcheck(char *arg)
{

	for(int i=0;i<0x100;i++)
	{
		if(arg[i]!='\0')
		{
			if(i==0) 
			{
				if(arg[i]!='-' && arg[i]!='.' && !(arg[i] >= '0' && arg[i] <='9'))
					return false;
			}
			else
			{
				if(arg[i]!='.' && !(arg[i] >= '0' && arg[i] <='9'))
					return false;
			}
		}
		else
		{
			i=0x100;
		}
	}
	return true;
}

char labels[65536][80];
int lines[65536];
int numlines;

int parse_script(ffscript **script)
{
  if(!getname("Import Script (.txt)","txt",datapath,false))
    return D_O_K;
  return parse_script_file(script,temppath);
}

int parse_script_file(ffscript **script, const char *path)
{
  saved=false;
  FILE *fscript = fopen(path,"rb");
  char *buffer = new char[0x400];
  char *combuf = new char[0x100];
  char *arg1buf = new char[0x100];
  char *arg2buf = new char[0x100];
  bool stop=false;
  numlines = 0;
  int num_commands;
  for(int i=0;;i++)
  {
    buffer[0]=0;
	if(stop)
	{
		num_commands=i+1;
		break;
	}
    for(int j=0;j<0x400;j++)
    {
      char temp;
      temp = getc(fscript);
      if(feof(fscript)) {stop=true; buffer[j]='\0'; j=0x400; ungetc(temp,fscript);}
      else
      {
        ungetc(temp,fscript);
        buffer[j] = getc(fscript);
        if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
        {
          if(buffer[j] == '\n')
          {
            buffer[j] = '\0';
            j=0x400;
          }
          else
          {
				while(getc(fscript)!='\n') 
				{
					if(feof(fscript)) 
					{
						stop=true;
						break;
					}
				}
            buffer[j] = '\0';
              j=0x400;
          }
        }
      }
    }
    int k=0;
	while(buffer[k] == ' ' || buffer[k] == '\t') k++;
	if(buffer[k] == '\0') { i--; continue; }
	k=0;
    if(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0')
    {
        while(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0') 
        {
          labels[numlines][k] = buffer[k];
          k++;
        }
        labels[numlines][k] = '\0';
        lines[numlines] = i;
        numlines++;
    }
  }

  fseek(fscript, 0, SEEK_SET);
  stop = false;
  if((*script)!=NULL) delete [] (*script);
  (*script) = new ffscript[num_commands];

  for(int i=0;i<num_commands;i++)
  {
    if(stop)
    {
      (*script)[i].command = 0xFFFF;
      break;
    }
    else
    {
/*
      sprintf(buffer, "");
      sprintf(combuf, "");
      sprintf(arg1buf, "");
      sprintf(arg2buf, "");
*/
      buffer[0]=0;
      combuf[0]=0;
      arg1buf[0]=0;
      arg2buf[0]=0;
      for(int j=0;j<0x400;j++)
      {
        char temp;
        temp = getc(fscript);
        if(feof(fscript)) {stop=true; buffer[j]='\0'; j=0x400; ungetc(temp,fscript);}
        else
        {
          ungetc(temp,fscript);
          buffer[j] = getc(fscript);
          if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
          {
            if(buffer[j] == '\n')
            {
              buffer[j] = '\0';
              j=0x400;
            }
            else
            {
				while(getc(fscript)!='\n') 
				{
					if(feof(fscript)) 
					{
						stop=true;
						break;
					}
				}
              buffer[j] = '\0';
                j=0x400;
            }
          }
        }
      }
      int k=0, l=0;
	  while(buffer[k] == ' ' || buffer[k] == '\t') k++;
	  if(buffer[k] == '\0') { i--; continue; }
	  k=0;
      while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0') k++;
      while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0')  k++;
      while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
      {
        combuf[l] = buffer[k]; k++; l++;
      }
      combuf[l] = '\0';
      l=0;
      while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0') k++;
      while(buffer[k] != ',' && buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
      {
        arg1buf[l] = buffer[k]; k++; l++;
      }
      arg1buf[l] = '\0';
      l=0;
      while((buffer[k] == ' ' || buffer[k] == '\t' || buffer[k] == ',') && buffer[k] != '\0') k++;
      while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
      {
        arg2buf[l] = buffer[k]; k++; l++;
      }
      arg2buf[l] = '\0';
	  int parse_err;
      if(!(parse_script_section(combuf, arg1buf, arg2buf, script, i, parse_err)))
      {
        char buf[80],buf2[80],buf3[80],name[13];
		char* errstrbuf[] = {
		"Invalid instruction.",
		"Parameter 1 invalid.",
		"Parameter 2 invalid."
		};
        extract_name(temppath,name,FILENAME8_3);
        sprintf(buf,"Unable to parse instruction %d from script %s",i+1,name);
		sprintf(buf2,"The error was: %s",errstrbuf[parse_err]);
		sprintf(buf3,"The command was (%s) (%s,%s)",combuf,arg1buf,arg2buf);
        jwin_alert("Error",buf,buf2,buf3,"O&K",NULL,'k',0,lfont);
        stop=true;
        (*script)[0].command = 0xFFFF;
	    }
    }
  }
  if(!stop)
  {
    char buf[80],name[13];
    extract_name(temppath,name,FILENAME8_3);
    sprintf(buf,"Script %s has been parsed",name);
    jwin_alert("Success",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
  }
  delete [] buffer;
  delete [] combuf;
  delete [] arg1buf;
  delete [] arg2buf;
  fclose(fscript);
  return D_O_K;
}

int set_argument(char *argbuf, ffscript **script, int com, int argument)
{
  long *arg; 
  if(argument)
  {
    arg = &((*script)[com].arg2);
  }
  else
  {
    arg = &((*script)[com].arg1);
  }
  int i=0;
  char tempvar[20];
  while (variable_list[i].id>-1)
  {
    if (variable_list[i].maxcount>1)
    {
      for (int j=0; j<variable_list[i].maxcount; ++j)
      {
	    if(strcmp(variable_list[i].name,"A")==0)
		  sprintf(tempvar, "%s%d", variable_list[i].name, j+1);
        else sprintf(tempvar, "%s%d", variable_list[i].name, j);
        if(stricmp(argbuf,tempvar)==0)
        {
			long temp = variable_list[i].id+(j*max(1,variable_list[i].multiple));
          *arg = temp;
          return 1;
        }
      }
    }
    else
    {
	  if(stricmp(argbuf,variable_list[i].name)==0)
      {
        *arg = variable_list[i].id;
        return 1;
      }
    }
    ++i;
  }
  return 0;
}

#define ERR_INSTRUCTION 0
#define ERR_PARAM1 1
#define ERR_PARAM2 2

int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, ffscript **script, int com, int &retcode)
{
  (*script)[com].arg1 = 0;
  (*script)[com].arg2 = 0;
  bool found_command=false;
  
  for (int i=0; i<NUMCOMMANDS&&!found_command; ++i)
  {
    if (strcmp(combuf,command_list[i].name)==0)
    {
      found_command=true;
      (*script)[com].command = i;
      if (((strnicmp(combuf,"GOTO",4)==0)||(strnicmp(combuf,"LOOP",4)==0)) && stricmp(combuf, "GOTOR"))
      {
        bool nomatch = true;
        for(int j=0;j<numlines;j++)
        {
          if(stricmp(arg1buf,labels[j])==0)
          {
            (*script)[com].arg1 = lines[j];
            nomatch = false;
            j=numlines;
          }
        }
        if(nomatch)
        {
          (*script)[com].arg1 = atoi(arg1buf)-1;
        }
        if(strnicmp(combuf,"LOOP",4)==0)
        {
          if (command_list[i].arg2_type==1) //this should NEVER happen with a loop, as arg2 needs to be a variable
          {
            if (!ffcheck(arg2buf))
            {
              retcode=ERR_PARAM2;
              return 0;
            }
            (*script)[com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
          }
          else
          {
            if(!set_argument(arg2buf, script, com, 1))
            {
              retcode=ERR_PARAM2;
              return 0;
            }
          }
        }
      }
      else
      {
        if (command_list[i].args>0)
        {
		  if (command_list[i].arg1_type==1)
          {
            if (!ffcheck(arg1buf))
            {
              retcode=ERR_PARAM1;
              return 0;
            }
            (*script)[com].arg1 = vbound(ffparse(arg1buf),-327680000,327680000);
          }
          else
          {
            if(!set_argument(arg1buf, script, com, 0))
            {
              retcode=ERR_PARAM1;
              return 0;
            }
          }
          if (command_list[i].args>1)
          {
            if (command_list[i].arg2_type==1)
            {
              if (!ffcheck(arg2buf))
              {
                retcode=ERR_PARAM2;
                return 0;
              }
              (*script)[com].arg2 = vbound(ffparse(arg2buf),-327680000,327680000);
            }
            else
            {
              if(!set_argument(arg2buf, script, com, 1))
              {
                retcode=ERR_PARAM2;
                return 0;
              }
            }
          }
        }
      }
    }
  }
  if (found_command)
  {
    return 1;
  }
  retcode=ERR_INSTRUCTION;
  return 0;
}
