LOADDMAPDATA //command
DMAPDATANAME //command
DMAPDATATITLE //command
DMAPDATAINTRO //command
DMAPDATAMUSIC //command, string to load a music file

DMAPDATAMAP //byte
DMAPDATALEVEL //word
DMAPDATAOFFSET //char
DMAPDATACOMPASS //byte
DMAPDATAPALETTE //word
DMAPDATAMIDI //byte
DMAPDATACONTINUE //byte
DMAPDATATYPE //byte
DMAPDATAGRID //byte[8]
DMAPDATAMINIMAPTILE //word - two of these, so let's do MinimapTile[2]
DMAPDATAMINIMAPCSET //byte - two of these, so let's do MinimapCSet[2]
DMAPDATALARGEMAPTILE //word -- two of these, so let's to LargemapTile[2]
DMAPDATALARGEMAPCSET //word -- two of these, so let's to LargemaCSet[2]
DMAPDATAMUISCTRACK //byte
DMAPDATASUBSCRA //byte, active subscreen
DMAPDATASUBSCRP //byte, passive subscreen
DMAPDATADISABLEDITEMS //byte[iMax]
DMAPDATAFLAGS //long
DMAPDATAGRAVITY //unimplemented
DMAPDATAJUMPLAYER //unimplemented

static void do_loaddmapdata(const bool v);
void FFScript::do_loaddmapdata(const bool v)
{
    long ID = SH::get_arg(sarg1, v) / 10000;
    
    if ( ID < 0 || ID > 511 )
    {
	Z_scripterrlog("Invalid DMap ID passed to Game->LoadDMapData: %d\n", ID);
	ri->dmapsref = LONG_MAX;
    }
        
    else ri->dmapsref = ID;
    //Z_eventlog("Script loaded npcdata with ID = %ld\n", ri->idata);
}

static void do_getDMapData_dmapname(const bool v);
void FFScript::do_getDMapData_dmapname(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapName") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].name)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapName' not large enough\n");
}

static void do_setDMapData_dmapname(const bool v);
void FFScript::do_setDMapData_dmapname(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;

    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->Game->SetDMapName") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(DMaps[ID].name, filename_str.c_str(), 72);
    DMaps[ID].name[72]='\0';
}

static void do_getDMapData_dmaptitle(const bool v);
void FFScript::do_getDMapData_dmaptitle(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapTitle") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].title)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapTitle' not large enough\n");
}

static void do_setDMapData_dmaptitle(const bool v);
void FFScript::do_setDMapData_dmaptitle(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->Game->SetDMapTitle") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 21);
    strncpy(DMaps[ID].title, filename_str.c_str(), 20);
    DMaps[ID].title[20]='\0';
}

static void do_getDMapData_dmapintro(const bool v);
void FFScript::do_getDMapData_dmapintro(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    
    if(BC::checkDMapID(ID, "Game->GetDMapIntro") != SH::_NoError)
        return;
        
    if(ArrayH::setArray(arrayptr, string(DMaps[ID].intro)) == SH::_Overflow)
        Z_scripterrlog("Array supplied to 'Game->GetDMapIntro' not large enough\n");
}

static void do_setDMapData_dmapintro(const bool v);
void FFScript::do_setDMapData_dmapintro(const bool v)
{
    long ID = ri->zmsgref;
    long arrayptr = get_register(sarg2) / 10000;
    string filename_str;
    
    if(BC::checkDMapID(ID, "Game->Game->SetDMapIntro") != SH::_NoError)
        return;
        
        
    ArrayH::getString(arrayptr, filename_str, 73);
    strncpy(DMaps[ID].intro, filename_str.c_str(), 72);
    DMaps[ID].intro[72]='\0';
}


struct dmap
{
    byte map;
    word level;
    char xoff;
    byte compass;
    word color;
    byte midi;
    byte cont;
    byte type;
    //8
    byte grid[8];
    //16
    char name[21];
    char title[21];
    char intro[73];
    //byte padding;
    //132
    word minimap_1_tile;                                      //before getting map
    byte minimap_1_cset;                                      //cset for minimap 1
    //byte padding;
    word minimap_2_tile;                                      //after getting map
    byte minimap_2_cset;                                      //cset for minimap 2
    //byte padding;
    //140
    word largemap_1_tile;                                     //large map
    byte largemap_1_cset;                                     //cset for large
    //byte padding;
    word largemap_2_tile;                                     //large map
    byte largemap_2_cset;                                     //cset for large
    char tmusic[56];
    byte tmusictrack;
    byte active_subscreen;
    byte passive_subscreen;
    // int emusic;
    //byte padding;
    //204
    byte disableditems[iMax];
    // 460
    long flags;
    //gravity[2], jumpthreshold
};

// DMap flags
#define dmfCAVES			0x0001
#define dmf3STAIR			0x0002
#define dmfWHIRLWIND		0x0004
#define dmfGUYCAVES			0x0008
#define dmfNOCOMPASS		0x0010
#define dmfWAVY 			0x0020
#define dmfWHIRLWINDRET		0x0040
#define dmfALWAYSMSG   		0x0080
#define dmfVIEWMAP       	0x0100
#define dmfDMAPMAP         	0x0200
#define dmfMINIMAPCOLORFIX 	0x0400
#define dmfSCRIPT1 			0x0800
#define dmfSCRIPT2 			0x1000
#define dmfSCRIPT3 			0x2000
#define dmfSCRIPT4 			0x4000
#define dmfSCRIPT5 			0x8000