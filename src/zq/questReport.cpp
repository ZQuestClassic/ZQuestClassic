#include "zq/questReport.h"

#include <map>
#include <stdio.h>
#include <string>

#include "base/files.h"
#include "base/general.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "gui/editbox.h"
#include "gui/EditboxNew.h"
#include "base/gui.h"
#include "gui/jwin.h"
#include "tiles.h"
#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/zsys.h"
#include "zq/zq_class.h"
#include "zq/zq_misc.h"
#include "zq/zquest.h"
#include "base/qst.h"

extern int32_t bie_cnt;

std::string quest_report_str;

// TODO: the purpose of this is to align every row of text...but we
// aren't using a monospaced font.
char *palname_spaced(int32_t pal)
{
    static char buf[PALNAMESIZE+1];
    snprintf(buf, PALNAMESIZE+1, "%-*s", PALNAMESIZE, palnames[pal]);
    return buf;
}

static int32_t usesSecretTriggerFlag(int32_t type)
{
	switch(type)
	{
		case mfSECRETS01:
		case mfSECRETS02:
		case mfSECRETS03: 
		case mfSECRETS04:      
		case mfSECRETS05: 
		case mfSECRETS06:
		case mfSECRETS07: 
		case mfSECRETS08:
		case mfSECRETS09: 
		case mfSECRETS10: 
		case mfSECRETS11:
		case mfSECRETS12:
		case mfSECRETS13:
		case mfSECRETS14: 
		case mfSECRETS15: 
		case mfSECRETS16:
		case mfBLOCKTRIGGER:
		case mfNOBLOCKS:
		case mfBRANG:
		case mfMBRANG:
		case mfFBRANG:
		case mfSARROW:
		case mfGARROW:
		case mfSTRONGFIRE:
		case mfMAGICFIRE:
		case mfDIVINEFIRE:
		case mfWANDMAGIC:
		case mfREFMAGIC:
		case mfREFFIREBALL:
		case mfSWORD:
		case mfWSWORD:
		case mfMSWORD:
		case mfXSWORD:
		case mfSWORDBEAM:
		case mfWSWORDBEAM:
		case mfMSWORDBEAM:
		case mfXSWORDBEAM:
		case mfHOOKSHOT:
		case mfWAND:
		case mfHAMMER:
		case mfSTRIKE:
		case mfSINGLE:
		case mfSINGLE16:
		{
			return true;
		}
		default: return false;
	}
}

DIALOG integrity_report_dlg[] =
{
    /* (dialog proc)       (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)      (d2)      (dp) */
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL },
    { jwin_button_proc,     0,    0,    0,    0,   0,       0,       0,  D_EXIT,          0,             0,       (void *) "Save", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL, NULL, NULL }
};

std::string qst_cfg_header_from_path(std::string path);
extern char *filepath;
void showQuestReport()
{
	do_box_setup(integrity_report_dlg);
	//
	const int sz = 9;
	integrity_report_dlg[6].x = zq_screen_w/2-integrity_report_dlg[5].w;
	integrity_report_dlg[sz+0].x = zq_screen_w/2;
	integrity_report_dlg[sz+0].y = integrity_report_dlg[5].y;
	integrity_report_dlg[sz+0].w = integrity_report_dlg[5].w;
	integrity_report_dlg[sz+0].h = integrity_report_dlg[5].h;
	integrity_report_dlg[sz+0].dp2 = integrity_report_dlg[5].dp2;
	do_box_edit(integrity_report_dlg, [&](int ret)
		{
			switch(ret)
			{
				case 8:
				{
					if(!prompt_for_new_file_compat("Save Quest Report (.txt)","txt",NULL,datapath,false))
						return false;
						
					if(exists(temppath))
					{
						if(jwin_alert("Confirm Overwrite","File already exists.","Overwrite?",NULL,"Yes","No",'y','n',get_zc_font(font_lfont))==2)
							return false;
					}
					
					FILE *report = fopen(temppath,"w");
					
					if(!report)
					{
						jwin_alert("Error","Unable to open file for writing!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
						return false;
					}
					
					int32_t written = (int32_t)fwrite(quest_report_str.c_str(), sizeof(char), quest_report_str.size(), report);
					
					if(written != (int32_t)quest_report_str.size())
						jwin_alert("Error","IO error while writing script to file!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
						
					fclose(report);
					return false;
				}
			}
			return true;
		}, quest_report_str, "Quest Report", true, true);
}

void TileWarpsReport()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    int32_t *warp_check;
    warp_check=(int32_t *)malloc(Map.getMapCount()*MAPSCRS*sizeof(int32_t));
    
    for(int32_t i=0; i<Map.getMapCount()*MAPSCRS; ++i)
    {
        warp_check[i]=0;
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            for(int32_t w=0; w<4; ++w)
            {
                int32_t wdm=ts->tilewarpdmap[w];
                int32_t ws=(DMaps[wdm].map*MAPSCRS+ts->tilewarpscr[w]+DMaps[wdm].xoff);
                int32_t cs=Map.getCurrMap()*MAPSCRS+Map.getCurrScr();
                
                if(ws==cs)
                {
                    warp_check[i]=1;
                }
            }
        }
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            
            if(warp_check[i]!=0)
            {
                if(!type_found)
                {
                    sprintf(buf, "The following screens have tile warp references set to the current screen (%X:%02X):\n", Map.getCurrMap()+1, Map.getCurrScr());
                    quest_report_str+=buf;
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
    free(warp_check);
}

void SideWarpsReport()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    int32_t *warp_check;
    warp_check=(int32_t *)malloc(Map.getMapCount()*MAPSCRS*sizeof(int32_t));
    
    for(int32_t i=0; i<Map.getMapCount()*MAPSCRS; ++i)
    {
        warp_check[i]=0;
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            for(int32_t w=0; w<4; ++w)
            {
                int32_t wdm=ts->sidewarpdmap[w];
                int32_t ws=(DMaps[wdm].map*MAPSCRS+ts->sidewarpscr[w]+DMaps[wdm].xoff);
                int32_t cs=Map.getCurrMap()*MAPSCRS+Map.getCurrScr();
                
                if(ws==cs)
                {
                    warp_check[i]=1;
                }
            }
        }
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            
            if(warp_check[i]!=0)
            {
                if(!type_found)
                {
                    sprintf(buf, "The following screens have side warp references set to the current screen (%X:%02X):\n", Map.getCurrMap()+1, Map.getCurrScr());
                    quest_report_str+=buf;
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
    free(warp_check);
}

void LayersReport()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    int32_t *layer_check;
    layer_check =(int32_t *)malloc(Map.getMapCount()*MAPSCRS*sizeof(int32_t));
    
    for(int32_t i=0; i<Map.getMapCount()*MAPSCRS; ++i)
    {
        layer_check[i]=0;
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            // Search through each layer
            for(int32_t w=0; w<6; ++w)
            {
                if(ts->layerscreen[w]==Map.getCurrScr() && (ts->layermap[w]-1)==Map.getCurrMap())
                {
                    layer_check[i]=w+1;
                }
            }
        }
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            
            if(layer_check[i]!=0)
            {
                if(!type_found)
                {
                    sprintf(buf, "The following screens use the current screen as a layer (%X:%02X):\n", Map.getCurrMap()+1, Map.getCurrScr());
                    quest_report_str+=buf;
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (layer %d)\n", palname_spaced(ts->color), m+1, s, layer_check[i]);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
    free(layer_check);
}


bool integrityBoolSpecialItem(mapscr *ts)
{
    return (ts->room==rSP_ITEM&&ts->catchall==0);
}

void integrityCheckSpecialItem()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolSpecialItem(ts))
            {
                if(!type_found)
                    quest_report_str+="The following screens' Room Type is set to Special Item but have no special item assigned:\n";
                    
                type_found=true;
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

bool integrityBoolEnemiesItem(mapscr *ts)
{
    if((ts->flags)&fITEM)
    {
        if(Map.getCurrScr() >= 0x80) return false;
        
        switch(ts->room)
        {
        case rZELDA:
            return false;
            
        case rSP_ITEM:
        case rMONEY:
        case rGRUMBLE:
        case rBOMBS:
        case rARROWS:
        case rSWINDLE:
        case rMUPGRADE:
        case rLEARNSLASH:
        case rTAKEONE:
        case rTRIFORCE:
            if(ts->guy) return false;
        }
        
        bool problem_found=true;
        
        for(int32_t e=0; e<10; ++e)
        {
            if(ts->enemy[e]!=0)
            {
                problem_found=false;
                break;
            }
        }
        
        return problem_found;
    }
    
    return false;
}

void integrityCheckEnemiesItem()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool problem_found=false;
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            problem_found=true;
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolEnemiesItem(ts))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have the Enemies->Item flag set, but there are no enemies in the screen:\n";
                    type_found = true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

bool integrityBoolEnemiesSecret(mapscr *ts)
{
    if((ts->flags2)&fCLEARSECRET)
    {
        if(Map.getCurrScr() >= 0x80) return false;
        
        switch(ts->room)
        {
        case rZELDA:
            return false;
            
        case rSP_ITEM:
        case rMONEY:
        case rGRUMBLE:
        case rBOMBS:
        case rARROWS:
        case rSWINDLE:
        case rMUPGRADE:
        case rLEARNSLASH:
        case rTAKEONE:
        case rTRIFORCE:
            if(ts->guy) return false;
        }
        
        bool problem_found=true;
        
        for(int32_t e=0; e<10; ++e)
        {
            if(ts->enemy[e]!=0)
            {
                problem_found=false;
                break;
            }
        }
        
        return problem_found;
    }
    
    return false;
}

void integrityCheckEnemiesSecret()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool problem_found=false;
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            problem_found=true;
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolEnemiesSecret(ts))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have the Enemies->Secret flag set, but there are no enemies in the room. This may not indicate a problem:\n";
                    type_found = true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
}

void integrityCheckTileWarpDestSquare()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    int32_t *warp_check;
    mapscr *wscr;
    warp_check=(int32_t *)malloc(Map.getMapCount()*MAPSCRS*sizeof(int32_t));
    
    for(int32_t i=0; i<Map.getMapCount()*MAPSCRS; ++i)
    {
        warp_check[i]=0;
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            for(int32_t w=0; w<4; ++w)
            {
                int32_t wdm=ts->tilewarpdmap[w];
                if (DMaps[wdm].map >= Map.getMapCount())
                    continue;

                int32_t ws=(DMaps[wdm].map*MAPSCRS+ts->tilewarpscr[w]+DMaps[wdm].xoff);
                wscr=&TheMaps[ws];
                
                if(ts->tilewarptype[w]!=wtPASS)
                {
                    int32_t wx, wy, retc = (ts->warpreturnc>>(w*2))&3;
                    
                    if(get_qr(qr_NOARRIVALPOINT))
                    {
                        wx=wscr->warpreturnx[retc];
                        wy=wscr->warpreturny[retc];
                    }
                    else
                    {
                        wx=wscr->warparrivalx;
                        wy=wscr->warparrivaly;
                    }
                    
                    if(wx==0 && wy==0)
                    {
                        warp_check[ws]=m*MAPSCRS+s+1;
                    }
                }
            }
        }
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            
            if(warp_check[i]!=0)
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens are non-passage tile warp destinations, but the warp destination square is set to 0,0 (since room 1:00 is the default warp assignment, its presence in this list does not necessarily indicate a problem with that screen):\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (%3d:%02X)\n", palname_spaced(ts->color), m+1, s, ((warp_check[i]-1)/MAPSCRS)+1, (warp_check[i]-1)%MAPSCRS);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
    free(warp_check);
}

// does not check cycling combos
void integrityCheckTileWarpDest()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    bool warpa = false, warpb = false, warpc = false, warpd = false, warpr = false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            if(!(ts->valid&mVALID))
                continue;
                
            warpa = warpb = warpc = warpd = warpr = false;
            
            for(int32_t c=0; c<176+128; ++c)
            {
                // Checks both combos and secret combos.
                int32_t ctype = combobuf[(c>=176 ? ts->secretcombo[c-176] : ts->data[c])].type;
                
                switch(ctype)
                {
                case cCAVE:
                case cPIT:
                case cSTAIR:
                case cCAVE2:
                case cSWIMWARP:
                case cDIVEWARP:
                case cSWARPA:
                    if(ts->tilewarptype[0]==wtCAVE)
                    {
                        warpa = true;
                    }
                    
                    break;
                    
                case cCAVEB:
                case cPITB:
                case cSTAIRB:
                case cCAVE2B:
                case cSWIMWARPB:
                case cDIVEWARPB:
                case cSWARPB:
                    if(ts->tilewarptype[1]==wtCAVE)
                    {
                        warpb = true;
                    }
                    
                    break;
                    
                case cCAVEC:
                case cPITC:
                case cSTAIRC:
                case cCAVE2C:
                case cSWIMWARPC:
                case cDIVEWARPC:
                case cSWARPC:
                    if(ts->tilewarptype[2]==wtCAVE)
                    {
                        warpc = true;
                    }
                    
                    break;
                    
                case cCAVED:
                case cPITD:
                case cSTAIRD:
                case cCAVE2D:
                case cSWIMWARPD:
                case cDIVEWARPD:
                case cSWARPD:
                    if(ts->tilewarptype[3]==wtCAVE)
                    {
                        warpd = true;
                    }
                    
                    break;
                    
                case cSTAIRR:
                case cPITR:
                case cSWARPR:
                    if(ts->tilewarptype[0]==wtCAVE || ts->tilewarptype[1]==wtCAVE ||
                            ts->tilewarptype[2]==wtCAVE || ts->tilewarptype[3]==wtCAVE)
                    {
                        warpr = true;
                    }
                    
                    break;
                }
                
            }
            
            if(warpa || warpb || warpc || warpd || warpr)
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have warp-type combos or warp-type secret combos, and their corresponding Tile Warp type is 'Cave/Item Cellar'. For some screens, this may not indicate a problem.\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X %s%s%s%s%s\n", palname_spaced(ts->color), m+1, s, warpa ? "[A] ":"", warpb ? "[B] ":"",
                        warpc ? "[C] ":"", warpd ? "[D] ":"", warpr ? "[Random]":"");
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

void integrityCheckSideWarpDest()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    bool warpa = false, warpb = false, warpc = false, warpd = false, warpr = false, warpt = false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRSNORMAL; ++s) // not MAPSCRS
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            if(!(ts->valid&mVALID))
                continue;
                
            warpa = warpb = warpc = warpd = warpr = warpt = false;
            
            for(int32_t c=0; c<176+128; ++c)
            {
                // Checks both combos and secret combos.
                int32_t ctype = combobuf[(c>=176 ? ts->secretcombo[c-176] : ts->data[c])].type;
                
                // Check Triforce items as well.
                bool triforce = (itemsbuf[ts->item].family==itype_triforcepiece && itemsbuf[ts->item].flags & item_flag1);
                
                if(ts->room==rSP_ITEM && !triforce)
                {
                    triforce = (itemsbuf[ts->item].family==itype_triforcepiece && itemsbuf[ts->item].flags & item_flag1);
                }
                
                if(ctype==cAWARPA || triforce)
                {
                    if(ts->sidewarptype[0]==wtCAVE)
                    {
                        (ctype==cAWARPA ? warpa : warpt) = true;
                    }
                    
                    break;
                }
                
                if(ctype==cAWARPB)
                {
                    if(ts->sidewarptype[1]==wtCAVE)
                    {
                        warpb = true;
                    }
                    
                    break;
                }
                
                if(ctype==cAWARPC)
                {
                    if(ts->sidewarptype[2]==wtCAVE)
                    {
                        warpc = true;
                    }
                    
                    break;
                }
                
                if(ctype==cAWARPD)
                {
                    if(ts->sidewarptype[3]==wtCAVE)
                    {
                        warpd = true;
                    }
                    
                    break;
                }
                
                if(ctype==cAWARPR)
                {
                    if(ts->sidewarptype[0]==wtCAVE || ts->sidewarptype[1]==wtCAVE ||
                            ts->sidewarptype[2]==wtCAVE || ts->sidewarptype[3]==wtCAVE)
                    {
                        warpr = true;
                    }
                    
                    break;
                }
                
            }
            
            if(warpa || warpb || warpc || warpd || warpr)
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have Auto Side Warp combos, Auto Side Warp secret combos, or Triforce items that Side Warp Out when collected, but their corresponding Side Warp type is 'Cave/Item Cellar'. For some screens, this may not indicate a problem.\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X %s%s%s%s%s%s\n", palname_spaced(ts->color), m+1, s, warpa ? "[A] ":"", warpb ? "[B] ":"",
                        warpc ? "[C] ":"", warpd ? "[D] ":"", warpr ? "[Random]":"", warpt ? "[Triforce]" : "");
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}


bool integrityBoolUnderCombo(mapscr *ts, int32_t ctype)
{
    switch(ctype)
    {
    case cARMOS:
    case cBUSH:
    case cTALLGRASS:
    case cFLOWERS:
    case cSLASH:
    case cSLASHITEM:
    case cBUSHTOUCHY:
    case cTALLGRASSTOUCHY:
    case cFLOWERSTOUCHY:
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    
        // Not pushblocks - there could be a layer 1 combo.
        if(ts->undercombo == 0)
        {
            return true;
        }
    }
    
    return false;
}

// does not check cycling combos
void integrityCheckUnderCombo()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    bool case_found;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            if(!(ts->valid&mVALID))
                continue;
                
            case_found=false;
            
            for(int32_t c=0; c<176+128; ++c)
            {
                // Checks both combos and secret combos.
                if(integrityBoolUnderCombo(ts,combobuf[(c>=176 ? ts->secretcombo[c-176] : ts->data[c])].type))
                {
                    case_found = true;
                    break;
                }
            }
            
            if(case_found)
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens contain combo types or secret combos that are replaced with the Under Combo, but the Under Combo for that room is combo 0. In some cases, this may not indicate a problem. Also, this does not take cycling combos into account.\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}


bool integrityBoolSaveCombo(mapscr *ts, int32_t ctype)
{
    switch(ctype)
    {
    case cSAVE:
    case cSAVE2:
        if((ts->flags4&fSAVEROOM) == 0 && (ts->flags4&fAUTOSAVE) == 0)
        {
            return true;
        }
    }
    
    return false;
}

// Save Combo check
void integrityCheckSaveCombo()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    bool case_found;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            if(!(ts->valid&mVALID))
                continue;
                
            case_found=false;
            
            for(int32_t c=0; c<176+128; ++c)
            {
                // Checks both combos and secret combos.
                if(integrityBoolSaveCombo(ts,combobuf[(c>=176 ? ts->secretcombo[c-176] : ts->data[c])].type))
                {
                    case_found = true;
                }
            }
            
			word maxffc = ts->numFFC();
            for(word c=0; c< maxffc; c++)
            {
                // Checks both combos and secret combos.
                if(integrityBoolSaveCombo(ts,combobuf[ts->ffcs[c].data].type))
                    case_found = true;
            }
            
            if(case_found)
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens contain combo types, secret combos or freeform combos that are Save Points, but the screen does not have a 'Use As Save Screen' screen flag checked. In some cases, this may not indicate a problem.\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}


bool integrityBoolStringNoGuy(mapscr *ts)
{
    return false;//(ts->str!=0&&ts->guy==0&&ts->room==0);
}

bool integrityBoolGuyNoString(mapscr *ts)
{
    return (ts->guy!=0&&!(ts->roomflags&RFL_ALWAYS_GUY)&&ts->room==0&&ts->str==0);
}


void integrityCheckGuyNoString()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolGuyNoString(ts))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have a guy set, but no string:\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

bool integrityBoolRoomNoGuy(mapscr *ts)
{
    switch(ts->room)
    {
    case rNONE:
    case rSP_ITEM:
    case r10RUPIES:
    case rGANON:
    case rZELDA:
        break;
        
    case rINFO:
    case rMONEY:
    case rGAMBLE:
    case rREPAIR:
    case rRP_HC:
    case rGRUMBLE:
    case rTRIFORCE:
    case rP_SHOP:
    case rSHOP:
    case rBOMBS:
    case rSWINDLE:
    case rWARP:
    case rITEMPOND:
    case rMUPGRADE:
    case rLEARNSLASH:
    case rARROWS:
    case rTAKEONE:
    case rBOTTLESHOP:
    default:
        if(ts->guy==0&&ts->str!=0)
            return true;
    }
    
    return false;
}

void integrityCheckRoomNoGuy()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolRoomNoGuy(ts))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have a room type set that requires a guy and string to be set, but no guy is set for the screens:\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

bool integrityBoolRoomNoString(mapscr *ts)
{
    switch(ts->room)
    {
    case rNONE:
    case rSP_ITEM:
    case r10RUPIES:
    case rGANON:
    case rZELDA:
        break;
        
    case rINFO:
    case rMONEY:
    case rGAMBLE:
    case rREPAIR:
    case rRP_HC:
    case rGRUMBLE:
    case rTRIFORCE:
    case rP_SHOP:
    case rSHOP:
    case rBOMBS:
    case rSWINDLE:
    case rWARP:
    case rITEMPOND:
    case rMUPGRADE:
    case rLEARNSLASH:
    case rARROWS:
    case rTAKEONE:
    default:
        if(ts->str==0&&ts->guy!=0)
            return true;
    }
    
    return false;
}

void integrityCheckRoomNoString()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolRoomNoString(ts))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have a room type set that requires a guy and string to be set, but no string is set for the screens:\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
                type_found=true;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

bool integrityBoolRoomNoGuyNoString(mapscr *ts)
{
    switch(ts->room)
    {
    case rNONE:
    case rSP_ITEM:
    case r10RUPIES:
    case rGANON:
    case rZELDA:
        break;
        
    case rINFO:
    case rMONEY:
    case rGAMBLE:
    case rREPAIR:
    case rRP_HC:
    case rGRUMBLE:
    case rTRIFORCE:
    case rP_SHOP:
    case rSHOP:
    case rBOMBS:
    case rSWINDLE:
    case rWARP:
    case rITEMPOND:
    case rMUPGRADE:
    case rLEARNSLASH:
    case rARROWS:
    case rTAKEONE:
    default:
        if(ts->str==0&&ts->guy==0) return true;
    }
    
    return false;
}

void integrityCheckRoomNoGuyNoString()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(integrityBoolRoomNoGuyNoString(ts))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have a room type set that requires a guy and string to be set, but neither a guy nor a string is set for the screens:\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
                type_found=true;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

void integrityCheckQuestNumber()
{
    // if(header.quest_number!=0)
    // {
        // quest_report_str+="The quest number (in the Quest->Header menu) is not set to 0.  This quest will not be playable unless this is changed!\n\n";
    // }
}

void integrityCheckItemWalkability()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            ts=&TheMaps[m*MAPSCRS+s];
            
            if(ts->item!=0&&
                    (((combobuf[ts->data[(ts->itemy    &0xF0)+(ts->itemx    >>4)]].walk&15)!=0) ||
                     ((combobuf[ts->data[(ts->itemy    &0xF0)+((ts->itemx+15)>>4)]].walk&15)!=0) ||
                     ((combobuf[ts->data[((ts->itemy+15)&0xF0)+(ts->itemx    >>4)]].walk&15)!=0) ||
                     ((combobuf[ts->data[((ts->itemy+15)&0xF0)+((ts->itemx+15)>>4)]].walk&15)!=0)))
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens have items whose item locations are set onto fully or partially unwalkable combos:\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

void integrityCheckTileWarpDestSquareWalkability()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    int32_t *warp_check;
    mapscr *wscr;
    warp_check=(int32_t *)malloc(Map.getMapCount()*MAPSCRS*sizeof(int32_t));
    
    for(int32_t i=0; i<Map.getMapCount()*MAPSCRS; ++i)
    {
        warp_check[i]=0;
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            for(int32_t w=0; w<4; ++w)
            {
                int32_t i=(m*MAPSCRS+s);
                ts=&TheMaps[i];
                int32_t wdm=ts->tilewarpdmap[w];
                if (DMaps[wdm].map >= Map.getMapCount())
                    continue;

                int32_t ws=(DMaps[wdm].map*MAPSCRS+ts->tilewarpscr[w]+DMaps[wdm].xoff);
                wscr=&TheMaps[ws];
                
                if(ts->tilewarptype[w]!=wtPASS)
                {
                    if(((combobuf[wscr->data[(wscr->warparrivaly    &0xF0)+(wscr->warparrivalx    >>4)]].walk&15)!=0) ||
                            ((combobuf[wscr->data[(wscr->warparrivaly    &0xF0)+((wscr->warparrivalx+15)>>4)]].walk&15)!=0) ||
                            ((combobuf[wscr->data[((wscr->warparrivaly+15)&0xF0)+(wscr->warparrivalx    >>4)]].walk&15)!=0) ||
                            ((combobuf[wscr->data[((wscr->warparrivaly+15)&0xF0)+((wscr->warparrivalx+15)>>4)]].walk&15)!=0))
                    {
                        warp_check[ws]=m*MAPSCRS+s+1;
                    }
                }
            }
        }
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            
            if(warp_check[i]!=0)
            {
                if(!type_found)
                {
                    quest_report_str+="The following screens are non-passage tile warp destinations, but the warp destination square is set onto a partially or fully unwalkable combo (since screen 1:00 is the default warp assignment, its presence in this list does not necessarily indicate a problem with that screen):\n";
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (%3d:%02X)\n", palname_spaced(ts->color), m+1, s, ((warp_check[i]-1)/MAPSCRS)+1, (warp_check[i]-1)%MAPSCRS);
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
    free(warp_check);
}

void integrityCheckTileWarpDestScreenInvalid()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    int32_t *warp_check;
    mapscr *wscr;
    warp_check=(int32_t *)malloc(Map.getMapCount()*MAPSCRS*sizeof(int32_t));
    
    for(int32_t i=0; i<Map.getMapCount()*MAPSCRS; ++i)
    {
        warp_check[i]=0;
    }
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            for(int32_t w=0; w<4; ++w)
            {
                int32_t i=(m*MAPSCRS+s);
                ts=&TheMaps[i];
                int32_t wdm=ts->tilewarpdmap[w];
                if (DMaps[wdm].map >= Map.getMapCount())
                    continue;

                int32_t ws=(DMaps[wdm].map*MAPSCRS+ts->tilewarpscr[w]+DMaps[wdm].xoff);
                wscr=&TheMaps[ws];
                
                if(!(wscr->valid&mVALID))
                {
                    if(!type_found)
                    {
                        quest_report_str+="The following screens have tile warps to screens that are undefined/invalid:\n";
                        type_found=true;
                    }
                    
                    buf[0]=0;
                    sprintf(buf, "%s %3d:%02X\n", palname_spaced(ts->color), m+1, s);
                    quest_report_str+=buf;
                    type_found=true;
                }
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
    
    free(warp_check);
}

int32_t onIntegrityCheckSpecialItem()
{
    quest_report_str="";
    integrityCheckSpecialItem();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckEnemiesItem()
{
    quest_report_str="";
    integrityCheckEnemiesItem();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckEnemiesSecret()
{
    quest_report_str="";
    integrityCheckEnemiesSecret();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckTileWarpDestSquare()
{
    quest_report_str="";
    integrityCheckTileWarpDestSquare();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckGuyNoString()
{
    quest_report_str="";
    integrityCheckGuyNoString();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckRoomNoGuy()
{
    quest_report_str="";
    integrityCheckRoomNoGuy();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckRoomNoString()
{
    quest_report_str="";
    integrityCheckRoomNoString();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckRoomNoGuyNoString()
{
    quest_report_str="";
    integrityCheckRoomNoGuyNoString();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckQuestNumber()
{
    quest_report_str="";
    integrityCheckQuestNumber();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckItemWalkability()
{
    quest_report_str="";
    integrityCheckItemWalkability();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckTileWarpDestSquareWalkability()
{
    quest_report_str="";
    integrityCheckTileWarpDestSquareWalkability();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckTileWarpDestScreenInvalid()
{
    quest_report_str="";
    integrityCheckTileWarpDestScreenInvalid();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

void integrityCheckAllRooms()
{
    integrityCheckSpecialItem();
    integrityCheckEnemiesItem();
    integrityCheckEnemiesSecret();
    integrityCheckGuyNoString();
    integrityCheckRoomNoGuy();
    integrityCheckRoomNoString();
    integrityCheckRoomNoGuyNoString();
    integrityCheckItemWalkability();
    integrityCheckUnderCombo();
    integrityCheckSaveCombo();
}

void integrityCheckAllWarps()
{
    integrityCheckSideWarpDest();
    integrityCheckTileWarpDest();
    integrityCheckTileWarpDestSquare();
    integrityCheckTileWarpDestSquareWalkability();
    integrityCheckTileWarpDestScreenInvalid();
}

int32_t onIntegrityCheckRooms()
{
    quest_report_str="";
    integrityCheckAllRooms();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckWarps()
{
    quest_report_str="";
    integrityCheckAllWarps();
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onIntegrityCheckAll()
{
    quest_report_str="";
    // Quest Checks!
    integrityCheckQuestNumber();
    // Other checks
    integrityCheckAllRooms();
    integrityCheckAllWarps();
    
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

typedef struct item_location_node
{
    int32_t map;
    int32_t screen;
    int32_t extra1;
    int32_t extra2;
    int32_t enemy;
    int32_t pal;
    item_location_node* next;
} item_location_node;

void itemLocationReport()
{
	quest_report_str += "WARNING: This report does not account"
		" for items obtained in indirect methods, such as SCCs,"
		" Scripts, Progressive Items and Item Bundles.\n\n";
	
    mapscr *ts=NULL;
    int32_t sc=0;
    int32_t location_types=6;
    char buf[255];
    
    item_location_node **item_location_grid;
    
    item_location_grid = new item_location_node*[MAXITEMS];
    
    for(int32_t i=0; i<MAXITEMS; i++)
    {
        item_location_grid[i] = new item_location_node[location_types];
    }
    
    item_location_node *tempnode=NULL;
    item_location_node *tempnode2=NULL;
    item_location_node *newnode=NULL;
    
    for(int32_t i=0; i<MAXITEMS; ++i)
    {
        for(int32_t j=0; j<location_types; ++j)
        {
            item_location_grid[i][j].map=-1;
            item_location_grid[i][j].screen=-1;
            item_location_grid[i][j].extra1=-1;
            item_location_grid[i][j].extra2=-1;
            item_location_grid[i][j].enemy=-1;
            item_location_grid[i][j].pal=0;
            item_location_grid[i][j].next=NULL;
        }
    }
    
    bool type_found=false;
    bool item_found=false;
    quest_report_str+="The following items have been found in the quest at the following locations. This may not include items placed with or given by scripts:\n";
    
    //check all the screens on all the maps
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            sc=m*MAPSCRS+s;
            ts=&TheMaps[sc];
            
            //if the room item is set
            if(ts->hasitem)
            {
                //start at the room item in the item location grid
                tempnode=&(item_location_grid[ts->item][0]);
                //loop to the end of the list
                int32_t count=0;
                
                while(tempnode->next!=NULL)
                {
                    ++count;
                    tempnode=tempnode->next;
                }
                
                //make a new node
                newnode=(item_location_node*)malloc(sizeof(item_location_node));
                //insert the map and screen data
                newnode->map=m+1;
                newnode->screen=s;
                newnode->extra1=-1;
                newnode->extra2=-1;
                newnode->enemy=(ts->flags&fITEM ? -1 : (ts->flags11&efCARRYITEM) ? ts->enemy[0] : 0);
                newnode->pal=ts->color;
                newnode->next=NULL;
                tempnode->next=newnode;
            }
            
            if(ts->room==rSP_ITEM)
            {
                //start at the special item in the item location grid
                tempnode=&(item_location_grid[ts->catchall][1]);
                
                //loop to the end of the list
                while(tempnode->next!=NULL)
                {
                    tempnode=tempnode->next;
                }
                
                //make a new node
                newnode=(item_location_node*)malloc(sizeof(item_location_node));
                //insert the map and screen data
                newnode->map=m+1;
                newnode->screen=s;
                newnode->extra1=-1;
                newnode->extra2=-1;
                newnode->enemy=-1;
                newnode->pal=ts->color;
                newnode->next=NULL;
                tempnode->next=newnode;
            }
            
            if(ts->room==rRP_HC)
            {
                //start at the hc/rp room item in the item location grid
                tempnode=&(item_location_grid[iRPotion][2]);
                
                //loop to the end of the list
                while(tempnode->next!=NULL)
                {
                    tempnode=tempnode->next;
                }
                
                //make a new node
                newnode=(item_location_node*)malloc(sizeof(item_location_node));
                //insert the map and screen data
                newnode->map=m+1;
                newnode->screen=s;
                newnode->extra1=-1;
                newnode->extra2=-1;
                newnode->enemy=-1;
                newnode->pal=ts->color;
                newnode->next=NULL;
                tempnode->next=newnode;
                
                tempnode=&(item_location_grid[iHeartC][2]);
                
                //loop to the end of the list
                while(tempnode->next!=NULL)
                {
                    tempnode=tempnode->next;
                }
                
                //make a new node
                newnode=(item_location_node*)malloc(sizeof(item_location_node));
                //insert the map and screen data
                newnode->map=m+1;
                newnode->screen=s;
                newnode->extra1=-1;
                newnode->extra2=-1;
                newnode->enemy=-1;
                newnode->pal=ts->color;
                newnode->next=NULL;
                tempnode->next=newnode;
            }
            
            
            if(ts->room==rSHOP||ts->room==rP_SHOP||ts->room==rTAKEONE)
            {
                for(int32_t si=0; si<3; ++si)
                {
                    if(QMisc.shop[ts->catchall].item[si]>0)
                    {
                        //start at the special item in the item location grid
                        tempnode=&(item_location_grid[QMisc.shop[ts->catchall].item[si]][(ts->room==rSHOP?3:(ts->room==rP_SHOP?4:5))]);
                        
                        //loop to the end of the list
                        while(tempnode->next!=NULL)
                        {
                            tempnode=tempnode->next;
                        }
                        
                        //make a new node
                        newnode=(item_location_node*)malloc(sizeof(item_location_node));
                        //insert the map and screen data
                        newnode->map=m+1;
                        newnode->screen=s;
                        newnode->extra1=ts->catchall;
                        newnode->extra2=QMisc.shop[ts->catchall].price[si];
                        newnode->enemy=-1;
                        newnode->pal=ts->color;
                        newnode->next=NULL;
                        tempnode->next=newnode;
                    }
                }
            }
        }
    }
    
    build_bii_list(false);
    
    //for each item
    for(int32_t i2=0; i2<MAXITEMS; ++i2)
    {
        int32_t i=bii[i2].i;
        item_found=false;
        
        //check each item location type (room item, special item, shop item, choose any item, etc.)
        for(int32_t type=0; type<location_types; ++type)
        {
            //set the tempnode at the start
            tempnode=&(item_location_grid[i][type]);
            
            //if there is item location data
            if(tempnode->next!=NULL)
            {
                type_found=true;
                
                if(!item_found)
                {
                    buf[0]=0;
                    sprintf(buf, "\n--- %s ---\n", item_string[i]);
                    quest_report_str+=buf;
                }
                
                item_found=true;
                
                //loop through each item location for this item/type
                do
                {
                    tempnode=tempnode->next;
                    //add it to the list
                    buf[0]=0;
                    
                    switch(type)
                    {
                    case 1:
                        sprintf(buf, "%s %3d:%02X (special item)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen);
                        break;
                        
                    case 2:
                        sprintf(buf, "%s %3d:%02X (Heart Container / Red Potion room)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen);
                        break;
                        
                    case 3:
                        sprintf(buf, "%s %3d:%02X (shop %d @ %d rupees)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen, tempnode->extra1, tempnode->extra2);
                        break;
                        
                    case 4:
                        sprintf(buf, "%s %3d:%02X (potion shop %d @ %d rupees)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen, tempnode->extra1, tempnode->extra2);
                        break;
                        
                    case 5:
                        sprintf(buf, "%s %3d:%02X (take one item room)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen);
                        break;
                        
                    case 0:
                    default:
                        sprintf(buf, "%s %3d:%02X (room item%s%s)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen,
                                tempnode->enemy==-1 ? ", enemies -> item" : tempnode->enemy ? ", carried by " : "",
                                tempnode->enemy>0 ? guy_string[tempnode->enemy] : "");
                        break;
                    }
                    
                    quest_report_str+=buf;
                }
                while(tempnode->next!=NULL);
            }
        }
    }
    
    for(int32_t i=0; i<MAXITEMS; ++i)
    {
        for(int32_t type=0; type<location_types; ++type)
        {
            if(item_location_grid[i][type].next!=NULL)
            {
                tempnode=&(item_location_grid[i][type]);
                tempnode=tempnode->next;
                
                while(tempnode!=NULL)
                {
                    tempnode2=tempnode->next;
                    free(tempnode);
                    tempnode=tempnode2;
                }
            }
        }
        
        //don't forget to free this too -DD
        delete[] item_location_grid[i];
    }
    
    if(!type_found)
    {
        buf[0]=0;
        sprintf(buf, "None\n\n");
        quest_report_str+=buf;
    }
    else
    {
        quest_report_str += '\n';
    }
    
    //and this -DD
    delete[] item_location_grid;
}

int32_t onItemLocationReport()
{
    quest_report_str="";
    itemLocationReport();
    
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

typedef struct enemy_location_node
{
    int32_t map;
    int32_t screen;
    int32_t pal;
    int32_t list;
    int32_t eflag;
    int32_t comboflag;
    int32_t combotype;
    int32_t ganonscr;
    enemy_location_node* next;
} enemy_location_node;

void enemyLocationReport()
{
    mapscr *ts=NULL;
    int32_t sc=0;
    char buf[255];
    
    enemy_location_node *enemy_location_grid;
    
    enemy_location_grid = new enemy_location_node[MAXGUYS];
    
    for(int32_t i=0; i<MAXGUYS; i++)
    {
        enemy_location_grid[i].map=-1;
        enemy_location_grid[i].screen=-1;
        enemy_location_grid[i].pal=0;
        enemy_location_grid[i].list=0;
        enemy_location_grid[i].eflag=0;
        enemy_location_grid[i].comboflag=0;
        enemy_location_grid[i].combotype=0;
        enemy_location_grid[i].ganonscr=0;
        enemy_location_grid[i].next=NULL;
    }
    
    enemy_location_node *tempnode=NULL;
    enemy_location_node *tempnode2=NULL;
    enemy_location_node *newnode=NULL;
    
    bool type_found=false;
    bool enemy_found=false;
    quest_report_str+="The following enemies are instantiated in the quest at the following locations (note that this does not include combo type, combo flag or enemy screen flag spawns):\n";
    
    //check all the screens on all the maps
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            sc=m*MAPSCRS+s;
            ts=&TheMaps[sc];
            int32_t enemytally[MAXGUYS];
            memset(enemytally,0,sizeof(enemytally));
            
            for(int32_t i=0; i<10; i++)
            {
                int32_t enemy = ts->enemy[i];
                
                if(!enemy) continue;
                
                enemytally[enemy]++;
            }
            
            for(int32_t i=0; i<MAXGUYS; ++i)
            {
                if(enemytally[i]==0) continue;
                
                tempnode=&(enemy_location_grid[i]);
                
                //loop to the end of the list
                while(tempnode->next!=NULL)
                {
                    tempnode=tempnode->next;
                }
                
                //make a new node
                newnode=(enemy_location_node*)malloc(sizeof(enemy_location_node));
                //insert the map and screen data
                newnode->map=m+1;
                newnode->screen=s;
                newnode->pal=ts->color;
                newnode->list=enemytally[i];
                newnode->eflag=0;
                newnode->comboflag=0;
                newnode->combotype=0;
                newnode->ganonscr=0;
                newnode->next=NULL;
                tempnode->next=newnode;
            }
        }
    }
    
    build_bie_list(false);
    
    for(int32_t i2=1; i2<bie_cnt; ++i2)
    {
        int32_t i=bie[i2].i;
        enemy_found=false;
        //set the tempnode at the start
        tempnode=&(enemy_location_grid[i]);
        
        //if there is location data
        if(tempnode->next!=NULL)
        {
            type_found=true;
            
            if(!enemy_found)
            {
                buf[0]=0;
                sprintf(buf, "\n--- %s ---\n", guy_string[i]);
                quest_report_str+=buf;
            }
            
            enemy_found=true;
            
            //loop through each location for this script
            do
            {
                tempnode=tempnode->next;
                //add it to the list
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (%d)\n",
                        palname_spaced(tempnode->pal),
                        tempnode->map,
                        tempnode->screen,
                        tempnode->list /* Possibly add more details later/never */);
                quest_report_str+=buf;
            }
            while(tempnode->next!=NULL);
        }
    }
    
    for(int32_t i=0; i<MAXGUYS; ++i)
    {
        if(enemy_location_grid[i].next!=NULL)
        {
            tempnode=&(enemy_location_grid[i]);
            tempnode=tempnode->next;
            
            while(tempnode!=NULL)
            {
                tempnode2=tempnode->next;
                free(tempnode);
                tempnode=tempnode2;
            }
        }
    }
    
    delete[] enemy_location_grid;
    
    if(!type_found)
    {
        buf[0]=0;
        sprintf(buf, "None\n\n");
        quest_report_str+=buf;
    }
    else
    {
        quest_report_str += '\n';
    }
}

int32_t onEnemyLocationReport()
{
    quest_report_str="";
    enemyLocationReport();
    
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

typedef struct script_location_node
{
    int32_t map;
    int32_t screen;
    int32_t pal;
    int32_t d[8];
    script_location_node* next;
} script_location_node;

void scriptLocationReport()
{
    mapscr *ts=NULL;
    int32_t sc=0;
    char buf[255];
    
    script_location_node *script_location_grid;
    
    script_location_grid = new script_location_node[NUMSCRIPTFFC];
    
    for(int32_t i=0; i<NUMSCRIPTFFC-1; i++)
    {
        script_location_grid[i].map=-1;
        script_location_grid[i].screen=-1;
        script_location_grid[i].pal=0;
        
        for(int32_t k=0; k<8; ++k)
            script_location_grid[i].d[k]=0;
            
        script_location_grid[i].next=NULL;
    }
    
    script_location_node *tempnode=NULL;
    script_location_node *tempnode2=NULL;
    script_location_node *newnode=NULL;
    
    bool type_found=false;
    bool script_found=false;
    quest_report_str+="The following FFC scripts are instantiated in the quest at the following locations:\n";
    
    //check all the screens on all the maps
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            sc=m*MAPSCRS+s;
            ts=&TheMaps[sc];
            
			word num_ffcs = ts->numFFC();
            for(word i=0; i<num_ffcs; i++)
            {
                int32_t script = ts->ffcs[i].script;
                
                if(!script || !ts->ffcs[i].data) continue;
                
                tempnode=&(script_location_grid[script]);
                
                //loop to the end of the list
                while(tempnode->next!=NULL)
                {
                    tempnode=tempnode->next;
                }
                
                //make a new node
                newnode=(script_location_node*)malloc(sizeof(script_location_node));
                //insert the map and screen data
                newnode->map=m+1;
                newnode->screen=s;
                newnode->pal=ts->color;
                
                for(int32_t j=0; j<8; ++j)
                    newnode->d[j] = ts->ffcs[i].initd[j];
                    
                newnode->next=NULL;
                tempnode->next=newnode;
            }
        }
    }
    
    for(int32_t i=0; i<NUMSCRIPTFFC-1; ++i)
    {
        script_found=false;
        //set the tempnode at the start
        tempnode=&(script_location_grid[i]);
        
        //if there is location data
        if(tempnode->next!=NULL)
        {
            type_found=true;
            
            if(!script_found)
            {
                buf[0]=0;
                sprintf(buf, "\n--- %s ---\n", ffcmap[i-1].scriptname.c_str());
                quest_report_str+=buf;
            }
            
            script_found=true;
            
            //loop through each location for this script
            do
            {
                tempnode=tempnode->next;
                //add it to the list
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (Args: %d, %d, %d, %d, %d, %d, %d, %d)\n", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen,
                        tempnode->d[0]/10000,tempnode->d[1]/10000,tempnode->d[2]/10000,tempnode->d[3]/10000,
                        tempnode->d[4]/10000,tempnode->d[5]/10000,tempnode->d[6]/10000,tempnode->d[7]/10000);
                quest_report_str+=buf;
            }
            while(tempnode->next!=NULL);
        }
    }
    
    for(int32_t i=0; i<NUMSCRIPTFFC-1; ++i)
    {
        if(script_location_grid[i].next!=NULL)
        {
            tempnode=&(script_location_grid[i]);
            tempnode=tempnode->next;
            
            while(tempnode!=NULL)
            {
                tempnode2=tempnode->next;
                free(tempnode);
                tempnode=tempnode2;
            }
        }
    }
    
    delete[] script_location_grid;
    
    if(!type_found)
    {
        buf[0]=0;
        sprintf(buf, "None\n\n");
        quest_report_str+=buf;
    }
    else
    {
        quest_report_str += '\n';
    }
}

int32_t onScriptLocationReport()
{
    quest_report_str="";
    scriptLocationReport();
    
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

void ComboLocationReport()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            if(!(ts->valid&mVALID))
                continue;
                
            int32_t uses = 0;
            int32_t secretuses = 0;
            int32_t ffuses = 0;
            bool undercombouses = false;
            
			word maxffc = ts->numFFC();
			word max = zc_max(maxffc,176);
            for(int32_t c=0; c<max; ++c)
            {
				if(c < 128 && ts->secretcombo[c] == Combo) secretuses++;
				if(c < 176 && ts->data[c] == Combo) uses++;
				if(c < maxffc && ts->ffcs[c].data == Combo && Combo > 0) ffuses++;
            }
			if(ts->undercombo == Combo) undercombouses = true;
            
            if(uses > 0 || secretuses > 0 || ffuses > 0 || undercombouses)
            {
                if(!type_found)
                {
                    buf[0]=0;
                    sprintf(buf, "The following screens use the currently selected combo (%d):\n", Combo);
                    quest_report_str+=buf;
                    type_found=true;
                }
                
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (%d use%s", palname_spaced(ts->color), m+1, s, uses, uses>1 ? "s" : "");
                quest_report_str+=buf;
                
                if(secretuses>0)
                {
                    buf[0]=0;
                    sprintf(buf, ", %d secret%s", secretuses, secretuses>1 ? "s" : "");
                    quest_report_str+=buf;
                }
                
                if(ffuses>0)
                {
                    buf[0]=0;
                    sprintf(buf, ", %d FFC%s", ffuses, ffuses>1 ? "s" : "");
                    quest_report_str+=buf;
                }
                
                buf[0]=0;
                sprintf(buf, "%s)\n", undercombouses ? ", under combo" : "");
                quest_report_str+=buf;
            }
        }
    }
    
    if(type_found)
    {
        quest_report_str += '\n';
    }
}

void BuggedNextComboLocationReport()
{
    mapscr *ts=NULL;
    char buf[1024];
    
    for(int32_t m=0; m<Map.getMapCount(); ++m)
    {
        for(int32_t s=0; s<MAPSCRS; ++s)
        {
            int32_t i=(m*MAPSCRS+s);
            ts=&TheMaps[i];
            
            if(!(ts->valid&mVALID))
                continue;
            
            for(int32_t c=0; c<176; ++c)
            {
                // Checks both combos and secret combos.
                if(c<176)
                {
		    int32_t cmbid = ts->data[c];
                    if(isNextType(combobuf[cmbid].type))
		    {
			if ( usesSecretTriggerFlag(ts->sflag[c]) )
			{
				memset(buf, 0, 1024);
				sprintf(buf, "Found a buggy Next-> Combo ID (%d) using flag (%d) on map (%d), screen (%d) at position (%d).\n", ts->data[c], ts->sflag[c], m+1, s, c);
				quest_report_str+=buf;
			}
			    
		    }
                }
            }
        }
    }
}

int32_t onComboLocationReport()
{
    quest_report_str="";
    ComboLocationReport();
    
    restore_mouse();
    
    if(quest_report_str!="")
        showQuestReport();
    else
        jwin_alert("Combo Locations", "No other screens use this combo.", NULL,NULL,"OK",NULL,13,27,get_zc_font(font_lfont));
        
    return D_O_K;
}

int32_t onBuggedNextComboLocationReport()
{
    quest_report_str="";
    BuggedNextComboLocationReport();
    
    restore_mouse();
    
    if(quest_report_str!="")
        showQuestReport();
    else
        jwin_alert("Combo Locations", "No other screens use this combo.", NULL,NULL,"OK",NULL,13,27,get_zc_font(font_lfont));
        
    return D_O_K;
}

typedef struct combotype_location_node
{
    int32_t map;
    int32_t screen;
    int32_t pal;
    int32_t uses;
    int32_t secretuses;
    int32_t ffuses;
    bool undercombouses;
    combotype_location_node* next;
} combotype_location_node;

void ComboTypeLocationReport()
{
    mapscr *ts=NULL;
    char buf[255];
    
    bool type_found=false;
    quest_report_str+="The following combo types appear in the quest at the following locations:\n";
    
    combotype_location_node *combotype_location_grid;
    combotype_location_grid = new combotype_location_node[MAXCOMBOTYPES];
    
    for(int32_t i=0; i<MAXCOMBOTYPES; i++)
    {
        combotype_location_grid[i].map=-1;
        combotype_location_grid[i].screen=-1;
        combotype_location_grid[i].pal=0;
        combotype_location_grid[i].uses=0;
        combotype_location_grid[i].secretuses=0;
        combotype_location_grid[i].ffuses=0;
        combotype_location_grid[i].undercombouses=false;
        combotype_location_grid[i].next=NULL;
    }
    
    combotype_location_node *tempnode=NULL;
    combotype_location_node *tempnode2=NULL;
    combotype_location_node *newnode=NULL;
    
    for(int32_t Type=1; Type < MAXCOMBOTYPES; Type++)   // Don't do the (None) type.
    {
        for(int32_t m=0; m<Map.getMapCount(); ++m) for(int32_t s=0; s<MAPSCRS; ++s)
            {
                int32_t i=(m*MAPSCRS+s);
                ts=&TheMaps[i];
                
                if(!(ts->valid&mVALID))
                    continue;
                    
                int32_t uses = 0;
                int32_t secretuses = 0;
                int32_t ffuses = 0;
                bool undercombouses = false;
                
                for(int32_t c=0; c<337; ++c)
                {
                    // Checks both combos and secret combos.
                    if(c<176)
                    {
                        if(combobuf[ts->data[c]].type == Type) uses++;
                    }
                    else if(c<304)
                    {
                        if(combobuf[ts->secretcombo[c-176]].type == Type) secretuses++;
                    }
                    else if(c<336)
                    {
                        if(combobuf[ts->ffcs[c-304].data].type == Type) ffuses++;
                    }
                    else if(combobuf[ts->undercombo].type == Type) undercombouses = true;
                }
                
                if(uses > 0 || secretuses > 0 || ffuses > 0 || undercombouses)
                {
                    tempnode=&(combotype_location_grid[Type]);
                    
                    //loop to the end of the list
                    while(tempnode->next!=NULL)
                    {
                        tempnode=tempnode->next;
                    }
                    
                    //make a new node
                    newnode=(combotype_location_node*)malloc(sizeof(combotype_location_node));
                    //insert the map and screen data
                    newnode->map=m+1;
                    newnode->screen=s;
                    newnode->pal=ts->color;
                    newnode->uses=uses;
                    newnode->secretuses=secretuses;
                    newnode->ffuses=ffuses;
                    newnode->undercombouses=undercombouses;
                    newnode->next=NULL;
                    tempnode->next=newnode;
                }
            }
    }
    
    for(int32_t i=0; i<MAXCOMBOTYPES; ++i)
    {
        bool ctype_found=false;
        //set the tempnode at the start
        tempnode=&(combotype_location_grid[i]);
        
        //if there is location data
        if(tempnode->next!=NULL)
        {
            type_found=true;
            
            if(!ctype_found)
            {
                buf[0]=0;
                sprintf(buf, "\n--- %s ---\n", combo_class_buf[i].name);
                quest_report_str+=buf;
            }
            
            ctype_found=true;
            
            //loop through each location for this script
            do
            {
                tempnode=tempnode->next;
                //add it to the list
                buf[0]=0;
                sprintf(buf, "%s %3d:%02X (%d uses", palname_spaced(tempnode->pal), tempnode->map, tempnode->screen, tempnode->uses);
                quest_report_str+=buf;
                
                if(tempnode->secretuses>0)
                {
                    buf[0]=0;
                    sprintf(buf, ", %d secret%s", tempnode->secretuses, tempnode->secretuses > 1 ? "s" : "");
                    quest_report_str+=buf;
                }
                
                if(tempnode->ffuses>0)
                {
                    buf[0]=0;
                    sprintf(buf, ", %d FFC%s", tempnode->ffuses, tempnode->ffuses > 1 ? "s" : "");
                    quest_report_str+=buf;
                }
                
                buf[0]=0;
                sprintf(buf, "%s)\n", tempnode->undercombouses ? ", under combo" : "");
                quest_report_str+=buf;
            }
            while(tempnode->next!=NULL);
        }
    }
    
    for(int32_t i=0; i<MAXCOMBOTYPES; ++i)
    {
        if(combotype_location_grid[i].next!=NULL)
        {
            tempnode=&(combotype_location_grid[i]);
            tempnode=tempnode->next;
            
            while(tempnode!=NULL)
            {
                tempnode2=tempnode->next;
                free(tempnode);
                tempnode=tempnode2;
            }
        }
    }
    
    delete[] combotype_location_grid;
    
    if(!type_found)
    {
        buf[0]=0;
        sprintf(buf, "None\n\n");
        quest_report_str+=buf;
    }
    else
    {
        quest_report_str += '\n';
    }
}

int32_t onComboTypeLocationReport()
{
    quest_report_str="";
    ComboTypeLocationReport();
    
    restore_mouse();
    showQuestReport();
    return D_O_K;
}

int32_t onWhatWarpsReport()
{
    quest_report_str="";
    TileWarpsReport();
    SideWarpsReport();
    LayersReport();
    restore_mouse();
    
    if(quest_report_str!="")
        showQuestReport();
    else
        jwin_alert("What Links Here", "No other screens warp to this screen", "or use this screen as a layer.",NULL,"OK",NULL,13,27,get_zc_font(font_lfont));
        
    return D_O_K;
}
