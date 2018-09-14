//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_class.cc
//
//  Contains zmap class and other main code for ZQuest.
//
//--------------------------------------------------------

/************************/
/****** ZMAP class ******/
/************************/

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <string>
#include <stdexcept>
#include <map>

#include "gui.h"
#include "zq_class.h"
#include "zq_misc.h"
#include "zquest.h"
#include "qst.h"
#include "colors.h"
#include "tiles.h"
#include "zquestdat.h"
#include "zsys.h"
#include "sprite.h"
#include "items.h"
#include "maps.h"
#include "zc_sys.h"
#include "md5.h"
#include "zc_custom.h"
#include "subscr.h"
#include "zq_strings.h"
#include "zq_subscr.h"
#include "mem_debug.h"

using std::string;
using std::pair;
#define EPSILON 0.01 // Define your own tolerance
#define FLOAT_EQ(x,v) (((v - EPSILON) < x) && (x <( v + EPSILON)))

//const char zqsheader[30]="Zelda Classic String Table\n\x01";
extern char msgbuf[MSGSIZE*3];

extern string zScript;
extern std::map<int, pair<string, string> > ffcmap;
extern std::map<int, pair<string, string> > globalmap;
extern std::map<int, pair<string, string> > itemmap;
zmap Map;
int prv_mode=0;
short ffposx[32]= {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                   -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000
                  };
short ffposy[32]= {-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
                   -1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000
                  };
long ffprvx[32]= {-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000
                 };
long ffprvy[32]= {-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
                  -10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000
                 };

bool save_warn=true;

void reset_dmap(int index)
{
    bound(index,0,MAXDMAPS-1);
    memset(&DMaps[index],0,sizeof(dmap));
    sprintf(DMaps[index].title, "                    ");
    sprintf(DMaps[index].intro, "                                                                        ");
}

void reset_dmaps()
{
    for(int i=0; i<MAXDMAPS; i++)
        reset_dmap(i);
}


mapscr* zmap::get_prvscr()
{
    return &prvscr;
}

zmap::zmap()
{
    can_undo=can_paste=false;
    prv_cmbcycle=0;
    prv_advance=0;
    prv_freeze=0;
    copyffc=-1;
    
    screens=NULL;
    prv_time=0;
    prv_scr=0;
    prv_map=0;
    copyscr=0;
    currscr=0;
    copymap=0;
    currmap=0;
    layer_target_map = 0;
    layer_target_scr = 0;
    layer_target_multiple = 0;
    can_undo_map=false;
    can_paste_map=false;
    screen_copy=false;
    
}
zmap::~zmap()
{
}

bool zmap::CanUndo()
{
    return can_undo;
}
bool zmap::CanPaste()
{
    return can_paste;
}
int  zmap::CopyScr()
{
    return (copymap<<8)+copyscr;
}
int zmap::getCopyFFC()
{
    return copyffc;
}
int zmap::getMapCount()
{
    return map_count;
}
int zmap::getLayerTargetMap()
{
    return layer_target_map;
}
int zmap::getLayerTargetScr()
{
    return layer_target_scr;
}
int zmap::getLayerTargetMultiple()
{
    return layer_target_multiple;
}
bool zmap::isDungeon(int scr)
{
    for(int i=0; i<4; i++)
    {
        if(screens[scr].data[i]!=screens[TEMPLATE].data[i])
        {
            return false;
        }
    }
    
    return true;
}

bool zmap::clearall(bool validate)
{
    Color=0;
    char tbuf[10];
    
    if((header.templatepath[0]!=0)&&validate)
    {
        if(!valid_zqt(header.templatepath))
        {
            jwin_alert("Error","Invalid Quest Template",NULL,NULL,"O&K",NULL,'k',0,lfont);
            return false;
        }
    }
    
    for(int i=0; i<map_count; i++)
    {
        setCurrMap(i);
        sprintf(tbuf, "%d", i);
        clearmap(true);
    }
    
    setCurrMap(0);
    return true;
}

bool zmap::reset_templates(bool validate)
{
    //why are we doing this?
    if(colordata==NULL)
    {
        return false;
    }
    
    char *deletefilename;
    deletefilename=(char *)zc_malloc(1);
    deletefilename[0]=0;
    
    //int ret;
    word version, build, dummy, sversion=0;
    //long section_size;
    word temp_map_count;
    mapscr temp_mapscr;
    PACKFILE *f=NULL;
    
//  setPackfilePassword(datapwd);
    f=open_quest_template(&header, deletefilename, validate);
    get_version_and_build(f, &version, &build);
    
    if(!find_section(f, ID_MAPS))
    {
//	  setPackfilePassword(NULL);
        return false;
    }
    
    //section version info
    if(!p_igetw(&sversion,f,true))
    {
        return false;
    }
    
    if(!p_igetw(&dummy,f,true))
    {
        return false;
    }
    
    //section size
    if(!p_igetl(&dummy,f,true))
    {
        return false;
    }
    
    //finally...  section data
    if(!p_igetw(&temp_map_count,f,true))
    {
        return false;
    }
    
    zcmap temp_map;
    
    if(version>12)
    {
        if(!p_getc(&(temp_map.tileWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_map.tileHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subaWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subaHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subpWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subpHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.scrResWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.scrResHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewX),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewY),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_map.subaTrans),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_map.subpTrans),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_map.scrResWidth = 256;
        temp_map.scrResHeight = 224;
        temp_map.tileWidth = 16;
        temp_map.tileHeight = 11;
        temp_map.viewWidth = 256;
        temp_map.viewHeight = 176;
        temp_map.viewX = 0;
        temp_map.viewY = 64;
        temp_map.subaWidth = 256;
        temp_map.subaHeight = 168;
        temp_map.subaTrans = false;
        temp_map.subpWidth = 256;
        temp_map.subpHeight = 56;
        temp_map.subpTrans = false;
    }
    
    for(int i=0; i<MAPSCRSNORMAL; ++i)
    {
        readmapscreen(f, &header, &temp_mapscr, &temp_map, sversion);
    }
    
    readmapscreen(f, &header, &TheMaps[128], &temp_map, sversion);
    readmapscreen(f, &header, &TheMaps[129], &temp_map, sversion);
    
    for(int i=0; i<(MAPSCRS-(MAPSCRSNORMAL+2)); ++i)
    {
        readmapscreen(f, &header, &temp_mapscr, &temp_map, sversion);
    }
    
    if(version>12)
    {
        if(!p_getc(&(temp_map.tileWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_map.tileHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subaWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subaHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subpWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.subpHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.scrResWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.scrResHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewWidth),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewHeight),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewX),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_igetw(&(temp_map.viewY),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_map.subaTrans),f,true))
        {
            return qe_invalid;
        }
        
        if(!p_getc(&(temp_map.subpTrans),f,true))
        {
            return qe_invalid;
        }
    }
    else
    {
        temp_map.scrResWidth = 256;
        temp_map.scrResHeight = 224;
        temp_map.tileWidth = 16;
        temp_map.tileHeight = 11;
        temp_map.viewWidth = 256;
        temp_map.viewHeight = 176;
        temp_map.viewX = 0;
        temp_map.viewY = 64;
        temp_map.subaWidth = 256;
        temp_map.subaHeight = 168;
        temp_map.subaTrans = false;
        temp_map.subpWidth = 256;
        temp_map.subpHeight = 56;
        temp_map.subpTrans = false;
    }
    
    for(int i=0; i<MAPSCRSNORMAL; ++i)
    {
        readmapscreen(f, &header, &temp_mapscr, &temp_map, sversion);
    }
    
    readmapscreen(f, &header, &TheMaps[MAPSCRS+128], &temp_map, sversion);
    readmapscreen(f, &header, &TheMaps[MAPSCRS+129], &temp_map, sversion);
    
    pack_fclose(f);
    
    if(deletefilename[0]==0)
    {
        delete_file(deletefilename);
    }
    
//  setPackfilePassword(NULL);

    return true;
}

void zmap::clearzcmap(int map)
{
    ZCMaps[map].scrResWidth = 256;
    ZCMaps[map].scrResHeight = 224;
    ZCMaps[map].tileWidth = 16;
    ZCMaps[map].tileHeight = 11;
    ZCMaps[map].viewWidth = 256;
    ZCMaps[map].viewHeight = 176;
    ZCMaps[map].viewX = 0;
    ZCMaps[map].viewY = 64;
    ZCMaps[map].subaWidth = 256;
    ZCMaps[map].subaHeight = 168;
    ZCMaps[map].subaTrans = false;
    ZCMaps[map].subpWidth = 256;
    ZCMaps[map].subpHeight = 56;
    ZCMaps[map].subpTrans = false;
}

bool zmap::clearmap(bool newquest)
{
    if(currmap<map_count)
    {
        clearzcmap(currmap);
        
        for(int i=0; i<MAPSCRS-(newquest?0:TEMPLATES); i++)
        {
            clearscr(i);
        }
        
        setCurrScr(0);
        
        if(newquest)
        {
            if(!reset_templates(false))
            {
                jwin_alert("Error","Error resetting","template screens.",NULL,"O&K",NULL,'k',0,lfont);
            }
        }
    }
    
    return true;
}

mapscr* zmap::CurrScr()
{
    return screens+currscr;
}
mapscr* zmap::Scr(int scr)
{
    return screens+scr;
}
mapscr* zmap::AbsoluteScr(int scr)
{
    return &TheMaps[scr];
}
mapscr* zmap::AbsoluteScr(int map, int scr)
{
    return &TheMaps[(map*MAPSCRS)+scr];
}
void zmap::set_prvscr(int map, int scr)
{

    prvscr=TheMaps[(map*MAPSCRS)+scr];
    
    const int _mapsSize = ZCMaps[map].tileWidth*ZCMaps[map].tileHeight;
    
    prvscr.data.resize(_mapsSize, 0);
    prvscr.sflag.resize(_mapsSize, 0);
    prvscr.cset.resize(_mapsSize, 0);
    
    for(int i=0; i<6; i++)
    {
        if(prvscr.layermap[i]>0)
        {
        
            if((ZCMaps[prvscr.layermap[i]-1].tileWidth==ZCMaps[map].tileWidth) && (ZCMaps[prvscr.layermap[i]-1].tileHeight==ZCMaps[map].tileHeight))
            {
                prvlayers[i]=TheMaps[(prvscr.layermap[i]-1)*MAPSCRS+prvscr.layerscreen[i]];
                
                prvlayers[i].data.resize(_mapsSize, 0);
                prvlayers[i].sflag.resize(_mapsSize, 0);
                prvlayers[i].cset.resize(_mapsSize, 0);
                
            }
            else
            {
                // memset(prvlayers+i,0,sizeof(mapscr));
            }
        }
    }
    
    prv_map=map;
    prv_scr=scr;
}
int  zmap::getCurrMap()
{
    return currmap;
}
bool zmap::isDark()
{
    return (screens[currscr].flags&4)!=0;
}
void zmap::setCurrMap(int index)
{
    int oldmap=currmap;
    scrpos[currmap]=currscr;
    currmap=bound(index,0,map_count);
    screens=&TheMaps[currmap*MAPSCRS];
    
    currscr=scrpos[currmap];
    loadlvlpal(getcolor());
    
    if(currmap!=oldmap)
    {
        can_undo=false;
    }
    
    reset_combo_animations2();
}

int  zmap::getCurrScr()
{
    return currscr;
}
void zmap::setCurrScr(int scr)
{
    if(scr==currscr) return;
    
    int oldscr=currscr;
    int oldcolor=getcolor();
    
    if(!(screens[currscr].valid&mVALID))
    {
        oldcolor=-1;
    }
    
    currscr=bound(scr,0,MAPSCRS-1);
    int newcolor=getcolor();
    loadlvlpal(newcolor);
    
    //setcolor(newcolor);
    if(!(screens[currscr].valid&mVALID))
    {
        newcolor=-1;
    }
    
    if(newcolor!=oldcolor)
    {
        rebuild_trans_table();
    }
    
    if(currscr!=oldscr)
    {
        can_undo=false;
    }
    
    reset_combo_animations2();
    setlayertarget();
}

void zmap::setlayertarget()
{
    layer_target_map = 0;
    layer_target_multiple = 0;
    
    for(int m=0; m<getMapCount(); ++m)
    {
        for(int s=0; s<MAPSCRS; ++s)
        {
            int i=(m*MAPSCRS+s);
            mapscr *ts=&TheMaps[i];
            
            // Search through each layer
            for(int w=0; w<6; ++w)
            {
                if(ts->layerscreen[w]==currscr && (ts->layermap[w]-1)==currmap)
                {
                    if(layer_target_map > 0)
                    {
                        layer_target_multiple += 1;
                        continue;
                    }
                    
                    layer_target_map = m+1;
                    layer_target_scr = s;
                }
            }
        }
    }
}

void zmap::setcolor(int c)
{
    if(screens[currscr].valid&mVALID)
    {
        screens[currscr].color = c;
        
        if(Color!=c)
        {
            Color = c;
            loadlvlpal(c);
        }
    }
}

int zmap::getcolor()
{
    if(prv_mode)
    {
        return prvscr.color;
    }
    
    return screens[currscr].color;
}

void zmap::resetflags()
{
    byte *di=&(screens[currscr].valid);
    
    for(int i=1; i<48; i++)
    {
        *(di+i)=0;
    }
}

word zmap::tcmbdat(int pos)
{
    return screens[TEMPLATE].data[pos];
}

word zmap::tcmbcset(int pos)
{
    return screens[TEMPLATE].cset[pos];
}

int zmap::tcmbflag(int pos)
{
    return screens[TEMPLATE].sflag[pos];
}

word zmap::tcmbdat2(int pos)
{
    return screens[TEMPLATE2].data[pos];
}

word zmap::tcmbcset2(int pos)
{
    return screens[TEMPLATE2].cset[pos];
}

int zmap::tcmbflag2(int pos)
{
    return screens[TEMPLATE2].sflag[pos];
}

void zmap::TemplateAll()
{
    for(int i=0; i<128; i++)
    {
        if((screens[i].valid&mVALID) && isDungeon(i))
            Template(-1,i);
    }
}

void zmap::Template(int floorcombo, int floorcset)
{
    Template(floorcombo, floorcset, currscr);
}

void zmap::Template(int floorcombo, int floorcset, int scr)
{
    if(scr==TEMPLATE)
        return;
        
    if(!(screens[scr].valid&mVALID))
        screens[scr].color=Color;
        
    screens[scr].valid|=mVALID;
    
    for(int i=0; i<32; i++)
    {
        screens[scr].data[i]=screens[TEMPLATE].data[i];
        screens[scr].cset[i]=screens[TEMPLATE].cset[i];
        screens[scr].sflag[i]=screens[TEMPLATE].sflag[i];
    }
    
    for(int i=144; i<176; i++)
    {
        screens[scr].data[i]=screens[TEMPLATE].data[i];
        screens[scr].cset[i]=screens[TEMPLATE].cset[i];
        screens[scr].sflag[i]=screens[TEMPLATE].sflag[i];
    }
    
    for(int y=2; y<=9; y++)
    {
        int j=y<<4;
        screens[scr].data[j]=screens[TEMPLATE].data[j];
        screens[scr].cset[j]=screens[TEMPLATE].cset[j];
        screens[scr].sflag[j]=screens[TEMPLATE].sflag[j];
        ++j;
        screens[scr].data[j]=screens[TEMPLATE].data[j];
        screens[scr].cset[j]=screens[TEMPLATE].cset[j];
        screens[scr].sflag[j]=screens[TEMPLATE].sflag[j];
        ++j;
        j+=12;
        screens[scr].data[j]=screens[TEMPLATE].data[j];
        screens[scr].cset[j]=screens[TEMPLATE].cset[j];
        screens[scr].sflag[j]=screens[TEMPLATE].sflag[j];
        ++j;
        screens[scr].data[j]=screens[TEMPLATE].data[j];
        screens[scr].cset[j]=screens[TEMPLATE].cset[j];
        
        screens[scr].sflag[j]=screens[TEMPLATE].sflag[j];
        ++j;
    }
    
    if(floorcombo!=-1)
    {
        for(int y=2; y<9; y++)
            for(int x=2; x<14; x++)
            {
                int i=(y<<4)+x;
                screens[scr].data[i] = floorcombo;
                screens[scr].cset[i] = floorcset;
            }
    }
    
    for(int i=0; i<4; i++)
        putdoor(scr,i,screens[scr].door[i]);
}

void zmap::putdoor(int side,int door)
{
    putdoor(currscr,side,door);
}

void zmap::putdoor2(int side,int door)
{
    putdoor2(currscr,side,door);
}



// void putdoor(int scr,int side,int door);
// void putdoor2(int scr,int side,int door);
// void dowarp(int type);
// void dowarp(int ring,int index);

void zmap::clearscr(int scr)
{
    screens[scr].zero_memory();
    
    for(int i=0; i<6; i++)
        screens[scr].layeropacity[i]=255;
        
    screens[scr].valid=mVERSION;
    screens[scr].screen_midi=-1;
    screens[scr].csensitive=1;
    screens[scr].bosssfx=screens[scr].oceansfx=0;
    screens[scr].secretsfx=27; // WAV_SECRET
    screens[scr].holdupsfx=20; // WAV_HOLDUP
    
    for(int i=0; i<32; i++)
    {
        screens[scr].ffwidth[i]=15;
        screens[scr].ffheight[i]=15;
    }
}

const char *loaderror[] =
{

    "OK","File not found","Incomplete data",
    "Invalid version","Invalid file"
    
};

int zmap::load(const char *path)
{
    // int size=file_size(path);
    
    PACKFILE *f=pack_fopen_password(path,F_READ, "");
    
    if(!f)
        return 1;
        
        
    short version;
    byte build;
    
    //get the version
    if(!p_igetw(&version,f,true))
    {
        goto file_error;
    }
    
    //get the build
    if(!p_getc(&build,f,true))
    {
        goto file_error;
    }
    
    zcmap temp_map;
    temp_map.scrResWidth = 256;
    temp_map.scrResHeight = 224;
    temp_map.tileWidth = 16;
    temp_map.tileHeight = 11;
    temp_map.viewWidth = 256;
    temp_map.viewHeight = 176;
    temp_map.viewX = 0;
    temp_map.viewY = 64;
    temp_map.subaWidth = 256;
    temp_map.subaHeight = 168;
    temp_map.subaTrans = false;
    temp_map.subpWidth = 256;
    temp_map.subpHeight = 56;
    temp_map.subpTrans = false;
    
    for(int i=0; i<MAPSCRS; i++)
    {
        mapscr tmpimportscr;
        
        if(readmapscreen(f,&header,&tmpimportscr,&temp_map,version)==qe_invalid)
            goto file_error;
            
        bool copied = false;
        
        switch(ImportMapBias)
        {
        case 0:
            *(screens+i) = tmpimportscr;
            copied = true;
            break;
            
        case 1:
            if(!(screens[i].valid&mVALID))
            {
                *(screens+i) = tmpimportscr;
                copied = true;
            }
            
            break;
            
        case 2:
            if(tmpimportscr.valid&mVALID)
            {
                *(screens+i) = tmpimportscr;
                copied = true;
            }
            
            break;
        }
        
        if(!copied)
        {
        }
    }
    
    
    pack_fclose(f);
    
    if(!(screens[0].valid&mVERSION))
    {
        jwin_alert("Confirm Clear All","Clear all?",NULL,NULL,"O&K",NULL,'k',0,lfont);
        clearmap(false);
        return 3;
    }
    
    setCurrScr(0);
    return 0;
    
file_error:
    pack_fclose(f);
    clearmap(false);
    return 2;
}

int zmap::save(const char *path)
{
    PACKFILE *f=pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
        return 1;
        
    short version=ZELDA_VERSION;
    byte  build=VERSION_BUILD;
    
    if(!p_iputw(version,f))
    {
        pack_fclose(f);
        return 3;
    }
    
    if(!p_putc(build,f))
    {
        pack_fclose(f);
        return 3;
    }
    
    for(int i=0; i<MAPSCRS; i++)
    {
        if(writemapscreen(f,this->getCurrMap(),i) == qe_invalid)
        {
            pack_fclose(f);
            return 2;
        }
    }
    
    pack_fclose(f);
    return 0;
}


bool zmap::ishookshottable(int bx, int by, int i)
{
    // Hookshots can be blocked by solid combos on all 3 ground layers.
    newcombo c = combobuf[MAPCOMBO(bx,by)];
    
    if(c.type != cHOOKSHOTONLY && c.type != cLADDERHOOKSHOT && c.walk&(1<<i))
    {
        return false;
    }
    
    for(int k=0; k<2; k++)
    {
        c = combobuf[MAPCOMBO2(k+1,bx,by)];
        
        if(c.type != cHOOKSHOTONLY && c.type != cLADDERHOOKSHOT && c.walk&(1<<i))
        {
            return false;
        }
    }
    
    return true;
}

bool zmap::isstepable(int combo)
{
    // This is kind of odd but it's true to the engine (see maps.cpp)
    return (combo_class_buf[combobuf[combo].type].ladder_pass);
}

// Returns the letter of the warp combo.
int zmap::warpindex(int combo)
{
    switch(combobuf[combo].type)
    {
    case cCAVE:
    case cPIT:
    case cSTAIR:
    case cCAVE2:
    case cSWIMWARP:
    case cDIVEWARP:
    case cSWARPA:
        return 0;
        
    case cCAVEB:
    case cPITB:
    case cSTAIRB:
    case cCAVE2B:
    case cSWIMWARPB:
    case cDIVEWARPB:
    case cSWARPB:
        return 1;
        
    case cCAVEC:
    case cPITC:
    case cSTAIRC:
    case cCAVE2C:
    case cSWIMWARPC:
    case cDIVEWARPC:
    case cSWARPC:
        return 2;
        
    case cCAVED:
    case cPITD:
    case cSTAIRD:
    case cCAVE2D:
    case cSWIMWARPD:
    case cDIVEWARPD:
    case cSWARPD:
        return 3;
        
    case cPITR:
    case cSTAIRR:
    case cSWARPR:
        return 4;
    }
    
    return -1;
    
}

void zmap::put_walkflags_layered(BITMAP *dest,int x,int y,int pos,int layer)
{
    int cx = COMBOX(pos);
    int cy = COMBOY(pos);
    
    newcombo c = combobuf[ MAPCOMBO2(layer,cx,cy) ];
    
    for(int i=0; i<4; i++)
    {
        int tx=((i&2)<<2)+x;
        int ty=((i&1)<<3)+y;
        
        if(layer==0 && combo_class_buf[c.type].water!=0 && get_bit(quest_rules, qr_DROWN))
            rectfill(dest,tx,ty,tx+7,ty+7,vc(9));
            
        if(c.walk&(1<<i))
        {
            if(c.type==cLADDERHOOKSHOT && isstepable(MAPCOMBO(cx,cy)) && ishookshottable(cx,cy,i))
            {
                for(int k=0; k<8; k+=2)
                    for(int j=0; j<8; j+=2)
                        rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,vc(6+((k+j)/2)%2));
            }
            else
            {
                int color = vc(12);
                
                if(isstepable(MAPCOMBO(cx,cy)))
                    color=vc(6);
                else if((c.type==cHOOKSHOTONLY || c.type==cLADDERHOOKSHOT) && ishookshottable(cx,cy,i))
                    color=vc(7);
                    
                rectfill(dest,tx,ty,tx+7,ty+7,color);
            }
        }
    }
    
    // Draw damage combos
    bool dmg = combo_class_buf[combobuf[MAPCOMBO2(-1,cx,cy)].type].modify_hp_amount
               || combo_class_buf[combobuf[MAPCOMBO2(0,cx,cy)].type].modify_hp_amount
               || combo_class_buf[combobuf[MAPCOMBO2(1,cx,cy)].type].modify_hp_amount;
               
    if(dmg)
    {
        for(int k=0; k<16; k+=2)
            for(int j=0; j<16; j+=2)
                if(((k+j)/2)%2)
                    rectfill(dest,x+k,y+j,x+k+1,y+j+1,vc(14));
    }
}

void put_walkflags(BITMAP *dest,int x,int y,word cmbdat,int layer)
{
    newcombo c = combobuf[cmbdat];
    
    for(int i=0; i<4; i++)
    {
        int tx=((i&2)<<2)+x;
        int ty=((i&1)<<3)+y;
        
        if(layer==0 && combo_class_buf[c.type].water!=0 && get_bit(quest_rules, qr_DROWN))
            rectfill(dest,tx,ty,tx+7,ty+7,vc(9));
            
        if(c.walk&(1<<i))
        {
            if(c.type==cLADDERHOOKSHOT)
            {
                for(int k=0; k<8; k+=2)
                    for(int j=0; j<8; j+=2)
                        rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,vc(6+((k+j)/2)%2));
            }
            else
            {
                int color = vc(12);
                
                if(c.type==cLADDERONLY)
                    color=vc(6);
                else if(c.type==cHOOKSHOTONLY)
                    color=vc(7);
                    
                rectfill(dest,tx,ty,tx+7,ty+7,color);
            }
        }
        
        // Draw damage combos
        if(combo_class_buf[c.type].modify_hp_amount != 0)
        {
            for(int k=0; k<8; k+=2)
                for(int j=0; j<8; j+=2)
                    if(((k+j)/2)%2) rectfill(dest,tx+k,ty+j,tx+k+1,ty+j+1,vc(4));
        }
    }
}


void put_flags(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag)
{

    newcombo c = combobuf[cmbdat];
    
    if((flags&cFLAGS)&&(sflag||combobuf[cmbdat].flag))
    {
        //    rectfill(dest,x,y,x+15,y+15,vc(cmbdat>>10+1));
        //    text_mode(-1);
        //    textprintf_ex(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),-1,"%d",sflag);
        if(sflag)
        {
            rectfill(dest,x,y,x+15,y+15,vc(sflag&15));
            textprintf_ex(dest,z3smallfont,x+1,y+1,vc(15-(sflag&15)),-1,"%d",sflag);
        }
        
        if(combobuf[cmbdat].flag)
        {
            rectfill(dest,x,y+(sflag?8:0),x+15,y+15,vc((combobuf[cmbdat].flag)&15));
            textprintf_ex(dest,z3smallfont,x+1,y+9,vc(15-((combobuf[cmbdat].flag)&15)),-1,"%d",combobuf[cmbdat].flag);
        }
    }
    
    if(flags&cCSET)
    {
        bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
        //    text_mode(inv?vc(15):vc(0));
        textprintf_ex(dest,z3smallfont,x+9,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",cset);
    }
    else if(flags&cCTYPE)
    {
        bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
        //    text_mode(inv?vc(15):vc(0));
        textprintf_ex(dest,z3smallfont,x+1,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",c.type);
    }
}

void put_combo(BITMAP *dest,int x,int y,word cmbdat,int cset,int flags,int sflag)
{

    newcombo c = combobuf[cmbdat];
    
    if(c.tile==0)
    {
        rectfill(dest,x,y,x+15,y+15,0);
        rectfill(dest,x+3,y+3,x+12,y+12,vc(4));
        return;
    }
    
    putcombo(dest,x,y,cmbdat,cset);
    
    /* moved to put_walkflags
      for(int i=0; i<4; i++) {
    
      int tx=((i&2)<<2)+x;
      int ty=((i&1)<<3)+y;
      if((flags&cWALK) && (c.walk&(1<<i)))
      rectfill(dest,tx,ty,tx+7,ty+7,vc(12));
      }
      */
    
    //  if((flags&cFLAGS)&&(cmbdat&0xF800))
    if((flags&cFLAGS)&&(sflag||combobuf[cmbdat].flag))
    {
        //    rectfill(dest,x,y,x+15,y+15,vc(cmbdat>>10+1));
        //    text_mode(-1);
        //    textprintf_ex(dest,sfont,x+1,y+1,(sflag)==0x7800?vc(0):vc(15),-1,"%d",sflag);
        if(sflag)
        {
            rectfill(dest,x,y,x+15,y+15,vc(sflag&15));
            textprintf_ex(dest,z3smallfont,x+1,y+1,vc(15-(sflag&15)),-1,"%d",sflag);
        }
        
        if(combobuf[cmbdat].flag)
        {
            rectfill(dest,x,y+(sflag?8:0),x+15,y+15,vc((combobuf[cmbdat].flag)&15));
            textprintf_ex(dest,z3smallfont,x+1,y+1,vc(15-((combobuf[cmbdat].flag)&15)),-1,"%d",combobuf[cmbdat].flag);
        }
    }
    
    if(flags&cWALK)
    {
        put_walkflags(dest,x,y,cmbdat,0);
    }
    
    if(flags&cCSET)
    {
        bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
        //    text_mode(inv?vc(15):vc(0));
        textprintf_ex(dest,z3smallfont,x+9,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",cset);
    }
    else if(flags&cCTYPE)
    {
        bool inv = (((cmbdat&0x7800)==0x7800)&&(flags&cFLAGS));
        //    text_mode(inv?vc(15):vc(0));
        textprintf_ex(dest,z3smallfont,x+1,y+9,inv?vc(0):vc(15),inv?vc(15):vc(0),"%d",c.type);
    }
}


void copy_mapscr(mapscr *dest, const mapscr *src)
{
    // oops, my bad. ..nvrmnd. XD
    
    dest->valid=src->valid;
    dest->guy=src->guy;
    dest->str=src->str;
    dest->room=src->room;
    dest->item=src->item;
    dest->hasitem=src->hasitem;
    
    for(int i=0; i<4; i++)
        dest->tilewarptype[i]=src->tilewarptype[i];
        
    dest->tilewarpoverlayflags=src->tilewarpoverlayflags;
    dest->door_combo_set=src->door_combo_set;
    
    for(int i=0; i<4; i++)
    {
        dest->warpreturnx[i]=src->warpreturnx[i];
        dest->warpreturny[i]=src->warpreturny[i];
    }
    
    dest->warpreturnc=src->warpreturnc;
    dest->stairx=src->stairx;
    dest->stairy=src->stairy;
    dest->itemx=src->itemx;
    dest->itemy=src->itemy;
    dest->color=src->color;
    dest->enemyflags=src->enemyflags;
    
    for(int i=0; i<4; i++)
        dest->door[i]=src->door[i];
        
    for(int i=0; i<4; i++)
    {
        dest->tilewarpdmap[i]=src->tilewarpdmap[i];
        dest->tilewarpscr[i]=src->tilewarpscr[i];
    }
    
    dest->exitdir=src->exitdir;
    
    for(int i=0; i<10; i++)
        dest->enemy[i]=src->enemy[i];
        
    dest->pattern=src->pattern;
    
    for(int i=0; i<4; i++)
        dest->sidewarptype[i]=src->sidewarptype[i];
        
    dest->sidewarpoverlayflags=src->sidewarpoverlayflags;
    dest->warparrivalx=src->warparrivalx;
    dest->warparrivaly=src->warparrivaly;
    
    for(int i=0; i<4; i++)
        dest->path[i]=src->path[i];
        
    for(int i=0; i<4; i++)
    {
        dest->sidewarpscr[i]=src->sidewarpscr[i];
        dest->sidewarpdmap[i]=src->sidewarpdmap[i];
    }
    
    dest->sidewarpindex=src->sidewarpindex;
    dest->undercombo=src->undercombo;
    dest->undercset=src->undercset;
    dest->catchall=src->catchall;
    dest->flags=src->flags;
    dest->flags2=src->flags2;
    dest->flags3=src->flags3;
    dest->flags4=src->flags4;
    dest->flags5=src->flags5;
    dest->flags6=src->flags6;
    dest->flags7=src->flags7;
    dest->flags8=src->flags8;
    dest->flags9=src->flags9;
    dest->flags10=src->flags10;
    dest->csensitive=src->csensitive;
    dest->noreset=src->noreset;
    dest->nocarry=src->nocarry;
    
    for(int i=0; i<6; i++)
    {
        dest->layermap[i]=src->layermap[i];
        dest->layerscreen[i]=src->layerscreen[i];
        dest->layeropacity[i]=src->layeropacity[i];
    }
    
    dest->timedwarptics=src->timedwarptics;
    dest->nextmap=src->nextmap;
    dest->nextscr=src->nextscr;
    
    for(int i=0; i<128; i++)
    {
        dest->secretcombo[i]=src->secretcombo[i];
        dest->secretcset[i]=src->secretcset[i];
        dest->secretflag[i]=src->secretflag[i];
    }
    
    dest->data=src->data;
    dest->sflag=src->sflag;
    dest->cset=src->cset;
    dest->viewX=src->viewX;
    dest->viewY=src->viewY;
    dest->scrWidth=src->scrWidth;
    dest->scrHeight=src->scrHeight;
    dest->numff=src->numff;
    
    for(int i=0; i<32; i++)
    {
        for(int j=0; j<8; j++)
        {
            //dest->d[i][j]=src->d[i][j];
            dest->initd[i][j]=src->initd[i][j];
        }
        
        for(int j=0; j<2; j++)
        {
            //dest->a[i][j]=src->a[i][j];
            dest->inita[i][j]=src->inita[i][j];
        }
        
        dest->ffdata[i]=src->ffdata[i];
        dest->ffcset[i]=src->ffcset[i];
        dest->ffdelay[i]=src->ffdelay[i];
        dest->ffx[i]=src->ffx[i];
        dest->ffy[i]=src->ffy[i];
        dest->ffxdelta[i]=src->ffxdelta[i];
        dest->ffydelta[i]=src->ffydelta[i];
        dest->ffxdelta2[i]=src->ffxdelta2[i];
        dest->ffydelta2[i]=src->ffydelta2[i];
        dest->ffflags[i]=src->ffflags[i];
        dest->ffwidth[i]=src->ffwidth[i];
        dest->ffheight[i]=src->ffheight[i];
        dest->fflink[i]=src->fflink[i];
        dest->ffscript[i]=src->ffscript[i];
        dest->initialized[i]=src->initialized[i];
        /*dest->pc[i]=src->pc[i];
        dest->scriptflag[i]=src->scriptflag[i];
        dest->sp[i]=src->sp[i];
        dest->itemref[i]=src->itemref[i];
        dest->ffcref[i]=src->ffcref[i];
        dest->itemclass[i]=src->itemclass[i];
        dest->lwpnref[i]=src->lwpnref[i];
        dest->ewpnref[i]=src->ewpnref[i];
        dest->guyref[i]=src->guyref[i];*/
    }
    
    /*for(int i=0; i<256; i++)
      dest->map_stack[i]=src->map_stack[i];
    for(int i=0; i<8; i++)
      dest->map_d[i]=src->map_d[i];
    dest->map_pc=src->map_pc;
    dest->map_scriptflag=src->map_scriptflag;
    dest->map_sp=src->map_sp;
    dest->map_itemref=src->map_itemref;
    dest->map_itemclass=src->map_itemclass;
    dest->map_lwpnref=src->map_lwpnref;
    dest->map_lwpnclass=src->map_lwpnclass;
    dest->map_ewpnref=src->map_ewpnref;
    dest->map_ewpnclass=src->map_ewpnclass;
    dest->map_guyref=src->map_guyref;
    dest->map_guyclass=src->map_guyclass;
    dest->map_ffcref=src->map_ffcref;*/ //Not implemented
    dest->script_entry=src->script_entry;
    dest->script_occupancy=src->script_occupancy;
    dest->script_exit=src->script_exit;
    dest->oceansfx=src->oceansfx;
    dest->bosssfx=src->bosssfx;
    dest->secretsfx=src->secretsfx;
    dest->holdupsfx=src->holdupsfx;
    dest->old_cpage=src->old_cpage;
    dest->screen_midi=src->screen_midi;
    dest->lens_layer=src->lens_layer;
}

void zmap::put_door(BITMAP *dest,int pos,int side,int type,int xofs,int yofs,bool ignorepos, int scr)
{
    int x=0,y=0;
    mapscr *doorscreen=(prv_mode?get_prvscr():screens+scr);
    
    switch(side)
    {
    case up:
    case down:
        x=((pos&15)<<4)+xofs;
        y=(ignorepos?0:(pos&0xF0))+yofs;
        break;
        
    case left:
    case right:
        x=(ignorepos?0:((pos&15)<<4))+xofs;
        y=(pos&0xF0)+yofs;
        break;
    }
    
    switch(type)
    {
    case dt_lock:
    case dt_shut:
    case dt_boss:
    case dt_bomb:
        switch(side)
        {
        case up:
            put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][0],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][0],0,0);
            put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][1],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][1],0,0);
            put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][2],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][2],0,0);
            put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_u[type][3],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_u[type][3],0,0);
            break;
            
        case down:
            put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][0],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][0],0,0);
            put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][1],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][1],0,0);
            put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][2],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][2],0,0);
            put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_d[type][3],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_d[type][3],0,0);
            break;
            
        case left:
            put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][0],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][0],0,0);
            put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][2],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][2],0,0);
            put_combo(dest,x,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][4],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][4],0,0);
                      
            if(x+16 >= dest->w)
                break;
                
            put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][1],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][1],0,0);
            put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][3],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][3],0,0);
            put_combo(dest,x+16,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_l[type][5],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_l[type][5],0,0);
            break;
            
        case right:
        
            put_combo(dest,x+16,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][1],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][1],0,0);
            put_combo(dest,x+16,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][3],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][3],0,0);
            put_combo(dest,x+16,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][5],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][5],0,0);
                      
            if(x+16 <= 0)
                break;
                
            put_combo(dest,x,y,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][0],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][0],0,0);
            put_combo(dest,x,y+16,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][2],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][2],0,0);
            put_combo(dest,x,y+32,DoorComboSets[doorscreen->door_combo_set].doorcombo_r[type][4],
                      DoorComboSets[doorscreen->door_combo_set].doorcset_r[type][4],0,0);
            break;
        }
        
        break;
        
    case dt_pass:
    case dt_wall:
    case dt_walk:
    default:
        break;
    }
}

void zmap::over_door(BITMAP *dest,int pos,int side,int xofs,int yofs,bool, int scr)
{
    int x=((pos&15)<<4)+xofs;
    int y=(pos&0xF0)+yofs;
    mapscr *doorscreen=(prv_mode?get_prvscr():screens+scr);
    
    
    switch(side)
    {
    case up:
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[0]!=0)
        {
            overcombo(dest,x,y,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[0],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_u[0]);
        }
        
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[1]!=0)
        {
            overcombo(dest,x+16,y,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_u[1],
                      
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_u[1]);
        }
        
        break;
        
    case down:
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[0]!=0)
        {
            overcombo(dest,x,y,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[0],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_d[0]);
        }
        
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[1]!=0)
        {
            overcombo(dest,x+16,y,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_d[1],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_d[1]);
        }
        
        break;
        
    case left:
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[0]!=0)
        {
            overcombo(dest,x,y,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[0],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_l[0]);
        }
        
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[1]!=0)
        {
            overcombo(dest,x,y+16,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[1],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_l[1]);
        }
        
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[2]!=0)
        {
            overcombo(dest,x,y+32,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_l[2],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_l[2]);
        }
        
        break;
        
    case right:
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[0]!=0)
        {
            overcombo(dest,x,y,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[0],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_r[0]);
        }
        
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[1]!=0)
        {
            overcombo(dest,x,y+16,
            
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[1],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_r[1]);
        }
        
        if(DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[2]!=0)
        {
            overcombo(dest,x,y+32,
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcombo_r[2],
                      DoorComboSets[doorscreen->door_combo_set].bombdoorcset_r[2]);
        }
        
        break;
    }
}

bool zmap::misaligned(int map, int scr, int i, int dir)
{
    word cmbcheck1, cmbcheck2;
    newcombo combocheck1, combocheck2;
    combocheck1 = combobuf[0];
    combocheck2 = combobuf[0];
    combocheck1.walk = 0;
    combocheck2.walk = 0;
    
    int layermap, layerscreen;
    
    switch(dir)
    {
    case up:
        if(i>15)                                              //not top row of combos
        {
            return false;
        }
        
        if(scr<16)                                            //top row of screens
        {
            return false;
            
        }
        
        //check main screen
        cmbcheck1 = vbound(AbsoluteScr(map, scr)->data[i], 0, MAXCOMBOS-1);
        cmbcheck2 = vbound(AbsoluteScr(map, scr-16)->data[i+160], 0, MAXCOMBOS-1);
        combocheck1.walk|=combobuf[cmbcheck1].walk;
        combocheck2.walk|=combobuf[cmbcheck2].walk;
        
        //check layer 1
        layermap=AbsoluteScr(map, scr)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr-16)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr-16)->layerscreen[0];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+160];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        //check layer 2
        layermap=AbsoluteScr(map, scr)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
            
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr-16)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr-16)->layerscreen[1];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+160];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        if(((combocheck1.walk&5)*2)!=(combocheck2.walk&10))
        {
            return true;
        }
        
        break;
        
    case down:
        if(i<160)                                             //not bottom row of combos
        {
            return false;
        }
        
        if(scr>111)                                           //bottom row of screens
        {
            return false;
        }
        
        //check main screen
        cmbcheck1 = vbound(AbsoluteScr(map, scr)->data[i], 0, MAXCOMBOS-1);
        cmbcheck2 = vbound(AbsoluteScr(map, scr+16)->data[i-160], 0, MAXCOMBOS-1);
        combocheck1.walk|=combobuf[cmbcheck1].walk;
        combocheck2.walk|=combobuf[cmbcheck2].walk;
        
        
        //check layer 1
        layermap=AbsoluteScr(map, scr)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr+16)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr+16)->layerscreen[0];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-160];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        //check layer 2
        layermap=AbsoluteScr(map, scr)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr+16)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr+16)->layerscreen[1];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-160];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        if((combocheck1.walk&10)!=((combocheck2.walk&5)*2))
        {
            return true;
        }
        
        break;
        
    case left:
        if((i&0xF)!=0)                                        //not left column of combos
        {
            return false;
        }
        
        if((scr&0xF)==0)                                      //left column of screens
        {
            return false;
        }
        
        //check main screen
        cmbcheck1 = AbsoluteScr(map, scr)->data[i];
        cmbcheck2 = AbsoluteScr(map, scr-1)->data[i+15];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
        combocheck2.walk|=combobuf[cmbcheck2].walk;
        
        //check layer 1
        layermap=AbsoluteScr(map, scr)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr-1)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr-1)->layerscreen[0];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+15];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        //check layer 2
        layermap=AbsoluteScr(map, scr)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr-1)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr-1)->layerscreen[1];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i+15];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        if(((combocheck1.walk&3)*4)!=(combocheck2.walk&12))
        {
            return true;
        }
        
        break;
        
    case right:
    
        if((i&0xF)!=15)                                       //not right column of combos
        {
            return false;
        }
        
        if((scr&0xF)==15)                                     //right column of screens
        {
            return false;
        }
        
        //check main screen
        cmbcheck1 = AbsoluteScr(map, scr)->data[i];
        cmbcheck2 = AbsoluteScr(map, scr+1)->data[i-15];
        combocheck1.walk|=combobuf[cmbcheck1].walk;
        combocheck2.walk|=combobuf[cmbcheck2].walk;
        
        //check layer 1
        layermap=AbsoluteScr(map, scr)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[0];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr+1)->layermap[0]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr+1)->layerscreen[0];
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-15];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        //check layer 2
        layermap=AbsoluteScr(map, scr)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr)->layerscreen[1];
            cmbcheck1 = AbsoluteScr(layermap, layerscreen)->data[i];
            combocheck1.walk|=combobuf[cmbcheck1].walk;
        }
        
        layermap=AbsoluteScr(map, scr+1)->layermap[1]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=AbsoluteScr(map, scr+1)->layerscreen[1];
            
            cmbcheck2 = AbsoluteScr(layermap, layerscreen)->data[i-15];
            combocheck2.walk|=combobuf[cmbcheck2].walk;
        }
        
        if((combocheck1.walk&12)!=((combocheck2.walk&3)*4))
        {
            return true;
        }
        
        break;
    }
    
    return false;
}

void zmap::check_alignments(BITMAP* dest,int x,int y,int scr)
{
    int checkcombo;
    
    if(alignment_arrow_timer>31)
    {
        if(scr<0)
        {
            scr=currscr;
        }
        
        if((scr<128))                                           //do the misalignment arrows
        {
            for(checkcombo=1; checkcombo<15; checkcombo++)        //check the top row (except the corners)
            {
                if(misaligned(currmap, scr, checkcombo, up))
                {
                    masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,0*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                }
            }
            
            for(checkcombo=161; checkcombo<175; checkcombo++)     //check the top row (except the corners)
            {
                if(misaligned(currmap, scr, checkcombo, down))
                {
                    masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,1*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                }
            }
            
            for(checkcombo=16; checkcombo<160; checkcombo+=16)    //check the left side (except the corners)
            {
                if(misaligned(currmap, scr, checkcombo, left))
                {
                    masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,2*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                }
            }
            
            for(checkcombo=31; checkcombo<175; checkcombo+=16)    //check the right side (except the corners)
            {
                if(misaligned(currmap, scr, checkcombo, right))
                {
                    masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,3*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                }
            }
            
            int tempalign;
            
            //check top left corner
            checkcombo=0;
            tempalign=0;
            tempalign+=(misaligned(currmap, scr, checkcombo, up))?1:0;
            tempalign+=(misaligned(currmap, scr, checkcombo, left))?2:0;
            
            switch(tempalign)
            {
            case 0:
                break;
                
            case 1:                                             //up
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,0*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 2:                                             //left
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,2*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 3:                                             //up-left
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,4*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
            }
            
            //check top right corner
            checkcombo=15;
            tempalign=0;
            tempalign+=(misaligned(currmap, scr, checkcombo, up))?1:0;
            tempalign+=(misaligned(currmap, scr, checkcombo, right))?2:0;
            
            switch(tempalign)
            {
            case 0:
                break;
                
            case 1:                                             //up
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,0*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 2:                                             //right
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,3*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 3:                                             //up-right
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,5*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
            }
            
            //check bottom left corner
            checkcombo=160;
            tempalign=0;
            tempalign+=(misaligned(currmap, scr, checkcombo, down))?1:0;
            tempalign+=(misaligned(currmap, scr, checkcombo, left))?2:0;
            
            switch(tempalign)
            {
            case 0:
                break;
                
            case 1:                                             //down
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,1*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 2:                                             //left
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,2*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 3:                                             //down-left
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,6*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
            }
            
            //check bottom right corner
            
            checkcombo=175;
            tempalign=0;
            tempalign+=(misaligned(currmap, scr, checkcombo, down))?1:0;
            tempalign+=(misaligned(currmap, scr, checkcombo, right))?2:0;
            
            switch(tempalign)
            {
            case 0:
                break;
                
            case 1:                                             //down
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,1*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 2:                                             //right
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,3*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
                
            case 3:                                             //down-right
                masked_blit((BITMAP*)zcdata[BMP_ARROWS].dat,dest,7*17+1,1,((checkcombo&15)<<4)+x,(checkcombo&0xF0)+y,16,16);
                break;
            }
        }
    }
}

// Takes array index layer num., not actual layer num.
int zmap::MAPCOMBO2(int lyr,int x,int y, int map, int scr)
{
    if(lyr<=-1) return MAPCOMBO(x,y,map,scr);
    
    if(map<0)
        map=currmap;
        
    if(scr<0)
        scr=currscr;
        
    mapscr *screen1;
    
    if(prv_mode)
    {
        screen1=get_prvscr();
    }
    else
    {
        screen1=AbsoluteScr(currmap,currscr);
    }
    
    int layermap;
    layermap=screen1->layermap[lyr]-1;
    
    if(layermap<0 || layermap >= map_count) return 0;
    
    mapscr *layer;
    
    if(prv_mode)
        layer = &prvlayers[lyr];
    else
        layer = AbsoluteScr(layermap,screen1->layerscreen[lyr]);
        
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return layer->data[combo];
}

int zmap::MAPCOMBO(int x,int y, int map, int scr) //map=-1,scr=-1
{
    if(map<0)
        map=currmap;
        
    if(scr<0)
        scr=currscr;
        
    mapscr *screen1;
    
    if(prv_mode)
    {
        screen1=get_prvscr();
    }
    else
    {
        screen1=AbsoluteScr(currmap,currscr);
    }
    
    x = vbound(x, 0, 16*16);
    y = vbound(y, 0, 11*16);
    int combo = COMBOPOS(x,y);
    
    if(combo>175 || combo < 0)
        return 0;
        
    return screen1->data[combo];
}

void zmap::draw(BITMAP* dest,int x,int y,int flags,int map,int scr)
{
    int antiflags=flags&~cFLAGS;
    
    if(map<0)
        map=currmap;
        
    if(scr<0)
        scr=currscr;
        
    mapscr *layer;
    
    if(prv_mode)
    {
        layer=get_prvscr();
    }
    else
    {
        layer=AbsoluteScr(map,scr);
    }
    
    int layermap, layerscreen;
    layermap=layer->layermap[CurrentLayer-1]-1;
    
    if(layermap<0)
    {
        CurrentLayer=0;
    }
    
    if(!(layer->valid&mVALID))
    {
        //  rectfill(dest,x,y,x+255,y+175,dvc(0+1));
        rectfill(dest,x,y,x+255,y+175,vc(1));
        
        if(ShowMisalignments)
        {
            check_alignments(dest,x,y,scr);
        }
        
        return;
    }
    
    resize_mouse_pos=true;
    
    for(int k=1; k<3; k++)
    {
        if(k==1&&layer->flags7&fLAYER2BG)
        {
            if(LayerMaskInt[k+1]!=0)
            {
                layermap=layer->layermap[k]-1;
                
                if(layermap>-1 && layermap<map_count)
                {
                    layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                    
                    if(layer->layeropacity[k]==255)
                    {
                        for(int i=0; i<176; i++)
                        {
                            put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags,0);
                        }
                    }
                    else
                    {
                        for(int i=0; i<176; i++)
                        {
                            put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags,0);
                        }
                    }
                }
            }
        }
        
        if(k==2&&layer->flags7&fLAYER3BG)
        {
            if(LayerMaskInt[k+1]!=0)
            {
                layermap=layer->layermap[k]-1;
                
                if(layermap>-1 && layermap<map_count)
                {
                    layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                    
                    if(layer->layeropacity[k]==255)
                    {
                        for(int i=0; i<176; i++)
                        {
                            if(!(layer->flags7&fLAYER2BG))
                                put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags,0);
                            else overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i]);
                        }
                    }
                    else
                    {
                        for(int i=0; i<176; i++)
                        {
                            if(!(layer->flags7&fLAYER2BG))
                                put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],antiflags,0);
                            else overcombotranslucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                        }
                    }
                }
            }
        }
    }
    
    if(LayerMaskInt[0]!=0)
    {
        for(int i=0; i<176; i++)
        {
            word cmbdat = layer->data[i];
            byte cmbcset = layer->cset[i];
            int cmbflag = layer->sflag[i];
            
            if(layer->flags7&fLAYER3BG||layer->flags7&fLAYER2BG)
                overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset);
            else put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,antiflags,cmbflag);
        }
    }
    else
    {
        rectfill(dest,x,y,x+255,y+175,0);
    }
    
    // int cs=2;
    
    for(int k=0; k<2; k++)
    {
        if(k==1&&layer->flags7&fLAYER2BG) continue;
        
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                if(layer->layeropacity[k]==255)
                {
                    for(int i=0; i<176; i++)
                    {
                        overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i]);
                    }
                }
                else
                {
                    for(int i=0; i<176; i++)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                }
            }
        }
        
        if(k==0)
        {
            for(int i=31; i>=0; i--)
            {
                if(layer->ffdata[i])
                {
                    if(!(layer->ffflags[i]&ffCHANGER))
                    {
                        if(!(layer->ffflags[i]&ffOVERLAY))
                        {
                            int tx=(layer->ffx[i]/10000)+x;
                            int ty=(layer->ffy[i]/10000)+y;
                            
                            if(layer->ffflags[i]&ffTRANS)
                            {
                                overcomboblocktranslucent(dest, tx, ty, layer->ffdata[i], layer->ffcset[i],1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6),128);
                                //overtiletranslucent16(dest, combo_tile(layer->ffdata[i],tx,ty)+(j*20)+(l), tx, ty, layer->ffcset[i], combobuf[layer->ffdata[i]].flip, 128);
                            }
                            else
                            {
                                overcomboblock(dest, tx, ty, layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6));
                                //overtile16(dest, combo_tile(layer->ffdata[i],tx,ty)+(j*20)+(l), tx, ty, layer->ffcset[i], combobuf[layer->ffdata[i]].flip);
                            }
                        }
                    }
                }
            }
        }
    }
    
    int doortype[4];
    
    for(int i=0; i<4; i++)
    {
        switch(layer->door[i])
        {
        case dOPEN:
            doortype[i]=dt_pass;
            break;
            
        case dLOCKED:
            doortype[i]=dt_lock;
            break;
            
        case d1WAYSHUTTER:
        case dSHUTTER:
            doortype[i]=dt_shut;
            break;
            
        case dBOSS:
            doortype[i]=dt_boss;
            break;
            
        case dBOMB:
            doortype[i]=dt_bomb;
            break;
        }
    }
    
    switch(layer->door[up])
    {
    case dBOMB:
        over_door(dest,39,up,x,y,false, scr);
        
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
        put_door(dest,7,up,doortype[up],x,y,false,scr);
        break;
        
    case dWALK:
        if(get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
        {
            overcombo(dest,((23&15)<<4)+8+x,(23&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[0],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[0]);
        }
        else
        
        {
            put_combo(dest,((23&15)<<4)+8+x,(23&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[0],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[0],0,0);
        }
        
        break;
    }
    
    switch(layer->door[down])
    {
    case dBOMB:
        over_door(dest,135,down,x,y,false,scr);
        
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
        put_door(dest,151,down,doortype[down],x,y,false,scr);
        break;
        
    case dWALK:
        if(get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
        {
            overcombo(dest,((151&15)<<4)+8+x,(151&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[1],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[1]);
        }
        else
        {
            put_combo(dest,((151&15)<<4)+8+x,(151&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[1],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[1],0,0);
        }
        
        break;
    }
    
    switch(layer->door[left])
    {
    case dBOMB:
        over_door(dest,66,left,x,y,false,scr);
        
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
        put_door(dest,64,left,doortype[left],x,y,false,scr);
        break;
        
    case dWALK:
        if(get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
        {
            overcombo(dest,((81&15)<<4)+x,(81&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[2],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[2]);
        }
        else
        {
            put_combo(dest,((81&15)<<4)+x,(81&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[2],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[2],0,0);
        }
        
        break;
    }
    
    switch(layer->door[right])
    {
    
    case dBOMB:
        over_door(dest,77,right,x,y,false,scr);
        
    case dOPEN:
    case dLOCKED:
    case d1WAYSHUTTER:
    case dSHUTTER:
    case dBOSS:
        put_door(dest,78,right,doortype[right],x,y,false,scr);
        break;
        
    case dWALK:
        if(get_bit(DoorComboSets[screens[currscr].door_combo_set].flags,df_walktrans))
        {
            overcombo(dest,((94&15)<<4)+x,(94&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[3],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[3]);
        }
        else
        {
            put_combo(dest,((94&15)<<4)+x,(94&0xF0)+y,
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcombo[3],
                      DoorComboSets[screens[currscr].door_combo_set].walkthroughcset[3],0,0);
        }
        
        break;
    }
    
    if((layer->hasitem != 0) && !(flags&cNOITEM))
    {
        frame=0;
        putitem2(dest,layer->itemx+x,layer->itemy+y+1-(get_bit(quest_rules, qr_NOITEMOFFSET)),layer->item,lens_hint_item[layer->item][0],lens_hint_item[layer->item][1], 0);
    }
    
    for(int k=2; k<4; k++)
    {
        if(k==2&&layer->flags7&fLAYER3BG) continue;
        
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                if(layer->layeropacity[k]==255)
                {
                    for(int i=0; i<176; i++)
                    {
                        overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i]);
                    }
                }
                else
                {
                    for(int i=0; i<176; i++)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                }
            }
        }
    }
    
    if(LayerMaskInt[0]!=0)
    {
        for(int i=0; i<176; i++)
        {
            int ct1=layer->data[i];
            //     int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
            int ct3=combobuf[ct1].type;
            
//      if (ct3==cOLD_OVERHEAD)
            if(combo_class_buf[ct3].overhead)
            {
                overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i]);
            }
        }
    }
    
    for(int k=4; k<6; k++)
    {
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                if(layer->layeropacity[k]==255)
                {
                    for(int i=0; i<176; i++)
                    {
                        overcombo(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i]);
                    }
                }
                else
                {
                    for(int i=0; i<176; i++)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,(i&0xF0)+y,prv_mode?prvlayers[k].data[i]:TheMaps[layerscreen].data[i],prv_mode?prvlayers[k].cset[i]:TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                }
            }
        }
        
        if(k==4)
        {
            for(int i=31; i>=0; i--)
            {
                if(layer->ffdata[i])
                {
                    if(!(layer->ffflags[i]&ffCHANGER))
                    {
                        if(layer->ffflags[i]&ffOVERLAY)
                        {
                            //overcombo(framebuf,(int)layer->ffx[i],(int)layer->ffy[i]+56,layer->ffdata[i],layer->ffcset[i]);
                            int tx=(layer->ffx[i]/10000)+x;
                            int ty=(layer->ffy[i]/10000)+y;
                            
                            if(layer->ffflags[i]&ffTRANS)
                            {
                                //overtiletranslucent16(dest, combo_tile(layer->ffdata[i],tx,ty)+(j*20)+(l), tx, ty, layer->ffcset[i], combobuf[layer->ffdata[i]].flip, 128);
                                overcomboblocktranslucent(dest,tx,ty,layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6),128);
                            }
                            else
                            {
                                //overtile16(dest, combo_tile(layer->ffdata[i],tx,ty)+(j*20)+(l), tx, ty, layer->ffcset[i], combobuf[layer->ffdata[i]].flip);
                                overcomboblock(dest, tx, ty, layer->ffdata[i], layer->ffcset[i], 1+(layer->ffwidth[i]>>6), 1+(layer->ffheight[i]>>6));
                            }
                        }
                    }
                }
            }
        }
        
        if(k==5)
        {
            for(int i=31; i>=0; i--)
            {
                if(layer->ffdata[i])
                {
                    if(layer->ffflags[i]&ffCHANGER)
                    {
                        putpixel(dest,(layer->ffx[i]/10000)+x,(layer->ffy[i]/10000)+y,vc(rand()%16));
                    }
                }
            }
        }
    }
    
    if(flags&cWALK)
    {
        if(LayerMaskInt[0]!=0)
        {
            for(int i=0; i<176; i++)
            {
                put_walkflags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i], 0);
            }
        }
        
        for(int k=0; k<2; k++)
        {
            if(LayerMaskInt[k+1]!=0)
            {
                layermap=layer->layermap[k]-1;
                
                if(layermap>-1 && layermap<map_count)
                {
                    layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                    
                    for(int i=0; i<176; i++)
                    {
                        put_walkflags_layered(dest,((i&15)<<4)+x,(i&0xF0)+y,i, k);
                    }
                }
            }
        }
    }
    
    if(flags&cFLAGS)
    {
        if(LayerMaskInt[CurrentLayer]!=0)
        {
            for(int i=0; i<176; i++)
            {
                if(CurrentLayer==0)
                {
                    put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags,layer->sflag[i]);
                }
                else
                {
                    if(prv_mode)
                    {
                        put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,prvlayers[CurrentLayer-1].data[i],prvlayers[CurrentLayer-1].cset[i],flags,prvlayers[CurrentLayer-1].sflag[i]);
                    }
                    else
                    {
                        int _lscr=(layer->layermap[CurrentLayer-1]-1)*MAPSCRS+layer->layerscreen[CurrentLayer-1];
                        
                        if(_lscr>-1 && _lscr<map_count*MAPSCRS)
                        {
                            put_flags(dest,((i&15)<<4)+x,(i&0xF0)+y,
                                      TheMaps[_lscr].data[i],
                                      TheMaps[_lscr].cset[i], flags,
                                      TheMaps[_lscr].sflag[i]);
                        }
                    }
                }
            }
        }
    }
    
    
    int dark = layer->flags&cDARK;
    
    if(dark && !(flags&cNODARK))
    {
        for(int j=0; j<80; j++)
        {
            for(int i=0; i<(80)-j; i++)
            {
                if(((i^j)&1)==0)
                {
                    putpixel(dest,x+i,y+j,vc(blackout_color));
                }
            }
        }
    }
    
    if(ShowMisalignments)
    {
        check_alignments(dest,x,y,scr);
    }
    
    resize_mouse_pos=false;
    
}

void zmap::drawrow(BITMAP* dest,int x,int y,int flags,int c,int map,int scr)
{
    if(map<0)
        map=currmap;
        
    if(scr<0)
        scr=currscr;
        
    mapscr* layer=AbsoluteScr(map,scr);
    int layermap=0, layerscreen=0;
    
    if(!(layer->valid&mVALID))
    {
        //  rectfill(dest,x,y,x+255,y+15,dvc(0+1));
        rectfill(dest,x,y,x+255,y+15,vc(1));
        return;
    }
    
    int dark = layer->flags&4;
    
    resize_mouse_pos=true;
    
    if(LayerMaskInt[0]!=0)
    {
        for(int i=c; i<(c&0xF0)+16; i++)
        {
            word cmbdat = (i < (int)layer->data.size() ? layer->data[i] : 0);
            byte cmbcset = (i < (int)layer->data.size() ? layer->cset[i] : 0);
            int cmbflag = (i < (int)layer->data.size() ? layer->sflag[i] : 0);
            put_combo(dest,((i&15)<<4)+x,y,cmbdat,cmbcset,flags|dark,cmbflag);
        }
    }
    else
    {
        rectfill(dest,x,y,x+255,y+15,0);
    }
    
    // int cs=2;
    
    for(int k=1; k<3; k++)
    {
        if(LayerMaskInt[k+1]!=0 && (k==1)?(layer->flags7&fLAYER2BG):(layer->flags7&fLAYER3BG))
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[2-1];
                
                for(int i=c; i<(c&0xF0)+16; i++)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    for(int k=0; k<2; k++)
    {
        if(LayerMaskInt[k+1]!=0 && !(k==1 && layer->flags7&fLAYER2BG))
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                for(int i=c; i<(c&0xF0)+16; i++)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    int doortype[4];
    
    for(int i=0; i<4; i++)
    {
        switch(layer->door[i])
        {
        case dOPEN:
            doortype[i]=dt_pass;
            break;
            
        case dLOCKED:
            doortype[i]=dt_lock;
            break;
            
        case d1WAYSHUTTER:
        case dSHUTTER:
            doortype[i]=dt_shut;
            break;
            
        case dBOSS:
            doortype[i]=dt_boss;
            break;
            
        case dBOMB:
            doortype[i]=dt_bomb;
            break;
        }
    }
    
    if(c<16)
    {
        switch(layer->door[up])
        {
        case dBOMB:
        case dOPEN:
        case dLOCKED:
        case d1WAYSHUTTER:
        case dSHUTTER:
        case dBOSS:
            put_door(dest,7,up,doortype[up],x,y+176,true,scr);
            break;
        }
    }
    else if(c>159)
    {
        switch(layer->door[down])
        {
        case dBOMB:
        case dOPEN:
        case dLOCKED:
        case d1WAYSHUTTER:
        case dSHUTTER:
        case dBOSS:
            put_door(dest,151,down,doortype[down],x,y-16,true,scr);
            break;
        }
    }
    
    for(int k=2; k<4; k++)
    {
        if(LayerMaskInt[k+1]!=0 && !(k==2 && layer->flags7&fLAYER3BG))
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                for(int i=c; i<(c&0xF0)+16; i++)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    if(LayerMaskInt[0]!=0)
    {
        for(int i=c; i<(c&0xF0)+16; i++)
        {
            int ct1=layer->data[i];
            //     int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
            int ct3=combobuf[ct1].type;
            
//      if (ct3==cOLD_OVERHEAD)
            if(combo_class_buf[ct3].overhead)
            {
                overcombo(dest,((i&15)<<4)+x,y,layer->data[i],layer->cset[i]);
            }
        }
    }
    
    for(int k=4; k<6; k++)
    {
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                for(int i=c; i<(c&0xF0)+16; i++)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,((i&15)<<4)+x,y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    if(flags&cWALK)
    {
        if(LayerMaskInt[0]!=0)
        {
            for(int i=c; i<(c&0xF0)+16; i++)
            {
                put_walkflags(dest,((i&15)<<4)+x,y,layer->data[i],0);
            }
        }
        
        for(int k=0; k<2; k++)
        {
            if(LayerMaskInt[k+1]!=0)
            {
                layermap=layer->layermap[k]-1;
                
                if(layermap>-1 && layermap<map_count)
                {
                    layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                    
                    for(int i=c; i<(c&0xF0)+16; i++)
                    {
                        put_walkflags_layered(dest,((i&15)<<4)+x,y,i, k);
                    }
                }
            }
        }
    }
    
    if(flags&cFLAGS)
    {
        if(LayerMaskInt[CurrentLayer]!=0)
        {
            for(int i=c; i<(c&0xF0)+16; i++)
            {
                if(CurrentLayer==0)
                {
                    put_flags(dest,((i&15)<<4)+x,/*(i&0xF0)+*/y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
                }
                else
                {
                    int _lscr=(layer->layermap[CurrentLayer-1]-1)*MAPSCRS+layer->layerscreen[CurrentLayer-1];
                    
                    if(_lscr>-1 && _lscr<map_count*MAPSCRS)
                    {
                        if(i < (int)TheMaps[_lscr].data.size())
                        {
                            put_flags(dest,((i&15)<<4)+x,/*(i&0xF0)+*/y,
                                      TheMaps[_lscr].data[i],
                                      TheMaps[_lscr].cset[i], flags|dark,
                                      TheMaps[_lscr].sflag[i]);
                        }
                        else
                        {
                            put_flags(dest,((i&15)<<4)+x,/*(i&0xF0)+*/y,0,0, flags|dark,0);
                        }
                    }
                }
            }
        }
        
        /*
          if (LayerMaskInt[0]!=0) {
          for(int i=c; i<(c&0xF0)+16; i++) {
          put_flags(dest,((i&15)<<4)+x,y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
          }
          }
          */
    }
    
    if(ShowMisalignments)
    {
        if(c<16)
        {
            check_alignments(dest,x,y,scr);
        }
        else if(c>159)
        {
            check_alignments(dest,x,y-160,scr);
        }
    }
    
    resize_mouse_pos=false;
    
}

void zmap::drawcolumn(BITMAP* dest,int x,int y,int flags,int c,int map,int scr)
{
    if(map<0)
        map=currmap;
        
    if(scr<0)
        scr=currscr;
        
    mapscr* layer=AbsoluteScr(map,scr);
    int layermap=0, layerscreen=0;
    
    if(!(layer->valid&mVALID))
    {
        //  rectfill(dest,x,y,x+15,y+175,dvc(0+1));
        rectfill(dest,x,y,x+15,y+175,vc(1));
        return;
    }
    
    int dark = layer->flags&4;
    
    resize_mouse_pos=true;
    
    
    if(LayerMaskInt[0]!=0)
    {
        for(int i=c; i<176; i+=16)
        {
            word cmbdat = layer->data[i];
            byte cmbcset = layer->cset[i];
            int cmbflag = layer->sflag[i];
            put_combo(dest,x,(i&0xF0)+y,cmbdat,cmbcset,flags|dark,cmbflag);
        }
    }
    else
    {
        rectfill(dest,x,y,x+15,y+175,0);
    }
    
    // int cs=2;
    
    for(int k=1; k<3; k++)
    {
        if(LayerMaskInt[k+1]!=0 && (k==1)?(layer->flags7&fLAYER2BG):(layer->flags7&fLAYER3BG))
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[2-1];
                
                for(int i=c; i<176; i+=16)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    
    for(int k=0; k<2; k++)
    {
        if(LayerMaskInt[k+1]!=0 && !(k==1 && layer->flags7&fLAYER2BG))
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                for(int i=c; i<176; i+=16)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    int doortype[4];
    
    for(int i=0; i<4; i++)
    {
        switch(layer->door[i])
        {
        case dOPEN:
            doortype[i]=dt_pass;
            break;
            
        case dLOCKED:
            doortype[i]=dt_lock;
            break;
            
        case d1WAYSHUTTER:
        case dSHUTTER:
            doortype[i]=dt_shut;
            break;
            
        case dBOSS:
            doortype[i]=dt_boss;
            break;
            
        case dBOMB:
            doortype[i]=dt_bomb;
            break;
        }
    }
    
    if((c&0x0F)==0)
    {
        switch(layer->door[left])
        {
        
        case dBOMB:
        case dOPEN:
        case dLOCKED:
        case d1WAYSHUTTER:
        case dSHUTTER:
        case dBOSS:
            //       put_door(dest,64,left,doortype[left],x+256,y,true);
            put_door(dest,64,left,doortype[left],x,y,true,scr);
            break;
        }
    }
    else if((c&0x0F)==15)
    {
        switch(layer->door[right])
        {
        case dBOMB:
        case dOPEN:
        case dLOCKED:
        case d1WAYSHUTTER:
        case dSHUTTER:
        case dBOSS:
            put_door(dest,78,right,doortype[right],x-16,y,true,scr);
            break;
        }
    }
    
    for(int k=2; k<4; k++)
    {
        if(LayerMaskInt[k+1]!=0 && !(k==2 && layer->flags7&fLAYER3BG))
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                for(int i=c; i<176; i+=16)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    else
                    {
                        overcombo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    if(LayerMaskInt[0]!=0)
    {
        for(int i=c; i<176; i+=16)
        {
            int ct1=layer->data[i];
            //     int ct2=(ct1&0xFF)+(screens[currscr].cpage<<8);
            int ct3=combobuf[ct1].type;
            
//      if (ct3==cOLD_OVERHEAD)
            if(combo_class_buf[ct3].overhead)
            {
                overcombo(dest,x,(i&0xF0)+y,layer->data[i],layer->cset[i]);
            }
        }
    }
    
    for(int k=4; k<6; k++)
    {
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                for(int i=c; i<176; i+=16)
                {
                    if(layer->layeropacity[k]<255)
                    {
                        overcombotranslucent(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i],layer->layeropacity[k]);
                    }
                    
                    else
                    {
                        overcombo(dest,x,(i&0xF0)+y,TheMaps[layerscreen].data[i],TheMaps[layerscreen].cset[i]);
                    }
                }
            }
        }
    }
    
    if(flags&cWALK)
    {
        if(LayerMaskInt[0]!=0)
        {
            for(int i=c; i<176; i+=16)
            {
                put_walkflags(dest,x,(i&0xF0)+y,layer->data[i],0);
            }
        }
        
        for(int k=0; k<2; k++)
        {
            if(LayerMaskInt[k+1]!=0)
            {
                layermap=layer->layermap[k]-1;
                
                if(layermap>-1 && layermap<map_count)
                {
                
                    layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                    
                    for(int i=c; i<(c&0xF0)+16; i++)
                    {
                        put_walkflags_layered(dest,x,(i&0xF0)+y,i, k);
                    }
                }
            }
        }
    }
    
    if(flags&cFLAGS)
    {
        if(LayerMaskInt[CurrentLayer]!=0)
        {
            for(int i=c; i<176; i+=16)
            {
                if(CurrentLayer==0)
                {
                    put_flags(dest,/*((i&15)<<4)+*/x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
                }
                else
                {
                    int _lscr=(layer->layermap[CurrentLayer-1]-1)*MAPSCRS+layer->layerscreen[CurrentLayer-1];
                    
                    if(_lscr>-1 && _lscr<map_count*MAPSCRS)
                    {
                        put_flags(dest,/*((i&15)<<4)+*/x,(i&0xF0)+y,
                                  TheMaps[_lscr].data[i],
                                  TheMaps[_lscr].cset[i], flags|dark,
                                  TheMaps[_lscr].sflag[i]);
                    }
                }
            }
        }
        
        /*
          if (LayerMaskInt[0]!=0) {
          for(int i=c; i<176; i+=16) {
          put_flags(dest,x,(i&0xF0)+y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
          }
          }
          */
    }
    
    if(ShowMisalignments)
    {
        if((c&0x0F)==0)
        {
            check_alignments(dest,x,y,scr);
        }
        else if((c&0x0F)==15)
        {
            check_alignments(dest,x-240,y,scr);
        }
    }
    
    resize_mouse_pos=false;
}

void zmap::drawblock(BITMAP* dest,int x,int y,int flags,int c,int map,int scr)
{
    if(map<0)
        map=currmap;
        
    if(scr<0)
        scr=currscr;
        
    mapscr* layer=AbsoluteScr(map,scr);
    int layermap=0, layerscreen=0;
    
    if(!(layer->valid&mVALID))
    {
        //  rectfill(dest,x,y,x+15,y+15,dvc(0+1));
        rectfill(dest,x,y,x+15,y+15,vc(1));
        return;
    }
    
    int dark = layer->flags&4;
    
    resize_mouse_pos=true;
    
    if(LayerMaskInt[0]!=0)
    {
        word cmbdat = layer->data[c];
        byte cmbcset = layer->cset[c];
        int cmbflag = layer->sflag[c];
        put_combo(dest,x,y,cmbdat,cmbcset,flags|dark,cmbflag);
    }
    else
    {
        rectfill(dest,x,y,x+15,y+15,0);
    }
    
    // int cs=2;
    
    for(int k=0; k<2; k++)
    {
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                if(layer->layeropacity[k]<255)
                {
                    overcombotranslucent(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],layer->layeropacity[k]);
                }
                else
                {
                    overcombo(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c]);
                }
            }
        }
    }
    
    for(int k=2; k<4; k++)
    {
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                if(layer->layeropacity[k]<255)
                {
                    overcombotranslucent(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],layer->layeropacity[k]);
                }
                else
                {
                    overcombo(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c]);
                }
            }
        }
    }
    
    if(LayerMaskInt[0]!=0)
    {
        int ct1=layer->data[c];
        int ct3=combobuf[ct1].type;
        
//    if (ct3==cOLD_OVERHEAD)
        if(combo_class_buf[ct3].overhead)
        {
            overcombo(dest,x,y,layer->data[c],layer->cset[c]);
        }
    }
    
    for(int k=4; k<6; k++)
    {
        if(LayerMaskInt[k+1]!=0)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                
                if(layer->layeropacity[k]<255)
                {
                    overcombotranslucent(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c],layer->layeropacity[k]);
                }
                else
                {
                    overcombo(dest,x,y,TheMaps[layerscreen].data[c],TheMaps[layerscreen].cset[c]);
                }
            }
        }
    }
    
    if(flags&cWALK)
    {
        if(LayerMaskInt[0]!=0)
        {
            put_walkflags(dest,x,y,layer->data[c],0);
        }
        
        for(int k=0; k<2; k++)
        {
            if(LayerMaskInt[k+1]!=0)
            {
                layermap=layer->layermap[k]-1;
                
                if(layermap>-1 && layermap<map_count)
                {
                    layerscreen=layermap*MAPSCRS+layer->layerscreen[k];
                    put_walkflags_layered(dest,x,y,c,k);
                }
            }
        }
    }
    
    if(flags&cFLAGS)
    {
        if(LayerMaskInt[CurrentLayer]!=0)
        {
            for(int i=c; i==c; i++)
            {
                if(CurrentLayer==0)
                {
                    put_flags(dest,/*((i&15)<<4)+*/x,/*(i&0xF0)+*/y,layer->data[i],layer->cset[i],flags|dark,layer->sflag[i]);
                }
                else
                {
                    int _lscr=(layer->layermap[CurrentLayer-1]-1)*MAPSCRS+layer->layerscreen[CurrentLayer-1];
                    
                    if(_lscr>-1 && _lscr<map_count*MAPSCRS)
                    {
                        put_flags(dest,/*((i&15)<<4)+*/x,/*(i&0xF0)+*/y,
                                  TheMaps[_lscr].data[i],
                                  TheMaps[_lscr].cset[i], flags|dark,
                                  TheMaps[_lscr].sflag[i]);
                    }
                }
            }
        }
        
        /*
          if (LayerMaskInt[0]!=0) {
          put_flags(dest,x,y,layer->data[c],layer->cset[c],flags|dark,layer->sflag[c]);
          }
          */
    }
    
    if(ShowMisalignments)
    {
        switch(c)
        {
        case 0:
            check_alignments(dest,x,y,scr);
            break;
            
        case 15:
            check_alignments(dest,x-240,y,scr);
            break;
            
        case 160:
            check_alignments(dest,x,y-160,scr);
            break;
            
        case 175:
            check_alignments(dest,x-240,y-160,scr);
            break;
        }
    }
    
    resize_mouse_pos=false;
    
}

void zmap::drawstaticblock(BITMAP* dest,int x,int y)
{
    if(InvalidStatic)
    {
        for(int dy=0; dy<16; dy++)
        {
            for(int dx=0; dx<16; dx++)
            {
                dest->line[y+dy][x+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
            }
        }
    }
    else
    {
        rectfill(dest, x, y, x+15, y+15, vc(0));
        rect(dest, x, y, x+15, y+15, vc(15));
        line(dest, x, y, x+15, y+15, vc(15));
        line(dest, x, y+15, x+15, y, vc(15));
    }
}

void zmap::drawstaticcolumn(BITMAP* dest,int x,int y)
{
    if(InvalidStatic)
    {
        for(int dy=0; dy<176; dy++)
        {
            for(int dx=0; dx<16; dx++)
            {
                dest->line[y+dy][x+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
            }
        }
    }
    else
    {
        rectfill(dest, x, y, x+15, y+175, vc(0));
        rect(dest, x, y, x+15, y+175, vc(15));
        line(dest, x, y, x+15, y+175, vc(15));
        line(dest, x, y+175, x+15, y, vc(15));
    }
}

void zmap::drawstaticrow(BITMAP* dest,int x,int y)
{
    if(InvalidStatic)
    {
        for(int dy=0; dy<16; dy++)
        {
            for(int dx=0; dx<256; dx++)
            {
                dest->line[y+dy][x+dx]=vc((((rand()%100)/50)?0:8)+(((rand()%100)/50)?0:7));
            }
        }
    }
    else
    {
        rectfill(dest, x, y, x+255, y+15, vc(0));
        rect(dest, x, y, x+255, y+15, vc(15));
        line(dest, x, y, x+255, y+15, vc(15));
        line(dest, x, y+15, x+255, y, vc(15));
    }
}

void zmap::draw_template(BITMAP* dest,int x,int y)
{
    for(int i=0; i<176; i++)
    {
        word cmbdat = screens[TEMPLATE].data[i];
        byte cmbcset = screens[TEMPLATE].cset[i];
        int cmbflag = screens[TEMPLATE].sflag[i];
        put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,0,cmbflag);
    }
}

void zmap::draw_template2(BITMAP* dest,int x,int y)
{
    for(int i=0; i<176; i++)
    {
        word cmbdat = screens[TEMPLATE2].data[i];
        byte cmbcset = screens[TEMPLATE2].cset[i];
        int cmbflag = screens[TEMPLATE2].sflag[i];
        put_combo(dest,((i&15)<<4)+x,(i&0xF0)+y,cmbdat,cmbcset,0,cmbflag);
    }
}

void zmap::draw_secret(BITMAP *dest, int pos)
{
    word cmbdat = screens[TEMPLATE].data[pos];
    byte cmbcset = screens[TEMPLATE].cset[pos];
    int cmbflag = screens[TEMPLATE].sflag[pos];
    put_combo(dest,0,0,cmbdat,cmbcset,0,cmbflag);
}

void zmap::draw_secret2(BITMAP *dest, int scombo)
{
    word cmbdat =  screens[currscr].secretcombo[scombo];
    byte cmbcset = screens[currscr].secretcset[scombo];
    byte cmbflag = screens[currscr].secretflag[scombo];
    put_combo(dest,0,0,cmbdat,cmbcset,0,cmbflag);
}

void zmap::scroll(int dir)
{
    if(currmap<map_count)
    {
        switch(dir)
        {
        case up:
            if((key[KEY_LCONTROL] || key[KEY_RCONTROL]) && Map.CurrScr()->flags2&wfUP)
            {
                dowarp(1,Map.CurrScr()->sidewarpindex&3);
            }
            else if(currscr>15)
            {
                setCurrScr(currscr-16);
            }
            
            break;
            
        case down:
            if((key[KEY_LCONTROL] || key[KEY_RCONTROL]) && Map.CurrScr()->flags2&wfDOWN)
            {
                dowarp(1,(Map.CurrScr()->sidewarpindex>>2)&3);
            }
            else if(currscr<MAPSCRS-16)
            {
                setCurrScr(currscr+16);
            }
            
            break;
            
        case left:
            if((key[KEY_LCONTROL] || key[KEY_RCONTROL]) && Map.CurrScr()->flags2&wfLEFT)
            {
                dowarp(1,(Map.CurrScr()->sidewarpindex>>4)&3);
            }
            else if(currscr&15)
            {
                setCurrScr(currscr-1);
            }
            
            break;
            
        case right:
            if((key[KEY_LCONTROL] || key[KEY_RCONTROL]) && Map.CurrScr()->flags2&wfRIGHT)
            {
                dowarp(1,(Map.CurrScr()->sidewarpindex>>6)&3);
            }
            else if((currscr&15)<15 && currscr<MAPSCRS-1)
            {
                setCurrScr(currscr+1);
            }
            
            break;
        }
    }
}

void zmap::putdoor2(int scr,int side,int door)
{
    if(door!=dWALL)
    {
        putdoor(scr,side,door);
    }
}

void zmap::putdoor(int scr,int side,int door)
{
    screens[scr].door[side]=door;
    word *di = &screens[scr].data.front();
    byte *di2 = &screens[scr].cset.front();
    
    switch(side)
    {
    case up:
        switch(door)
        {
        case dWALL:
        case dBOMB:
        case dWALK:
            di[7]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][0];
            di2[7]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][0];
            di[8]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][1];
            di2[8]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][1];
            di[23]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][2];
            di2[23]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][2];
            di[24]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_wall][3];
            di2[24]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_wall][3];
            break;
            
        default:
            di[7]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][0];
            di2[7]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][0];
            di[8]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][1];
            di2[8]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][1];
            di[23]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][2];
            di2[23]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][2];
            di[24]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_u[dt_pass][3];
            di2[24]  = DoorComboSets[screens[scr].door_combo_set].doorcset_u[dt_pass][3];
            break;
        }
        
        break;
        
    case down:
        switch(door)
        {
        case dWALL:
        case dBOMB:
        case dWALK:
            di[151]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][0];
            di2[151]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][0];
            di[152]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][1];
            di2[152]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][1];
            di[167]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][2];
            di2[167]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][2];
            di[168]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_wall][3];
            di2[168]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_wall][3];
            break;
            
        default:
            di[151]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][0];
            di2[151]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][0];
            di[152]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][1];
            di2[152]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][1];
            di[167]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][2];
            di2[167]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][2];
            di[168]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_d[dt_pass][3];
            di2[168]  = DoorComboSets[screens[scr].door_combo_set].doorcset_d[dt_pass][3];
            break;
        }
        
        break;
        
    case left:
        switch(door)
        {
        case dWALL:
        case dBOMB:
        case dWALK:
            di[64]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][0];
            di2[64]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][0];
            di[65]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][1];
            di2[65]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][1];
            di[80]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][2];
            di2[80]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][2];
            di[81]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][3];
            di2[81]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][3];
            di[96]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][4];
            di2[96]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][4];
            di[97]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_wall][5];
            di2[97]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_wall][5];
            break;
            
        default:
            di[64]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][0];
            di2[64]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][0];
            di[65]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][1];
            di2[65]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][1];
            di[80]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][2];
            di2[80]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][2];
            di[81]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][3];
            di2[81]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][3];
            di[96]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][4];
            di2[96]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][4];
            di[97]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_l[dt_pass][5];
            di2[97]  = DoorComboSets[screens[scr].door_combo_set].doorcset_l[dt_pass][5];
            break;
        }
        
        break;
        
    case right:
        switch(door)
        {
        case dWALL:
        case dBOMB:
        case dWALK:
            di[78]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][0];
            di2[78]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][0];
            di[79]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][1];
            di2[79]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][1];
            di[94]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][2];
            di2[94]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][2];
            di[95]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][3];
            di2[95]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][3];
            di[110]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][4];
            di2[110]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][4];
            di[111]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_wall][5];
            di2[111]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_wall][5];
            break;
            
        default:
            di[78]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][0];
            di2[78]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][0];
            di[79]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][1];
            di2[79]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][1];
            di[94]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][2];
            di2[94]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][2];
            di[95]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][3];
            di2[95]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][3];
            di[110]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][4];
            di2[110]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][4];
            di[111]   = DoorComboSets[screens[scr].door_combo_set].doorcombo_r[dt_pass][5];
            di2[111]  = DoorComboSets[screens[scr].door_combo_set].doorcset_r[dt_pass][5];
            break;
        }
        
        break;
    }
}

void zmap::Ugo()
{
    mapscr *layer;
    int layermap, layerscreen;
    layer=AbsoluteScr(currmap,currscr);
    
    for(int x=0; x<MAPSCRS; x++)
    {
        copy_mapscr(&undomap[x], &screens[x]);
    }
    
    for(int k=0; k<6; ++k)
    {
        undomap[MAPSCRS+k].zero_memory();
        layermap=layer->layermap[k]-1;
        
        if(layermap>-1 && layermap<map_count)
        {
            layerscreen=layer->layerscreen[k];
            copy_mapscr(&undomap[MAPSCRS+k], AbsoluteScr(layermap,layerscreen));
        }
    }
    
    can_undo=true;
}

void zmap::Uhuilai()
{
    mapscr *layer;
    int layermap, layerscreen;
    layer=AbsoluteScr(currmap,currscr);
    
    if(can_undo)
    {
        for(int x=0; x<MAPSCRS; x++)
        {
            zc_swap(screens[x],undomap[x]);
        }
        
        for(int k=0; k<6; ++k)
        {
            layermap=layer->layermap[k]-1;
            
            if(layermap>-1 && layermap<map_count)
            {
                layerscreen=layer->layerscreen[k];
                zc_swap(*AbsoluteScr(layermap,layerscreen),undomap[MAPSCRS+k]);
            }
        }
    }
}

void zmap::Copy()
{
    if(screens[currscr].valid&mVALID)
    {
        copy_mapscr(&copymapscr, &screens[currscr]);
        //copymapscr=screens[currscr];
        can_paste=true;
        copymap=currmap;
        copyscr=currscr;
        copyffc = -1;
    }
}

void zmap::CopyFFC(int n)
{
    if(screens[currscr].valid&mVALID)
    {
        copy_mapscr(&copymapscr, &screens[currscr]);
        // Can't paste the screen itself
        can_paste = false;
        copymap=currmap;
        copyscr=currscr;
        copyffc = n;
    }
}

void zmap::Paste()
{
    if(can_paste)
    {
        Ugo();
        int oldcolor=getcolor();
        
        if(!(screens[currscr].valid&mVALID))
        {
            screens[currscr].valid |= mVALID;
            screens[currscr].color = copymapscr.color;
        }
        
        for(int i=0; i<4; i++)
        {
            putdoor(currscr,i,0);
        }
        
        screens[currscr].door_combo_set = copymapscr.door_combo_set;
        
        for(int i=0; i<4; i++)
        {
            screens[currscr].door[i]=copymapscr.door[i];
        }
        
        for(int i=0; i<176; i++)
        {
            screens[currscr].data[i] = copymapscr.data[i];
            screens[currscr].cset[i] = copymapscr.cset[i];
            screens[currscr].sflag[i] = copymapscr.sflag[i];
        }
        
        for(int i=0; i<4; i++)
        {
            putdoor2(currscr,i,screens[currscr].door[i]);
        }
        
        int newcolor=getcolor();
        loadlvlpal(newcolor);
        
        if(!(screens[currscr].valid&mVALID))
        {
            newcolor=-1;
        }
        
        if(newcolor!=oldcolor)
        {
            rebuild_trans_table();
        }
        
        saved=false;
    }
}

void zmap::PasteUnderCombo()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].undercombo = copymapscr.undercombo;
        screens[currscr].undercset = copymapscr.undercset;
        saved=false;
    }
}

void zmap::PasteSecretCombos()
{
    if(can_paste)
    {
        Ugo();
        
        for(int i=0; i<128; i++)
        {
            screens[currscr].secretcombo[i] = copymapscr.secretcombo[i];
            screens[currscr].secretcset[i] = copymapscr.secretcset[i];
            screens[currscr].secretflag[i] = copymapscr.secretflag[i];
        }
        
        saved=false;
    }
}

void zmap::PasteFFCombos()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].numff = copymapscr.numff;
        
        for(int i=0; i<32; i++)
        {
            screens[currscr].ffdata[i] = copymapscr.ffdata[i];
            screens[currscr].ffcset[i] = copymapscr.ffcset[i];
            screens[currscr].ffx[i] = copymapscr.ffx[i];
            screens[currscr].ffy[i] = copymapscr.ffy[i];
            screens[currscr].ffxdelta[i] = copymapscr.ffxdelta[i];
            screens[currscr].ffydelta[i] = copymapscr.ffydelta[i];
            screens[currscr].ffxdelta2[i] = copymapscr.ffxdelta2[i];
            screens[currscr].ffydelta2[i] = copymapscr.ffydelta2[i];
            screens[currscr].fflink[i] = copymapscr.fflink[i];
            screens[currscr].ffdelay[i] = copymapscr.ffdelay[i];
            screens[currscr].ffwidth[i] = copymapscr.ffwidth[i];
            screens[currscr].ffheight[i] = copymapscr.ffheight[i];
            screens[currscr].ffwidth[i] = copymapscr.ffwidth[i];
            screens[currscr].ffheight[i] = copymapscr.ffheight[i];
            screens[currscr].ffflags[i] = copymapscr.ffflags[i];
            screens[currscr].ffscript[i] = copymapscr.ffscript[i];
            
            for(int j=0; j<8; j++)
                screens[currscr].initd[i][j] = copymapscr.initd[i][j];
                
            for(int j=0; j<2; j++)
                screens[currscr].inita[i][j] = copymapscr.inita[i][j];
        }
        
        saved=false;
    }
}

void zmap::PasteOneFFC(int i) //i - destination ffc slot
{
    if(copyffc < 0)  // Sanity check
        return;
        
    Ugo();
    screens[currscr].ffdata[i] = copymapscr.ffdata[copyffc];
    screens[currscr].ffcset[i] = copymapscr.ffcset[copyffc];
    // Don't copy X or Y
    screens[currscr].ffxdelta[i] = copymapscr.ffxdelta[copyffc];
    screens[currscr].ffydelta[i] = copymapscr.ffydelta[copyffc];
    screens[currscr].ffxdelta2[i] = copymapscr.ffxdelta2[copyffc];
    screens[currscr].ffydelta2[i] = copymapscr.ffydelta2[copyffc];
    screens[currscr].fflink[i] = copymapscr.fflink[copyffc];
    screens[currscr].ffdelay[i] = copymapscr.ffdelay[copyffc];
    screens[currscr].ffwidth[i] = copymapscr.ffwidth[copyffc];
    screens[currscr].ffheight[i] = copymapscr.ffheight[copyffc];
    screens[currscr].ffwidth[i] = copymapscr.ffwidth[copyffc];
    screens[currscr].ffheight[i] = copymapscr.ffheight[copyffc];
    screens[currscr].ffflags[i] = copymapscr.ffflags[copyffc];
    screens[currscr].ffscript[i] = copymapscr.ffscript[copyffc];
    
    for(int j=0; j<8; j++)
        screens[currscr].initd[i][j] = copymapscr.initd[copyffc][j];
        
    for(int j=0; j<2; j++)
        screens[currscr].inita[i][j] = copymapscr.inita[copyffc][j];
        
    screens[currscr].numff|=(1<<i);
    copyffc = -1;
    saved=false;
}

void zmap::PasteWarps()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].sidewarpindex = copymapscr.sidewarpindex;
        
        for(int i=0; i<4; i++)
        {
            screens[currscr].tilewarptype[i] = copymapscr.tilewarptype[i];
            screens[currscr].tilewarpdmap[i] = copymapscr.tilewarpdmap[i];
            screens[currscr].tilewarpscr[i] = copymapscr.tilewarpscr[i];
            screens[currscr].sidewarptype[i] = copymapscr.sidewarptype[i];
            screens[currscr].sidewarpdmap[i] = copymapscr.sidewarpdmap[i];
            screens[currscr].sidewarpscr[i] = copymapscr.sidewarpscr[i];
            screens[currscr].flags2 &= ~(wfUP|wfDOWN|wfLEFT|wfRIGHT);
            screens[currscr].flags2 |= copymapscr.flags2 & (wfUP|wfDOWN|wfLEFT|wfRIGHT);
            screens[currscr].sidewarpoverlayflags = copymapscr.sidewarpoverlayflags;
            screens[currscr].tilewarpoverlayflags = copymapscr.tilewarpoverlayflags;
        }
        
        saved=false;
    }
}

void zmap::PasteScreenData()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].csensitive = copymapscr.csensitive;
        screens[currscr].oceansfx = copymapscr.oceansfx;
        screens[currscr].bosssfx = copymapscr.bosssfx;
        screens[currscr].secretsfx = copymapscr.secretsfx;
        screens[currscr].holdupsfx = copymapscr.holdupsfx;
        screens[currscr].flags = copymapscr.flags;
        screens[currscr].flags2 &= (wfUP|wfDOWN|wfLEFT|wfRIGHT);
        screens[currscr].flags2 |= copymapscr.flags2 & ~(wfUP|wfDOWN|wfLEFT|wfRIGHT);
        screens[currscr].flags3 = copymapscr.flags3;
        screens[currscr].flags4 = copymapscr.flags4;
        screens[currscr].flags5 = copymapscr.flags5;
        screens[currscr].flags6 = copymapscr.flags6;
        screens[currscr].flags7 = copymapscr.flags7;
        screens[currscr].flags8 = copymapscr.flags8;
        screens[currscr].flags9 = copymapscr.flags9;
        screens[currscr].flags10 = copymapscr.flags10;
        screens[currscr].item = copymapscr.item;
        screens[currscr].hasitem = copymapscr.hasitem;
        screens[currscr].itemx = copymapscr.itemx;
        screens[currscr].itemy = copymapscr.itemy;
        screens[currscr].nextmap = copymapscr.nextmap;
        screens[currscr].nextscr = copymapscr.nextscr;
        screens[currscr].nocarry = copymapscr.nocarry;
        screens[currscr].noreset = copymapscr.noreset;
        screens[currscr].path[0] = copymapscr.path[0];
        screens[currscr].path[1] = copymapscr.path[1];
        screens[currscr].path[2] = copymapscr.path[2];
        screens[currscr].path[3] = copymapscr.path[3];
        screens[currscr].pattern = copymapscr.pattern;
        screens[currscr].exitdir = copymapscr.exitdir;
        screens[currscr].enemyflags = copymapscr.enemyflags;
        screens[currscr].screen_midi = copymapscr.screen_midi;
        screens[currscr].stairx = copymapscr.stairx;
        screens[currscr].stairy = copymapscr.stairy;
        screens[currscr].timedwarptics = copymapscr.timedwarptics;
        saved=false;
    }
}

void zmap::PasteWarpLocations()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].warpreturnc = copymapscr.warpreturnc;
        screens[currscr].warparrivalx = copymapscr.warparrivalx;
        screens[currscr].warparrivaly = copymapscr.warparrivaly;
        
        for(int i=0; i<4; i++)
        {
            screens[currscr].warpreturnx[i] = copymapscr.warpreturnx[i];
            screens[currscr].warpreturny[i] = copymapscr.warpreturny[i];
        }
        
        saved=false;
    }
}

void zmap::PasteDoors()
{
    if(can_paste)
    {
        Ugo();
        
        for(int i=0; i<4; i++)
            screens[currscr].door[i] = copymapscr.door[i];
            
        screens[currscr].door_combo_set = copymapscr.door_combo_set;
        saved=false;
    }
}

void zmap::PasteLayers()
{
    if(can_paste)
    {
        Ugo();
        
        for(int i=0; i<6; i++)
        {
            screens[currscr].layermap[i] = copymapscr.layermap[i];
            screens[currscr].layerscreen[i] = copymapscr.layerscreen[i];
            screens[currscr].layeropacity[i] = copymapscr.layeropacity[i];
        }
        
        saved=false;
    }
}

void zmap::PasteRoom()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].room = copymapscr.room;
        screens[currscr].catchall = copymapscr.catchall;
        saved=false;
    }
}

void zmap::PasteGuy()
{
    if(can_paste)
    {
        Ugo();
        screens[currscr].guy = copymapscr.guy;
        screens[currscr].str = copymapscr.str;
        saved=false;
    }
}

void zmap::PastePalette()
{
    if(can_paste)
    {
        Ugo();
        int oldcolor=getcolor();
        screens[currscr].color = copymapscr.color;
        int newcolor=getcolor();
        loadlvlpal(newcolor);
        
        if(!(screens[currscr].valid&mVALID))
        {
            newcolor=-1;
        }
        
        if(newcolor!=oldcolor)
        {
            rebuild_trans_table();
        }
        
        saved=false;
    }
}

void zmap::PasteAll()
{
    if(can_paste)
    {
        Ugo();
        int oldcolor=getcolor();
        copy_mapscr(&screens[currscr], &copymapscr);
        //screens[currscr]=copymapscr;
        int newcolor=getcolor();
        loadlvlpal(newcolor);
        
        if(!(screens[currscr].valid&mVALID))
        {
            newcolor=-1;
        }
        
        if(newcolor!=oldcolor)
        {
            rebuild_trans_table();
        }
        
        saved=false;
    }
}


void zmap::PasteToAll()
{
    if(can_paste)
    {
        Ugo();
        int oldcolor=getcolor();
        
        for(int x=0; x<128; x++)
        {
            if(!(screens[x].valid&mVALID))
            {
                screens[x].valid |= mVALID;
                screens[x].color = copymapscr.color;
            }
            
            for(int i=0; i<176; i++)
            {
                screens[x].data[i] = copymapscr.data[i];
                screens[x].cset[i] = copymapscr.cset[i];
                screens[x].sflag[i] = copymapscr.sflag[i];
            }
            
            if(isDungeon(x))
                for(int i=0; i<4; i++)
                    putdoor(currscr,i,screens[x].door[i]);
                    
        }
        
        int newcolor=getcolor();
        loadlvlpal(newcolor);
        
        if(!(screens[currscr].valid&mVALID))
        {
            newcolor=-1;
        }
        
        if(newcolor!=oldcolor)
        {
            rebuild_trans_table();
        }
        
        saved=false;
    }
}

void zmap::PasteAllToAll()
{
    if(can_paste)
    {
        Ugo();
        int oldcolor=getcolor();
        
        for(int x=0; x<128; x++)
        {
            copy_mapscr(&screens[x], &copymapscr);
            //screens[x]=copymapscr;
        }
        
        int newcolor=getcolor();
        loadlvlpal(newcolor);
        
        if(!(screens[currscr].valid&mVALID))
        {
            newcolor=-1;
        }
        
        if(newcolor!=oldcolor)
        {
            rebuild_trans_table();
        }
        
        saved=false;
    }
}

void zmap::PasteEnemies()
{
    if(can_paste)
    {
        for(int i=0; i<10; i++)
            screens[currscr].enemy[i]=copymapscr.enemy[i];
    }
}

void zmap::setCanUndo(bool _set)
{
    can_paste=can_paste_map=_set;
}

void zmap::setCanPaste(bool _set)
{
    can_undo=can_undo_map=_set;
}

void zmap::update_combo_cycling()
{
    if(!prv_mode||!prv_cmbcycle)
    {
        return;
    }
    
    int x,y;
    int newdata[176];
    int newcset[176];
    bool restartanim[MAXCOMBOS];
    bool restartanim2[MAXCOMBOS];
    
    memset(restartanim, 0, MAXCOMBOS);
    memset(restartanim2, 0, MAXCOMBOS);
    
    for(int i=0; i<176; i++)
    {
        newdata[i]=-1;
        newcset[i]=-1;
        
        x=prvscr.data[i];
        y=animated_combo_table[x][0];
        
        //time to restart
        if((animated_combo_table4[y][1]>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c = newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim[c]=true;
            }
        }
    }
    
    for(int i=0; i<176; i++)
    {
        x=prvscr.data[i];
        y=animated_combo_table2[x][0];
        
        //time to restart
        if((animated_combo_table24[y][1]>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c = newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2[c]=true;
            }
        }
    }
    
    for(int i=0; i<176; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        prvscr.data[i]=newdata[i];
        prvscr.cset[i]=newcset[i];
    }
    
    for(int i=0; i<32; i++)
    {
        newdata[i]=-1;
        newcset[i]=-1;
        
        x=prvscr.ffdata[i];
        y=animated_combo_table[x][0];
        
        //time to restart
        if((animated_combo_table4[y][1]>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c = newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim[c]=true;
            }
        }
    }
    
    for(int i=0; i<32; i++)
    {
        x=prvscr.ffdata[i];
        y=animated_combo_table2[x][0];
        
        //time to restart
        if((animated_combo_table24[y][1]>=combobuf[x].speed) &&
                (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
                (combobuf[x].nextcombo!=0))
        {
            newdata[i]=combobuf[x].nextcombo;
            newcset[i]=combobuf[x].nextcset;
            int c = newdata[i];
            
            if(combobuf[c].animflags & AF_CYCLE)
            {
                restartanim2[c]=true;
            }
        }
    }
    
    for(int i=0; i<32; i++)
    {
        if(newdata[i]==-1)
            continue;
            
        prvscr.ffdata[i]=newdata[i];
        prvscr.ffcset[i]=newcset[i];
    }
    
    if(get_bit(quest_rules,qr_CMBCYCLELAYERS))
    {
        for(int j=0; j<6; j++)
        {
            if(prvlayers[j].data.empty())
                continue;
                
            for(int i=0;	i<176; i++)
            {
                newdata[i]=-1;
                newcset[i]=-1;
                
                x=(prvlayers[j]).data[i];
                y=animated_combo_table[x][0];
                
                //time to restart
                if((animated_combo_table4[y][1]>=combobuf[x].speed) &&
                        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table[x][1]-1)	&&
                        (combobuf[x].nextcombo!=0))
                {
                    newdata[i]=combobuf[x].nextcombo;
                    newcset[i]=combobuf[x].nextcset;
                    int c = newdata[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim[c]=true;
                    }
                }
            }
            
            for(int i=0; i<176; i++)
            {
                x=(prvlayers[j]).data[i];
                y=animated_combo_table2[x][0];
                
                //time to restart
                if((animated_combo_table24[y][1]>=combobuf[x].speed) &&
                        (combobuf[x].tile-combobuf[x].frames>=animated_combo_table2[x][1]-1) &&
                        (combobuf[x].nextcombo!=0))
                {
                    newdata[i]=combobuf[x].nextcombo;
                    newcset[i]=combobuf[x].nextcset;
                    int c = newdata[i];
                    
                    if(combobuf[c].animflags & AF_CYCLE)
                    {
                        restartanim2[c]=true;
                    }
                }
            }
            
            for(int i=0; i<176; i++)
            {
                if(newdata[i]==-1)
                    continue;
                    
                prvlayers[j].data[i]=newdata[i];
                prvlayers[j].cset[i]=newcset[i];
            }
        }
    }
    
    for(int i=0; i<MAXCOMBOS; i++)
    {
        if(restartanim[i])
        {
            combobuf[i].tile = animated_combo_table[i][1];
            animated_combo_table4[animated_combo_table[i][0]][1]=0;
        }
        
        if(restartanim2[i])
        {
            combobuf[i].tile = animated_combo_table2[i][1];
            animated_combo_table4[animated_combo_table[i][0]][1]=0;
        }
    }
    
    return;
}

void zmap::update_freeform_combos()
{
    if(!prv_mode||!prv_cmbcycle)
    {
        return;
    }
    
    for(int i=0; i<32; i++)
    {
        if(!(prvscr.ffflags[i]&ffCHANGER) && prvscr.ffdata[i]!=0 && !(prvscr.ffflags[i]&ffSTATIONARY))
        {
        
            for(int j=0; j<32; j++)
            {
                if(i!=j)
                {
                    if(prvscr.ffflags[j]&ffCHANGER && prvscr.ffdata[j] != 0)
                    {
                        if((((prvscr.ffx[j]/10000)!=ffposx[i])||((prvscr.ffy[j]/10000)!=ffposy[i]))&&(prvscr.fflink[i]==0))
                        {
                            if((isonline(prvscr.ffx[i],prvscr.ffy[i],ffprvx[i],ffprvy[i],prvscr.ffx[j],prvscr.ffy[j])||
                                    ((prvscr.ffx[i]==prvscr.ffx[j])&&(prvscr.ffy[i]==prvscr.ffy[j])))&&(ffprvx[i]>-10000000&&ffprvy[i]>-10000000))
                            {
                                //prvscr.ffdata[i]=prvscr.ffdata[j];
                                //prvscr.ffcset[i]=prvscr.ffcset[j];
                                if(prvscr.ffflags[j]&ffCHANGETHIS)
                                {
                                    prvscr.ffdata[i] = prvscr.ffdata[j];
                                    prvscr.ffcset[i] = prvscr.ffcset[j];
                                }
                                
                                if(prvscr.ffflags[j]&ffCHANGENEXT)
                                    prvscr.ffdata[i]++;
                                    
                                if(prvscr.ffflags[j]&ffCHANGEPREV)
                                    prvscr.ffdata[i]--;
                                    
                                prvscr.ffdelay[i]=prvscr.ffdelay[j];
                                prvscr.ffx[i]=prvscr.ffx[j];
                                prvscr.ffy[i]=prvscr.ffy[j];
                                
                                prvscr.ffxdelta[i]=prvscr.ffxdelta[j];
                                prvscr.ffydelta[i]=prvscr.ffydelta[j];
                                prvscr.ffxdelta2[i]=prvscr.ffxdelta2[j];
                                prvscr.ffydelta2[i]=prvscr.ffydelta2[j];
                                
                                prvscr.fflink[i]=prvscr.fflink[j];
                                prvscr.ffwidth[i]=prvscr.ffwidth[j];
                                prvscr.ffheight[i]=prvscr.ffheight[j];
                                
                                if(prvscr.ffflags[i]&ffCARRYOVER)
                                    prvscr.ffflags[i]=prvscr.ffflags[j]&ffCARRYOVER;
                                else prvscr.ffflags[i]=prvscr.ffflags[j];
                                
                                prvscr.ffflags[i]&=~ffCHANGER;
                                ffposx[i]=(short)(prvscr.ffx[j]/10000);
                                ffposy[i]=(short)(prvscr.ffy[j]/10000);
                                
                                if(combobuf[prvscr.ffdata[j]].flag>15 && combobuf[prvscr.ffdata[j]].flag<32)
                                {
                                    prvscr.ffdata[j]=prvscr.secretcombo[combobuf[prvscr.ffdata[j]].flag-16+4];
                                }
                                
                                if((prvscr.ffflags[j]&ffSWAPNEXT)||(prvscr.ffflags[j]&ffSWAPPREV))
                                {
                                    int k=0;
                                    
                                    if(prvscr.ffflags[j]&ffSWAPNEXT)
                                        k=j<31?j+1:0;
                                        
                                    if(prvscr.ffflags[j]&ffSWAPPREV)
                                        k=j>0?j-1:31;
                                        
                                    zc_swap(prvscr.ffxdelta[j],prvscr.ffxdelta[k]);
                                    zc_swap(prvscr.ffydelta[j],prvscr.ffydelta[k]);
                                    zc_swap(prvscr.ffxdelta2[j],prvscr.ffxdelta2[k]);
                                    zc_swap(prvscr.ffydelta2[j],prvscr.ffydelta2[k]);
                                    zc_swap(prvscr.fflink[j],prvscr.fflink[k]);
                                    zc_swap(prvscr.ffwidth[j],prvscr.ffwidth[k]);
                                    zc_swap(prvscr.ffheight[j],prvscr.ffheight[k]);
                                    zc_swap(prvscr.ffflags[j],prvscr.ffflags[k]);
                                }
                            }
                        }
                    }
                }
            }
            
            if(prvscr.fflink[i] ? !prvscr.ffdelay[prvscr.fflink[i]] : !prvscr.ffdelay[i])
            {
                if(prvscr.fflink[i]&&(prvscr.fflink[i]-1)!=i)
                {
                    ffprvx[i] = prvscr.ffx[i];
                    ffprvy[i] = prvscr.ffy[i];
                    prvscr.ffx[i]+=prvscr.ffxdelta[prvscr.fflink[i]-1];
                    prvscr.ffy[i]+=prvscr.ffydelta[prvscr.fflink[i]-1];
                }
                else
                {
                    ffprvx[i] = prvscr.ffx[i];
                    ffprvy[i] = prvscr.ffy[i];
                    prvscr.ffx[i]+=prvscr.ffxdelta[i];
                    prvscr.ffy[i]+=prvscr.ffydelta[i];
                    prvscr.ffxdelta[i]+=prvscr.ffxdelta2[i];
                    prvscr.ffydelta[i]+=prvscr.ffydelta2[i];
                    
                    if(prvscr.ffxdelta[i]>1280000) prvscr.ffxdelta[i]=1280000;
                    
                    if(prvscr.ffxdelta[i]<-1280000) prvscr.ffxdelta[i]=-1280000;
                    
                    if(prvscr.ffydelta[i]>1280000) prvscr.ffydelta[i]=1280000;
                    
                    if(prvscr.ffydelta[i]<-1280000) prvscr.ffydelta[i]=-1280000;
                }
            }
            else
            {
                if(!prvscr.fflink[i] || (prvscr.fflink[i]-1)==i)
                    prvscr.ffdelay[i]--;
            }
            
            if(prvscr.ffx[i]<-320000)
            {
                if(prvscr.flags6&fWRAPAROUNDFF)
                {
                    prvscr.ffx[i] = (2880000+(prvscr.ffx[i]+320000));
                    ffprvy[i] = prvscr.ffy[i];
                }
                else
                {
                    prvscr.ffdata[i]=0;
                    prvscr.ffflags[i]&=~ffCARRYOVER;
                }
            }
            
            if(prvscr.ffy[i]<-320000)
            {
                if(prvscr.flags6&fWRAPAROUNDFF)
                {
                    prvscr.ffy[i] = 2080000+(prvscr.ffy[i]+320000);
                    ffprvx[i] = prvscr.ffx[i];
                }
                else
                {
                    prvscr.ffdata[i]=0;
                    prvscr.ffflags[i]&=~ffCARRYOVER;
                }
            }
            
            if(prvscr.ffx[i]>=2880000)
            {
                if(prvscr.flags6&fWRAPAROUNDFF)
                {
                    prvscr.ffx[i] = prvscr.ffx[i]-2880000-320000;
                    ffprvy[i] = prvscr.ffy[i];
                }
                else
                {
                    prvscr.ffdata[i]=0;
                    prvscr.ffflags[i]&=~ffCARRYOVER;
                }
            }
            
            if(prvscr.ffy[i]>=2080000)
            {
                if(prvscr.flags6&fWRAPAROUNDFF)
                {
                    prvscr.ffy[i] = prvscr.ffy[i]-2080000-320000;
                    ffprvy[i] = prvscr.ffy[i];
                }
                else
                {
                    prvscr.ffdata[i]=0;
                    prvscr.ffflags[i]&=~ffCARRYOVER;
                }
            }
            
        }
    }
}

void zmap::dowarp(int type, int index)
{
    if(type==0)
    {
    
        int dmap=screens[currscr].tilewarpdmap[index];
        int scr=screens[currscr].tilewarpscr[index];
        
        switch(screens[currscr].tilewarptype[index])
        {
        case wtCAVE:
        case wtNOWARP:
            break;
            
        default:
            setCurrMap(DMaps[dmap].map);
            setCurrScr(scr+DMaps[dmap].xoff);
            break;
        }
    }
    else if(type==1)
    {
        int dmap=screens[currscr].sidewarpdmap[index];
        int scr=screens[currscr].sidewarpscr[index];
        
        switch(screens[currscr].sidewarptype[index])
        {
        case wtCAVE:
        case wtNOWARP:
            break;
            
        default:
            setCurrMap(DMaps[dmap].map);
            setCurrScr(scr+DMaps[dmap].xoff);
            break;
        }
    }
}

extern int prv_twon;

void zmap::prv_dowarp(int type, int index)
{
    if(type==0)
    {
    
        int dmap=prvscr.tilewarpdmap[index];
        int scr=prvscr.tilewarpscr[index];
        
        switch(prvscr.tilewarptype[index])
        {
        case wtCAVE:
        case wtNOWARP:
            break;
            
        default:
            //setCurrMap(DMaps[dmap].map);
            //setCurrScr(scr+DMaps[dmap].xoff);
            set_prvscr(DMaps[dmap].map,scr+DMaps[dmap].xoff);
            loadlvlpal(getcolor());
            rebuild_trans_table();
            //prv_cmbcycle=0;
            break;
        }
    }
    else if(type==1)
    {
        int dmap=prvscr.sidewarpdmap[index];
        int scr=prvscr.sidewarpscr[index];
        
        switch(prvscr.sidewarptype[index])
        {
        case wtCAVE:
        case wtNOWARP:
            break;
            
        default:
            //setCurrMap(DMaps[dmap].map);
            //setCurrScr(scr+DMaps[dmap].xoff);
            set_prvscr(DMaps[dmap].map,scr+DMaps[dmap].xoff);
            loadlvlpal(getcolor());
            rebuild_trans_table();
            //prv_cmbcycle=0;
            break;
        }
    }
    
    if(prv_twon)
    {
        prv_time=get_prvscr()->timedwarptics;
    }
    
    //also reset FFC information (so that changers will work correctly) -DD
    memset(ffposx,0xFF,sizeof(short)*32);
    memset(ffposy,0xFF,sizeof(short)*32);
    memset(ffprvx,0xFF,sizeof(float)*32);
    memset(ffprvy,0xFF,sizeof(float)*32);
}

void zmap::dowarp2(int ring,int index)
{
    int dmap=misc.warp[ring].dmap[index];
    int scr=misc.warp[ring].scr[index];
    setCurrMap(DMaps[dmap].map);
    setCurrScr(scr+DMaps[dmap].xoff);
}

/******************************/
/******** ZQuest stuff ********/
/******************************/

bool save_msgstrs(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    if(writestrings(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXMSGS)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool save_msgstrs_text(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    if(writestrings_text(f)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_msgstrs(const char *path, int startstring)
{
    //these are here to bypass compiler warnings about unused arguments
    startstring=startstring;
    
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        return false;
    }
    
    if(section_id==ID_STRINGS)
    {
        if(readstrings(f, &header, true)==0)
        {
            pack_fclose(f);
            return true;
        }
        else
        {
            pack_fclose(f);
            return false;
        }
    }
    
    pack_fclose(f);
    return false;
}

bool save_pals(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    if(writecolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, 0, newerpdTOTAL)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_pals(const char *path, int startcset)
{
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        return false;
    }
    
    if(section_id==ID_CSETS)
    {
        if(readcolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, startcset, newerpdTOTAL-startcset, true)==0)
        {
            pack_fclose(f);
            loadlvlpal(Color);
            return true;
        }
        else
        {
            pack_fclose(f);
            return false;
        }
    }
    
    return false;
}

bool save_dmaps(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    if(writedmaps(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXDMAPS)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_dmaps(const char *path, int startdmap)
{
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        return false;
    }
    
    if(section_id==ID_DMAPS)
    {
        if(readdmaps(f, NULL, ZELDA_VERSION, VERSION_BUILD, startdmap, MAXDMAPS-startdmap, true)==0)
        {
            pack_fclose(f);
            return true;
        }
        else
        {
            pack_fclose(f);
            return false;
        }
    }
    
    return false;
}
bool save_combos(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    reset_combo_animations();
    reset_combo_animations2();
    
    if(writecombos(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXCOMBOS)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_combos(const char *path, int startcombo)
{
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        return false;
    }
    
    if(section_id==ID_COMBOS)
    {
        if(readcombos(f, NULL, ZELDA_VERSION, VERSION_BUILD, startcombo, MAXCOMBOS-startcombo, true)==0)
        {
            pack_fclose(f);
            return true;
        }
        else
        {
            pack_fclose(f);
            //      init_combos(true, &header);
            return false;
        }
    }
    
    return false;
}

bool save_tiles(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    //  reset_combo_animations();
    if(writetiles(f, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_tiles(const char *path, int starttile)
{
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        return false;
    }
    
    if(section_id==ID_TILES)
    {
        if(readtiles(f, newtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, starttile, NEWMAXTILES-starttile, false, true)==0)
        {
            pack_fclose(f);
            return true;
        }
        else
        {
            pack_fclose(f);
            init_tiles(true, &header);
            return false;
        }
    }
    
    return false;
}

int writeguys(PACKFILE *f, zquestheader *Header);
bool save_guys(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    /*
    int id = ID_GUYS;
    if(!p_mputl(id,f))
    {
      return false;
    }
    */
    
    zquestheader h;
    h.zelda_version = 0x250;
    h.build = 21;
    
    if(writeguys(f, &h)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_guys(const char *path)
{
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    zquestheader h;
    h.zelda_version = 0x250;
    h.build = 21;
    
    if(section_id==ID_GUYS)
    {
        if(readguys(f, &h, true)==0)
        {
            pack_fclose(f);
            return true;
        }
    }
    
    pack_fclose(f);
    return false;
}


//int writeguys(PACKFILE *f, zquestheader *Header);
bool save_combo_alias(const char *path)
{
    PACKFILE *f = pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
    {
        return false;
    }
    
    zquestheader h;
    h.zelda_version = 0x250;
    h.build = 21;
    
    if(writecomboaliases(f, 0, 0)==0)
    {
        pack_fclose(f);
        return true;
    }
    
    pack_fclose(f);
    return false;
}

bool load_combo_alias(const char *path)
{
    dword section_id;
    PACKFILE *f = pack_fopen_password(path,F_READ, "");
    
    if(!f)
    {
        return false;
    }
    
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    zquestheader h;
    h.zelda_version = 0x250;
    h.build = 21;
    
    if(section_id==ID_COMBOALIASES)
    {
        if(readcomboaliases(f, &h, 0, 0, true)==0)
        {
            pack_fclose(f);
            return true;
        }
    }
    
    pack_fclose(f);
    return false;
}

bool load_zgp(const char *path)
{
    dword section_id;
    dword section_version;
    dword section_cversion;
//  setPackfilePassword(NULL);
    PACKFILE *f=pack_fopen_password(path,F_READ,"");
    
    if(!f)
        return false;
        
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id!=ID_GRAPHICSPACK)
    {
        pack_fclose(f);
        return false;
    }
    
    //section version info
    if(!p_igetw(&section_version,f,true))
    {
        return 2;
    }
    
    if(!p_igetw(&section_cversion,f,true))
    {
        return 3;
    }
    
    //tiles
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_TILES)
    {
        if(readtiles(f, newtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)!=0)
        {
            pack_fclose(f);
            init_tiles(true, &header);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //combos
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_COMBOS)
    {
        if(readcombos(f, NULL, ZELDA_VERSION, VERSION_BUILD, 0, MAXCOMBOS, true)!=0)
        {
            pack_fclose(f);
            //      init_combos(true, &header);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //palettes
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_CSETS)
    {
        if(readcolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, 0, newerpdTOTAL, true)!=0)
        {
            pack_fclose(f);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //items
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_ITEMS)
    {
        if(readitems(f, ZELDA_VERSION, VERSION_BUILD, false, true)!=0)
        {
            pack_fclose(f);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //weapons
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_WEAPONS)
    {
        if(readweapons(f, &header, true)!=0)
        {
            pack_fclose(f);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //read the triforce pieces info and make sure it worked
    //really do this?
    
    //read the game icons info and make sure it worked
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_ICONS)
    {
        if(readgameicons(f, &header, &misc, true)!=0)
        {
            pack_fclose(f);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //read the misc colors info and map styles info and make sure it worked
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_COLORS)
    {
        if(readmisccolors(f, &header, &misc, true)!=0)
        {
            pack_fclose(f);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //read the door combo sets and make sure it worked
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id==ID_DOORS)
    {
        if(readdoorcombosets(f, &header, true)!=0)
        {
            pack_fclose(f);
            return false;
        }
    }
    else
    {
        pack_fclose(f);
        return false;
    }
    
    //read the template screens and make sure it worked
    //really do this?
    
    //yay!  it worked!  close the file and say everything was ok.
    loadlvlpal(Color);
    setup_combo_animations();
    setup_combo_animations2();
    pack_fclose(f);
    return true;
}

bool save_zgp(const char *path)
{
//  jwin_alert("Error","This feature not yet implemented.",NULL,NULL,"O&K",NULL,'k',0,lfont);
//  return false;
    reset_combo_animations();
    reset_combo_animations2();
    
    //open the file
    PACKFILE *f=pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
        return false;
        
    dword section_id=ID_GRAPHICSPACK;
    dword section_version=V_GRAPHICSPACK;
    dword section_cversion=CV_GRAPHICSPACK;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        return 1;
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        return 2;
    }
    
    if(!p_iputw(section_cversion,f))
    {
        return 3;
    }
    
    //tiles
    if(writetiles(f, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //combos
    if(writecombos(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXCOMBOS)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //palettes
    if(writecolordata(f, &misc, ZELDA_VERSION, VERSION_BUILD, 0, newerpdTOTAL)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //items
    if(writeitems(f, &header)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //weapons
    if(writeweapons(f, &header)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //write the triforce pieces info and make sure it worked
    //really do this?
    
    //write the game icons info and make sure it worked
    if(writegameicons(f, &header, &misc)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //write the misc colors info and map styles info and make sure it worked
    if(writemisccolors(f, &header, &misc)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //write the door combo sets and make sure it worked
    if(writedoorcombosets(f, &header)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    //write the template screens and make sure it worked
    //really do this?
    
    pack_fclose(f);
    return true;
}

bool save_subscreen(const char *path, bool *cancel)
{
//  jwin_alert("Error","This feature not yet implemented.",NULL,NULL,"O&K",NULL,'k',0,lfont);
//  return false;
    reset_combo_animations();
    reset_combo_animations2();
    *cancel = false;
    
    int ret;
    sslist_dlg[0].dp2=lfont;
    char *oldtitlestr=(char*)sslist_dlg[0].dp;
    char *editstr=(char*)sslist_dlg[3].dp;
    char *donestr=(char*)sslist_dlg[5].dp;
    const char *newtitlestr="Export Subscreen";
    const char *okstr="OK";
    const char *cancelstr="Cancel";
    sslist_dlg[0].dp=(void *)newtitlestr;
    sslist_dlg[3].dp=(void *)okstr;
    sslist_dlg[4].proc = d_dummy_proc;
    sslist_dlg[5].dp=(void *)cancelstr;
    show_new_ss=false;
    //strcpy((char*)sslist_dlg[3].dp,"Save");
    //strcpy((char*)sslist_dlg[4].dp,"Cancel");
    ret = zc_popup_dialog(sslist_dlg,2);
    //strcpy((char*)sslist_dlg[3].dp,"Edit");
    //strcpy((char*)sslist_dlg[4].dp,"Done");
    sslist_dlg[0].dp=oldtitlestr;
    sslist_dlg[3].dp=editstr;
    sslist_dlg[4].proc = jwin_button_proc;
    sslist_dlg[5].dp=donestr;
    show_new_ss=true;
    
    if(ret==0||ret==5)
    {
        *cancel=true;
        return true;
    }
    
    //open the file
    PACKFILE *f=pack_fopen_password(path,F_WRITE, "");
    
    if(!f)
        return false;
        
    dword section_id=ID_SUBSCREEN;
    dword section_version=V_SUBSCREEN;
    dword section_cversion=CV_SUBSCREEN;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        pack_fclose(f);
        return false;
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        pack_fclose(f);
        return false;
    }
    
    if(!p_iputw(section_cversion,f))
    {
        pack_fclose(f);
        return false;
    }
    
    //subscreens
    if(write_one_subscreen(f,&header,sslist_dlg[2].d1)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    pack_fclose(f);
    return true;
}

bool load_subscreen(const char *path)
{
    int ret;
    sslist_dlg[0].dp2=lfont;
    char *oldtitlestr=(char*)sslist_dlg[0].dp;
    char *editstr=(char*)sslist_dlg[3].dp;
    char *donestr=(char*)sslist_dlg[5].dp;
    const char *newtitlestr="Import Subscreen";
    const char *okstr="OK";
    const char *cancelstr="Cancel";
    sslist_dlg[0].dp=(void *)newtitlestr;
    sslist_dlg[3].dp=(void *)okstr;
    sslist_dlg[4].proc = d_dummy_proc;
    sslist_dlg[5].dp=(void *)cancelstr;
    //strcpy((char*)sslist_dlg[3].dp,"Write");
    //strcpy((char*)sslist_dlg[4].dp,"Cancel");
    ret = zc_popup_dialog(sslist_dlg,2);
    //strcpy((char*)sslist_dlg[3].dp,"Edit");
    //strcpy((char*)sslist_dlg[4].dp,"Done");
    sslist_dlg[0].dp=(void *)oldtitlestr;
    sslist_dlg[3].dp=(void *)editstr;
    sslist_dlg[4].proc = jwin_button_proc;
    sslist_dlg[5].dp=(void *)donestr;
    
    if(ret==0||ret==5)
    {
        return true;
    }
    
    //open the file
    PACKFILE *f=pack_fopen_password(path,F_READ, "");
    
    if(!f)
        return false;
        
    dword section_id;
    dword section_version;
    dword section_cversion;
    
    //section id
    if(!p_mgetl(&section_id,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(section_id!=ID_SUBSCREEN)
    {
        pack_fclose(f);
        return false;
    }
    
    //section version info
    if(!p_igetw(&section_version,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    if(!p_igetw(&section_cversion,f,true))
    {
        pack_fclose(f);
        return false;
    }
    
    //subscreens
    if(read_one_subscreen(f,&header,true,sslist_dlg[2].d1,section_version,section_cversion)!=0)
    {
        pack_fclose(f);
        return false;
    }
    
    pack_fclose(f);
    return true;
}

bool setMapCount2(int c)
{
    int oldmapcount=map_count;
    int currmap=Map.getCurrMap();
    
    bound(c,1,MAXMAPS2);
    map_count=c;
    
    try
    {
        TheMaps.resize(c*MAPSCRS);
    }
    catch(...)
    {
        jwin_alert("Error","Failed to change map count.",NULL,NULL,"O&K",NULL,'k',0,lfont);
        return false;
    }
    
    if(map_count>oldmapcount)
    {
        for(int mc=oldmapcount; mc<map_count; mc++)
        {
            Map.setCurrMap(mc);
            Map.clearzcmap(mc);
            
            for(int ms=0; ms<MAPSCRS; ms++)
            {
                Map.clearscr(ms);
            }
        }
    }
    else
    {
        if(!layers_valid(Map.CurrScr()))
            fix_layers(Map.CurrScr(), false);
            
        for(int i=0; i<MAXDMAPS; i++)
        {
            if(DMaps[i].map>=map_count)
            {
                DMaps[i].map=map_count-1;
            }
        }
    }
    
    Map.setCurrMap(bound(currmap,0,c-1));
    
    return true;
}

extern BITMAP *bmap;

int init_quest(const char *templatefile)
{
    char buf[2048];
    templatefile=templatefile; //here to prevent compiler warnings
    load_quest("qst.dat#NESQST_NEW_QST",true,true);
    sprintf(buf,"ZQuest - Untitled Quest");
    set_window_title(buf);
    zinit.last_map = 0;
    zinit.last_screen = 0;
    
    if(bmap != NULL)
    {
        destroy_bitmap(bmap);
        bmap=NULL;
    }
    
    return 0;
}

void set_questpwd(const char *pwd, bool use_keyfile)
{
    //these are here to bypass compiler warnings about unused arguments
    use_keyfile=use_keyfile;
    
    memset(header.password,0,256);
    strcpy(header.password,pwd);
    header.dirty_password=true;
    
    cvs_MD5Context ctx;
    cvs_MD5Init(&ctx);
    cvs_MD5Update(&ctx, (const unsigned char*)pwd, (unsigned)strlen(pwd));
    cvs_MD5Final(header.pwd_hash, &ctx);
}


bool is_null_pwd_hash(unsigned char *pwd_hash)
{
    cvs_MD5Context ctx;
    unsigned char md5sum[16];
    char pwd[2]="";
    
    cvs_MD5Init(&ctx);
    cvs_MD5Update(&ctx, (const unsigned char*)pwd, (unsigned)strlen(pwd));
    cvs_MD5Final(md5sum, &ctx);
    
    return (memcmp(md5sum,pwd_hash,16)==0);
}

static DIALOG pwd_dlg[] =
{
    /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
    { jwin_win_proc,        0,   0,  224+22+1,   88+10+1,   vc(14),  vc(1),  0,       0,          0,             0, (void *) "Requires Authorization", NULL, NULL },
    { jwin_text_proc,       16,   28,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "File name:", NULL, NULL },
    // 2 (filename)
    { jwin_text_proc,       72,  28, 128,   8,    vc(11),  vc(1),  0,       0,          24,            0,       NULL, NULL, NULL },
    { jwin_text_proc,       16, 38, 0, 8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Challenge:", NULL, NULL },
    // 4 (challenge hash)
    { jwin_text_proc,       72, 38, 0, 8,    vc(15),  vc(1),  0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_text_proc,       16,  42+10,  96,   8,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Password:", NULL, NULL },
    // 6 (password)
    { jwin_edit_proc,       72,  38+10,  120+39,   16,    vc(12),  vc(1),  0,       0,          255,            0,       NULL, NULL, NULL },
    { jwin_button_proc,     42,   62+10,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
    { jwin_button_proc,     122,  62+10,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    { NULL,                 0,    0,      0,    0,    0,       0,      0,       0,          0,             0,       NULL, NULL, NULL }
};

int reverse_string(char* str)
{

    if(NULL==str)
    {
        return -1; //no string
    }
    
    int l=(int)strlen(str)-1; //get the string length
    
    if(1==l)
    {
        return 1;
    }
    
    char c;
    
    for(int x=0; x < l; x++,l--)
    {
        c = str[x];
        str[x] = str[l];
        str[l] = c;
    }
    
    return 0;
}


int quest_access(const char *filename, zquestheader *hdr, bool compressed)
{
    //Protection against compiling a release version with password protection off.
    static bool passguard = false;
    
#if ( !(defined _DEBUG) || (defined _RELEASE || defined NDEBUG || defined _NDEBUG) )
#define MUST_HAVE_PASSWORD
    passguard = true;
#endif
    
#if ( !(defined MUST_HAVE_PASSWORD) || defined _NPASS )
#if (defined _MSC_VER || defined _NPASS)
    assert(!passguard);
    return 1;
#endif
#endif
    
    
    char hash_string[33];
    
    if(!compressed)
    {
        return 1;
    }
    
    if((get_debug() && (!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))) || is_null_pwd_hash(hdr->pwd_hash))
    {
        return 1;
    }
    
    char pwd[256];
    char prompt[256]="";
    
    char keyfilename[2048];
    replace_extension(keyfilename, filename, "key", 2047);
    bool gotfromkey=false;
    
    if(exists(keyfilename))
    {
        char password[256];
        PACKFILE *fp = pack_fopen_password(keyfilename, F_READ, "");
        char msg[80];
        memset(msg,0,80);
        pfread(msg, 80, fp,true);
        
        if(strcmp(msg,"ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!")==0)
        {
            short ver = 0;
            byte  bld = 0;
            short pwd_len;
            p_igetw(&ver,fp,true);
            p_getc(&bld,fp,true);
            memset(password,0,256);
            
            if((ver > 0x211)||((ver == 0x211)&&(bld>1)))
            {
                pwd_len=256;
            }
            else
            {
                pwd_len=30;
            }
            
            pfread(password, pwd_len, fp,true);
            gotfromkey=check_questpwd(hdr, password);
            memset(password,0,256);
            memset(pwd,0,256);
        }
        
        pack_fclose(fp);
    }
    
    if(gotfromkey)
    {
        return true;
    }
    
    pwd_dlg[0].dp2=lfont;
    pwd_dlg[2].dp=get_filename(filename);
    cvs_MD5Context ctx;
    unsigned char md5sum[16];
    char response[33];
    
    memcpy(md5sum, hdr->pwd_hash, 16);
    
    for(int i=0; i<300; ++i)
    {
        for(int j=0; j<16; ++j)
        {
            sprintf(response+j*2, "%02x", md5sum[j]);
        }
        
        if(i&1)
        {
            reverse_string(response);
        }
        
        if(i==149)
        {
            strcpy(hash_string, response);
        }
        
        cvs_MD5Init(&ctx);
        cvs_MD5Update(&ctx, (const unsigned char*)response, (unsigned)strlen(response));
        cvs_MD5Final(md5sum, &ctx);
    }
    
    pwd_dlg[4].dp=hash_string;
    
    if(get_debug() && (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
    {
        sprintf(prompt,"%s",response);
    }
    
    pwd_dlg[6].dp=prompt;
    
    if(is_large)
        large_dialog(pwd_dlg);
        
    int cancel = zc_popup_dialog(pwd_dlg,6);
    
    if(cancel == 8)
        return 2;
        
    bool ret=check_questpwd(hdr, prompt);
    
    if(!ret)
    {
        ret=(strcmp(response,prompt)==0);
    }
    
    memset(pwd,0,256);
    return ret ? 1 : 0;
}

// wrapper to reinitialize everything on an error
int load_quest(const char *filename, bool compressed, bool encrypted)
{
    char buf[2048];
//  if(encrypted)
//	  setPackfilePassword(datapwd);
    byte skip_flags[4];
    
    for(int i=0; i<4; ++i)
    {
        skip_flags[i]=0;
    }
    
    int ret=loadquest(filename,&header,&misc,customtunes,true,compressed,encrypted,true,skip_flags);
//  setPackfilePassword(NULL);

    if(ret!=qe_OK)
    {
        init_quest(NULL);
    }
    else
    {
        int accessret = quest_access(filename, &header, compressed);
        
        if(accessret != 1)
        {
            init_quest(NULL);
            
            if(accessret == 0)
                ret=qe_pwd;
            else
                ret=qe_cancel;
        }
        else
        {
            Map.setCurrMap(vbound(Map.getCurrMap(),0,map_count-1));
            refresh(rALL);
            refresh_pal();
            
            if(bmap != NULL)
            {
                destroy_bitmap(bmap);
                bmap=NULL;
            }
            
            sprintf(buf,"ZQuest - [%s]", get_filename(filename));
//      if (compressed)
            {
                set_window_title(buf);
            }
        }
    }
    
    return ret;
}

bool write_midi(MIDI *m,PACKFILE *f)
{
    int c;
    
    if(!p_mputw(m->divisions,f)) return false;
    
    for(c=0; c<MIDI_TRACKS; c++)
    {
        if(!p_mputl(m->track[c].len,f)) return false;
        
        if(m->track[c].len > 0)
        {
            if(!pfwrite(m->track[c].data,m->track[c].len,f))
                return false;
        }
    }
    
    return true;
}

bool write_music(int format, MIDI* m, PACKFILE *f)
{
    // format - data format (midi, nsf, ...)
    // m - pointer to data.
    
    int c;
    
    switch(format)
    {
    case MFORMAT_MIDI:
    
        if(!p_mputw(m->divisions,f)) return false;
        
        for(c=0; c<MIDI_TRACKS; c++)
        {
            if(!p_mputl(m->track[c].len,f)) return false;
            
            if(m->track[c].len > 0)
            {
                if(!pfwrite(m->track[c].data,m->track[c].len,f))
                    return false;
            }
        }
        
        break;
        
    case MFORMAT_NSF:
    
        break;
        
    default:
        return false;
        break;
    }
    
    return true;
}

int writeheader(PACKFILE *f, zquestheader *Header)
{
    dword section_id=ID_HEADER;
    dword section_version=V_HEADER;
    dword section_cversion=CV_HEADER;
    dword section_size=0;
    
    //file header string
    if(!pfwrite(Header->id_str,sizeof(Header->id_str),f))
    {
        new_return(1);
    }
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(2);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(3);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(4);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(5);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(Header->zelda_version,f))
        {
            new_return(6);
        }
        
        if(!p_putc(Header->build,f))
        {
            new_return(7);
        }
        
        if(!pfwrite(Header->pwd_hash,sizeof(Header->pwd_hash),f))
        {
            new_return(8);
        }
        
        if(!p_iputw(Header->internal,f))
        {
            new_return(10);
        }
        
        if(!p_putc(Header->quest_number,f))
        {
            new_return(11);
        }
        
        if(!pfwrite(Header->version,sizeof(Header->version),f))
        {
            new_return(12);
        }
        
        if(!pfwrite(Header->minver,sizeof(Header->minver),f))
        {
            new_return(13);
        }
        
        if(!pfwrite(Header->title,sizeof(Header->title),f))
        {
            new_return(14);
        }
        
        if(!pfwrite(Header->author,sizeof(Header->author),f))
        {
            new_return(15);
        }
        
        if(!p_putc(Header->use_keyfile,f))
        {
            new_return(16);
        }
        
        if(!pfwrite(Header->data_flags,sizeof(Header->data_flags),f))
        {
            new_return(17);
        }
        
        if(!pfwrite(Header->templatepath,sizeof(Header->templatepath),f))
        {
            new_return(19);
        }
        
        if(!p_putc(0,f)) //why are we doing this?
        {
            new_return(20);
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeheader()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writerules(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_RULES;
    dword section_version=V_RULES;
    dword section_cversion=CV_RULES;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!pfwrite(quest_rules,QUESTRULES_SIZE,f))
        {
            new_return(5);
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writerules()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}


int writedoorcombosets(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_DOORS;
    dword section_version=V_DOORS;
    dword section_cversion=CV_DOORS;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(door_combo_set_count,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<door_combo_set_count; i++)
        {
            //name
            if(!pfwrite(&DoorComboSets[i].name,sizeof(DoorComboSets[0].name),f))
            {
                new_return(6);
            }
            
            //up door
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<4; k++)
                {
                    if(!p_iputw(DoorComboSets[i].doorcombo_u[j][k],f))
                    {
                        new_return(7);
                    }
                }
            }
            
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<4; k++)
                {
                    if(!p_putc(DoorComboSets[i].doorcset_u[j][k],f))
                    {
                        new_return(8);
                    }
                }
            }
            
            //down door
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<4; k++)
                {
                    if(!p_iputw(DoorComboSets[i].doorcombo_d[j][k],f))
                    {
                        new_return(9);
                    }
                }
            }
            
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<4; k++)
                {
                    if(!p_putc(DoorComboSets[i].doorcset_d[j][k],f))
                    {
                        new_return(10);
                    }
                }
            }
            
            
            //left door
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<6; k++)
                {
                    if(!p_iputw(DoorComboSets[i].doorcombo_l[j][k],f))
                    
                    {
                        new_return(11);
                    }
                }
            }
            
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<6; k++)
                {
                    if(!p_putc(DoorComboSets[i].doorcset_l[j][k],f))
                    {
                        new_return(12);
                    }
                }
            }
            
            //right door
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<6; k++)
                {
                    if(!p_iputw(DoorComboSets[i].doorcombo_r[j][k],f))
                    {
                        new_return(13);
                    }
                }
            }
            
            for(int j=0; j<9; j++)
            {
                for(int k=0; k<6; k++)
                {
                    if(!p_putc(DoorComboSets[i].doorcset_r[j][k],f))
                    {
                        new_return(14);
                    }
                }
            }
            
            
            //up bomb rubble
            for(int j=0; j<2; j++)
            {
                if(!p_iputw(DoorComboSets[i].bombdoorcombo_u[j],f))
                {
                    new_return(15);
                }
            }
            
            for(int j=0; j<2; j++)
            {
                if(!p_putc(DoorComboSets[i].bombdoorcset_u[j],f))
                {
                    new_return(16);
                }
            }
            
            //down bomb rubble
            for(int j=0; j<2; j++)
            {
                if(!p_iputw(DoorComboSets[i].bombdoorcombo_d[j],f))
                {
                    new_return(17);
                }
            }
            
            for(int j=0; j<2; j++)
            {
                if(!p_putc(DoorComboSets[i].bombdoorcset_d[j],f))
                {
                    new_return(18);
                }
            }
            
            //left bomb rubble
            for(int j=0; j<3; j++)
            {
                if(!p_iputw(DoorComboSets[i].bombdoorcombo_l[j],f))
                {
                    new_return(19);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_putc(DoorComboSets[i].bombdoorcset_l[j],f))
                {
                    new_return(20);
                }
            }
            
            //right bomb rubble
            for(int j=0; j<3; j++)
            {
                if(!p_iputw(DoorComboSets[i].bombdoorcombo_r[j],f))
                {
                    new_return(21);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_putc(DoorComboSets[i].bombdoorcset_r[j],f))
                {
                    new_return(22);
                }
            }
            
            //walkthrough stuff
            for(int j=0; j<4; j++)
            {
                if(!p_iputw(DoorComboSets[i].walkthroughcombo[j],f))
                {
                    new_return(23);
                }
            }
            
            for(int j=0; j<4; j++)
            {
                if(!p_putc(DoorComboSets[i].walkthroughcset[j],f))
                {
                    new_return(24);
                }
            }
            
            //flags
            for(int j=0; j<2; j++)
            {
                if(!p_putc(DoorComboSets[i].flags[j],f))
                {
                    new_return(25);
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writedoorcombosets()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writedmaps(PACKFILE *f, word version, word build, word start_dmap, word max_dmaps)
{
    //these are here to bypass compiler warnings about unused arguments
    version=version;
    build=build;
    
    word dmap_count=count_dmaps();
    dword section_id=ID_DMAPS;
    dword section_version=V_DMAPS;
    dword section_cversion=CV_DMAPS;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        dmap_count=zc_min(dmap_count, max_dmaps);
        dmap_count=zc_min(dmap_count, MAXDMAPS-start_dmap);
        
        //finally...  section data
        if(!p_iputw(dmap_count,f))
        {
            new_return(5);
        }
        
        
        for(int i=start_dmap; i<start_dmap+dmap_count; i++)
        {
            if(!p_putc(DMaps[i].map,f))
            {
                new_return(6);
            }
            
            if(!p_iputw(DMaps[i].level,f))
            {
                new_return(7);
            }
            
            if(!p_putc(DMaps[i].xoff,f))
            {
                new_return(8);
            }
            
            if(!p_putc(DMaps[i].compass,f))
            {
                new_return(9);
            }
            
            if(!p_iputw(DMaps[i].color,f))
            {
                new_return(10);
            }
            
            if(!p_putc(DMaps[i].midi,f))
            {
                new_return(11);
            }
            
            if(!p_putc(DMaps[i].cont,f))
            {
                new_return(12);
            }
            
            if(!p_putc(DMaps[i].type,f))
            {
                new_return(13);
            }
            
            for(int j=0; j<8; j++)
            {
                if(!p_putc(DMaps[i].grid[j],f))
                {
                    new_return(14);
                }
            }
            
            //16
            if(!pfwrite(&DMaps[i].name,sizeof(DMaps[0].name),f))
            {
                new_return(15);
            }
            
            if(!pfwrite(&DMaps[i].title,sizeof(DMaps[0].title),f))
            {
                new_return(16);
            }
            
            if(!pfwrite(&DMaps[i].intro,sizeof(DMaps[0].intro),f))
            {
                new_return(17);
            }
            
            if(!p_iputl(DMaps[i].minimap_1_tile,f))
            {
                new_return(18);
            }
            
            if(!p_putc(DMaps[i].minimap_1_cset,f))
            {
                new_return(19);
            }
            
            if(!p_iputl(DMaps[i].minimap_2_tile,f))
            {
                new_return(20);
            }
            
            if(!p_putc(DMaps[i].minimap_2_cset,f))
            {
                new_return(21);
            }
            
            if(!p_iputl(DMaps[i].largemap_1_tile,f))
            {
                new_return(22);
            }
            
            if(!p_putc(DMaps[i].largemap_1_cset,f))
            {
                new_return(23);
            }
            
            if(!p_iputl(DMaps[i].largemap_2_tile,f))
            {
                new_return(24);
            }
            
            if(!p_putc(DMaps[i].largemap_2_cset,f))
            {
                new_return(25);
            }
            
            if(!pfwrite(&DMaps[i].tmusic,sizeof(DMaps[0].tmusic),f))
            {
                new_return(26);
            }
            
            if(!p_putc(DMaps[i].tmusictrack,f))
            {
                new_return(25);
            }
            
            if(!p_putc(DMaps[i].active_subscreen,f))
            {
                new_return(26);
            }
            
            if(!p_putc(DMaps[i].passive_subscreen,f))
            {
                new_return(27);
            }
            
            byte disabled[32];
            memset(disabled,0,32);
            
            for(int j=0; j<MAXITEMS; j++)
            {
                if(DMaps[i].disableditems[j])
                {
                    disabled[j/8] |= (1 << (j%8));
                }
            }
            
            if(!pfwrite(disabled,32,f))
            {
                new_return(28);
            }
            
            if(!p_iputl(DMaps[i].flags,f))
            {
                new_return(29);
            }
	    if(!p_putc(DMaps[i].sideview,f))
            {
                new_return(30);
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writedmaps()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writemisccolors(PACKFILE *f, zquestheader *Header, miscQdata *Misc)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_COLORS;
    dword section_version=V_COLORS;
    dword section_cversion=CV_COLORS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        if(!p_putc(Misc->colors.text,f))
        {
            new_return(5);
        }
        
        if(!p_putc(Misc->colors.caption,f))
        {
            new_return(6);
        }
        
        if(!p_putc(Misc->colors.overw_bg,f))
        {
            new_return(7);
        }
        
        if(!p_putc(Misc->colors.dngn_bg,f))
        {
            new_return(8);
        }
        
        if(!p_putc(Misc->colors.dngn_fg,f))
        {
            new_return(9);
        }
        
        if(!p_putc(Misc->colors.cave_fg,f))
        {
            new_return(10);
        }
        
        if(!p_putc(Misc->colors.bs_dk,f))
        {
            new_return(11);
        }
        
        if(!p_putc(Misc->colors.bs_goal,f))
        {
            new_return(12);
        }
        
        if(!p_putc(Misc->colors.compass_lt,f))
        {
            new_return(13);
        }
        
        if(!p_putc(Misc->colors.compass_dk,f))
        {
            new_return(14);
        }
        
        if(!p_putc(Misc->colors.subscr_bg,f))
        {
            new_return(15);
        }
        
        if(!p_putc(Misc->colors.triframe_color,f))
        {
            new_return(16);
        }
        
        if(!p_putc(Misc->colors.link_dot,f))
        {
            new_return(17);
        }
        
        if(!p_putc(Misc->colors.bmap_bg,f))
        {
            new_return(18);
        }
        
        if(!p_putc(Misc->colors.bmap_fg,f))
        {
            new_return(19);
        }
        
        if(!p_putc(Misc->colors.triforce_cset,f))
        {
            new_return(20);
        }
        
        if(!p_putc(Misc->colors.triframe_cset,f))
        {
            new_return(21);
        }
        
        if(!p_putc(Misc->colors.overworld_map_cset,f))
        {
            new_return(22);
        }
        
        if(!p_putc(Misc->colors.dungeon_map_cset,f))
        {
            new_return(23);
        }
        
        if(!p_putc(Misc->colors.blueframe_cset,f))
        {
            new_return(24);
        }
        
        if(!p_iputw(Misc->colors.triforce_tile,f))
        {
            new_return(25);
        }
        
        if(!p_iputw(Misc->colors.triframe_tile,f))
        {
            new_return(26);
        }
        
        if(!p_iputw(Misc->colors.overworld_map_tile,f))
        {
            new_return(27);
        }
        
        if(!p_iputw(Misc->colors.dungeon_map_tile,f))
        {
            new_return(28);
        }
        
        if(!p_iputw(Misc->colors.blueframe_tile,f))
        {
            new_return(29);
        }
        
        if(!p_iputw(Misc->colors.HCpieces_tile,f))
        {
            new_return(30);
        }
        
        if(!p_putc(Misc->colors.HCpieces_cset,f))
        {
            new_return(31);
        }
        
        if(!p_putc(Misc->colors.subscr_shadow,f))
        {
            new_return(32);
        }
        
        if(!p_putc(Misc->colors.msgtext,f))
        {
            new_return(33);
        }
	
        if(!p_iputl(Misc->colors.new_triforce_tile,f))
        {
            new_return(34);
        }
        
        if(!p_iputl(Misc->colors.new_triframe_tile,f))
        {
            new_return(35);
        }
        
        if(!p_iputl(Misc->colors.new_overworld_map_tile,f))
        {
            new_return(36);
        }
        
        if(!p_iputl(Misc->colors.new_dungeon_map_tile,f))
        {
            new_return(37);
        }
        
        if(!p_iputl(Misc->colors.new_blueframe_tile,f))
        {
            new_return(38);
        }
        
        if(!p_iputl(Misc->colors.new_HCpieces_tile,f))
        {
            new_return(39);
        }
        
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writemisccolors()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writegameicons(PACKFILE *f, zquestheader *Header, miscQdata *Misc)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_ICONS;
    dword section_version=V_ICONS;
    dword section_cversion=CV_ICONS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputl(Misc->icons[i],f))
            {
                new_return(5);
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writegameicons()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writemisc(PACKFILE *f, zquestheader *Header, miscQdata *Misc)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_MISC;
    dword section_version=V_MISC;
    dword section_cversion=CV_MISC;
    word shops=count_shops(Misc);
    word infos=count_infos(Misc);
    word warprings=count_warprings(Misc);
    word triforces=8;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //shops
        if(!p_iputw(shops,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<shops; i++)
        {
            if(!pfwrite(Misc->shop[i].name,sizeof(Misc->shop[i].name),f))
            {
                new_return(6);
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_putc(Misc->shop[i].item[j],f))
                {
                    new_return(7);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_iputw(Misc->shop[i].price[j],f))
                {
                    new_return(8);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_putc(Misc->shop[i].hasitem[j],f))
                {
                    new_return(9);
                }
            }
	    
        }
        
        //infos
        if(!p_iputw(infos,f))
        {
            new_return(10);
        }
        
        for(int i=0; i<infos; i++)
        {
            if(!pfwrite(Misc->info[i].name,sizeof(Misc->info[i].name),f))
            {
                new_return(11);
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_iputw(Misc->info[i].str[j],f))
                {
                    new_return(12);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_iputw(Misc->info[i].price[j],f))
                {
                    new_return(13);
                }
            }
        }
        
        //warp rings
        if(!p_iputw(warprings,f))
        {
            new_return(14);
        }
        
        for(int i=0; i<warprings; i++)
        {
            for(int j=0; j<9; j++)
            {
                if(!p_iputw(Misc->warp[i].dmap[j],f))
                {
                    new_return(15);
                }
            }
            
            for(int j=0; j<9; j++)
            {
                if(!p_putc(Misc->warp[i].scr[j],f))
                {
                    new_return(16);
                }
            }
            
            if(!p_putc(Misc->warp[i].size,f))
            {
                new_return(17);
            }
        }
        
        //triforce pieces
        for(int i=0; i<triforces; i++)
        {
            if(!p_putc(Misc->triforce[i],f))
            {
                new_return(18);
            }
        }
        
        //end string
        if(!p_iputw(Misc->endstring,f))
        {
            new_return(19);
        }
	
	//V_MISC >= 8
	for(int i=0; i<shops; i++)
	{
	    for(int j=0; j<3; j++)
            {
                if(!p_iputw(Misc->shop[i].str[j],f))
                {
                    new_return(20);
                }
            }
        }
	//V_MISC >= 9
	for ( int q = 0; q < 32; q++ ) 
	{
		if(!p_iputl(Misc->questmisc[q],f))
                    new_return(21);
	}
	for ( int q = 0; q < 32; q++ ) 
	{
		for ( int j = 0; j < 128; j++ )
		if(!p_putc(Misc->questmisc_strings[q][j],f))
                     new_return(22);
	}
	
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writemisc()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writeitems(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_ITEMS;
    dword section_version=V_ITEMS;
    dword section_cversion=CV_ITEMS;
    //  dword section_size=0;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(iMax,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<iMax; i++)
        {
            if(!pfwrite(item_string[i], 64, f))
            {
                new_return(5);
            }
        }
        
        for(int i=0; i<iMax; i++)
        {
            if(!p_iputl(itemsbuf[i].tile,f))
            {
                new_return(6);
            }
            
            if(!p_putc(itemsbuf[i].misc,f))
            {
                new_return(7);
            }
            
            if(!p_putc(itemsbuf[i].csets,f))
            {
                new_return(8);
            }
            
            if(!p_putc(itemsbuf[i].frames,f))
            {
                new_return(9);
            }
            
            if(!p_putc(itemsbuf[i].speed,f))
            {
                new_return(10);
            }
            
            if(!p_putc(itemsbuf[i].delay,f))
            {
                new_return(11);
            }
            
            if(!p_iputl(itemsbuf[i].ltm,f))
            {
                new_return(12);
            }
            
            if(!p_iputl(itemsbuf[i].family,f))
            {
                new_return(13);
            }
            
            if(!p_putc(itemsbuf[i].fam_type,f))
            {
                new_return(14);
            }
            
            if(!p_iputl(itemsbuf[i].power,f))
            {
                new_return(14);
            }
            
            if(!p_iputw(itemsbuf[i].flags,f))
            {
                new_return(15);
            }
            
            if(!p_iputw(itemsbuf[i].script,f))
            {
                new_return(16);
            }
            
            if(!p_putc(itemsbuf[i].count,f))
            {
                new_return(17);
            }
            
            if(!p_iputw(itemsbuf[i].amount,f))
            {
                new_return(18);
            }
            
            if(!p_iputw(itemsbuf[i].collect_script,f))
            {
                new_return(19);
            }
            
            if(!p_iputw(itemsbuf[i].setmax,f))
            {
                new_return(21);
            }
            
            if(!p_iputw(itemsbuf[i].max,f))
            {
                new_return(22);
            }
            
            if(!p_putc(itemsbuf[i].playsound,f))
            {
                new_return(23);
            }
            
            for(int j=0; j<8; j++)
            {
                if(!p_iputl(itemsbuf[i].initiald[j],f))
                {
                    new_return(24);
                }
            }
            
            for(int j=0; j<2; j++)
            {
                if(!p_putc(itemsbuf[i].initiala[j],f))
                {
                    new_return(25);
                }
            }
            
            if(!p_putc(itemsbuf[i].wpn,f))
            {
                new_return(26);
            }
            
            if(!p_putc(itemsbuf[i].wpn2,f))
            {
                new_return(27);
            }
            
            if(!p_putc(itemsbuf[i].wpn3,f))
            {
                new_return(28);
            }
            
            if(!p_putc(itemsbuf[i].wpn4,f))
            {
                new_return(29);
            }
            
            if(!p_putc(itemsbuf[i].wpn5,f))
            {
                new_return(30);
            }
            
            if(!p_putc(itemsbuf[i].wpn6,f))
            {
                new_return(31);
            }
            
            if(!p_putc(itemsbuf[i].wpn7,f))
            {
                new_return(32);
            }
            
            if(!p_putc(itemsbuf[i].wpn8,f))
            {
                new_return(33);
            }
            
            if(!p_putc(itemsbuf[i].wpn9,f))
            {
                new_return(34);
            }
            
            if(!p_putc(itemsbuf[i].wpn10,f))
            {
                new_return(35);
            }
            
            if(!p_putc(itemsbuf[i].pickup_hearts,f))
            {
                new_return(36);
            }
            
            if(!p_iputl(itemsbuf[i].misc1,f))
            {
                new_return(37);
            }
            
            if(!p_iputl(itemsbuf[i].misc2,f))
            {
                new_return(38);
            }
            
            if(!p_putc(itemsbuf[i].magic,f))
            {
                new_return(39);
            }
            
            if(!p_iputl(itemsbuf[i].misc3,f))
            {
                new_return(40);
            }
            
            if(!p_iputl(itemsbuf[i].misc4,f))
            {
                new_return(41);
            }
            
            if(!p_iputl(itemsbuf[i].misc5,f))
            {
                new_return(42);
            }
            
            if(!p_iputl(itemsbuf[i].misc6,f))
            {
                new_return(43);
            }
            
            if(!p_iputl(itemsbuf[i].misc7,f))
            {
                new_return(44);
            }
            
            if(!p_iputl(itemsbuf[i].misc8,f))
            {
                new_return(45);
            }
            
            if(!p_iputl(itemsbuf[i].misc9,f))
            {
                new_return(46);
            }
            
            if(!p_iputl(itemsbuf[i].misc10,f))
            {
                new_return(47);
            }
            
            if(!p_putc(itemsbuf[i].usesound,f))
            {
                new_return(48);
            }
	    
	    //New itemdata vars -Z
	    //! version 27
	    
	    if(!p_putc(itemsbuf[i].useweapon,f))
            {
                new_return(49);
            }
	    if(!p_putc(itemsbuf[i].usedefence,f))
            {
                new_return(50);
            }
	    if(!p_iputl(itemsbuf[i].weaprange,f))
            {
                new_return(51);
            }
	    if(!p_iputl(itemsbuf[i].weapduration,f))
            {
                new_return(52);
            }
	    for ( int q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ ) {
		    if(!p_iputl(itemsbuf[i].weap_pattern[q],f))
		    {
			new_return(53);
		    }
	    }
	    //version 28
		if(!p_iputl(itemsbuf[i].duplicates,f))
		{
		    new_return(54);
		}
		for ( int q = 0; q < INITIAL_D; q++ )
		{
			if(!p_iputl(itemsbuf[i].weap_initiald[q],f))
			{
				new_return(55);
			}
		}
		for ( int q = 0; q < INITIAL_A; q++ )
		{
			if(!p_putc(itemsbuf[i].weap_initiala[q],f))
			{
				new_return(56);
			}
		}

		if(!p_putc(itemsbuf[i].drawlayer,f))
		{
		    new_return(57);
		}


		if(!p_iputl(itemsbuf[i].hxofs,f))
		{
		    new_return(58);
		}
		if(!p_iputl(itemsbuf[i].hyofs,f))
		{
		    new_return(59);
		}
		if(!p_iputl(itemsbuf[i].hxsz,f))
		{
		    new_return(60);
		}
		if(!p_iputl(itemsbuf[i].hysz,f))
		{
		    new_return(61);
		}
		if(!p_iputl(itemsbuf[i].hzsz,f))
		{
		    new_return(62);
		}
		if(!p_iputl(itemsbuf[i].xofs,f))
		{
		    new_return(63);
		}
		if(!p_iputl(itemsbuf[i].yofs,f))
		{
		    new_return(64);
		}
		if(!p_iputl(itemsbuf[i].weap_hxofs,f))
		{
		    new_return(65);
		}
		if(!p_iputl(itemsbuf[i].weap_hyofs,f))
		{
		    new_return(66);
		}
		if(!p_iputl(itemsbuf[i].weap_hxsz,f))
		{
		    new_return(67);
		}
		if(!p_iputl(itemsbuf[i].weap_hysz,f))
		{
		    new_return(68);
		}
		if(!p_iputl(itemsbuf[i].weap_hzsz,f))
		{
		    new_return(69);
		}
		if(!p_iputl(itemsbuf[i].weap_xofs,f))
		{
		    new_return(70);
		}
		if(!p_iputl(itemsbuf[i].weap_yofs,f))
		{
		    new_return(71);
		}
		if(!p_iputw(itemsbuf[i].weaponscript,f))
		{
		    new_return(72);
		}
		if(!p_iputl(itemsbuf[i].wpnsprite,f))
		{
		    new_return(73);
		}
		if(!p_iputl(itemsbuf[i].magiccosttimer,f))
		{
		    new_return(74);
		}
		if(!p_iputl(itemsbuf[i].overrideFLAGS,f))
		{
		    new_return(75);
		}
		if(!p_iputl(itemsbuf[i].tilew,f))
		{
		    new_return(76);
		}
		if(!p_iputl(itemsbuf[i].tileh,f))
		{
		    new_return(77);
		}
		if(!p_iputl(itemsbuf[i].weapoverrideFLAGS,f))
		{
		    new_return(78);
		}
		if(!p_iputl(itemsbuf[i].weap_tilew,f))
		{
		    new_return(79);
		}
		if(!p_iputl(itemsbuf[i].weap_tileh,f))
		{
		    new_return(80);
		}
		if(!p_iputl(itemsbuf[i].pickup,f))
		{
		    new_return(81);
		}
		if(!p_iputw(itemsbuf[i].pstring,f))
		{
		    new_return(82);
		}
		if(!p_iputw(itemsbuf[i].pickup_string_flags,f))
		{
		    new_return(83);
		}
		
		if(!p_putc(itemsbuf[i].cost_counter,f))
		{
		    new_return(84);
		}
		
		
	    
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeitems()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writeweapons(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_WEAPONS;
    dword section_version=V_WEAPONS;
    dword section_cversion=CV_WEAPONS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(wMAX,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<wMAX; i++)
        {
            if(!pfwrite((char *)weapon_string[i], 64, f))
            {
                new_return(5);
            }
        }
        
        for(int i=0; i<wMAX; i++)
        {
            if(!p_iputw(wpnsbuf[i].tile,f))
            {
                new_return(6);
            }
            
            if(!p_putc(wpnsbuf[i].misc,f))
            {
                new_return(7);
            }
            
            if(!p_putc(wpnsbuf[i].csets,f))
            {
                new_return(8);
            }
            
            if(!p_putc(wpnsbuf[i].frames,f))
            {
                new_return(9);
            }
            
            if(!p_putc(wpnsbuf[i].speed,f))
            {
                new_return(10);
            }
            
            if(!p_putc(wpnsbuf[i].type,f))
            {
                new_return(11);
            }
	    
	    if(!p_iputw(wpnsbuf[i].script,f))
            {
                new_return(12);
            }
	    
	    if(!p_iputl(wpnsbuf[i].newtile,f))
            {
                new_return(12);
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeweapons()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writemapscreen(PACKFILE *f, int i, int j)
{
    if((i*MAPSCRS+j)>=int(TheMaps.size()))
    {
        return qe_invalid;
    }
    
    mapscr& screen=TheMaps.at(i*MAPSCRS+j);
    
    if(!p_putc(screen.valid,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.guy,f))
    {
        return qe_invalid;
    }
    
    {
        if(!p_iputw(screen.str,f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_putc(screen.room,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.item,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.hasitem, f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.tilewarptype[k],f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_iputw(screen.door_combo_set,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.warpreturnx[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.warpreturny[k],f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_iputw(screen.warpreturnc,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.stairx,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.stairy,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.itemx,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.itemy,f))
    {
        return qe_invalid;
    }
    
    if(!p_iputw(screen.color,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.enemyflags,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.door[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_iputw(screen.tilewarpdmap[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.tilewarpscr[k],f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_putc(screen.tilewarpoverlayflags,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.exitdir,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<10; k++)
    {
        {
            if(!p_iputw(screen.enemy[k],f))
            {
                return qe_invalid;
            }
        }
    }
    
    if(!p_putc(screen.pattern,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.sidewarptype[k],f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_putc(screen.sidewarpoverlayflags,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.warparrivalx,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.warparrivaly,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.path[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_putc(screen.sidewarpscr[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<4; k++)
    {
        if(!p_iputw(screen.sidewarpdmap[k],f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_putc(screen.sidewarpindex,f))
    {
        return qe_invalid;
    }
    
    if(!p_iputw(screen.undercombo,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.undercset,f))
    {
        return qe_invalid;
    }
    
    if(!p_iputw(screen.catchall,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags2,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags3,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags4,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags5,f))
    {
        return qe_invalid;
    }
    
    if(!p_iputw(screen.noreset,f))
    {
        return qe_invalid;
    }
    
    if(!p_iputw(screen.nocarry,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags6,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags7,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags8,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags9,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.flags10,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.csensitive,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.oceansfx,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.bosssfx,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.secretsfx,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.holdupsfx,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<6; k++)
    {
        if(!p_putc(screen.layermap[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<6; k++)
    {
        if(!p_putc(screen.layerscreen[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<6; k++)
    {
        if(!p_putc(screen.layeropacity[k],f))
        {
            return qe_invalid;
        }
    }
    
    if(!p_iputw(screen.timedwarptics,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.nextmap,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.nextscr,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<128; k++)
    {
        if(!p_iputw(screen.secretcombo[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<128; k++)
    {
        if(!p_putc(screen.secretcset[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<128; k++)
    {
        if(!p_putc(screen.secretflag[k],f))
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<(ZCMaps[i].tileWidth)*(ZCMaps[i].tileHeight); k++)
    {
        try
        {
            if(!p_iputw(screen.data.at(k),f))
            {
                return qe_invalid;
            }
        }
        catch(std::out_of_range& e)
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<(ZCMaps[i].tileWidth)*(ZCMaps[i].tileHeight); k++)
    {
        try
        {
            if(!p_putc(screen.sflag.at(k),f))
            {
                return qe_invalid;
            }
        }
        catch(std::out_of_range& e)
        {
            return qe_invalid;
        }
    }
    
    for(int k=0; k<(ZCMaps[i].tileWidth)*(ZCMaps[i].tileHeight); k++)
    {
        try
        {
            if(!p_putc(screen.cset.at(k),f))
            {
                return qe_invalid;
            }
        }
        catch(std::out_of_range& e)
        {
            return qe_invalid;
        }
    }
    
    if(!p_iputw(screen.screen_midi,f))
    {
        return qe_invalid;
    }
    
    if(!p_putc(screen.lens_layer,f))
    {
        return qe_invalid;
    }
    
    if(!p_iputl(screen.numff,f))
    {
        return qe_invalid;
    }
    
    for(int k=0; k<32; k++)
    {
        if((screen.numff>>k)&1)
        {
            if(!p_iputw(screen.ffdata[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(screen.ffcset[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(screen.ffdelay[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffx[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffy[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffxdelta[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffydelta[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffxdelta2[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffydelta2[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(screen.fflink[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(screen.ffwidth[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(screen.ffheight[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.ffflags[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(screen.ffscript[k],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][0],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][1],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][2],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][3],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][4],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][5],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][6],f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(screen.initd[k][7],f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(screen.inita[k][0]/10000,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(screen.inita[k][1]/10000,f))
            {
                return qe_invalid;
            }
        }
    }
    
    for ( int q = 0; q < 10; q++ ) 
    {
	if(!p_iputl(screen.npcstrings[q],f))
	{
		return qe_invalid;
	} 
    }
    for ( int q = 0; q < 10; q++ ) 
    {
	if(!p_iputw(screen.new_items[q],f))
	{
		return qe_invalid;
	} 
    }
    for ( int q = 0; q < 10; q++ ) 
    {
	if(!p_iputw(screen.new_item_x[q],f))
	{
		return qe_invalid;
	} 
    }
    for ( int q = 0; q < 10; q++ ) 
    {
	if(!p_iputw(screen.new_item_y[q],f))
	{
		return qe_invalid;
	} 
    }
    
    
    return qe_OK;
}

int writemaps(PACKFILE *f, zquestheader *)
{
    dword section_id=ID_MAPS;
    dword section_version=V_MAPS;
    dword section_cversion=CV_MAPS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(map_count,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<map_count && i<MAXMAPS2; i++)
        {
            for(int j=0; j<MAPSCRS; j++)
                writemapscreen(f,i,j);
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writemaps()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writecombos(PACKFILE *f, word version, word build, word start_combo, word max_combos)
{
    //these are here to bypass compiler warnings about unused arguments
    version=version;
    build=build;
    
    word combos_used;
    dword section_id=ID_COMBOS;
    dword section_version=V_COMBOS;
    dword section_cversion=CV_COMBOS;
    //  dword section_size=0;
    combos_used = count_combos()-start_combo;
    combos_used = zc_min(combos_used, max_combos);
    combos_used = zc_min(combos_used, MAXCOMBOS);
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        combos_used=count_combos()-start_combo;
        combos_used=zc_min(combos_used, max_combos);
        combos_used=zc_min(combos_used, MAXCOMBOS);
        
        if(!p_iputw(combos_used,f))
        {
            new_return(5);
        }
        
        for(int i=start_combo; i<start_combo+combos_used; i++)
        {
            if(!p_iputl(combobuf[i].tile,f))
            {
                new_return(6);
            }
            
            if(!p_putc(combobuf[i].flip,f))
            {
                new_return(7);
            }
            
            if(!p_putc(combobuf[i].walk,f))
            {
                new_return(8);
            }
            
            if(!p_putc(combobuf[i].type,f))
            {
                new_return(9);
            }
            
            if(!p_putc(combobuf[i].csets,f))
            {
                new_return(10);
            }
            
            if(!p_putc(combobuf[i].frames,f))
            {
                new_return(11);
            }
            
            if(!p_putc(combobuf[i].speed,f))
            {
                new_return(12);
            }
            
            if(!p_iputw(combobuf[i].nextcombo,f))
            {
                new_return(13);
            }
            
            if(!p_putc(combobuf[i].nextcset,f))
            {
                new_return(14);
            }
            
            if(!p_putc(combobuf[i].flag,f))
            {
                new_return(15);
            }
            
            if(!p_putc(combobuf[i].skipanim,f))
            {
                new_return(16);
            }
            
            if(!p_iputw(combobuf[i].nexttimer,f))
            {
                new_return(17);
            }
            
            if(!p_putc(combobuf[i].skipanimy,f))
            {
                new_return(18);
            }
            
            if(!p_putc(combobuf[i].animflags,f))
            {
                new_return(19);
            }
	    
	    for ( int q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
	    {
		if(!p_iputl(combobuf[i].attributes[q],f))
		{
			new_return(20);
		}
	    }
	    if(!p_iputl(combobuf[i].usrflags,f))
	    {
			new_return(21);
	    }	 
	    for ( int q = 0; q < 3; q++ ) 
	    {
	        if(!p_iputl(combobuf[i].triggerflags[q],f))
	        {
			new_return(22);
	        }
	    }
	   
	    if(!p_iputl(combobuf[i].triggerlevel,f))
	    {
			new_return(23);
	    }		    
		    
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writecombos()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writecomboaliases(PACKFILE *f, word version, word build)
{
    //these are here to bypass compiler warnings about unused arguments
    version=version;
    build=build;
    
    dword section_id=ID_COMBOALIASES;
    dword section_version=V_COMBOALIASES;
    dword section_cversion=CV_COMBOALIASES;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        for(int j=0; j<MAXCOMBOALIASES; j++)
        {
            if(!p_iputw(combo_aliases[j].combo,f))
            {
                new_return(5);
            }
            
            if(!p_putc(combo_aliases[j].cset,f))
            {
                new_return(6);
            }
            
            int count = ((combo_aliases[j].width+1)*(combo_aliases[j].height+1))*(comboa_lmasktotal(combo_aliases[j].layermask)+1);
            
            if(!p_putc(combo_aliases[j].width,f))
            {
                new_return(7);
            }
            
            if(!p_putc(combo_aliases[j].height,f))
            {
                new_return(8);
            }
            
            if(!p_putc(combo_aliases[j].layermask,f))
            {
                new_return(9);
            }
            
            for(int k=0; k<count; k++)
            {
                if(!p_iputw(combo_aliases[j].combos[k],f))
                {
                    new_return(10);
                }
            }
            
            for(int k=0; k<count; k++)
            {
                if(!p_putc(combo_aliases[j].csets[k],f))
                {
                    new_return(11);
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writecomboaliases()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writecolordata(PACKFILE *f, miscQdata *Misc, word version, word build, word start_cset, word max_csets)
{
    //these are here to bypass compiler warnings about unused arguments
    version=version;
    build=build;
    start_cset=start_cset;
    max_csets=max_csets;
    
    dword section_id=ID_CSETS;
    dword section_version=V_CSETS;
    dword section_cversion=CV_CSETS;
    int palcycles = count_palcycles(Misc);
// int palcyccount = count_palcycles(Misc);
    dword section_size = 0;
    
    //section id
    
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!pfwrite(colordata,newerpsTOTAL,f))
        {
            new_return(5);
        }
        
        if(!pfwrite(palnames,MAXLEVELS*PALNAMESIZE,f))
        {
            new_return(6);
        }
        
        if(!p_iputw(palcycles,f))
        {
            new_return(15);
        }
        
        for(int i=0; i<palcycles; i++)
        {
            for(int j=0; j<3; j++)
            {
                if(!p_putc(Misc->cycles[i][j].first,f))
                {
                    new_return(16);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_putc(Misc->cycles[i][j].count,f))
                {
                    new_return(17);
                }
            }
            
            for(int j=0; j<3; j++)
            {
                if(!p_putc(Misc->cycles[i][j].speed,f))
                {
                    new_return(18);
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writecolordata()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writestrings(PACKFILE *f, word version, word build, word start_msgstr, word max_msgstrs)
{
    //these are here to bypass compiler warnings about unused arguments
    version=version;
    build=build;
    start_msgstr=start_msgstr;
    max_msgstrs=max_msgstrs;
    
    dword section_id=ID_STRINGS;
    dword section_version=V_STRINGS;
    dword section_cversion=CV_STRINGS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(msg_count,f))
        {
            return qe_invalid;
        }
        
        for(int i=0; i<msg_count; i++)
        {
            if(!pfwrite(MsgStrings[i].s,sizeof(MsgStrings[i].s),f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(MsgStrings[i].nextstring,f))
            {
                return qe_invalid;
            }
            
            if(!p_iputl(MsgStrings[i].tile,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].cset,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].trans?1:0,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].font,f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(MsgStrings[i].x,f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(MsgStrings[i].y,f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(MsgStrings[i].w,f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(MsgStrings[i].h,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].hspace,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].vspace,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].stringflags,f))
            {
                return qe_invalid;
            }
            
            if(!p_putc(MsgStrings[i].sfx,f))
            {
                return qe_invalid;
            }
            
            if(!p_iputw(MsgStrings[i].listpos,f))
            {
                return qe_invalid;
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writestrings()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writestrings_text(PACKFILE *f)
{
    std::map<int, int> msglistcache;
    
    for(int index = 1; index<msg_count; index++)
    {
        for(int i=1; i<msg_count; i++)
        {
            if(MsgStrings[i].listpos==index)
            {
                msglistcache[index-1]=i;
                break;
            }
        }
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        char ebuf[32];
        
        sprintf(ebuf,"Total strings: %d\n", msg_count-1);
        
        if(!pfwrite(&ebuf,(long)strlen(ebuf),f))
        {
            return qe_invalid;
        }
        
        for(int i=1; i<msg_count; i++)
        {
            int str = msglistcache[i-1];
            
            if(!str)
                continue;
                
            if(MsgStrings[str].nextstring != 0)
                sprintf(ebuf,"\n\n___%d(->%d)___\n", str,MsgStrings[str].nextstring);
            else
                sprintf(ebuf,"\n\n___%d___\n", str);
                
            if(!pfwrite(&ebuf,(long)strlen(ebuf),f))
            {
                return qe_invalid;
            }
            
            encode_msg_str(str);
            
            if(!pfwrite(&msgbuf,(int)strlen(msgbuf),f))
            {
                return qe_invalid;
            }
        }
    }
    
    new_return(0);
}


int writetiles(PACKFILE *f, word version, word build, int start_tile, int max_tiles)
{
    //these are here to bypass compiler warnings about unused arguments
    version=version;
    build=build;
    
    int tiles_used;
    dword section_id=ID_TILES;
    dword section_version=V_TILES;
    dword section_cversion=CV_TILES;
	al_trace("Counting tiles used\n");
    tiles_used = count_tiles(newtilebuf)-start_tile;
    tiles_used = zc_min(tiles_used, max_tiles);
    tiles_used = zc_min(tiles_used, NEWMAXTILES);
	al_trace("writetiles counted %dtiles used.\n",tiles_used); 
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        tiles_used=count_tiles(newtilebuf)-start_tile;
        tiles_used=zc_min(tiles_used, max_tiles);
        tiles_used=zc_min(tiles_used, NEWMAXTILES);
        
        if(!p_iputl(tiles_used,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<tiles_used; ++i)
        //for(int i=0; i<NEWMAXTILES; ++i)
        {
            if(!p_putc(newtilebuf[start_tile+i].format,f))
            {
                new_return(6);
            }
            
            if(!pfwrite(newtilebuf[start_tile+i].data,tilesize(newtilebuf[start_tile+i].format),f))
            {
                new_return(7);
            }
	    /*
            if(!p_putc(newtilebuf[start_tile+i].format,f))
            {
                new_return(6);
            }
            
            if(!pfwrite(newtilebuf[start_tile+i].data,tilesize(newtilebuf[start_tile+i].format),f))
            {
                new_return(7);
            }
	    */
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writetiles()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

/* MIDI Format
section_id			LONG
section_version		WORD
section_cversion	WORD
section_size		LONG
midi_flags			32 Byte ? BITFIELD[252]

[
title		36
start 		 4
loop_start	 4
loop_end	 4
loop		 2
volume		 2
midi		 *
]

*/

int writemidis(PACKFILE *f)
{
    dword section_id=ID_MIDIS;
    dword section_version=V_MIDIS;
    dword section_cversion=CV_MIDIS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!pfwrite(midi_flags,sizeof(midi_flags),f))
        {
            new_return(5);
        }
        
        for(int i=0; i<MAXCUSTOMMIDIS; i++)
        {
            if(get_bit(midi_flags,i))
            {
                if(!pfwrite(&customtunes[i].title,sizeof(customtunes[0].title),f))
                {
                    new_return(6);
                }
                
                if(!p_iputl(customtunes[i].start,f))
                {
                    new_return(7);
                }
                
                if(!p_iputl(customtunes[i].loop_start,f))
                {
                    new_return(8);
                }
                
                if(!p_iputl(customtunes[i].loop_end,f))
                {
                    new_return(9);
                }
                
                if(!p_iputw(customtunes[i].loop,f))
                {
                    new_return(10);
                }
                
                if(!p_iputw(customtunes[i].volume,f))
                {
                    new_return(11);
                }
                
                if(!pfwrite(&customtunes[i].flags, sizeof(customtunes[i].flags),f))
                {
                    new_return(12);
                }
                
                if(!pfwrite(&customtunes[i].format, sizeof(customtunes[i].format),f))
                {
                    new_return(13);
                }
                
                switch(customtunes[i].format)
                {
                case MFORMAT_MIDI:
                    if(!write_midi((MIDI*) customtunes[i].data,f)) new_return(14);
                    
                    break;
                    
                default:
                    new_return(15);
                    break;
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writemidis()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writecheats(PACKFILE *f, zquestheader *Header)
{
    dword section_id=ID_CHEATS;
    dword section_version=V_CHEATS;
    dword section_cversion=CV_CHEATS;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_putc(Header->data_flags[ZQ_CHEATS2],f))
        {
            new_return(5);
        }
        
        if(Header->data_flags[ZQ_CHEATS2])
        {
            if(!p_iputl(zcheats.flags,f))
            {
                new_return(6);
            }
            
            if(!pfwrite(&zcheats.codes, sizeof(zcheats.codes), f))
            {
                new_return(7);
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writecheats()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writeguys(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_GUYS;
    dword section_version=V_GUYS;
    dword section_cversion=CV_GUYS;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        for(int i=0; i<MAXGUYS; i++)
        {
            if(!pfwrite((char *)guy_string[i], 64, f))
            {
                new_return(5);
            }
        }
        
        for(int i=0; i<MAXGUYS; i++)
        {
            if(!p_iputl(guysbuf[i].flags,f))
            {
                new_return(6);
            }
            
            if(!p_iputl(guysbuf[i].flags2,f))
            {
                new_return(7);
            }
            
            if(!p_iputl(guysbuf[i].tile,f))
            {
                new_return(8);
            }
            
            if(!p_putc(guysbuf[i].width,f))
            {
                new_return(9);
            }
            
            if(!p_putc(guysbuf[i].height,f))
            {
                new_return(10);
            }
            
            if(!p_iputl(guysbuf[i].s_tile,f))
            {
                new_return(11);
            }
            
            if(!p_putc(guysbuf[i].s_width,f))
            {
                new_return(12);
            }
            
            if(!p_putc(guysbuf[i].s_height,f))
            {
                new_return(13);
            }
            
            if(!p_iputl(guysbuf[i].e_tile,f))
            {
                new_return(14);
            }
            
            if(!p_putc(guysbuf[i].e_width,f))
            {
                new_return(15);
            }
            
            if(!p_putc(guysbuf[i].e_height,f))
            {
                new_return(16);
            }
            
            if(!p_iputw(guysbuf[i].hp,f))
            {
                new_return(17);
            }
            
            if(!p_iputw(guysbuf[i].family,f))
            {
                new_return(18);
            }
            
            if(!p_iputw(guysbuf[i].cset,f))
            {
                new_return(19);
            }
            
            if(!p_iputw(guysbuf[i].anim,f))
            {
                new_return(20);
            }
            
            if(!p_iputw(guysbuf[i].e_anim,f))
            {
                new_return(21);
            }
            
            if(!p_iputw(guysbuf[i].frate,f))
            {
                new_return(22);
            }
            
            if(!p_iputw(guysbuf[i].e_frate,f))
            {
                new_return(23);
            }
            
            if(!p_iputw(guysbuf[i].dp,f))
            {
                new_return(24);
            }
            
            if(!p_iputw(guysbuf[i].wdp,f))
            {
                new_return(25);
            }
            
            if(!p_iputw(guysbuf[i].weapon,f))
            {
                new_return(26);
            }
            
            if(!p_iputw(guysbuf[i].rate,f))
            {
                new_return(27);
            }
            
            if(!p_iputw(guysbuf[i].hrate,f))
            {
                new_return(28);
            }
            
            if(!p_iputw(guysbuf[i].step,f))
            {
                new_return(29);
            }
            
            if(!p_iputw(guysbuf[i].homing,f))
            {
                new_return(30);
            }
            
            if(!p_iputw(guysbuf[i].grumble,f))
            {
                new_return(31);
            }
            
            if(!p_iputw(guysbuf[i].item_set,f))
            {
                new_return(32);
            }
            
            if(!p_iputl(guysbuf[i].misc1,f))
            {
                new_return(33);
            }
            
            if(!p_iputl(guysbuf[i].misc2,f))
            {
                new_return(34);
            }
            
            if(!p_iputl(guysbuf[i].misc3,f))
            {
                new_return(35);
            }
            
            if(!p_iputl(guysbuf[i].misc4,f))
            {
                new_return(36);
            }
            
            if(!p_iputl(guysbuf[i].misc5,f))
            {
                new_return(37);
            }
            
            if(!p_iputl(guysbuf[i].misc6,f))
            {
                new_return(38);
            }
            
            if(!p_iputl(guysbuf[i].misc7,f))
            {
                new_return(39);
            }
            
            if(!p_iputl(guysbuf[i].misc8,f))
            {
                new_return(40);
            }
            
            if(!p_iputl(guysbuf[i].misc9,f))
            {
                new_return(41);
            }
            
            if(!p_iputl(guysbuf[i].misc10,f))
            {
                new_return(42);
            }
            
            if(!p_iputw(guysbuf[i].bgsfx,f))
            {
                new_return(43);
            }
            
            if(!p_iputw(guysbuf[i].bosspal,f))
            {
                new_return(44);
            }
            
            if(!p_iputw(guysbuf[i].extend,f))
            {
                new_return(45);
            }
            
            for(int j=0; j < edefLAST; j++)
            {
                if(!p_putc(guysbuf[i].defense[j],f))
                {
                    new_return(46);
                }
            }
            
            if(!p_putc(guysbuf[i].hitsfx,f))
            {
                new_return(47);
            }
            
            if(!p_putc(guysbuf[i].deadsfx,f))
            {
                new_return(48);
            }
            
            if(!p_iputl(guysbuf[i].misc11,f))
            {
                new_return(49);
            }
            
            if(!p_iputl(guysbuf[i].misc12,f))
            {
                new_return(50);
            }
	    
	    //New 2.6 defences
	    for(int j=edefLAST; j < edefLAST255; j++)
            {
                if(!p_putc(guysbuf[i].defense[j],f))
                {
                    new_return(51);
                }
            }
	    
	    //tilewidth, tileheight, hitwidth, hitheight, hitzheight, hitxofs, hityofs, hitzofs
	    if(!p_iputl(guysbuf[i].txsz,f))
            {
                new_return(52);
            }
	    if(!p_iputl(guysbuf[i].tysz,f))
            {
                new_return(53);
            }
	    if(!p_iputl(guysbuf[i].hxsz,f))
            {
                new_return(54);
            }
	    if(!p_iputl(guysbuf[i].hysz,f))
            {
                new_return(55);
            }
	    if(!p_iputl(guysbuf[i].hzsz,f))
            {
                new_return(56);
            }
	    // These are not fixed types, but ints, so they are safe to use here. 
	    if(!p_iputl(guysbuf[i].hxofs,f))
            {
                new_return(57);
            }
	    if(!p_iputl(guysbuf[i].hyofs,f))
            {
                new_return(58);
            }
	    if(!p_iputl(guysbuf[i].xofs,f))
            {
                new_return(59);
            }
	    if(!p_iputl(guysbuf[i].yofs,f))
            {
                new_return(60);
            }
	    if(!p_iputl(guysbuf[i].zofs,f))
            {
                new_return(61);
            }
	    if(!p_iputl(guysbuf[i].wpnsprite,f))
            {
                new_return(62);
            }
	    if(!p_iputl(guysbuf[i].SIZEflags,f))
            {
                new_return(63);
            }
	    if(!p_iputl(guysbuf[i].frozentile,f))
            {
                new_return(64);
            }
	    if(!p_iputl(guysbuf[i].frozencset,f))
            {
                new_return(65);
            }
	    if(!p_iputl(guysbuf[i].frozenclock,f))
            {
                new_return(66);
            }
	    
	    for ( int q = 0; q < 10; q++ ) 
	    {
		if(!p_iputw(guysbuf[i].frozenmisc[q],f))
		{
			new_return(67);
		}
	    }
	    if(!p_iputw(guysbuf[i].firesfx,f))
            {
                new_return(68);
            }
	    //misc 16->31
	    if(!p_iputl(guysbuf[i].misc16,f))
            {
                new_return(69);
            }
	    if(!p_iputl(guysbuf[i].misc17,f))
            {
                new_return(70);
            }
	    if(!p_iputl(guysbuf[i].misc18,f))
            {
                new_return(71);
            }
	    if(!p_iputl(guysbuf[i].misc19,f))
            {
                new_return(72);
            }
	    if(!p_iputl(guysbuf[i].misc20,f))
            {
                new_return(73);
            }
	    if(!p_iputl(guysbuf[i].misc21,f))
            {
                new_return(74);
            }
	    if(!p_iputl(guysbuf[i].misc22,f))
            {
                new_return(75);
            }
	    if(!p_iputl(guysbuf[i].misc23,f))
            {
                new_return(76);
            }
	    if(!p_iputl(guysbuf[i].misc24,f))
            {
                new_return(77);
            }
	    if(!p_iputl(guysbuf[i].misc25,f))
            {
                new_return(78);
            }
	    if(!p_iputl(guysbuf[i].misc26,f))
            {
                new_return(79);
            }
	    if(!p_iputl(guysbuf[i].misc27,f))
            {
                new_return(80);
            }
	    if(!p_iputl(guysbuf[i].misc28,f))
            {
                new_return(81);
            }
	    if(!p_iputl(guysbuf[i].misc29,f))
            {
                new_return(82);
            }
	    if(!p_iputl(guysbuf[i].misc30,f))
            {
                new_return(83);
            }
	    if(!p_iputl(guysbuf[i].misc31,f))
            {
                new_return(84);
            }
	    if(!p_iputl(guysbuf[i].misc32,f))
            {
                new_return(85);
            }
	    for ( int q = 0; q < 32; q++ )
	    {
		    if(!p_iputl(guysbuf[i].movement[q],f))
		    {
			new_return(86);
		    }
	    }
	    for ( int q = 0; q < 32; q++ )
	    {
		    if(!p_iputl(guysbuf[i].new_weapon[q],f))
		    {
			new_return(87);
		    }
	    }
	    if(!p_iputw(guysbuf[i].npcscript,f))
            {
                new_return(88);
            }
	    for ( int q = 0; q < 8; q++ )
	    {
		if(!p_iputl(guysbuf[i].initD[q],f))
		{
			new_return(89);
		}
	    }
	    for ( int q = 0; q < 2; q++ )
	    {
		if(!p_iputl(guysbuf[i].initA[q],f))
		{
			new_return(90);
		}
	    }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeguys()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writelinksprites(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_LINKSPRITES;
    dword section_version=V_LINKSPRITES;
    dword section_cversion=CV_LINKSPRITES;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)walkspr[i][spr_tile],f))
            {
                new_return(5);
            }
            
            if(!p_putc((byte)walkspr[i][spr_flip],f))
            {
                new_return(5);
            }
            
            if(!p_putc((byte)walkspr[i][spr_extend],f))
            {
                new_return(5);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)stabspr[i][spr_tile],f))
            {
                new_return(6);
            }
            
            if(!p_putc((byte)stabspr[i][spr_flip],f))
            {
                new_return(6);
            }
            
            if(!p_putc((byte)stabspr[i][spr_extend],f))
            {
                new_return(6);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)slashspr[i][spr_tile],f))
            {
                new_return(7);
            }
            
            if(!p_putc((byte)slashspr[i][spr_flip],f))
            {
                new_return(7);
            }
            
            if(!p_putc((byte)slashspr[i][spr_extend],f))
            {
                new_return(7);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)floatspr[i][spr_tile],f))
            {
                new_return(8);
            }
            
            if(!p_putc((byte)floatspr[i][spr_flip],f))
            {
                new_return(8);
            }
            
            if(!p_putc((byte)floatspr[i][spr_extend],f))
            {
                new_return(8);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)swimspr[i][spr_tile],f))
            {
                new_return(8);
            }
            
            if(!p_putc((byte)swimspr[i][spr_flip],f))
            {
                new_return(8);
            }
            
            if(!p_putc((byte)swimspr[i][spr_extend],f))
            {
                new_return(8);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)divespr[i][spr_tile],f))
            {
                new_return(9);
            }
            
            if(!p_putc((byte)divespr[i][spr_flip],f))
            {
                new_return(9);
            }
            
            if(!p_putc((byte)divespr[i][spr_extend],f))
            {
                new_return(9);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)poundspr[i][spr_tile],f))
            {
                new_return(10);
            }
            
            if(!p_putc((byte)poundspr[i][spr_flip],f))
            {
                new_return(10);
            }
            
            if(!p_putc((byte)poundspr[i][spr_extend],f))
            {
                new_return(10);
            }
        }
        
        if(!p_iputw((word)castingspr[spr_tile],f))
        {
            new_return(11);
        }
        
        if(!p_putc((byte)castingspr[spr_flip],f))
        {
            new_return(11);
        }
        
        if(!p_putc((byte)castingspr[spr_extend],f))
        {
            new_return(11);
        }
        
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<2; j++)
            {
                if(!p_iputw((word)holdspr[i][j][spr_tile],f))
                {
                    new_return(12);
                }
                
                if(!p_putc((byte)holdspr[i][j][spr_flip],f))
                {
                    new_return(12);
                }
                
                if(!p_putc((byte)holdspr[i][j][spr_extend],f))
                {
                    new_return(12);
                }
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)jumpspr[i][spr_tile],f))
            {
                new_return(13);
            }
            
            if(!p_putc((byte)jumpspr[i][spr_flip],f))
            {
                new_return(13);
            }
            
            if(!p_putc((byte)jumpspr[i][spr_extend],f))
            {
                new_return(13);
            }
        }
        
        for(int i=0; i<4; i++)
        {
            if(!p_iputw((word)chargespr[i][spr_tile],f))
            {
                new_return(13);
            }
            
            if(!p_putc((byte)chargespr[i][spr_flip],f))
            {
                new_return(13);
            }
            
            if(!p_putc((byte)chargespr[i][spr_extend],f))
            {
                new_return(13);
            }
        }
        
        if(!p_putc((byte)zinit.link_swim_speed,f))
        {
            new_return(14);
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    //More data will come here
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writelinksprites()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writesubscreens(PACKFILE *f, zquestheader *Header)
{
    dword section_id=ID_SUBSCREEN;
    dword section_version=V_SUBSCREEN;
    dword section_cversion=CV_SUBSCREEN;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        for(int i=0; i<MAXCUSTOMSUBSCREENS; i++)
        {
            int ret = write_one_subscreen(f, Header, i);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writesubscreens()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int write_one_subscreen(PACKFILE *f, zquestheader *Header, int i)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    int numsub = 0;
    
    if(!pfwrite(custom_subscreen[i].name, 64,f))
    {
        new_return(28);
    }
    
    if(!p_putc(custom_subscreen[i].ss_type,f))
    {
        new_return(29);
    }
    
    for(int k=0; (k<MAXSUBSCREENITEMS&&(custom_subscreen[i].objects[k].type != ssoNULL)); k++)
    {
        numsub++;
    }
    
    if(!p_iputw(numsub,f))
    {
        new_return(4);
    }
    
    for(int j=0; (j<MAXSUBSCREENITEMS&&j<numsub); j++)
    {
        if(!p_putc(custom_subscreen[i].objects[j].type, f))
        {
            new_return(5);
        }
        
        if(!p_putc(custom_subscreen[i].objects[j].pos, f))
        {
            new_return(6);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].x, f))
        {
            new_return(7);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].y, f))
        {
            new_return(8);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].w, f))
        {
            new_return(9);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].h, f))
        {
            new_return(10);
        }
        
        if(!p_putc(custom_subscreen[i].objects[j].colortype1, f))
        {
            new_return(11);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].color1, f))
        {
            new_return(12);
        }
        
        if(!p_putc(custom_subscreen[i].objects[j].colortype2, f))
        {
            new_return(13);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].color2, f))
        {
            new_return(14);
        }
        
        if(!p_putc(custom_subscreen[i].objects[j].colortype3, f))
        {
            new_return(15);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].color3, f))
        {
            new_return(16);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d1, f))
        {
            new_return(17);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d2, f))
        {
            new_return(18);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d3, f))
        {
            new_return(19);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d4, f))
        {
            new_return(20);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d5, f))
        {
            new_return(21);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d6, f))
        {
            new_return(22);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d7, f))
        {
            new_return(23);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d8, f))
        {
            new_return(24);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d9, f))
        {
            new_return(25);
        }
        
        if(!p_iputl(custom_subscreen[i].objects[j].d10, f))
        {
            new_return(26);
        }
        
        if(!p_putc(custom_subscreen[i].objects[j].speed, f))
        {
            new_return(27);
        }
        
        if(!p_putc(custom_subscreen[i].objects[j].delay, f))
        {
            new_return(28);
        }
        
        if(!p_iputw(custom_subscreen[i].objects[j].frame, f))
        {
            new_return(29);
        }
        
        switch(custom_subscreen[i].objects[j].type)
        {
        case ssoTEXT:
        case ssoTEXTBOX:
        case ssoCURRENTITEMTEXT:
        case ssoCURRENTITEMCLASSTEXT:
            if(custom_subscreen[i].objects[j].dp1 != NULL)
            {
                if(strlen((char*)custom_subscreen[i].objects[j].dp1))
                {
                    if(!p_iputw((int)strlen((char*)custom_subscreen[i].objects[j].dp1), f))
                    {
                        new_return(27);
                    }
                    
                    if(!pfwrite(custom_subscreen[i].objects[j].dp1, (long)strlen((char*)custom_subscreen[i].objects[j].dp1)+1,f))
                    {
                        new_return(28);
                    }
                }
                else
                {
                    if(!p_iputw(0, f))
                    {
                        new_return(27);
                    }
                }
            }
            else
            {
                if(!p_iputw(0, f))
                {
                    new_return(27);
                }
            }
            
            break;
            
        default:
            if(!p_putc(0, f))
            {
                new_return(27);
            }
        }
    }
    
    new_return(0);
}

extern ffscript *ffscripts[512];
extern ffscript *itemscripts[256];
extern ffscript *guyscripts[256];
extern ffscript *wpnscripts[256];
extern ffscript *globalscripts[NUMSCRIPTGLOBAL];
extern ffscript *linkscripts[3];
extern ffscript *screenscripts[256];

int writeffscript(PACKFILE *f, zquestheader *Header)
{
    dword section_id       = ID_FFSCRIPT;
    dword section_version  = V_FFSCRIPT;
    dword section_cversion = CV_FFSCRIPT;
    dword section_size     = 0;
    byte numscripts        = 0;
    numscripts = numscripts; //to avoid unused variables warnings
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        for(int i=0; i<512; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &ffscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        for(int i=0; i<256; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &itemscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        for(int i=0; i<256; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &guyscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        for(int i=0; i<256; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &wpnscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        for(int i=0; i<256; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &screenscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        for(int i=0; i<NUMSCRIPTGLOBAL; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &globalscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        for(int i=0; i<3; i++)
        {
            int ret = write_one_ffscript(f, Header, i, &linkscripts[i]);
            fake_pack_writing=(writecycle==0);
            
            if(ret!=0)
            {
                new_return(ret);
            }
        }
        
        if(!p_iputl((long)zScript.size(), f))
        {
            new_return(2001);
        }
        
        if(!pfwrite((void *)zScript.c_str(), (long)zScript.size(), f))
        {
            new_return(2002);
        }
        
        word numffcbindings=0;
        
        for(std::map<int, pair<string, string> >::iterator it = ffcmap.begin(); it != ffcmap.end(); it++)
        {
            if(it->second.second != "")
            {
                numffcbindings++;
            }
        }
        
        if(!p_iputw(numffcbindings, f))
        {
            new_return(2003);
        }
        
        for(std::map<int, pair<string, string> >::iterator it = ffcmap.begin(); it != ffcmap.end(); it++)
        {
            if(it->second.second != "")
            {
                if(!p_iputw(it->first,f))
                {
                    new_return(2004);
                }
                
                if(!p_iputl((long)it->second.second.size(), f))
                {
                    new_return(2005);
                }
                
                if(!pfwrite((void *)it->second.second.c_str(), (long)it->second.second.size(),f))
                {
                    new_return(2006);
                }
            }
        }
        
        word numglobalbindings=0;
        
        for(std::map<int, pair<string, string> >::iterator it = globalmap.begin(); it != globalmap.end(); it++)
        {
            if(it->second.second != "")
            {
                numglobalbindings++;
            }
        }
        
        if(!p_iputw(numglobalbindings, f))
        {
            new_return(2007);
        }
        
        for(std::map<int, pair<string, string> >::iterator it = globalmap.begin(); it != globalmap.end(); it++)
        {
            if(it->second.second != "")
            {
                if(!p_iputw(it->first,f))
                {
                    new_return(2008);
                }
                
                if(!p_iputl((long)it->second.second.size(), f))
                {
                    new_return(2009);
                }
                
                if(!pfwrite((void *)it->second.second.c_str(), (long)it->second.second.size(),f))
                {
                    new_return(2010);
                }
            }
        }
        
        word numitembindings=0;
        
        for(std::map<int, pair<string, string> >::iterator it = itemmap.begin(); it != itemmap.end(); it++)
        {
            if(it->second.second != "")
            {
                numitembindings++;
            }
        }
        
        if(!p_iputw(numitembindings, f))
        {
            new_return(2011);
        }
        
        for(std::map<int, pair<string, string> >::iterator it = itemmap.begin(); it != itemmap.end(); it++)
        {
            if(it->second.second != "")
            {
                if(!p_iputw(it->first,f))
                {
                    new_return(2012);
                }
                
                if(!p_iputl((long)it->second.second.size(), f))
                {
                    new_return(2013);
                }
                
                if(!pfwrite((void *)it->second.second.c_str(), (long)it->second.second.size(),f))
                {
                    new_return(2014);
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeffscript()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
    //return 0;  //this is just here to stomp the compiler from whining.
    //the irony is that it causes an "unreachable code" warning.
}

int write_one_ffscript(PACKFILE *f, zquestheader *Header, int i, ffscript **script)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    i=i;
    
    int num_commands;
    
    for(int j=0;; j++)
    {
        if((*script)[j].command==0xFFFF)
        {
            num_commands = j+1;
            break;
        }
    }
    
    if(!p_iputl(num_commands,f))
    {
        new_return(6);
    }
    
    for(int j=0; j<num_commands; j++)
    {
        if(!p_iputw((*script)[j].command,f))
        {
            new_return(7);
        }
        
        if((*script)[j].command==0xFFFF)
        {
            break;
        }
        else
        {
            if(!p_iputl((*script)[j].arg1,f))
            {
                new_return(8);
            }
            
            if(!p_iputl((*script)[j].arg2,f))
            {
                new_return(9);
            }
        }
    }
    
    new_return(0);
}

extern SAMPLE customsfxdata[WAV_COUNT];
extern unsigned char customsfxflag[WAV_COUNT>>3];

int writesfx(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_SFX;
    dword section_version=V_SFX;
    dword section_cversion=CV_SFX;
    dword section_size=0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        for(int i=0; i<WAV_COUNT>>3; i++)
        {
            if(!p_putc(customsfxflag[i],f))
            {
                new_return(5);
            }
        }
        
        for(int i=1; i<WAV_COUNT; i++)
        {
            if(get_bit(customsfxflag, i-1) == 0)
                continue;
                
            if(!pfwrite(sfx_string[i], 36, f))
            {
                new_return(5);
            }
        }
        
        for(int i=1; i<WAV_COUNT; i++)
        {
            if(get_bit(customsfxflag, i-1) == 0)
                continue;
                
            if(!p_iputl(customsfxdata[i].bits,f))
            {
                new_return(5);
            }
            
            if(!p_iputl(customsfxdata[i].stereo,f))
            {
                new_return(6);
            }
            
            if(!p_iputl(customsfxdata[i].freq,f))
            {
                new_return(7);
            }
            
            if(!p_iputl(customsfxdata[i].priority,f))
            {
                new_return(8);
            }
            
            if(!p_iputl(customsfxdata[i].len,f))
            {
                new_return(9);
            }
            
            if(!p_iputl(customsfxdata[i].loop_start,f))
            {
                new_return(10);
            }
            
            if(!p_iputl(customsfxdata[i].loop_end,f))
            {
                new_return(11);
            }
            
            if(!p_iputl(customsfxdata[i].param,f))
            {
                new_return(12);
            }
            
            //de-endianfy the data
            int wordstowrite = (customsfxdata[i].bits==8?1:2)*(customsfxdata[i].stereo==0?1:2)*customsfxdata[i].len/sizeof(word);
            
            for(int j=0; j<wordstowrite; j++)
            {
                if(!p_iputw(((word *)customsfxdata[i].data)[j],f))
                {
                    new_return(13);
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writesfx()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writeinitdata(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_INITDATA;
    dword section_version=V_INITDATA;
    dword section_cversion=CV_INITDATA;
    dword section_size = 0;
    
    zinit.last_map=Map.getCurrMap();
    zinit.last_screen=Map.getCurrScr();
    zinit.usecustomsfx=1;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        //write the new items
        for(int i=0; i<MAXITEMS; i++)
        {
            if(!p_putc(zinit.items[i] ? 1 : 0, f))
            {
                new_return(5);
            }
        }
        
        //bomb counter RANDOMLY in the middle of items :-/
        if(!p_putc(zinit.bombs,f))
        {
            new_return(23);
        }
        
        if(!p_putc(zinit.super_bombs,f))
        {
            new_return(24);
        }
        
        if(!p_putc(zinit.hc,f))
        {
            new_return(25);
        }
        
        if(!p_iputw(zinit.start_heart,f))
        {
            new_return(26);
        }
        
        if(!p_iputw(zinit.cont_heart,f))
        {
            new_return(27);
        }
        
        if(!p_putc(zinit.hcp,f))
        {
            new_return(28);
        }
        
        if(!p_putc(zinit.hcp_per_hc,f))
        {
            new_return(29);
        }
        
        if(!p_putc(zinit.max_bombs,f))
        {
            new_return(30);
        }
        
        if(!p_putc(zinit.keys,f))
        {
            new_return(31);
        }
        
        if(!p_iputw(zinit.rupies,f))
        {
            new_return(32);
        }
        
        if(!p_putc(zinit.triforce,f))
        {
            new_return(33);
        }
        
        for(int i=0; i<64; i++)
        {
            if(!p_putc(zinit.map[i],f))
            {
                new_return(34);
            }
        }
        
        for(int i=0; i<64; i++)
        {
            if(!p_putc(zinit.compass[i],f))
            {
                new_return(35);
            }
        }
        
        for(int i=0; i<64; i++)
        {
            if(!p_putc(zinit.boss_key[i],f))
            {
                new_return(36);
            }
        }
        
        for(int i=0; i<16; i++)
        {
            if(!p_putc(zinit.misc[i],f))
            {
                new_return(37);
            }
        }
        
        if(!p_putc(zinit.last_map,f))
        {
            new_return(38);
        }
        
        if(!p_putc(zinit.last_screen,f))
        {
            new_return(39);
        }
        
        if(!p_iputw(zinit.max_magic,f))
        {
            new_return(40);
        }
        
        if(!p_iputw(zinit.magic,f))
        {
            new_return(41);
        }
        
        if(!p_putc(zinit.bomb_ratio,f))
        {
            new_return(41);
        }
        
        if(!p_putc(zinit.msg_more_x,f))
        {
            new_return(42);
        }
        
        if(!p_putc(zinit.msg_more_y,f))
        {
            new_return(43);
        }
        
        if(!p_putc(zinit.subscreen,f))
        {
            new_return(44);
        }
        
        if(!p_iputw(zinit.start_dmap,f))
        {
            new_return(45);
        }
        
        if(!p_putc(zinit.linkanimationstyle,f))
        {
            new_return(46);
        }
        
        if(!p_putc(zinit.arrows,f))
        {
            new_return(47);
        }
        
        if(!p_putc(zinit.max_arrows,f))
        {
            new_return(48);
        }
        
        for(int i=0; i<MAXLEVELS; i++)
        {
            if(!p_putc(zinit.level_keys[i],f))
            {
                new_return(49);
            }
        }
        
        if(!p_iputw(zinit.ss_grid_x,f))
        {
            new_return(50);
        }
        
        if(!p_iputw(zinit.ss_grid_y,f))
        {
            new_return(51);
        }
        
        if(!p_iputw(zinit.ss_grid_xofs,f))
        {
            new_return(52);
        }
        
        if(!p_iputw(zinit.ss_grid_yofs,f))
        {
            new_return(53);
        }
        
        if(!p_iputw(zinit.ss_grid_color,f))
        {
            new_return(54);
        }
        
        if(!p_iputw(zinit.ss_bbox_1_color,f))
        {
            new_return(55);
        }
        
        if(!p_iputw(zinit.ss_bbox_2_color,f))
        {
            new_return(56);
        }
        
        if(!p_iputw(zinit.ss_flags,f))
        {
            new_return(57);
        }
        
        if(!p_putc(zinit.subscreen_style,f))
        {
            new_return(58);
        }
        
        if(!p_putc(zinit.usecustomsfx,f))
        {
            new_return(59);
        }
        
        if(!p_iputw(zinit.max_rupees,f))
        {
            new_return(60);
        }
        
        if(!p_iputw(zinit.max_keys,f))
        {
            new_return(61);
        }
        
        if(!p_putc(zinit.gravity,f))
        {
            new_return(62);
        }
        
        if(!p_iputw(zinit.terminalv,f))
        {
            new_return(63);
        }
        
        if(!p_putc(zinit.msg_speed,f))
        {
            new_return(64);
        }
        
        if(!p_putc(zinit.transition_type,f))
        {
            new_return(65);
        }
        
        if(!p_putc(zinit.jump_link_layer_threshold,f))
        {
            new_return(66);
        }
        
        if(!p_putc(zinit.msg_more_is_offset,f))
        {
            new_return(67);
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeinitdata()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writeitemdropsets(PACKFILE *f, zquestheader *Header)
{
    //these are here to bypass compiler warnings about unused arguments
    Header=Header;
    
    dword section_id=ID_ITEMDROPSETS;
    dword section_version=V_ITEMDROPSETS;
    dword section_cversion=CV_ITEMDROPSETS;
    //  dword section_size=0;
    dword section_size = 0;
    word num_item_drop_sets=count_item_drop_sets();
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(num_item_drop_sets,f))
        {
            new_return(5);
        }
        
        for(int i=0; i<num_item_drop_sets; i++)
        {
            if(!pfwrite(item_drop_sets[i].name, sizeof(item_drop_sets[i].name), f))
            {
                new_return(6);
            }
            
            for(int j=0; j<10; ++j)
            {
                if(!p_iputw(item_drop_sets[i].item[j],f))
                {
                    new_return(7);
                }
            }
            
            for(int j=0; j<11; ++j)
            {
                if(!p_iputw(item_drop_sets[i].chance[j],f))
                {
                    new_return(8);
                }
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeitemdropsets()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int writefavorites(PACKFILE *f, zquestheader*)
{
    dword section_id=ID_FAVORITES;
    dword section_version=V_FAVORITES;
    dword section_cversion=CV_FAVORITES;
    dword section_size = 0;
    
    //section id
    if(!p_mputl(section_id,f))
    {
        new_return(1);
    }
    
    //section version info
    if(!p_iputw(section_version,f))
    {
        new_return(2);
    }
    
    if(!p_iputw(section_cversion,f))
    {
        new_return(3);
    }
    
    for(int writecycle=0; writecycle<2; ++writecycle)
    {
        fake_pack_writing=(writecycle==0);
        
        //section size
        if(!p_iputl(section_size,f))
        {
            new_return(4);
        }
        
        writesize=0;
        
        //finally...  section data
        if(!p_iputw(MAXFAVORITECOMBOS,f)) // This'll probably never change, huh?
        {
            new_return(5);
        }
        
        for(int i=0; i<MAXFAVORITECOMBOS; i++)
        {
            if(!p_iputl(favorite_combos[i],f))
            {
                new_return(6);
            }
        }
        
        if(!p_iputw(MAXFAVORITECOMBOALIASES,f))
        {
            new_return(7);
        }
        
        for(int i=0; i<MAXFAVORITECOMBOALIASES; i++)
        {
            if(!p_iputl(favorite_comboaliases[i],f))
            {
                new_return(8);
            }
        }
        
        if(writecycle==0)
        {
            section_size=writesize;
        }
    }
    
    if(writesize!=int(section_size) && save_warn)
    {
        char ebuf[80];
        sprintf(ebuf, "%d != %d", writesize, int(section_size));
        jwin_alert("Error:  writeitemdropsets()","writesize != section_size",ebuf,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    new_return(0);
}

int save_unencoded_quest(const char *filename, bool compressed)
{
    reset_combo_animations();
    reset_combo_animations2();
    strcpy(header.id_str,QH_NEWIDSTR);
    header.zelda_version = ZELDA_VERSION;
    header.internal = INTERNAL_VERSION;
    // header.str_count = msg_count;
    // header.data_flags[ZQ_TILES] = usetiles;
    header.data_flags[ZQ_TILES] = true;
    header.data_flags[ZQ_CHEATS2] = 1;
    header.build=VERSION_BUILD;
    
    for(int i=0; i<MAXCUSTOMMIDIS; i++)
    {
        set_bit(midi_flags,i,int(customtunes[i].data!=NULL));
    }
    
    char keyfilename[2048];
    // word combos_used;
    // word tiles_used;
    replace_extension(keyfilename, filepath, "key", 2047);
    
    
    
    
    box_start(1, "Saving Quest", lfont, font, true);
    box_out("Saving Quest...");
    box_eol();
    box_eol();
    
    PACKFILE *f = pack_fopen_password(filename,compressed?F_WRITE_PACKED:F_WRITE, compressed ? datapwd : "");
    
    if(!f)
    {
        fake_pack_writing = false;
        return 1;
    }
    
    box_out("Writing Header...");
    
    if(writeheader(f,&header)!=0)
    {
        new_return(2);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Rules...");
    
    if(writerules(f,&header)!=0)
    {
        new_return(3);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Strings...");
    
    if(writestrings(f, ZELDA_VERSION, VERSION_BUILD, 0, MAXMSGS)!=0)
    {
        new_return(4);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Doors...");
    
    if(writedoorcombosets(f,&header)!=0)
    {
        new_return(5);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing DMaps...");
    
    if(writedmaps(f,header.zelda_version,header.build,0,MAXDMAPS)!=0)
    {
        new_return(6);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Misc. Data...");
    
    if(writemisc(f,&header,&misc)!=0)
    {
        new_return(7);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Misc. Colors...");
    
    if(writemisccolors(f,&header,&misc)!=0)
    {
        new_return(8);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Game Icons...");
    
    if(writegameicons(f,&header,&misc)!=0)
    {
        new_return(9);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Items...");
    
    if(writeitems(f,&header)!=0)
    {
        new_return(10);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Weapons...");
    
    if(writeweapons(f,&header)!=0)
    {
        new_return(11);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Maps...");
    
    if(writemaps(f,&header)!=0)
    {
        new_return(12);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Combos...");
    
    if(writecombos(f,header.zelda_version,header.build,0,MAXCOMBOS)!=0)
    {
        new_return(13);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Combo Aliases...");
    
    if(writecomboaliases(f,header.zelda_version,header.build)!=0)
    {
        new_return(14);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Color Data...");
    
    if(writecolordata(f,&misc,header.zelda_version,header.build,0,newerpdTOTAL)!=0)
    {
        new_return(15);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Tiles...");
    
    if(writetiles(f,header.zelda_version,header.build,0,NEWMAXTILES)!=0)
    {
        new_return(16);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing MIDIs...");
    
    if(writemidis(f)!=0)
    {
        new_return(17);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Cheat Codes...");
    
    if(writecheats(f,&header)!=0)
    {
        new_return(18);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Init. Data...");
    
    if(writeinitdata(f,&header)!=0)
    {
        new_return(19);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Custom Guy Data...");
    
    if(writeguys(f,&header)!=0)
    {
        new_return(20);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Custom Link Sprite Data...");
    
    if(writelinksprites(f,&header)!=0)
    {
        new_return(21);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Custom Subscreen Data...");
    
    if(writesubscreens(f,&header)!=0)
    {
        new_return(22);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing FF Script Data...");
    
    if(writeffscript(f,&header)!=0)
    {
        new_return(23);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing SFX Data...");
    
    if(writesfx(f,&header)!=0)
    {
        new_return(24);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Item Drop Sets...");
    
    if(writeitemdropsets(f, &header)!=0)
    {
        new_return(25);
    }
    
    box_out("okay.");
    box_eol();
    
    box_out("Writing Favorite Combos...");
    
    if(writefavorites(f, &header)!=0)
    {
        new_return(26);
    }
    
    box_out("okay.");
    box_eol();
    
    pack_fclose(f);
    
    replace_extension(keyfilename, get_filename(filepath), "key", 2047);
    
    if(header.use_keyfile&&header.dirty_password)
    {
        PACKFILE *fp = pack_fopen_password(keyfilename, F_WRITE, "");
        char msg[80];
        memset(msg,0,80);
        sprintf(msg, "ZQuest Auto-Generated Quest Password Key File.  DO NOT EDIT!");
        msg[78]=13;
        msg[79]=10;
        pfwrite(msg, 80, fp);
        p_iputw(header.zelda_version,fp);
        p_putc(header.build,fp);
        pfwrite(header.password, 256, fp);
        pack_fclose(fp);
    }
    
    new_return(0);
}

int save_quest(const char *filename, bool timed_save)
{
    int retention=timed_save?AutoSaveRetention:AutoBackupRetention;
    bool compress=!(timed_save&&UncompressedAutoSaves);
    char ext1[5];
    ext1[0]=0;
    
    if(timed_save)
    {
        sprintf(ext1, "qt");
    }
    else
    {
        sprintf(ext1, "qb");
    }
    
    if(retention)
    {
        char backupname[2048];
        char backupname2[2048];
        char ext[5];
        
        for(int i=retention-1; i>0; --i)
        {
            sprintf(ext, "%s%d", ext1, i-1);
            replace_extension(backupname, filepath, ext, 2047);
            
            if(exists(backupname))
            {
                sprintf(ext, "%s%d", ext1, i);
                replace_extension(backupname2, filepath, ext, 2047);
                
                if(exists(backupname2))
                {
                    remove(backupname2);
                }
                
                rename(backupname, backupname2);
            }
        }
        
        //don't do this if we're not saving to the same name -DD
        if(!timed_save && !strcmp(filepath, filename))
        {
            sprintf(ext, "%s%d", ext1, 0);
            replace_extension(backupname, filepath, ext, 2047);
            rename(filepath, backupname);
        }
    }
    
    char *tmpfilename;
    char tempfilestr[32]; // This is stupid...
    
    if(compress)
    {
        temp_name(tempfilestr);
        tmpfilename=tempfilestr;
    }
    else
    {
        tmpfilename=(char *)filename;
    }
    
    int ret;
    ret  = save_unencoded_quest(tmpfilename, compress);
    
    if(compress)
    {
        if(ret == 0)
        {
            box_out("Encrypting...");
            ret = encode_file_007(tmpfilename, filename,((INTERNAL_VERSION + rand()) & 0xffff) + 0x413F0000, ENC_STR, ENC_METHOD_MAX-1);
            
            if(ret)
            {
                ret += 100;
            }
            
            box_out("okay.");
            box_eol();
        }
        
        delete_file(tmpfilename);
    }
    
    return ret;
}

void center_zq_class_dialogs()
{
    jwin_center_dialog(pwd_dlg);
}

void zmap::prv_secrets(bool high16only)
{
    mapscr *s = &prvscr;
    mapscr *t = prvlayers;
    int ft=0;
    
    for(int i=0; i<176; i++)
    {
        bool putit;
        
        if(!high16only)
        {
            for(int j=-1; j<6; j++)
            {
                int newflag = -1;
                
                for(int iter=0; iter<2; ++iter)
                {
                    putit=true;
                    
                    if(t[j].data.empty())
                        continue;
                        
                    int checkflag=combobuf[t[j].data[i]].flag;
                    
                    if(iter==1)
                    {
                        checkflag=t[j].sflag[i];
                    }
                    
                    switch(checkflag)
                    {
                    case mfBCANDLE:
                        ft=sBCANDLE;
                        break;
                        
                    case mfRCANDLE:
                        ft=sRCANDLE;
                        break;
                        
                    case mfWANDFIRE:
                        ft=sWANDFIRE;
                        break;
                        
                    case mfDINSFIRE:
                        ft=sDINSFIRE;
                        break;
                        
                    case mfARROW:
                        ft=sARROW;
                        break;
                        
                    case mfSARROW:
                        ft=sSARROW;
                        break;
                        
                    case mfGARROW:
                        ft=sGARROW;
                        break;
                        
                    case mfSBOMB:
                        ft=sSBOMB;
                        break;
                        
                    case mfBOMB:
                        ft=sBOMB;
                        break;
                        
                    case mfBRANG:
                        ft=sBRANG;
                        break;
                        
                    case mfMBRANG:
                        ft=sMBRANG;
                        break;
                        
                    case mfFBRANG:
                        ft=sFBRANG;
                        break;
                        
                    case mfWANDMAGIC:
                        ft=sWANDMAGIC;
                        break;
                        
                    case mfREFMAGIC:
                        ft=sREFMAGIC;
                        break;
                        
                    case mfREFFIREBALL:
                        ft=sREFFIREBALL;
                        break;
                        
                    case mfSWORD:
                        ft=sSWORD;
                        break;
                        
                    case mfWSWORD:
                        ft=sWSWORD;
                        break;
                        
                    case mfMSWORD:
                        ft=sMSWORD;
                        break;
                        
                    case mfXSWORD:
                        ft=sXSWORD;
                        break;
                        
                    case mfSWORDBEAM:
                        ft=sSWORDBEAM;
                        break;
                        
                    case mfWSWORDBEAM:
                        ft=sWSWORDBEAM;
                        break;
                        
                    case mfMSWORDBEAM:
                        ft=sMSWORDBEAM;
                        break;
                        
                    case mfXSWORDBEAM:
                        ft=sXSWORDBEAM;
                        break;
                        
                    case mfHOOKSHOT:
                        ft=sHOOKSHOT;
                        break;
                        
                    case mfWAND:
                        ft=sWAND;
                        break;
                        
                    case mfHAMMER:
                        ft=sHAMMER;
                        break;
                        
                    case mfSTRIKE:
                        ft=sSTRIKE;
                        break;
                        
                    default:
                        putit = false;
                        break;
                    }
                    
                    if(putit)
                    {
                        if(j==-1)
                        {
                            s->data[i] = s->secretcombo[ft];
                            s->cset[i] = s->secretcset[ft];
                            newflag = s->secretflag[ft];
                        }
                        else
                        {
                            t[j].data[i] = t[j].secretcombo[ft];
                            t[j].cset[i] = t[j].secretcset[ft];
                            newflag = t[j].secretflag[ft];
                        }
                    }
                }
                
                if(newflag >-1)
                {
                    ((j==-1) ? s->sflag[i] : t[j].sflag[i]) = newflag;
                }
            }
        }
        
        //if(true)
        //{
        int newflag = -1;
        
        for(int iter=0; iter<2; ++iter)
        {
            int checkflag=combobuf[s->data[i]].flag;
            
            if(iter==1)
            {
                checkflag=s->sflag[i];
            }
            
            if((checkflag > 15)&&(checkflag < 32))
            {
                s->data[i] = s->secretcombo[(checkflag)-16+4];
                s->cset[i] = s->secretcset[(checkflag)-16+4];
                newflag = s->secretflag[(checkflag)-16+4];
                //        putit = true;
            }
        }
        
        if(newflag >-1) s->sflag[i] = newflag;
        
        for(int j=0; j<6; j++)
        {
            if(t[j].data.empty()||t[j].cset.empty()) continue;
            
            int newflag2 = -1;
            
            for(int iter=0; iter<2; ++iter)
            {
                int checkflag=combobuf[t[j].data[i]].flag;
                
                if(iter==1)
                {
                    checkflag=t[j].sflag[i];
                }
                
                if((checkflag > 15)&&(checkflag < 32))
                {
                    t[j].data[i] = t[j].secretcombo[(checkflag)-16+4];
                    t[j].cset[i] = t[j].secretcset[(checkflag)-16+4];
                    newflag2 = t[j].secretflag[(checkflag)-16+4];
                    //          putit = true;
                }
            }
            
            if(newflag2 >-1) t[j].sflag[i] = newflag2;
        }
        
        //} //if(true)
        
        /*
          if(putit && refresh)
          putcombo(scrollbuf,(i&15)<<4,i&0xF0,s->data[i],s->cset[i]);
          */
    }
    
    //FFCs
    for(int i=0; i<32; i++)
    {
        bool putit;
        
        if(!high16only)
        {
            for(int iter=0; iter<1; ++iter)
            {
                putit=true;
                int checkflag=combobuf[s->ffdata[i]].flag;
                
                if(iter==1)
                {
                    checkflag=s->sflag[i];
                }
                
                switch(checkflag)
                {
                case mfBCANDLE:
                    ft=sBCANDLE;
                    break;
                    
                case mfRCANDLE:
                    ft=sRCANDLE;
                    break;
                    
                case mfWANDFIRE:
                    ft=sWANDFIRE;
                    break;
                    
                case mfDINSFIRE:
                    ft=sDINSFIRE;
                    break;
                    
                case mfARROW:
                    ft=sARROW;
                    break;
                    
                case mfSARROW:
                    ft=sSARROW;
                    break;
                    
                case mfGARROW:
                    ft=sGARROW;
                    break;
                    
                case mfSBOMB:
                    ft=sSBOMB;
                    break;
                    
                case mfBOMB:
                    ft=sBOMB;
                    break;
                    
                case mfBRANG:
                    ft=sBRANG;
                    break;
                    
                case mfMBRANG:
                    ft=sMBRANG;
                    break;
                    
                case mfFBRANG:
                    ft=sFBRANG;
                    break;
                    
                case mfWANDMAGIC:
                    ft=sWANDMAGIC;
                    break;
                    
                case mfREFMAGIC:
                    ft=sREFMAGIC;
                    break;
                    
                case mfREFFIREBALL:
                    ft=sREFFIREBALL;
                    break;
                    
                case mfSWORD:
                    ft=sSWORD;
                    break;
                    
                case mfWSWORD:
                    ft=sWSWORD;
                    break;
                    
                case mfMSWORD:
                    ft=sMSWORD;
                    break;
                    
                case mfXSWORD:
                    ft=sXSWORD;
                    break;
                    
                case mfSWORDBEAM:
                    ft=sSWORDBEAM;
                    break;
                    
                case mfWSWORDBEAM:
                    ft=sWSWORDBEAM;
                    break;
                    
                case mfMSWORDBEAM:
                    ft=sMSWORDBEAM;
                    break;
                    
                case mfXSWORDBEAM:
                    ft=sXSWORDBEAM;
                    break;
                    
                case mfHOOKSHOT:
                    ft=sHOOKSHOT;
                    break;
                    
                case mfWAND:
                    ft=sWAND;
                    break;
                    
                case mfHAMMER:
                    ft=sHAMMER;
                    break;
                    
                case mfSTRIKE:
                    ft=sSTRIKE;
                    break;
                    
                default:
                    putit = false;
                    break;
                }
                
                if(putit)
                {
                    s->ffdata[i] = s->secretcombo[ft];
                    s->ffcset[i] = s->secretcset[ft];
                }
            }
        }
        
        if(!(s->flags2&fCLEARSECRET) || high16only || s->flags4&fENEMYSCRTPERM)
        {
            for(int iter=0; iter<1; ++iter)
            {
                int checkflag=combobuf[s->ffdata[i]].flag;
                
                if(iter==1)
                {
                    // FFCs can't have flags! Yet...
                }
                
                if((checkflag > 15)&&(checkflag < 32))
                {
                    s->ffdata[i] = s->secretcombo[checkflag-16+4];
                    s->ffcset[i] = s->secretcset[checkflag-16+4];
                    //        putit = true;
                }
            }
        }
    }
}
