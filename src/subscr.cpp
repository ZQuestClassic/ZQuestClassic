//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "precompiled.h" //always first

#include "subscr.h"
#include "zelda.h"
#include "tiles.h"
#include "zsys.h"
#include "guys.h"
#include "link.h"
#include "items.h"
#include <stdio.h>
#include <string.h>
//#include "gc.h"

bool show_subscreen_dmap_dots=true;
bool show_subscreen_numbers=true;
bool show_subscreen_items=true;
bool show_subscreen_life=true;
bool new_sel=false;

item *sel_a=NULL, *sel_b=NULL;


extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern LinkClass   *Link;

subscreen_group *current_subscreen_active;
subscreen_group *current_subscreen_passive;

subscreen_group::~subscreen_group()
{
    for (uint32_t i = 0; i < ss_objects.size(); i++)
    {
        delete ss_objects[i];
    }
}

subscreen_group::subscreen_group(const subscreen_group &other) : ss_type(other.ss_type), ss_name(other.ss_name)
{
    for (uint32_t i = 0; i < other.ss_objects.size(); i++)
    {
        ss_objects.push_back(other.ss_objects[i]->clone());
    }
}

subscreen_group &subscreen_group::operator=(const subscreen_group &other)
{
    if (this == &other)
        return *this;

    ss_type = other.ss_type;
    ss_name = other.ss_name;
    for (uint32_t i = 0; i < ss_objects.size(); i++)
        delete ss_objects[i];
    ss_objects.clear();

    for (uint32_t i = 0; i < other.ss_objects.size(); i++)
    {
        ss_objects.push_back(other.ss_objects[i]->clone());
    }

    return *this;
}

subscreen_group custom_subscreen[MAXCUSTOMSUBSCREENS];

static const int notscrolling = sspUP | sspDOWN;
static const int pos = notscrolling | sspSCROLLING;

int LwpnsIdCount(int id);

int subscreen_color(miscQdata *misc, int c1, int c2)
{
    int ret;

    switch(c1)
    {
    case ssctSYSTEM:
        ret=(c2==-1)?c2:vc(c2);
        break;

    case ssctMISC:
        switch(c2)
        {
        case ssctTEXT:
            ret=misc->colors.text;
            break;

        case ssctCAPTION:
            ret=misc->colors.caption;
            break;

        case ssctOVERWBG:
            ret=misc->colors.overw_bg;
            break;

        case ssctDNGNBG:
            ret=misc->colors.dngn_bg;
            break;

        case ssctDNGNFG:
            ret=misc->colors.dngn_fg;
            break;

        case ssctCAVEFG:
            ret=misc->colors.cave_fg;
            break;

        case ssctBSDK:
            ret=misc->colors.bs_dk;
            break;

        case ssctBSGOAL:
            ret=misc->colors.bs_goal;
            break;

        case ssctCOMPASSLT:
            ret=misc->colors.compass_lt;
            break;

        case ssctCOMPASSDK:
            ret=misc->colors.compass_dk;
            break;

        case ssctSUBSCRBG:
            ret=misc->colors.subscr_bg;
            break;

        case ssctSUBSCRSHADOW:
            ret=misc->colors.subscr_shadow;
            break;

        case ssctTRIFRAMECOLOR:
            ret=misc->colors.triframe_color;
            break;

        case ssctBMAPBG:
            ret=misc->colors.bmap_bg;
            break;

        case ssctBMAPFG:
            ret=misc->colors.bmap_fg;
            break;

        case ssctLINKDOT:
            ret=misc->colors.link_dot;
            break;

        default:
            ret=(rand()*1000)%256;
            break;
        }

        break;

    default:
        ret=(c1<<4)+c2;
    }

    return ret;
}


// The conditions on which a subcreen item should be displayed.
bool displaysubscreenitem(int itemtype, int d)
{
    if(game==NULL)  //ZQuest
        return true;

    if((itemtype == itype_bomb &&
        !(game->get_bombs()
            // Remote Bombs: the bomb icon can still be used when an undetonated bomb is onscreen.
            || (curQuest->getItemDefinition(current_item_id(itype_bomb)).misc1==0 && LwpnsIdCount(wLitBomb)>0)
            || current_item_power(itype_bombbag)))
        || (itemtype == itype_sbomb &&
            !(game->get_sbombs()
                || (curQuest->getItemDefinition(current_item_id(itype_sbomb)).misc1==0 && LwpnsIdCount(wLitSBomb)>0)
                || (current_item_power(itype_bombbag)
                    && curQuest->getItemDefinition(current_item_id(itype_bombbag)).flags & itemdata::IF_FLAG1))))
        return false;

    if(itemtype!=itype_bowandarrow ||
        d!=itype_arrow ||
        ((get_bit(quest_rules,qr_TRUEARROWS)&&game->get_arrows()) ||
        (!get_bit(quest_rules,qr_TRUEARROWS)&&game->get_rupies())))
        return true;

    return false;
}

void subscreenitemRef(BITMAP *dest, int x, int y, const ItemDefinitionRef &itemref)
{
    // We need to do a reverse loop to prevent the Bow from being drawn above the Arrow (Bow & Arrow).
    int overridecheck = 0xFFFF;

    for(int i=Sitems.Count()-1; i>=0; i--)
    {
        if (overridecheck == 0xFFFF)
        {
            if (((item *)Sitems.spr(i))->itemDefinition == itemref && Sitems.spr(i)->misc == -1)
            {
                overridecheck = i;
            }
        }
    }

    //Item Override stuff here
    if(has_item(curQuest->getItemDefinition(itemref).family, curQuest->getItemDefinition(itemref).fam_type)
        && !game->get_disabled_item(itemref) && displaysubscreenitem(curQuest->getItemDefinition(itemref).family, 0))
    {
        if(overridecheck == 0xFFFF)
        {
            add_subscr_item(new item((fix)x,(fix)y,(fix)0,itemref,0,0));
            overridecheck = Sitems.Count()-1;
            Sitems.spr(overridecheck)->misc = -1;
        }

        Sitems.spr(overridecheck)->x = x;
        Sitems.spr(overridecheck)->y = y;
        Sitems.spr(overridecheck)->yofs=0;
        Sitems.spr(overridecheck)->draw(dest);
    }
}

void subscreenitemType(BITMAP *dest, int x, int y, int itemtype)
{
    // We need to do a reverse loop to prevent the Bow from being drawn above the Arrow (Bow & Arrow).
    int overridecheck = 0xFFFF;

    for(int i=Sitems.Count()-1; i>=0; i--)
    {
        if(Sitems.spr(i)->misc!=-1)
        {
            int d = curQuest->getItemDefinition(((item *)Sitems.spr(i))->itemDefinition).family;

            if((d==itemtype)||
                (itemtype==itype_letterpotion&&((d==itype_letter && !curQuest->isValid(current_item_id(itype_potion)))||d==itype_potion))||
                (itemtype==itype_bowandarrow&&(d==itype_bow||d==itype_arrow)))
            {
                Sitems.spr(i)->x = x;
                Sitems.spr(i)->y = y;
                Sitems.spr(i)->yofs=0;

                if(displaysubscreenitem(itemtype, d))
                {
                    Sitems.spr(i)->draw(dest);
                }

                if(itemtype!=itype_bowandarrow)
                {
                    return;
                }
            }
        }
    }       
}

void draw_textbox(BITMAP *dest, int x, int y, int w, int h, FONT *tempfont, const char *thetext, bool wword, int tabsize, int alignment, int textstyle, int color, int shadowcolor, int backcolor)
{
    int y1 = y;
    int x1;
    int len;
    char s[16];
    char text[16];
    char space[16];
    const char *printed = text;
    const char *scanned = text;
    const char *oldscan = text;
    const char *ignore = NULL;
    const char *tmp, *ptmp;
    int width;
    int i = 0;
    int noignore;

    usetc(s+usetc(s, '.'), 0);
    usetc(text+usetc(text, ' '), 0);
    usetc(space+usetc(space, ' '), 0);

    /* find the correct text */
    if(thetext != NULL)
    {
        printed = thetext;
        scanned = thetext;
    }

    /* loop over the entire string */
    for(;;)
    {
        width = 0;

        /* find the next break */
        while(ugetc(scanned))
        {
            /* check for a forced break */
            if(ugetc(scanned) == '\n')
            {
                scanned += uwidth(scanned);
                /* we are done parsing the line end */
                break;
            }

            /* the next character length */
            usetc(s+usetc(s, ugetc(scanned)), 0);
            len = text_length(tempfont, s);

            /* modify length if its a tab */
            if(ugetc(s) == '\t')
            {
                len = tabsize * text_length(tempfont, space);
            }

            /* check for the end of a line by excess width of next char */
            if(width+len >= w)
            {
                /* we have reached end of line do we go back to find start */
                if(wword)
                {
                    /* remember where we were */
                    oldscan = scanned;
                    noignore = FALSE;

                    /* go backwards looking for start of word */
                    while(!uisspace(ugetc(scanned)))
                    {
                        /* don't wrap too far */
                        if(scanned == printed)
                        {
                            /* the whole line is filled, so stop here */
                            tmp = ptmp = scanned;

                            while(ptmp != oldscan)
                            {
                                ptmp = tmp;
                                tmp += uwidth(tmp);
                            }

                            scanned = ptmp;
                            noignore = TRUE;
                            break;
                        }

                        /* look further backwards to wrap */
                        tmp = ptmp = printed;

                        while(tmp < scanned)
                        {
                            ptmp = tmp;
                            tmp += uwidth(tmp);
                        }

                        scanned = ptmp;
                    }

                    /* put the space at the end of the line */
                    if(!noignore)
                    {
                        ignore = scanned;
                        scanned += uwidth(scanned);
                    }
                    else
                    {
                        ignore = NULL;
                    }

                    /* check for endline at the convenient place */
                    if(ugetc(scanned) == '\n')
                    {
                        scanned += uwidth(scanned);
                    }
                }

                /* we are done parsing the line end */
                break;
            }

            /* the character can be added */
            scanned += uwidth(scanned);
            width += len;
        }

        /* check if we are to print it */
        if((y1+text_height(tempfont) <= (y+h)))
        {
            int tempw=0;
            const char *tempprinted=printed;

            /* print up to the marked character */
            while(printed != scanned)
            {
                /* do special stuff for each charater */
                switch(ugetc(printed))
                {
                case '\r':
                case '\n':
                    /* don't print endlines in the text */
                    break;

                    /* possibly expand the tabs */
                case '\t':
                    for(i=0; i<tabsize; i++)
                    {
                        usetc(s+usetc(s, ' '), 0);
                        tempw += text_length(tempfont, s);
                    }

                    break;

                    /* print a normal character */
                default:
                    if(printed != ignore)
                    {
                        usetc(s+usetc(s, ugetc(printed)), 0);
                        tempw += text_length(tempfont, s);
                    }
                }

                /* goto the next character */
                printed += uwidth(printed);
            }

            printed=tempprinted;

            switch(alignment)
            {
            case sstaRIGHT:
                x1=x+w-tempw;
                break;

            case sstaCENTER:
                x1=x+((w-tempw)/2)+1;
                break;

            case sstaLEFT:
            default:
                x1 = x;
                break;
            }

            /* print up to the marked character */
            while(printed != scanned)
            {
                /* do special stuff for each charater */
                switch(ugetc(printed))
                {

                case '\r':
                case '\n':
                    /* don't print endlines in the text */
                    break;

                    /* possibly expand the tabs */
                case '\t':
                    for(i=0; i<tabsize; i++)
                    {
                        usetc(s+usetc(s, ' '), 0);
                        textout_styled_aligned_ex(dest, tempfont, s, x1, y1, textstyle, sstaLEFT, color, shadowcolor, backcolor);
                        x1 += text_length(tempfont, s);
                    }

                    break;

                    /* print a normal character */
                default:
                    if(printed != ignore)
                    {
                        usetc(s+usetc(s, ugetc(printed)), 0);
                        textout_styled_aligned_ex(dest, tempfont, s, x1, y1, textstyle, sstaLEFT, color, shadowcolor, backcolor);
                        x1 += text_length(tempfont, s);
                    }
                }

                /* goto the next character */
                printed += uwidth(printed);
            }

            /* print the line end */
            y1 += text_height(tempfont);
        }
        else
        {
            scanned += uwidth(scanned);
        }

        printed = scanned;

        /* check if we are at the end of the string */
        if(!ugetc(printed))
        {
            return;
        }
    }
}


void magicgauge(BITMAP *dest,int x,int y, int container, int notlast_tile, int notlast_cset, bool notlast_mod, int last_tile, int last_cset, bool last_mod,
    int cap_tile, int cap_cset, bool cap_mod, int aftercap_tile, int aftercap_cset, bool aftercap_mod, int frames, int speed, int delay, bool unique_last, int show)
{
    //these are here to bypass compiler warnings about unused arguments
    frames=frames;
    speed=speed;
    delay=delay;

    // show
    //   0: always
    //   1: 1/2 magic
    //   2: normal magic
    if(show!=0)
    {
        if(show==1 && game->get_magicdrainrate()!=1)
            return;
        if(show==2 && game->get_magicdrainrate()!=2)
            return;
    }

    int containers=game->get_maxmagic()/MAGICPERBLOCK;
    int tile=rand()%32767, cset=rand()%15;
    bool mod_value=(rand()%2)!=0;

    if(container<containers)
    {
        tile=notlast_tile;
        cset=notlast_cset;
        mod_value=notlast_mod;
    }
    else if(container==containers)
    {
        tile=last_tile;
        cset=last_cset;
        mod_value=last_mod;
    }
    else if(container==containers+1)
    {
        tile=cap_tile;
        cset=cap_cset;
        mod_value=cap_mod;
    }
    else //if (container>containers+1)
    {
        tile=aftercap_tile;
        cset=aftercap_cset;
        mod_value=aftercap_mod;
    }

    if(mod_value)
    {
        if(game->get_magic()>=container*MAGICPERBLOCK)
        {
            //tile=tile;                                        //full block
            if(game->get_magic()==container*MAGICPERBLOCK)
            {
                if(unique_last)
                {
                    tile+=35;
                }
            }
        }
        else
        {
            if(((container-1)*MAGICPERBLOCK)>game->get_magic())
            {
                tile+=4;                                //empty block
            }
            else
            {
                tile+=4+((game->get_magic()-((container-1)*MAGICPERBLOCK))%MAGICPERBLOCK);
            }
        }
    }

    //tile=14161*4;
    overtile8(dest,tile,x,y,cset,0);

}

void lifegauge(BITMAP *dest,int x,int y, int container, int notlast_tile, int notlast_cset, bool notlast_mod, int last_tile, int last_cset, bool last_mod,
    int cap_tile, int cap_cset, bool cap_mod, int aftercap_tile, int aftercap_cset, bool aftercap_mod, int frames, int speed, int delay, bool unique_last)
{
    //these are here to bypass compiler warnings about unused arguments
    frames=frames;
    speed=speed;
    delay=delay;

    int containers=game->get_maxlife()/HP_PER_HEART;
    int tile=rand()%32767, cset=rand()%15;
    bool mod_value=(rand()%2)!=0;

    if(container<containers)
    {
        tile=notlast_tile;
        cset=notlast_cset;
        mod_value=notlast_mod;
    }
    else if(container==containers)
    {
        tile=last_tile;
        cset=last_cset;
        mod_value=last_mod;
    }
    else if(container==containers+1)
    {
        tile=cap_tile;
        cset=cap_cset;
        mod_value=cap_mod;
    }
    else //if (container>containers+1)
    {
        tile=aftercap_tile;
        cset=aftercap_cset;
        mod_value=aftercap_mod;
    }

    if(mod_value)
    {
        if(game->get_life()>=container*HP_PER_HEART)
        {
            //tile=tile;                                        //full block
            if(game->get_life()==container*HP_PER_HEART)
            {
                if(unique_last)
                {
                    tile+=19;
                }
            }
        }
        else
        {
            if(((container-1)*HP_PER_HEART)>game->get_life())
            {
                tile+=4;                                //empty block
            }
            else
            {
                tile+=4+((game->get_life()-((container-1)*HP_PER_HEART))%HP_PER_HEART);
            }
        }
    }

    overtile8(dest,tile,x,y,cset,0);

}

subscreen_object::subscreen_object(byte type, PACKFILE *f, int &status) : type(type)
{
    status = qe_OK;

    if (!p_getc(&pos, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&x, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&y, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&w, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&h, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_getc(&colortype1, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&color1, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_getc(&colortype2, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&color2, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_getc(&colortype3, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetw(&color3, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object::serialize(PACKFILE *f)
{
    if (!p_putc(pos, f))
        return false;

    if (!p_iputw(x, f))
        return false;

    if (!p_iputw(y, f))
        return false;

    if (!p_iputw(w, f))
        return false;

    if (!p_iputw(h, f))
        return false;

    if (!p_putc(colortype1, f))
        return false;

    if (!p_iputw(color1, f))
        return false;

    if (!p_putc(colortype2, f))
        return false;

    if (!p_iputw(color2, f))
        return false;

    if (!p_putc(colortype3, f))
        return false;

    if (!p_iputw(color3, f))
        return false;

    return serializeExtraData(f);
}

subscreen_object_2x2frame::subscreen_object_2x2frame(PACKFILE *f, int &status) : subscreen_object(sso2X2FRAME, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flip, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (tmp != 0);
}

bool subscreen_object_2x2frame::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_iputl(flip, f))
        return false;

    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_2x2frame::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    frame2x2(dest, misc, x+xofs, y+yofs, tile, subscreen_cset(misc, colortype1, color1), w, h, flip, overlay, transparent);
}

subscreen_object_currentitem::subscreen_object_currentitem(PACKFILE *f, int &status) : subscreen_object(ssoCURRENTITEM, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&itemfamily, f,true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    visible = (tmp != 0);

    if (!p_igetl(&posselect, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&upselect, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&downselect, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&leftselect, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&rightselect, f, true))
    {
        status = qe_invalid;
        return;
    }

    uint32_t modlen;
    if (!p_igetl(&modlen, f, true))
    {
        status = qe_invalid;
        return;
    }

    char *buf = new char[modlen];
    if (!pfread(buf, modlen, f, true))
    {
        delete[] buf;
        status = qe_invalid;
        return;
    }

    std::string module(buf);
    delete[] buf;

    uint32_t slot;
    if (!p_igetl(&slot, f, true))
    {
        status = qe_invalid;
        return;
    }

    itemref = ItemDefinitionRef(module, slot);
}

bool subscreen_object_currentitem::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(itemfamily, f))
        return false;

    if (!p_putc(visible ? 1 : 0, f))
        return false;

    if (!p_iputl(posselect, f))
        return false;

    if (!p_iputl(upselect, f))
        return false;

    if (!p_iputl(downselect, f))
        return false;

    if (!p_iputl(leftselect, f))
        return false;

    if (!p_iputl(rightselect, f))
        return false;

    uint32_t modlen = itemref.module.length() + 1;
    if (!p_iputl(modlen, f))
        return false;

    if (!pfwrite((void *)itemref.module.c_str(), modlen, f))
        return false;

    if (!p_iputl(itemref.slot, f))
        return false;

    return true;
}

void subscreen_object_currentitem::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    // Shouldn't really be checking is_zquest here, but it's okay for now...
    if (visible || (is_zquest() && (zinit.ss_flags&ssflagSHOWINVIS) != 0))
    {
        if (curQuest->isValid(itemref))
        {
            subscreenitemRef(dest, x + xofs, y + yofs, itemref);
        }
        else
        {
            subscreenitemType(dest, x + xofs, y + yofs, itemfamily);
        }
    }
}

subscreen_object_selector1::subscreen_object_selector1(PACKFILE *f, int &status) : subscreen_object(ssoSELECTOR1, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flip, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }

    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }

    transparent = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }

    large = (tmp != 0);
}

bool subscreen_object_selector1::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_iputl(flip, f))
        return false;

    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    if (!p_putc(large ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_selector1::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    int p = -1;
    uint32_t numcss = css.ss_objects.size();
    for (uint32_t j = 0; j < numcss; j++)
    {
        if (css.ss_objects[j]->type == ssoCURRENTITEM)
        {
            const subscreen_object_currentitem *ci = (const subscreen_object_currentitem *)(css.ss_objects[j]);
            if (ci->posselect == (int)Bpos)
            {
                p = j;
                break;
            }
        }
    }

    item *tempsel = sel_a;
    int temptile = tempsel->tile;
    tempsel->drawstyle = 0;

    if (overlay)
    {
        tempsel->drawstyle = 1;
    }

    for (int j = 0; j < 4; ++j)
    {
        if (p != -1)
        {
            tempsel->x = css.ss_objects[p]->x + xofs + (large ? (j % 2 ? 8 : -8) : 0);
            tempsel->y = css.ss_objects[p]->y + yofs + (large ? (j > 1 ? 8 : -8) : 0);
            tempsel->tile += (zc_max(curQuest->getItemDefinition(tempsel->itemDefinition).frames, 1)*j);

            if (temptile)
            {
                tempsel->draw(dest);
                tempsel->tile = temptile;
            }

            if (!large)
            {
                break;
            }
        }
    }
}

subscreen_object_selector2::subscreen_object_selector2(PACKFILE *f, int &status) : subscreen_object(ssoSELECTOR2, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flip, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }

    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }

    transparent = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }

    large = (tmp != 0);
}

bool subscreen_object_selector2::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_iputl(flip, f))
        return false;

    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    if (!p_putc(large ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_selector2::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    int p = -1;
    uint32_t numcss = css.ss_objects.size();
    for (uint32_t j = 0; j < numcss; j++)
    {
        if (css.ss_objects[j]->type == ssoCURRENTITEM)
        {
            const subscreen_object_currentitem *ci = (const subscreen_object_currentitem *)(css.ss_objects[j]);
            if (ci->posselect == (int)Bpos)
            {
                p = j;
                break;
            }
        }
    }

    item *tempsel = sel_b;
    int temptile = tempsel->tile;
    tempsel->drawstyle = 0;

    if (overlay)
    {
        tempsel->drawstyle = 1;
    }

    for (int j = 0; j < 4; ++j)
    {
        if (p != -1)
        {
            tempsel->x = css.ss_objects[p]->x + xofs + (large ? (j % 2 ? 8 : -8) : 0);
            tempsel->y = css.ss_objects[p]->y + yofs + (large ? (j > 1 ? 8 : -8) : 0);
            tempsel->tile += (zc_max(curQuest->getItemDefinition(tempsel->itemDefinition).frames, 1)*j);

            if (temptile)
            {
                tempsel->draw(dest);
                tempsel->tile = temptile;
            }

            if (!large)
            {
                break;
            }
        }
    }
}

subscreen_object_counter::subscreen_object_counter(PACKFILE *f, int &status) : subscreen_object(ssoCOUNTER, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&digits, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_getc(&infdigit, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flags, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&countertype1, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&countertype2, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&countertype3, f, true))
    {
        status = qe_invalid;
        return;
    }

    uint32_t modlen;
    if (!p_igetl(&modlen, f, true))
    {
        status = qe_invalid;
        return;
    }

    char *buf = new char[modlen];
    if (!pfread(buf, modlen, f, true))
    {
        delete[] buf;
        status = qe_invalid;
        return;
    }

    std::string module(buf);
    delete[] buf;

    uint32_t slot;
    if (!p_igetl(&slot, f, true))
    {
        status = qe_invalid;
        return;
    }

    itemref = ItemDefinitionRef(module, slot);
}

bool subscreen_object_counter::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    if (!p_iputl(digits, f))
        return false;

    if (!p_putc(infdigit, f))
        return false;

    if (!p_iputl(flags, f))
        return false;

    if (!p_iputl(countertype1, f))
        return false;

    if (!p_iputl(countertype2, f))
        return false;

    if (!p_iputl(countertype3, f))
        return false;

    uint32_t modlen = itemref.module.length() + 1;
    if (!p_iputl(modlen, f))
        return false;
    if (!pfwrite((void *)itemref.module.c_str(), modlen, f))
        return false;
    if (!p_iputl(itemref.slot, f))
        return false;

    return true;
}

void subscreen_object_counter::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=zfont;    
    tempfont=ss_font(fontnum);

    counter(dest, x + xofs, y + yofs, tempfont, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), subscreen_color(misc, colortype3, color3), alignment, textstyle, digits, infdigit, flags & 1, countertype1, countertype2, countertype3, itemref, (flags & 2) != 0);
    
}

subscreen_object_line::subscreen_object_line(PACKFILE *f, int &status) : subscreen_object(ssoLINE, f, status)
{
    if (status != qe_OK)
        return;

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (tmp != 0);
}

bool subscreen_object_line::serializeExtraData(PACKFILE *f)
{
    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_line::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    if(transparent)
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }

    line(dest, x + xofs, y + yofs, x + xofs + w-1, y + yofs + h-1, subscreen_color(misc, colortype1, color1));

    if(transparent)
    {
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    }
}

subscreen_object_buttonitem::subscreen_object_buttonitem(PACKFILE *f, int &status) : subscreen_object(ssoBUTTONITEM, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&button, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (tmp != 0);
}

bool subscreen_object_buttonitem::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(button, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_buttonitem::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    if(transparent)
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }

    buttonitem(dest, button, x+xofs, y+yofs);

    if(transparent)
    {
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    }
}

subscreen_object_selecteditemname::subscreen_object_selecteditemname(PACKFILE *f, int &status) : subscreen_object(ssoSELECTEDITEMNAME, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    wword = (tmp != 0);

    if (!p_igetl(&tabsize, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_selecteditemname::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    if (!p_putc(wword ? 1 : 0, f))
        return false;

    if (!p_iputl(tabsize, f))
        return false;

    return true;
}

void subscreen_object_selecteditemname::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    bool bowandarrow;
    ItemDefinitionRef itemid = weaponFromSlot(Bpos, bowandarrow);

    FONT *tempfont=ss_font(fontnum);

    // 0 can mean either the item with index 0 is selected or there's no
    // valid item to select, so be sure Link has whatever it would be.
    if(!game->get_item(itemid))
        return;

    char itemname[140]="";
    sprintf(itemname, "%s", curQuest->getItemDefinition(itemid).name.c_str());

    switch(curQuest->getItemDefinition(itemid).family)
    {
    case itype_arrow:
        if(Bitem && Bitem->dummy_bool[0]==true)  //if we also have a bow
        {
            if(curQuest->isValid(current_item_id(itype_bow)))
            {
                bool hasarrows=((get_bit(quest_rules,qr_TRUEARROWS)&&(game==NULL || game->get_arrows()))||(!get_bit(quest_rules,qr_TRUEARROWS)&&(game==NULL || game->get_rupies())));
                sprintf(itemname, "%s%s%s", curQuest->getItemDefinition(current_item_id(itype_bow)).name.c_str(), hasarrows?" & ":"",hasarrows?curQuest->getItemDefinition(Bitem->itemDefinition).name.c_str() : "");                            
            }
        }

        break;
    }

    draw_textbox(dest, x+xofs, y+yofs, w, h, tempfont, itemname, wword, tabsize, alignment, textstyle, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), subscreen_color(misc, colortype3, color3));
    // draw_textbox(dest, x, y, w,                 h,                 tempfont, thetext,  wword,                 tabsize,            alignment,          textstyle,          color,                                             shadowcolor,                                       backcolor);
}

subscreen_object_text::subscreen_object_text(PACKFILE *f, int &status) : subscreen_object(ssoTEXT, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }

    uint32_t textlen;
    if (!p_igetl(&textlen, f, true))
    {
        status = qe_invalid;
        return;
    }

    char *buf = new char[textlen];
    if (!pfread(buf, textlen, f, true))
    {
        delete[] buf;
        status = qe_invalid;
        return;
    }

    text = std::string(buf);
    delete[] buf;
}

bool subscreen_object_text::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    uint32_t textlen = text.length() + 1;
    if (!p_iputl(textlen, f))
        return false;

    if (!pfwrite((void *)text.c_str(), textlen, f))
        return false;

    return true;
}

void subscreen_object_text::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    //textout_styled_aligned_ex(bmp, f,       s,                          x,y,textstyle,         alignment,         color,                                                                    shadow,                                                                   bg)
    textout_styled_aligned_ex(dest,tempfont,(char *)text.c_str(),x+xofs,y+yofs,textstyle,alignment,subscreen_color(misc, colortype1, color1),subscreen_color(misc, colortype2, color2),subscreen_color(misc, colortype3, color3));
}


subscreen_object_rect::subscreen_object_rect(PACKFILE *f, int &status) : subscreen_object(ssoRECT, f, status)
{
    if (status != qe_OK)
        return;

    byte trans;
    if (!p_getc(&trans, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (trans != 0);

    byte fill;
    if (!p_getc(&fill, f, true))
    {
        status = qe_invalid;
        return;
    }
    filled = (fill != 0);
}

bool subscreen_object_rect::serializeExtraData(PACKFILE *f)
{
    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    if (!p_putc(filled ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_rect::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    if(transparent)
    {
        drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    }

    if (filled)
    {
        rectfill(dest, x + xofs, y + yofs, x + xofs + w - 1, y + yofs + h - 1, subscreen_color(misc, colortype2, color2));
    }

    rect(dest, x + xofs, y + yofs, x + xofs + w - 1, y + yofs + h - 1, subscreen_color(misc, colortype1, color1));

    if (transparent)
    {
        drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
    }
}
    
void subscreen_object_triforce::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    puttriforce(dest,misc,x+xofs,y+yofs,tile,subscreen_cset(misc, colortype1, color1),w,h,flip,overlay,transparent,triforcenum);    
}

subscreen_object_triforce::subscreen_object_triforce(PACKFILE *f, int &status) : subscreen_object(ssoTRIFORCE, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flip, f, true))
    {
        status = qe_invalid;
        return;
    }

    char tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (tmp != 0);

    if (!p_igetl(&triforcenum, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_triforce::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_iputl(flip, f))
        return false;

    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    if (!p_iputl(triforcenum, f))
        return false;

    return true;
}

subscreen_object_tileblock::subscreen_object_tileblock(PACKFILE *f, int &status) : subscreen_object(ssoTILEBLOCK, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flip, f, true))
    {
        status = qe_invalid;
        return;
    }

    char tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (tmp != 0);
}

bool subscreen_object_tileblock::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_iputl(flip, f))
        return false;

    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_tileblock::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    draw_block_flip(dest, x + xofs, y + yofs, tile, subscreen_cset(misc, colortype1, color1), w, h, flip, overlay, transparent);
}

subscreen_object_clear::subscreen_object_clear(PACKFILE *f, int &status) : subscreen_object(ssoCLEAR, f, status)
{

}

bool subscreen_object_clear::serializeExtraData(PACKFILE *f)
{
    return true;
}

void subscreen_object_clear::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    clear_to_color(dest,subscreen_color(misc, colortype1, color1));
}

subscreen_object_minitile::subscreen_object_minitile(PACKFILE *f, int &status) : subscreen_object(ssoMINITILE, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&specialtile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&tileoffset, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flip, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    overlay = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    transparent = (tmp != 0);
}

bool subscreen_object_minitile::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_iputl(specialtile, f))
        return false;

    if (!p_iputl(tileoffset, f))
        return false;

    if (!p_iputl(flip, f))
        return false;

    if (!p_putc(overlay ? 1 : 0, f))
        return false;

    if (!p_putc(transparent ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_minitile::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    int t=tile;

    if(t==-1)
    {
        switch(specialtile)
        {
        case ssmstSSVINETILE:
        {
            SpriteDefinitionRef vines = curQuest->specialSprites().subscreenVine;
            t = curQuest->getSpriteDefinition(vines).tile * 4;
            break;
        }

        case ssmstMAGICMETER:
        {
            SpriteDefinitionRef mmeters = curQuest->specialSprites().magicMeter;
            t = curQuest->getSpriteDefinition(mmeters).tile * 4;
            break;
        }

        default:
            t=(rand()*100000)%32767;
            break;
        }
    }

    t+=tileoffset;

    if(overlay)
    {
        if(transparent)
        {
            overtiletranslucent8(dest,t,x+xofs,y+yofs,subscreen_cset(misc, colortype1, color1),flip,128);
        }
        else
        {
            overtile8(dest,t,x+xofs,y+yofs,subscreen_cset(misc, colortype1, color1),flip);
        }
    }
    else
    {
        if(transparent)
        {
            puttiletranslucent8(dest,t,x+xofs,y+yofs,subscreen_cset(misc,colortype1, color1),flip,128);
        }
        else
        {
            oldputtile8(dest,t,x+xofs,y+yofs,subscreen_cset(misc,colortype1, color1),flip);
        }
    }
}

subscreen_object_magicgauge::subscreen_object_magicgauge(PACKFILE *f, int &status) : subscreen_object(ssoMAGICGAUGE, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&containernum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&notlast_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&last_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&cap_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&aftercap_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&frames, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&speed, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&delay, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&showflag, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flags, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_magicgauge::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(containernum, f))
        return false;

    if (!p_iputl(notlast_tile, f))
        return false;

    if (!p_iputl(last_tile, f))
        return false;

    if (!p_iputl(cap_tile, f))
        return false;

    if (!p_iputl(aftercap_tile, f))
        return false;

    if (!p_iputl(frames, f))
        return false;

    if (!p_iputl(speed, f))
        return false;

    if (!p_iputl(delay, f))
        return false;

    if (!p_iputl(showflag, f))
        return false;

    if (!p_iputl(flags, f))
        return false;

    return true;
}

void subscreen_object_magicgauge::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    //          void magicgauge(BITMAP *dest,int x,int y, int container, int notlast_tile, int notlast_cset, bool notlast_mod, int last_tile, int last_cset, bool last_mod,
    //                          int cap_tile, int cap_cset, bool cap_mod, int aftercap_tile, int aftercap_cset, bool aftercap_mod, int frames, int speed, int delay, bool unique_last,
    //                          int show)
    magicgauge(dest,x+xofs,y+yofs, containernum, notlast_tile, colortype1, ((flags&1)?1:0), last_tile, color1, ((flags&2)?1:0),
        cap_tile, colortype2, ((flags&4)?1:0), aftercap_tile, color2, ((flags&8)?1:0), frames, speed, delay, ((flags&16)?1:0),
        showflag);
}

subscreen_object_lifegauge::subscreen_object_lifegauge(PACKFILE *f, int &status) : subscreen_object(ssoLIFEGAUGE, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&containernum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&notlast_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&last_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&cap_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&aftercap_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&frames, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&speed, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&delay, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&showflag, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&flags, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_lifegauge::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(containernum, f))
        return false;

    if (!p_iputl(notlast_tile, f))
        return false;

    if (!p_iputl(last_tile, f))
        return false;

    if (!p_iputl(cap_tile, f))
        return false;

    if (!p_iputl(aftercap_tile, f))
        return false;

    if (!p_iputl(frames, f))
        return false;

    if (!p_iputl(speed, f))
        return false;

    if (!p_iputl(delay, f))
        return false;

    if (!p_iputl(showflag, f))
        return false;

    if (!p_iputl(flags, f))
        return false;

    return true;
}

void subscreen_object_lifegauge::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    lifegauge(dest,x+xofs,y+yofs, containernum, notlast_tile, colortype1, ((flags&1)?1:0), last_tile, color1, ((flags&2)?1:0),
        cap_tile, colortype2, ((flags&4)?1:0), aftercap_tile, color2, ((flags&8)?1:0), frames, speed, delay, ((flags&16)?1:0));
}

subscreen_object_none::subscreen_object_none(PACKFILE *f, int &status) : subscreen_object(ssoNONE, f, status)
{

}

bool subscreen_object_none::serializeExtraData(PACKFILE *f)
{
    return true;
}

void subscreen_object_none::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
   
}

subscreen_object_triframe::subscreen_object_triframe(PACKFILE *f, int &status) : subscreen_object(ssoTRIFRAME, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&triframe_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&triframe_cset, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&triforce_tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&triforce_cset, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showframe = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showpieces = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    largepieces = (tmp != 0);
}

bool subscreen_object_triframe::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(triframe_tile, f))
        return false;

    if (!p_iputl(triframe_cset, f))
        return false;

    if (!p_iputl(triforce_tile, f))
        return false;

    if (!p_iputl(triforce_cset, f))
        return false;

    if (!p_putc(showframe ? 1 : 0, f))
        return false;

    if (!p_putc(showpieces ? 1 : 0, f))
        return false;

    if (!p_putc(largepieces ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_triframe::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    //        puttriframe(dest, misc, x, y, triframecolor,                                                             numbercolor,                                                               triframetile,       triframecset,       triforcetile,       triforcecset,       showframe,             showpieces,            largepieces)
    puttriframe(dest, misc, x+xofs, y+yofs, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), triframe_tile, triframe_cset, triforce_tile, triforce_cset, showframe, showpieces, largepieces);
}

subscreen_object_largemap::subscreen_object_largemap(PACKFILE *f, int &status) : subscreen_object(ssoLARGEMAP, f, status)
{
    if (status != qe_OK)
        return;

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showmap = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showlink = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showrooms = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    large = (tmp != 0);
}

bool subscreen_object_largemap::serializeExtraData(PACKFILE *f)
{
    if (!p_putc(showmap ? 1 : 0, f))
        return false;

    if (!p_putc(showlink ? 1 : 0, f))
        return false;

    if (!p_putc(showrooms ? 1 : 0, f))
        return false;

    if (!p_putc(large ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_largemap::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    //putBmap(dest, misc, x, y, showmap,                 showlink,              showrooms,             roomcolor,                                                                 linkcolor,                                                                 large)
    putBmap(dest, misc, x+xofs, y+yofs, showmap, showlink, showrooms, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), large);
}

subscreen_object_bstime::subscreen_object_bstime(PACKFILE *f, int &status) : subscreen_object(ssoBSTIME, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_bstime::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;
    
    if (!p_iputl(textstyle, f))
        return false;

    return true;
}

void subscreen_object_bstime::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    char *ts;

    if(game)
        ts = time_str_short2(game->get_time());
    else
        ts = time_str_short2(0);

    //          textout_shadowed_ex(dest,tempfont,ts,x,y,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
    textout_styled_aligned_ex(dest,tempfont,ts,x,y,textstyle,alignment,subscreen_color(misc, colortype1, color1),subscreen_color(misc, colortype2, color2),subscreen_color(misc, colortype3, color3));
}

subscreen_object_minimap::subscreen_object_minimap(PACKFILE *f, int &status) : subscreen_object(ssoMINIMAP, f, status)
{
    if (status != qe_OK)
        return;

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showmap = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showlink = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    showcompass = (tmp != 0);
}

bool subscreen_object_minimap::serializeExtraData(PACKFILE *f)
{
    if (!p_putc(showmap ? 1 : 0, f))
        return false;

    if (!p_putc(showlink ? 1 : 0, f))
        return false;

    if (!p_putc(showcompass ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_minimap::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    //drawdmap(dest, misc, x, y, showmap,                 showlink,           showcompass,        linkcolor,                                                                 lccolor,                                                                   dccolor)
    drawdmap(dest, misc, x, y, showmap, showlink && !(TheMaps[(DMaps[get_currdmap()].map*MAPSCRS)+get_homescr()].flags7&fNOLINKMARK),
        showcompass && !(DMaps[get_currdmap()].flags&dmfNOCOMPASS), subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), subscreen_color(misc, colortype3, color3));
}

subscreen_object_minimaptitle::subscreen_object_minimaptitle(PACKFILE *f, int &status) : subscreen_object(ssoMINIMAPTITLE, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    needmap = (tmp != 0);
}

bool subscreen_object_minimaptitle::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    if (!p_putc(needmap ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_minimaptitle::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    if(!needmap || has_item(itype_map, get_dlevel()))
    {
        minimaptitle(dest, x+xofs, y+yofs, tempfont, subscreen_color(misc, colortype1, color1),subscreen_color(misc, colortype2, color2),subscreen_color(misc, colortype3, color3), alignment, textstyle);
    }
}

subscreen_object_counters::subscreen_object_counters(PACKFILE *f, int &status) : subscreen_object(ssoCOUNTERS, f, status)
{
    if (status != qe_OK)
    {
        return;
    }

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    usex = (tmp != 0);

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&digits, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_getc(&idigit, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_counters::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_putc(usex ? 1 : 0, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    if (!p_iputl(digits, f))
        return false;

    if (!p_putc(idigit, f))
        return false;

    return true;
}


void subscreen_object_counters::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    defaultcounters(dest, x+xofs, y+yofs, tempfont, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), subscreen_color(misc, colortype3, color3), usex, textstyle, digits, idigit);
}

subscreen_object_sstime::subscreen_object_sstime(PACKFILE *f, int &status) : subscreen_object(ssoSSTIME, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_sstime::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    return true;
}

void subscreen_object_sstime::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    char *ts;

    if(game)
        ts = time_str_med(game->get_time());
    else
        ts = time_str_med(0);

    //            textout_right_ex(dest,tempfont,ts,x,y,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2));
    textout_styled_aligned_ex(dest, tempfont, ts, x+xofs, y+yofs, textstyle, alignment, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), subscreen_color(misc, colortype3, color3));
}

subscreen_object_lifemeter::subscreen_object_lifemeter(PACKFILE *f, int &status) : subscreen_object(ssoLIFEMETER, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&tile, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    bsstyle = (tmp != 0);

    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    threerows = (tmp != 0);
}

bool subscreen_object_lifemeter::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(tile, f))
        return false;

    if (!p_putc(bsstyle ? 1 : 0, f))
        return false;

    if (!p_putc(threerows ? 1 : 0, f))
        return false;

    return true;
}

void subscreen_object_lifemeter::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    lifemeter(dest, x, y, tile, bsstyle);
}

subscreen_object_magicmeter::subscreen_object_magicmeter(PACKFILE *f, int &status) : subscreen_object(ssoMAGICMETER, f, status)
{

}

bool subscreen_object_magicmeter::serializeExtraData(PACKFILE *f)
{
    return true;
}

void subscreen_object_magicmeter::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    magicmeter(dest, x+xofs, y+yofs);
}

subscreen_object_time::subscreen_object_time(PACKFILE *f, int &status) : subscreen_object(ssoTIME, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }
}

bool subscreen_object_time::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    return true;
}

void subscreen_object_time::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    char *ts;

    if(game)
        ts = time_str_med(game->get_time());
    else
        ts = time_str_med(0);

    //            textout_right_ex(dest,tempfont,ts,x,y,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2));
    textout_styled_aligned_ex(dest,tempfont,ts,x+xofs,y+yofs,textstyle,alignment,subscreen_color(misc, colortype1, color1),subscreen_color(misc, colortype2, color2),subscreen_color(misc, colortype3, color3));    
}

subscreen_object_textbox::subscreen_object_textbox(PACKFILE *f, int &status) : subscreen_object(ssoTEXTBOX, f, status)
{
    if (status != qe_OK)
        return;

    if (!p_igetl(&fontnum, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&alignment, f, true))
    {
        status = qe_invalid;
        return;
    }

    if (!p_igetl(&textstyle, f, true))
    {
        status = qe_invalid;
        return;
    }

    byte tmp;
    if (!p_getc(&tmp, f, true))
    {
        status = qe_invalid;
        return;
    }
    wword = (tmp != 0);

    if (!p_igetl(&tabsize, f, true))
    {
        status = qe_invalid;
        return;
    }

    uint32_t textlen;
    if (!p_igetl(&textlen, f, true))
    {
        status = qe_invalid;
        return;
    }

    char *buf = new char[textlen];
    if (!pfread(buf, textlen, f, true))
    {
        delete[] buf;
        status = qe_invalid;
        return;
    }
    thetext = std::string(buf);
    delete[] buf;
}

bool subscreen_object_textbox::serializeExtraData(PACKFILE *f)
{
    if (!p_iputl(fontnum, f))
        return false;

    if (!p_iputl(alignment, f))
        return false;

    if (!p_iputl(textstyle, f))
        return false;

    if (!p_putc(wword ? 1 : 0, f))
        return false;

    if (!p_iputl(tabsize, f))
        return false;

    uint32_t len = thetext.length() + 1;
    if (!p_iputl(len, f))
        return false;

    if (!pfwrite((void *)thetext.c_str(), len, f))
        return false;

    return true;
}

void subscreen_object_textbox::show(BITMAP *dest, const subscreen_group &css, miscQdata *misc, int xofs, int yofs, bool showtime)
{
    FONT *tempfont=ss_font(fontnum);
    //draw_textbox(dest, x, y, w,                 h,                 tempfont, thetext,                     wword,                 tabsize,            alignment,          textstyle,          color,                                                                     shadowcolor,                                                               backcolor)
    draw_textbox(dest, x+xofs, y+yofs, w, h, tempfont, thetext.c_str(), wword, tabsize, alignment, textstyle, subscreen_color(misc, colortype1, color1), subscreen_color(misc, colortype2, color2), subscreen_color(misc, colortype3, color3));
    //          draw_textbox(dest, 0, 0, 200, 50, sfont, "This is a test", 1, 4, 0, 0, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
}

static subscreen_group *z3_active_a = NULL;

const subscreen_group &get_z3_active_a()
{
    if (z3_active_a)
        return *z3_active_a;

    z3_active_a = new subscreen_group;
    z3_active_a->ss_type = sstACTIVE;
    z3_active_a->ss_name = std::string("Z3 Active Template (A)");
    //                                                                          pos     x       y       w       h       colortype1      color1          colortype2      color2              colortype3      color3          custom fields begin here
    z3_active_a->ss_objects.push_back(new subscreen_object_rect(                pos,    0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,   ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    8,      0,      19,     12,     7,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    8,      96,     19,     9,      8,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    168,    0,      10,     6,      7,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    168,    48,     10,     7,      8,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    168,    104,    10,     8,      8,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    8,      0,      19,     12,     7,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    8,      96,     19,     9,      8,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    168,    0,      10,     6,      7,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    168,    48,     10,     7,      8,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_2x2frame(            pos,    168,    104,    10,     8,      8,              0,              0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    176,    120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_sword, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    200,    120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    224,    120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    200,    144,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    28,     120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    52,     120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    76,     120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    100,    120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    124,    120,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    28,     144,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    52,     144,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    76,     144,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    100,    144,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    124,    144,    0,      0,      0,              0,              0,              0,                  0,              0,              itype_quiver, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_selector2(           pos,    0,      0,      0,      0,      0,              0,              0,              0,                  0,              0,              0, 0, false, false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    32,     16,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_bowandarrow, true, 0, 10, 5, 14, 1, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_counter(             pos,    44,     26,     0,      0,      ssctMISC,       ssctTEXT,       0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 0, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    56,     16,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_brang, true, 1, 11, 6, 0, 2, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    80,     16,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_hookshot, true, 2, 12, 7, 1, 3, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    104,    16,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_bomb, true, 3, 13, 8, 2, 4, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_counter(             pos,    116,    26,     0,      0,      ssctMISC,       ssctTEXT,       0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 0, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    128,    16,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_letterpotion, true, 4, 14, 9, 3, 5, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    32,     40,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_wand, true, 5, 0, 10, 4, 6, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    56,     40,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_sbomb, true, 6, 1, 11, 5, 7, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_counter(             pos,    68,     50,     0,      0,      ssctMISC,       ssctTEXT,       0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 0, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    80,     40,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_dinsfire, true, 7, 2, 12, 6, 8, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    104,    40,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_nayruslove, true, 8, 3, 13, 7, 9, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    128,    40,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_faroreswind, true, 9, 4, 14, 8, 10, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    32,     64,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_candle, true, 10, 5, 0, 9, 11, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    56,     64,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_hammer, true, 11, 6, 1, 10, 12, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    80,     64,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_whistle, true, 12, 7, 2, 11, 13, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    104,    64,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_lens, true, 13, 8, 3, 12, 14, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_currentitem(         pos,    128,    64,     0,      0,      0,              0,              0,              0,                  0,              0,              itype_bait, true, 14, 9, 4, 13, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    177,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    181,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    185,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    189,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    193,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    197,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    201,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    205,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    209,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    213,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    217,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    221,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    225,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    229,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    233,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_line(                pos,    237,    139,    2,      1,      ssctMISC,       ssctTEXT,       0,              0,                  0,              0,              false, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_buttonitem(          pos,    200,    8,      0,      0,      0,              0,              0,              0,                  0,              0,              1, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_counter(             pos,    212,    18,     0,      0,      ssctMISC,       ssctTEXT,       0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_counter(             pos,    212,    18,     0,      0,      ssctMISC,       ssctTEXT,       0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_counter(             pos,    212,    18,     0,      0,      ssctMISC,       ssctTEXT,       0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_a->ss_objects.push_back(new subscreen_object_selecteditemname(    pos,    173,    24,     70,     16,     ssctMISC,       ssctTEXT,       ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfPROP, sstaCENTER, sstsSHADOWED, true, 4));
    z3_active_a->ss_objects.push_back(new subscreen_object_rect(                pos,    24,     1,      16,     6,      0,              0,              0,              0,                  0,              0,              true, false));
    z3_active_a->ss_objects.push_back(new subscreen_object_text(                pos,    24,     1,      0,      0,      ssctMISC,       ssctTEXT,       ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("ITEM")));
    z3_active_a->ss_objects.push_back(new subscreen_object_text(                pos,    17,     105,    0,      0,      ssctMISC,       ssctTEXT,       ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("INVENTORY")));
    z3_active_a->ss_objects.push_back(new subscreen_object_text(                pos,    177,    57,     0,      0,      ssctMISC,       ssctTEXT,       ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("TRIFORCE")));
    z3_active_a->ss_objects.push_back(new subscreen_object_text(                pos,    177,    113,    0,      0,      ssctMISC,       ssctTEXT,       ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("EQUIPMENT")));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    192,    64,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 0, true, false, 1));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    208,    64,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 1, true, false, 2));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    176,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 0, true, false, 3));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    224,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 1, true, false, 4));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    192,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 2, true, false, 5));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    192,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 1, true, false, 6));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    208,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 3, true, false, 7));
    z3_active_a->ss_objects.push_back(new subscreen_object_triforce(            pos,    208,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET, 0,            0,                  0,              0,              0, 0, true, false, 8));
    z3_active_a->ss_objects.push_back(new subscreen_object_tileblock(           pos,    16,     8,      1,      1,      1,              0,              0,              0,                  0,              0,              28201, 0, true, false));
    return *z3_active_a;
}

static subscreen_group *z3_active_ab = NULL;

const subscreen_group &get_z3_active_ab()
{
    if (z3_active_ab)
        return *z3_active_ab;

    z3_active_ab = new subscreen_group;
    z3_active_ab->ss_type = sstACTIVE;
    z3_active_ab->ss_name = std::string("Z3 Active Template (AB)");
    //                                                                          pos     x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
    z3_active_ab->ss_objects.push_back(new subscreen_object_rect(               pos,    0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    8,      0,      19,     12,     7,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    8,      96,     19,     9,      8,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    168,    0,      10,     6,      7,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    168,    48,     10,     7,      8,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    168,    104,    10,     8,      8,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    8,      0,      19,     12,     7,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    8,      96,     19,     9,      8,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    168,    0,      10,     6,      7,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    168,    48,     10,     7,      8,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    168,    104,    10,     8,      8,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    176,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sword, true, 15, 14, 0, 14, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    200,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    224,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    200,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    28,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    52,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    76,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    100,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    124,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    28,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    52,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    76,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    100,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    124,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_quiver, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_selector2(          pos,    0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    32,     16,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bowandarrow, true, 0, 10, 5, 15, 1, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_counter(            pos,    44,     26,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 0, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    56,     16,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 1, 11, 6, 0, 2, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    80,     16,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 2, 12, 7, 1, 3, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    104,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 3, 13, 8, 2, 4, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_counter(            pos,    116,    26,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 0, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    128,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 4, 14, 9, 3, 5, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    32,     40,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 5, 0, 10, 4, 6, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    56,     40,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 6, 1, 11, 5, 7, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_counter(            pos,    68,     50,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 0, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    80,     40,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_dinsfire, true, 7, 2, 12, 6, 8, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    104,    40,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_nayruslove, true, 8, 3, 13, 7, 9, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    128,    40,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_faroreswind, true, 9, 4, 14, 8, 10, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    32,     64,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 10, 5, 0, 9, 11, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    56,     64,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 6, 1, 10, 12, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    80,     64,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 12, 7, 2, 11, 13, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    104,    64,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 13, 8, 3, 12, 14, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_currentitem(        pos,    128,    64,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 14, 9, 4, 13, 15, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_selector1(          pos,    0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    177,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    181,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    185,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    189,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    193,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    197,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    201,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    205,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    209,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    213,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    217,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    221,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    225,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    229,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    233,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_line(               pos,    237,    139,    2,      1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_buttonitem(         pos,    200,    8,      0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_counter(            pos,    212,    18,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_counter(            pos,    212,    18,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_counter(            pos,    212,    18,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_active_ab->ss_objects.push_back(new subscreen_object_selecteditemname(   pos,    173,    24,     70,     16,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfPROP, sstaCENTER, sstsSHADOWED, true, 4));
    z3_active_ab->ss_objects.push_back(new subscreen_object_rect(               pos,    24,     1,      16,     6,      0,              0,                  0,              0,                  0,              0,              true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_text(               pos,    24,     1,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("ITEM")));
    z3_active_ab->ss_objects.push_back(new subscreen_object_tileblock(          pos,    16,     73,     1,      1,      1,              0,                  0,              0,                  0,              0,              28201, 0, true, false));
    z3_active_ab->ss_objects.push_back(new subscreen_object_text(               pos,    17,     105,    0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("INVENTORY")));
    z3_active_ab->ss_objects.push_back(new subscreen_object_text(               pos,    177,    57,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("TRIFORCE")));
    z3_active_ab->ss_objects.push_back(new subscreen_object_text(               pos,    177,    113,    0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfZ3SMALL, sstaLEFT, sstsSHADOWED, std::string("EQUIPMENT")));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    192,    64,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 0, true, false, 1));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    208,    64,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 1, true, false, 2));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    176,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 0, true, false, 3));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    224,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 1, true, false, 4));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    192,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 2, true, false, 5));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    192,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 1, true, false, 6));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    208,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 3, true, false, 7));
    z3_active_ab->ss_objects.push_back(new subscreen_object_triforce(           pos,    208,    80,     1,      1,      ssctMISC,       sscsTRIFORCECSET,   0,              0,                  0,              0,              0, 0, true, false, 8));
    z3_active_ab->ss_objects.push_back(new subscreen_object_tileblock(          pos,    16,     8,      1,      1,      1,              0,                  0,              0,                  0,              0,              28200, 0, true, false));

    return *z3_active_ab;
}

static subscreen_group *z3_passive_a = NULL;

const subscreen_group &get_z3_passive_a()
{
    if (z3_passive_a)
        return *z3_passive_a;

    z3_passive_a = new subscreen_group;
    z3_passive_a->ss_type = sstPASSIVE;
    z3_passive_a->ss_name = std::string("Z3 Passive Template (A)");
    //                                                                          pos     x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
    z3_passive_a->ss_objects.push_back(new subscreen_object_clear(              pos,    0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
    z3_passive_a->ss_objects.push_back(new subscreen_object_minitile(           pos,    74,     6,      0,      0,      5,              0,                  0,              0,                  0,              0,              101069, 0, 0, 0, true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_minitile(           pos,    101,    6,      0,      0,      0,              0,                  0,              0,                  0,              0,              101369, 0, 0, 0, true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_minitile(           pos,    126,    7,      0,      0,      1,              0,                  0,              0,                  0,              0,              101683, 0, 0, 0, true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_minitile(           pos,    144,    6,      0,      0,      1,              0,                  0,              0,                  0,              0,              101212, 0, 0, 0, true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     8,      8,      8,      5,              5,                  5,              0,                  0,              0,              9, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     12,     8,      8,      5,              5,                  5,              0,                  0,              0,              8, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     16,     8,      8,      5,              5,                  5,              0,                  0,              0,              7, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     20,     8,      8,      5,              5,                  5,              0,                  0,              0,              6, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     24,     8,      8,      5,              5,                  5,              0,                  0,              0,              5, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     28,     8,      8,      5,              5,                  5,              0,                  0,              0,              4, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     32,     8,      8,      5,              5,                  5,              0,                  0,              0,              3, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     36,     8,      8,      5,              5,                  5,              0,                  0,              0,              2, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     40,     8,      8,      5,              5,                  5,              0,                  0,              0,              1, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     44,     8,      8,      5,              5,                  5,              5,                  0,              0,              0, 100322, 100260, 100322, 100322, 1, 0, 0, 2, 0));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     8,      8,      8,      5,              5,                  5,              0,                  0,              0,              9, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     12,     8,      8,      5,              5,                  5,              0,                  0,              0,              8, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     16,     8,      8,      5,              5,                  5,              0,                  0,              0,              7, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     20,     8,      8,      5,              5,                  5,              0,                  0,              0,              6, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     24,     8,      8,      5,              5,                  5,              0,                  0,              0,              5, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     28,     8,      8,      5,              5,                  5,              0,                  0,              0,              4, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     32,     8,      8,      5,              5,                  5,              0,                  0,              0,              3, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     36,     8,      8,      5,              5,                  5,              0,                  0,              0,              2, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     40,     8,      8,      5,              5,                  5,              0,                  0,              0,              1, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     44,     8,      8,      5,              5,                  5,              0,                  0,              0,              0, 100323, 100260, 100262, 100263, 1, 0, 0, 2, 0));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    16,     44,     8,      8,      5,              5,                  5,              5,                  0,              0,              0, 100402, 100260, 100322, 100322, 1, 0, 0, 1, 0));
    z3_passive_a->ss_objects.push_back(new subscreen_object_magicgauge(         pos,    24,     44,     8,      8,      5,              5,                  5,              0,                  0,              0,              0, 100403, 100260, 100262, 100263, 1, 0, 0, 1, 0));
    z3_passive_a->ss_objects.push_back(new subscreen_object_2x2frame(           pos,    32,     4,      4,      4,      7,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_buttonitem(         pos,    40,     12,     16,     16,     0,              0,                  0,              0,                  0,              0,              1, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    52,     22,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    52,     22,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    52,     22,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_a->ss_objects.push_back(new subscreen_object_text(               pos,    200,    6,      0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, std::string("LIFE")));
    z3_passive_a->ss_objects.push_back(new subscreen_object_rect(               pos,    178,    8,      12,     2,      0,              15,                 0,              15,                 0,              0,              true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_line(               pos,    179,    8,      10,     1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_rect(               pos,    210,    8,      12,     2,      0,              15,                 0,              15,                 0,              0,              true, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_line(               pos,    211,    8,      10,     1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    169,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              1, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    177,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              2, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    185,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              3, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    193,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              4, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    201,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              5, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    209,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              6, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    217,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              7, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    225,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              8, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    169,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              9, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    177,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              10, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    185,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              11, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    193,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              12, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    201,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              13, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    209,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              14, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    217,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              15, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    225,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              16, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    169,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              17, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    177,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              18, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    185,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              19, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    193,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              20, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    201,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              21, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    209,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              22, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    217,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              23, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_lifegauge(          pos,    225,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              24, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    77,     15,     25,     9,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 3, '~', 1, sscRUPEES, 0, 0, ItemDefinitionRef()));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    105,    15,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 2, '~', 1, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    129,    15,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 2, '~', 1, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_passive_a->ss_objects.push_back(new subscreen_object_counter(            pos,    148,    15,     9,      9,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 1, '~', 1, sscGENKEYMAGIC, 0, 0, ItemDefinitionRef()));

    return *z3_passive_a;
}


static subscreen_group *z3_passive_ab = NULL;

const subscreen_group &get_z3_passive_ab()
{
    if (z3_passive_ab)
        return *z3_passive_ab;

    z3_passive_ab = new subscreen_group;
    z3_passive_ab->ss_type = sstPASSIVE;
    z3_passive_ab->ss_name = std::string("Z3 Passive Template (AB)");
    //                                                                          pos     x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
    z3_passive_ab->ss_objects.push_back(new subscreen_object_clear(             pos,    0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_minitile(          pos,    80,     6,      0,      0,      5,              0,                  0,              0,                  0,              0,              101069, 0, 0, 0, true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_minitile(          pos,    107,    6,      0,      0,      0,              0,                  0,              0,                  0,              0,              101369, 0, 0, 0, true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_minitile(          pos,    132,    7,      0,      0,      1,              0,                  0,              0,                  0,              0,              101683, 0, 0, 0, true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_minitile(          pos,    150,    6,      0,      0,      1,              0,                  0,              0,                  0,              0,              101212, 0, 0, 0, true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     8,      8,      8,      5,              5,                  5,              0,                  0,              0,              9, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     12,     8,      8,      5,              5,                  5,              0,                  0,              0,              8, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     16,     8,      8,      5,              5,                  5,              0,                  0,              0,              7, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     20,     8,      8,      5,              5,                  5,              0,                  0,              0,              6, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     24,     8,      8,      5,              5,                  5,              0,                  0,              0,              5, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     28,     8,      8,      5,              5,                  5,              0,                  0,              0,              4, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     32,     8,      8,      5,              5,                  5,              0,                  0,              0,              3, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     36,     8,      8,      5,              5,                  5,              0,                  0,              0,              2, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     40,     8,      8,      5,              5,                  5,              0,                  0,              0,              1, 100320, 100640, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     44,     8,      8,      5,              5,                  5,              5,                  0,              0,              0, 100322, 100260, 100322, 100322, 1, 0, 0, 2, 0));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     8,      8,      8,      5,              5,                  5,              0,                  0,              0,              9, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     12,     8,      8,      5,              5,                  5,              0,                  0,              0,              8, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     16,     8,      8,      5,              5,                  5,              0,                  0,              0,              7, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     20,     8,      8,      5,              5,                  5,              0,                  0,              0,              6, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     24,     8,      8,      5,              5,                  5,              0,                  0,              0,              5, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     28,     8,      8,      5,              5,                  5,              0,                  0,              0,              4, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     32,     8,      8,      5,              5,                  5,              0,                  0,              0,              3, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     36,     8,      8,      5,              5,                  5,              0,                  0,              0,              2, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     40,     8,      8,      5,              5,                  5,              0,                  0,              0,              1, 100480, 100720, 100262, 100263, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     44,     8,      8,      5,              5,                  5,              0,                  0,              0,              0, 100323, 100260, 100262, 100263, 1, 0, 0, 2, 0));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    16,     44,     8,      8,      5,              5,                  5,              5,                  0,              0,              0, 100402, 100260, 100322, 100322, 1, 0, 0, 1, 0));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_magicgauge(        pos,    24,     44,     8,      8,      5,              5,                  5,              0,                  0,              0,              0, 100403, 100260, 100262, 100263, 1, 0, 0, 1, 0));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_2x2frame(          pos,    32,     26,     4,      4,      8,              0,                  0,              0,                  0,              0,              20040, 0, false, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_2x2frame(          pos,    32,     0,      4,      4,      7,              0,                  0,              0,                  0,              0,              20040, 0, true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_buttonitem(        pos,    40,     8,      16,     16,     0,              0,                  0,              0,                  0,              0,              0, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    52,     18,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    52,     18,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    52,     18,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_buttonitem(        pos,    40,     34,     16,     16,     0,              0,                  0,              0,                  0,              0,              1, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    52,     44,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    52,     44,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    52,     44,     9,      7,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfZ3SMALL, sstaCENTER, sstsOUTLINEDPLUS, 2, '~', 2, sscSBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_text(              pos,    200,    6,      0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, std::string("LIFE")));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_rect(              pos,    178,    8,      12,     2,      0,              15,                 0,              15,                 0,              0,              true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_line(              pos,    179,    8,      10,     1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_rect(              pos,    210,    8,      12,     2,      0,              15,                 0,              15,                 0,              0,              true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_line(              pos,    211,    8,      10,     1,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              false, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    169,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              1, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    177,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              2, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    185,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              3, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    193,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              4, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    201,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              5, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    209,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              6, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    217,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              7, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    225,    15,     8,      8,      1,              1,                  1,              1,                  0,              0,              8, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    169,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              9, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    177,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              10, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    185,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              11, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    193,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              12, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    201,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              13, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    209,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              14, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    217,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              15, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    225,    23,     8,      8,      1,              1,                  1,              1,                  0,              0,              16, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    169,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              17, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    177,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              18, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    185,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              19, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    193,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              20, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    201,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              21, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    209,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              22, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    217,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              23, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_lifegauge(         pos,    225,    31,     8,      8,      1,              1,                  1,              1,                  0,              0,              24, 100240, 100240, 100261, 100261, 1, 0, 0, 0, 3));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    83,     15,     25,     9,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 3, '~', 1, sscRUPEES, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    111,    15,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 2, '~', 1, sscBOMBS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    135,    15,     0,      0,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 2, '~', 1, sscARROWS, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_counter(           pos,    154,    15,     9,      9,      ssctMISC,       ssctTEXT,           0,              15,                 ssctSYSTEM,     -1,             ssfSS4, sstaCENTER, sstsSHADOWED, 1, '~', 1, sscGENKEYMAGIC, 0, 0, ItemDefinitionRef()));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_rect(              pos,    59,     11,     4,      10,     0,              0,                  0,              0,                  0,              0,              true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_rect(              pos,    59,     37,     4,      10,     0,              0,                  0,              0,                  0,              0,              true, false));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_text(              pos,    61,     13,     8,      8,      0,              1,                  0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
    z3_passive_ab->ss_objects.push_back(new subscreen_object_text(              pos,    61,     38,     8,      8,      0,              1,                  0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));

    return *z3_passive_ab;
}

// guaranteed to be NULL-initialized
static subscreen_group *default_subscreen_active[ssdtMAX][2];

const subscreen_group &get_default_subscreen_active(int style, int world)
{
    if (default_subscreen_active[style][world])
        return *default_subscreen_active[style][world];

    if (style == ssdtOLD)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Old Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            68,     18,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            56,     34,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            64,     66,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("USE B BUTTON")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            64,     74,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("FOR THIS")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            128,    154,    0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            80,     98,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            120,    34,     13,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   notscrolling,   64,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   128,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   148,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   160,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   176,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   192,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   204,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   172,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   180,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   128,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 4, 4, 7, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   152,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 5, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   176,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 6, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   200,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 7, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   128,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 0, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   152,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 1, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   176,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 2, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   200,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 3, 6, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Old Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            68,     18,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            56,     34,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            64,     66,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("USE B BUTTON")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            64,     74,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("FOR THIS")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            96,     82,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, true));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            52,     90,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("MAP")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            52,     130,    0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("COMPASS")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   44,     106,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   44,     146,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            120,    34,     13,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   notscrolling,   64,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   128,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   148,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   160,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   176,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   192,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   204,    18,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));;
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   172,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   180,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   128,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 4, 4, 7, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   152,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 5, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   176,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 6, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   200,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 7, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   128,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 0, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   152,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 1, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   176,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 2, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  notscrolling,   200,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 3, 6, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }        
    }
    else if (style == ssdtNEWSUBSCR)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("New Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            68,     18,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     34,     14,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            128,    154,    0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            80,     98,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            136,    18,     13,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            20,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            36,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            52,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            68,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            100,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            20,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            36,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            52,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            68,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            100,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            188,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            196,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            144,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 11, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            192,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            216,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            144,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            192,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            216,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            144,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 7, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            192,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            216,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("New Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            68,     18,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     34,     14,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            96,     82,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, true));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            52,     90,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("MAP")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            52,     130,    0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("COMPASS")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            44,     106,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            44,     146,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            136,    18,     13,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            20,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            36,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            52,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            68,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            100,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            20,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            36,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            52,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            68,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            100,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            188,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            196,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            144,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 11, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            192,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            216,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            144,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            192,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            216,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            144,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 7, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            192,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            216,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
    }
    else if (style == ssdtREV2)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Revision 2 Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            62,     18,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            6,      34,     14,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            128,    154,    0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            80,     98,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            118,    18,     16,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            30,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            46,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            78,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            94,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            30,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            46,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            78,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            94,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            170,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            178,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            126,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 14, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            150,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            174,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            198,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 12, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            222,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_dinsfire, true, 12, 14, 13, 3, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            126,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 12, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            150,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            174,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            198,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 13, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            222,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_faroreswind, true, 13, 12, 14, 7, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            126,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 13, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            150,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            174,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            198,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 14, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            222,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_nayruslove, true, 14, 13, 12, 11, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Revision 2 Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            62,     18,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            6,      34,     14,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            16,     98,     9,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            96,     82,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, true));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     106,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            44,     106,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            64,     106,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bosskey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            118,    18,     16,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            30,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            46,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            78,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            94,     42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            30,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            46,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            78,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            94,     58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            170,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            178,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            126,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 14, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            150,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            174,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            198,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 12, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            222,    26,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_dinsfire, true, 12, 14, 13, 3, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            126,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 12, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            150,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            174,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            198,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 13, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            222,    42,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_faroreswind, true, 13, 12, 14, 7, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            126,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 13, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            150,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            174,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            198,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 14, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            222,    58,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_nayruslove, true, 14, 13, 12, 11,0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
    }
    else if (style == ssdtBSZELDA)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("BS Zelda Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     24,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     80,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     80,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     96,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     40,     13,     4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            136,    48,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            136,    56,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     112,    14,     7,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     88,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            48,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            96,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            64,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            76,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 4, 4, 7, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 5, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 6, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 7, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 0, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 1, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 2, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 3, 6, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    25,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    25,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    25,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    32,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    26,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("BS Zelda Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     24,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     80,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     80,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     96,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     40,     13,     4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            160,    40,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            192,    40,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            144,    88,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            176,    72,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsNORMAL, std::string("Compass")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            208,    72,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsNORMAL, std::string("Map")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            200,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     112,    14,     7,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     88,     0,      0,      0,              0,                  0,              0,                  0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            48,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            96,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            64,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            76,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 4, 4, 7, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, 1, 5, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 6, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 7, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 0, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 1, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 2, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 3, 6, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    25,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    25,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    25,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    32,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    26,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
    }
    else if (style == ssdtBSZELDAMODIFIED)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Modified BS Zelda Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     24,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     80,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     80,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     96,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     40,     15,     4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            136,    48,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            136,    56,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     112,    14,     7,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     88,     0,      0,      0,              0,                  0,              0,                  0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            76,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 4, 4, 7, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 5, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 6, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 7, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 0, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 1, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 2, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 3, 6, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    25,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    25,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    25,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    32,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    26,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Modified BS Zelda Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    24,     0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     24,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     80,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     80,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     96,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     40,     15,     4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            160,    40,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            192,    40,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            144,    88,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            176,    72,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsNORMAL, std::string("Compass")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            208,    72,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsNORMAL, std::string("Map")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            200,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     112,    14,     7,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     88,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            76,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 4, 4, 7, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 5, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 6, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    120,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 7, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 0, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 1, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 2, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 3, 6, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    25,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    25,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    25,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    32,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    26,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
    }
    else if (style == ssdtBSZELDAENHANCED)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Enhanced BS Zelda Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     72,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     72,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     80,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     20,     15,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            136,    48,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            136,    56,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     104,    14,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     80,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            76,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 11, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 7, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    9,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    9,      44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    9,      1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    16,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    10,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("Enhanced BS Zelda Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     72,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     72,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     80,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     20,     15,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            160,    40,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            192,    40,     4,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            144,    88,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            176,    72,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsNORMAL, std::string("Compass")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            208,    72,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsNORMAL, std::string("Map")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            200,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            168,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            20,     104,    14,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     80,     0,      0,      0,              0,                  0,              0,                  0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            76,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            84,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 11, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 3, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            32,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 7, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            80,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    9,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    9,      44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    9,      1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    16,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    10,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }       
    }
    else if (style == ssdtBSZELDACOMPLETE)
    {
        if (world == 0)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("BS Zelda Complete Active Template (Overworld)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     72,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     72,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     80,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     20,     15,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            136,    48,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("TRIFORCE")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_triframe(     pos,            136,    56,     0,      0,      ssctMISC,       ssctTRIFRAMECOLOR,  ssctMISC,       ssctCAPTION,        0,              0,              0, 0, 0, 0, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            4,      104,    16,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     80,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            58,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            68,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 14, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            38,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            86,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 12, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            110,    112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_dinsfire, true, 12, 14, 13, 3, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 12, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            38,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            86,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 13, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            110,    128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_faroreswind, true, 13, 12, 14, 7, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 13, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            38,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            86,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 14, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            110,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_nayruslove, true, 14, 13, 12, 11, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    9,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    9,      44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    9,      1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    16,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    10,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
        else if (world == 1)
        {
            default_subscreen_active[style][world] = new subscreen_group;
            default_subscreen_active[style][world]->ss_type = sstACTIVE;
            default_subscreen_active[style][world]->ss_name = std::string("BS Zelda Complete Active Template (Dungeon)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         pos,            0,      0,      256,    168,    ssctMISC,       ssctSUBSCRBG,       ssctMISC,       ssctSUBSCRBG,       0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            0,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            8,      8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 1, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            88,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 2, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            96,     8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 3, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            104,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 4, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            112,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 5, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            120,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 6, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            128,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 7, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            136,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 8, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            144,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 9, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            152,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 10, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_minitile(     pos,            248,    8,      0,      0,      ssctMISC,       sscsSSVINECSET,     0,              0,                  0,              0,              -1, ssmstSSVINETILE, 11, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            51,     8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("INVENTORY")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            28,     72,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            40,     72,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     80,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("B Button")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         pos,            56,     88,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfSS2, sstaLEFT, sstsNORMAL, std::string("to use item")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            12,     20,     15,     6,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            152,    40,     10,     4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_largemap(     pos,            144,    88,     0,      0,      ssctMISC,       ssctBMAPFG,         ssctMISC,       ssctLINKDOT,        0,              0,              true, true, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_none(         pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            160,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_map, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            184,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_compass, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            208,    48,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bosskey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_2x2frame(     pos,            4,      104,    16,     8,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_buttonitem(   pos,            32,     80,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_raft, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_book, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ring, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_ladder, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_magickey, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     28,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bracelet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            24,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_shield, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            40,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wallet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            56,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_amulet, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            72,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_flippers, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            88,     44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_boots, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            104,    44,     0,      0,      0,              0,                  0,              0,                  0,              0,              itype_heartpiece, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector2(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            58,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            68,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bow, true, -1, 0, 0, 0, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_brang, true, 0, 8, 4, 14, 1, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            38,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bomb, true, 1, 9, 5, 0, 2, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_arrow, false, 2, 10, 6, 1, 3, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            86,     112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_candle, true, 3, 11, 7, 2, 12, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            110,    112,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_dinsfire, true, 12, 14, 13, 3, 4, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_whistle, true, 4, 0, 8, 12, 5, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            38,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_bait, true, 5, 1, 9, 4, 6, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_letterpotion, true, 6, 2, 10, 5, 7, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            86,     128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_wand, true, 7, 3, 11, 6, 13, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            110,    128,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_faroreswind, true, 13, 12, 14, 7, 8, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            14,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hookshot, true, 8, 4, 0, 13, 9, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            38,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_sbomb, true, 9, 5, 1, 8, 10, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            62,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_lens, true, 10, 6, 2, 9, 11, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            86,     144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_hammer, true, 11, 7, 3, 10, 14, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_currentitem(  pos,            110,    144,    0,      0,      0,              0,                  0,              0,                  0,              0,              itype_nayruslove, true, 14, 13, 12, 11, 0, ItemDefinitionRef()));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_bstime(       sspDOWN,        208,    9,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_rect(         sspDOWN,        160,    9,      44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        204,    9,      1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_line(         sspDOWN,        160,    16,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_text(         sspDOWN,        161,    10,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));
            default_subscreen_active[style][world]->ss_objects.push_back(new subscreen_object_selector1(    pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0,              0, 0, false, false, false));

            return *default_subscreen_active[style][world];
        }
    }
    assert(!"Invalid subscreen access (should be unreachable)");
    exit(-1);
}

static subscreen_group *default_subscreen_passive[ssdtMAX][2];

const subscreen_group &get_default_subscreen_passive(int style, int magic)
{
    if (default_subscreen_passive[style][magic])
        return *default_subscreen_passive[style][magic];

    if (style == ssdtOLD)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Old Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, true, sstsNORMAL, 3, 'A'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            120,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            144,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  sspUP,          124,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  sspUP,          148,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            132,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            156,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            208,    12,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("-LIFE-")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            176,    12,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            176,    44,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));

            return *default_subscreen_passive[style][magic];
        }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Old Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, true, sstsNORMAL, 3, 'A'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            120,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            144,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  sspUP,          124,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  sspUP,          148,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            132,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            156,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            208,    16,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("-LIFE-")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            176,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));

            return *default_subscreen_passive[style][magic];
        }
    }
    else if (style == ssdtNEWSUBSCR)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("New Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, true, sstsNORMAL, 3, 'A'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            120,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            144,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            124,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            148,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            132,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            156,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            208,    12,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("-LIFE-")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            176,    12,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            176,    44,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));

            return *default_subscreen_passive[style][magic];
        }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("New Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, true, sstsNORMAL, 3, 'A'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            120,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            144,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            124,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            148,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            132,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            156,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            208,    16,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("-LIFE-")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            176,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));

            return *default_subscreen_passive[style][magic];
        }
    }
    else if (style == ssdtREV2)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Revision 2 Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, true, sstsNORMAL, 3, 'A'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            120,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            144,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            124,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            148,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            132,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            156,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            208,    12,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("-LIFE-")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            176,    12,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            176,    44,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));

            return *default_subscreen_passive[style][magic];
        }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Revision 2 Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  0,              0,              ssfZELDA, sstaCENTER, sstsNORMAL, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, true, sstsNORMAL, 3, 'A'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            120,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            144,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            124,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            148,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            132,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            156,    16,     0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctMISC,       ssctSUBSCRBG,   ssfZELDA, sstaCENTER, sstsNORMAL, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            208,    16,     0,      0,      ssctMISC,       ssctCAPTION,        0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaCENTER, sstsNORMAL, std::string("-LIFE-")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            176,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));

            return *default_subscreen_passive[style][magic];
        }
    }
    else if (style == ssdtBSZELDA)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("BS Zelda Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            220,    8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            188,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            188,    40,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("BS Zelda Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            216,    16,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            184,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
    }
    else if (style == ssdtBSZELDAMODIFIED)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Modified BS Zelda Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            220,    8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            188,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            188,    40,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Modified BS Zelda Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            216,    16,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            184,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
    }
    else if (style == ssdtBSZELDAENHANCED)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Enhanced BS Zelda Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            220,    8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            188,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            188,    40,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
       }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("Enhanced BS Zelda Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            216,    16,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            184,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
    }
    else if (style == ssdtBSZELDACOMPLETE)
    {
        if (magic == 0)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("BS Zelda Complete Passive Template (Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            220,    8,      0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            188,    16,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_magicmeter(  pos,            188,    40,     0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
        else if (magic == 1)
        {
            default_subscreen_passive[style][magic] = new subscreen_group;
            default_subscreen_passive[style][magic]->ss_type = sstPASSIVE;
            default_subscreen_passive[style][magic]->ss_name = std::string("BS Zelda Complete Passive Template (No Magic)");
            //                                                                                              pos             x       y       w       h       colortype1      color1              colortype2      color2              colortype3      color3          custom fields begin here
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_clear(       pos,            0,      0,      0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimap(     pos,            8,      8,      0,      0,      ssctMISC,       ssctLINKDOT,        ssctMISC,       ssctCOMPASSLT,      ssctMISC,       ssctCOMPASSDK,  true, true, true));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_minimaptitle(pos,            48,     0,      0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS3, sstaCENTER, sstsSHADOWED, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_counters(    pos,            88,     16,     32,     32,     ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, true, sstsSHADOWED, 4, '~'));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_sstime(      pos,            240,    0,      0,      0,      ssctMISC,       ssctTEXT,           0,              0,                  ssctSYSTEM,     -1,             ssfZELDA, sstaRIGHT, sstsNORMAL));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            132,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_2x2frame(    pos,            156,    16,     3,      4,      ssctMISC,       sscsBLUEFRAMECSET,  0,              0,                  0,              0,              0, 0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            136,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              1, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_buttonitem(  pos,            160,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            144,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWEDU, std::string("B")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            168,    16,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsOUTLINED8, std::string("A")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        pos,            163,    15,     174,    1,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        pos,            216,    16,     0,      0,      ssctMISC,       ssctCAPTION,        ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaCENTER, sstsSHADOWED, std::string("-  life  -")));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_lifemeter(   pos,            184,    24,     0,      0,      0,              0,                  0,              0,                  0,              0,              0, true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_none(        pos,            0,      0,      0,      0,      0,              0,                  0,              0,                  0,              0               ));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_bstime(      sspUP,          208,    48,     0,      0,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctSUBSCRSHADOW,   ssctSYSTEM,     -1,             ssfSS1, sstaLEFT, sstsSHADOWED));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_rect(        sspUP,          160,    48,     44,     7,      ssctMISC,       ssctTEXT,           ssctMISC,       ssctTEXT,           0,              0,              true, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          204,    48,     1,      8,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_line(        sspUP,          160,    55,     45,     1,      ssctMISC,       ssctSUBSCRSHADOW,   0,              0,                  0,              0,              false, false));
            default_subscreen_passive[style][magic]->ss_objects.push_back(new subscreen_object_text(        sspUP,          161,    49,     0,      0,      ssctMISC,       ssctSUBSCRBG,       0,              0,                  ssctSYSTEM,     -1,             ssfZTIME, sstaLEFT, sstsNORMAL, std::string("ZELDA TIME")));

            return *default_subscreen_passive[style][magic];
        }
    }
    assert(!"invalid passive subscreen type (should be unreachable)");
    exit(-1);
}

void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int textstyle, int alignment, int color, int shadow, int bg)
{
    switch(alignment)
    {
    case sstaRIGHT:
        switch(textstyle)
        {
        case sstsSHADOW:
            textout_shadow_right_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsSHADOWU:
            textout_shadow_right_u_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINE8:
            textout_shadow_right_o_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINEPLUS:
            textout_shadow_right_plus_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINEX:
            textout_shadow_right_x_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsSHADOWED:
            textout_shadowed_right_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsSHADOWEDU:
            textout_shadowed_right_u_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINED8:
            textout_shadowed_right_o_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINEDPLUS:
            textout_shadowed_right_plus_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINEDX:
            textout_shadowed_right_x_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsNORMAL:
        default:
            textout_right_ex(bmp, f, s, x, y, color, bg);
            break;
        }
        
        break;
        
    case sstaCENTER:
        switch(textstyle)
        {
        case sstsSHADOW:
            textout_shadow_centre_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsSHADOWU:
            textout_shadow_centre_u_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINE8:
            textout_shadow_centre_o_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINEPLUS:
            textout_shadow_centre_plus_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINEX:
            textout_shadow_centre_x_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsSHADOWED:
            textout_shadowed_centre_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsSHADOWEDU:
            textout_shadowed_centre_u_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINED8:
            textout_shadowed_centre_o_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINEDPLUS:
            textout_shadowed_centre_plus_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINEDX:
            textout_shadowed_centre_x_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsNORMAL:
        default:
            textout_centre_ex(bmp, f, s, x, y, color, bg);
            break;
        }
        
        break;
        
    case sstaLEFT:
    default:
        switch(textstyle)
        {
        case sstsSHADOW:
            textout_shadow_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsSHADOWU:
            textout_shadow_u_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINE8:
            textout_shadow_o_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINEPLUS:
            textout_shadow_plus_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsOUTLINEX:
            textout_shadow_x_ex(bmp, f, s, x, y, shadow, bg);
            break;
            
        case sstsSHADOWED:
            textout_shadowed_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsSHADOWEDU:
            textout_shadowed_u_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINED8:
            textout_shadowed_o_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINEDPLUS:
            textout_shadowed_plus_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsOUTLINEDX:
            textout_shadowed_x_ex(bmp, f, s, x, y, color, shadow, bg);
            break;
            
        case sstsNORMAL:
        default:
            textout_ex(bmp, f, s, x, y, color, bg);
            break;
        }
        
        break;
    }
}

void textprintf_styled_aligned_ex(BITMAP *bmp, const FONT *f, int x, int y, int textstyle, int alignment, int color, int shadow, int bg, const char *format, ...)
{

    char buf[512];
    va_list ap;
    ASSERT(bmp);
    ASSERT(f);
    ASSERT(format);
    
    va_start(ap, format);
    uvszprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    
    textout_styled_aligned_ex(bmp, f, buf, x, y, textstyle, alignment, color, shadow, bg);
}


//void frame2x2(BITMAP *dest,miscQdata *misc,int x,int y,int xsize,int ysize,int t,int c)
//draw_block_flip(dest,x,y,css->objects[i].d1,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].w,css->objects[i].h,css->objects[i].d2,css->objects[i].d3,css->objects[i].d4);
void frame2x2(BITMAP *dest,miscQdata *misc,int x,int y,int tile,int cset,int w,int h,int /*flip*/,bool overlay,bool trans)
{
    /*
      /- -- -- -\
      |         |
      |         |
      \_ __ __ _/
    
      0  1  4  5
      2  3  6  7
      80 81 84 85
      82 83 86 87
      */
    if(tile==0)
    {
        tile=misc->colors.blueframe_tile;
    }
    
    int t8 = tile<<2;
    
    if(overlay)
    {
        if(trans)
        {
            for(int dx=0; dx<w; dx++)
            {
                int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4) : ((dx==0) ? t8    : t8+1);
                int bottom = (dx >= (w>>1)) ? ((dx==w-1) ? t8+87 : t8+86) : ((dx==0) ? t8+82 : t8+83);
                
                overtiletranslucent8(dest,top,(dx<<3)+x,y,cset,0,128);
                overtiletranslucent8(dest,bottom,(dx<<3)+x,((h-1)<<3)+y,cset,0,128);
            }
            
            for(int dy=1; dy<h-1; dy++)
            {
                int left  = (dy >= (h>>1)) ? t8+80 : t8+2;
                int right = (dy >= (h>>1)) ? t8+85 : t8+7;
                
                overtiletranslucent8(dest,left,x,(dy<<3)+y,cset,0,128);
                overtiletranslucent8(dest,right,((w-1)<<3)+x,(dy<<3)+y,cset,0,128);
                
                for(int dx=1; dx<w-1; dx++)
                {
                    int fill = (dy >= (h>>1)) ? ((dx >= (w>>1)) ? t8+84 : t8+81) : ((dx >= (w>>1)) ? t8+6 : t8+3);
                    overtiletranslucent8(dest,fill,(dx<<3)+x,(dy<<3)+y,cset,0,128);
                }
            }
        }
        else
        {
            for(int dx=0; dx<w; dx++)
            {
                int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4) : ((dx==0) ? t8    : t8+1);
                int bottom = (dx >= (w>>1)) ? ((dx==w-1) ? t8+87 : t8+86) : ((dx==0) ? t8+82 : t8+83);
                
                overtile8(dest,top,(dx<<3)+x,y,cset,0);
                overtile8(dest,bottom,(dx<<3)+x,((h-1)<<3)+y,cset,0);
            }
            
            for(int dy=1; dy<h-1; dy++)
            {
                int left  = (dy >= (h>>1)) ? t8+80 : t8+2;
                int right = (dy >= (h>>1)) ? t8+85 : t8+7;
                
                overtile8(dest,left,x,(dy<<3)+y,cset,0);
                overtile8(dest,right,((w-1)<<3)+x,(dy<<3)+y,cset,0);
                
                for(int dx=1; dx<w-1; dx++)
                {
                    int fill = (dy >= (h>>1)) ? ((dx >= (w>>1)) ? t8+84 : t8+81) : ((dx >= (w>>1)) ? t8+6 : t8+3);
                    overtile8(dest,fill,(dx<<3)+x,(dy<<3)+y,cset,0);
                }
            }
        }
    }
    else
    {
        if(trans)
        {
            for(int dx=0; dx<w; dx++)
            {
                int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4) : ((dx==0) ? t8    : t8+1);
                int bottom = (dx >= (w>>1)) ? ((dx==w-1) ? t8+87 : t8+86) : ((dx==0) ? t8+82 : t8+83);
                
                puttiletranslucent8(dest,top,(dx<<3)+x,y,cset,0,128);
                puttiletranslucent8(dest,bottom,(dx<<3)+x,((h-1)<<3)+y,cset,0,128);
            }
            
            for(int dy=1; dy<h-1; dy++)
            {
                int left  = (dy >= (h>>1)) ? t8+80 : t8+2;
                int right = (dy >= (h>>1)) ? t8+85 : t8+7;
                
                puttiletranslucent8(dest,left,x,(dy<<3)+y,cset,0,128);
                puttiletranslucent8(dest,right,((w-1)<<3)+x,(dy<<3)+y,cset,0,128);
                
                for(int dx=1; dx<w-1; dx++)
                {
                    int fill = (dy >= (h>>1)) ? ((dx >= (w>>1)) ? t8+84 : t8+81) : ((dx >= (w>>1)) ? t8+6 : t8+3);
                    puttiletranslucent8(dest,fill,(dx<<3)+x,(dy<<3)+y,cset,0,128);
                }
            }
        }
        else
        {
            for(int dx=0; dx<w; dx++)
            {
                int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4) : ((dx==0) ? t8    : t8+1);
                int bottom = (dx >= (w>>1)) ? ((dx==w-1) ? t8+87 : t8+86) : ((dx==0) ? t8+82 : t8+83);
                
                oldputtile8(dest,top,(dx<<3)+x,y,cset,0);
                oldputtile8(dest,bottom,(dx<<3)+x,((h-1)<<3)+y,cset,0);
            }
            
            for(int dy=1; dy<h-1; dy++)
            {
                int left  = (dy >= (h>>1)) ? t8+80 : t8+2;
                int right = (dy >= (h>>1)) ? t8+85 : t8+7;
                
                oldputtile8(dest,left,x,(dy<<3)+y,cset,0);
                oldputtile8(dest,right,((w-1)<<3)+x,(dy<<3)+y,cset,0);
                
                for(int dx=1; dx<w-1; dx++)
                {
                    int fill = (dy >= (h>>1)) ? ((dx >= (w>>1)) ? t8+84 : t8+81) : ((dx >= (w>>1)) ? t8+6 : t8+3);
                    oldputtile8(dest,fill,(dx<<3)+x,(dy<<3)+y,cset,0);
                }
            }
        }
    }
}

void drawgrid(BITMAP *dest,int x,int y,int c1,int c2)
{
    int si=0;
    
    for(int y2=0; y2<=7; ++y2)
    {
        byte dl = DMaps[get_currdmap()].grid[si];
        
        for(int x2=0; x2<=7; ++x2)
        {
            if(c2==-1)
            {
                if(dl&0x80)
                    rectfill(dest,(x2*8)+x,(y2*4)+y,(x2*8)+x+6,(y2*4)+y+2,c1);
            }
            else
            {
                rectfill(dest,(x2*8)+x,(y2*4)+y,(x2*8)+x+6,(y2*4)+y+2,c2);
                
                if(dl&0x80)
                    rectfill(dest,(x2*8)+x+2,(y2*4)+y,(x2*8)+x+4,(y2*4)+y+2,c1);
            }
            
            dl<<=1;
        }
        
        ++si;
    }
}

void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h)
{
    for(int j=0; j<h; j++)
        for(int i=0; i<w; i++)
            overtile16(dest,tile+j*20+i,x+(i<<4),y+(j<<4),cset,0);
}

void draw_block_flip(BITMAP *dest,int x,int y,int tile,int cset,int w,int h,int flip,bool overlay,bool trans)
{
    if(overlay)
    {
        switch(flip&3)
        {
        case 0:
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtiletranslucent16(dest,tile+j*20+i,x+(i<<4),y+(j<<4),cset,0,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtile16(dest,tile+j*20+i,x+(i<<4),y+(j<<4),cset,0);
                    }
                }
            }
            
            break;
            
        case 1: //horizontal
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtiletranslucent16(dest,tile+j*20+(w-i-1),x+(i<<4),y+(j<<4),cset,1,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtile16(dest,tile+j*20+(w-i-1),x+(i<<4),y+(j<<4),cset,1);
                    }
                }
            }
            
            break;
            
        case 2:
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtiletranslucent16(dest,tile+(h-j-1)*20+i,x+(i<<4),y+(j<<4),cset,2,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtile16(dest,tile+(h-j-1)*20+i,x+(i<<4),y+(j<<4),cset,2);
                    }
                }
            }
            
            break;
            
        case 3:
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtiletranslucent16(dest,tile+(h-j-1)*20+(w-i-1),x+(i<<4),y+(j<<4),cset,3,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        overtile16(dest,tile+(h-j-1)*20+(w-i-1),x+(i<<4),y+(j<<4),cset,3);
                    }
                }
            }
            
            break;
        }
    }
    else
    {
        switch(flip&3)
        {
        case 0:
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        puttiletranslucent16(dest,tile+j*20+i,x+(i<<4),y+(j<<4),cset,0,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        oldputtile16(dest,tile+j*20+i,x+(i<<4),y+(j<<4),cset,0);
                    }
                }
            }
            
            break;
            
        case 1: //horizontal
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        puttiletranslucent16(dest,tile+j*20+(w-i-1),x+(i<<4),y+(j<<4),cset,1,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        oldputtile16(dest,tile+j*20+(w-i-1),x+(i<<4),y+(j<<4),cset,1);
                    }
                }
            }
            
            break;
            
        case 2:
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        puttiletranslucent16(dest,tile+(h-j-1)*20+i,x+(i<<4),y+(j<<4),cset,2,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        oldputtile16(dest,tile+(h-j-1)*20+i,x+(i<<4),y+(j<<4),cset,2);
                    }
                }
            }
            
            break;
            
        case 3:
            if(trans)
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        puttiletranslucent16(dest,tile+(h-j-1)*20+(w-i-1),x+(i<<4),y+(j<<4),cset,3,255-trans);
                    }
                }
            }
            else
            {
                for(int j=0; j<h; j++)
                {
                    for(int i=0; i<w; i++)
                    {
                        oldputtile16(dest,tile+(h-j-1)*20+(w-i-1),x+(i<<4),y+(j<<4),cset,3);
                    }
                }
            }
            
            break;
        }
    }
}

void drawdmap(BITMAP *dest, miscQdata *misc, int x, int y, bool showmap, int showlink, int showcompass, int linkcolor, int lccolor, int dccolor)
{
    zcolors c=misc->colors;
    int type = (DMaps[get_currdmap()].type&dmfTYPE);
    
    if(showmap)
    {
        switch(type)
        {
        case dmOVERW:
        case dmBSOVERW:
        
            if(DMaps[get_currdmap()].minimap_1_tile)
            {
                draw_block(dest,x,y,DMaps[get_currdmap()].minimap_1_tile,DMaps[get_currdmap()].minimap_1_cset,5,3);
            }
            else if(c.overworld_map_tile)
            {
                draw_block(dest,x,y,c.overworld_map_tile,c.overworld_map_cset,5,3);
            }
            else
            {
                rectfill(dest,x+8,y+8,x+71,y+39,c.overw_bg);
            }
            
            if(!DMaps[get_currdmap()].minimap_1_tile && ((DMaps[get_currdmap()].type&dmfTYPE) == dmBSOVERW))
            {
                drawgrid(dest,x+8,y+8,c.bs_goal,c.bs_dk);
            }
            
            break;
            
        case dmDNGN:
        case dmCAVE:
            int maptile=has_item(itype_map, get_dlevel())?DMaps[get_currdmap()].minimap_2_tile:DMaps[get_currdmap()].minimap_1_tile;
            int mapcset=has_item(itype_map, get_dlevel())?DMaps[get_currdmap()].minimap_2_cset:DMaps[get_currdmap()].minimap_1_cset;
            
            if(maptile)
            {
                draw_block(dest,x,y,maptile,mapcset,5,3);
            }
            else if(c.dungeon_map_tile)
            {
                draw_block(dest,x,y,c.dungeon_map_tile,c.dungeon_map_cset,5,3);
            }
            else
            {
                rectfill(dest,x+8,y+8,x+71,y+39,c.dngn_bg);
            }
            
            if(!DMaps[get_currdmap()].minimap_2_tile && has_item(itype_map, get_dlevel()))
            {
                if((DMaps[get_currdmap()].flags&dmfMINIMAPCOLORFIX) != 0)
                {
                    drawgrid(dest,x+8,y+8,c.cave_fg,-1);
                }
                else
                {
                    drawgrid(dest,x+8,y+8,c.dngn_fg,-1);
                }
            }
            
            break;
        }
    }
    
    if(showcompass)
    {
        if(type==dmDNGN || type==dmCAVE)
        {
            if(show_subscreen_dmap_dots&&has_item(itype_compass, get_dlevel()))
            {
                int c2 = dccolor;
                
                if(!has_item(itype_triforcepiece, get_dlevel()) && (frame&16))
                    c2 = lccolor;
                    
                int cx = ((DMaps[get_currdmap()].compass&15)<<3)+x+10;
                int cy = ((DMaps[get_currdmap()].compass&0xF0)>>2)+y+8;
                putdot(dest,cx,cy,c2);
            }
        }
    }
    
    if(showlink)
    {
        if(show_subscreen_dmap_dots && linkcolor != 255)
        {
            if(type==dmOVERW)
            {
                putdot(dest,((get_homescr()&15)<<2)+x+9,((get_homescr()&0xF0)>>2)+y+8,linkcolor);
            }
            else if(type==dmBSOVERW || ((type==dmDNGN || type==dmCAVE) && get_currscr()<128))
            {
                putdot(dest,(((get_homescr()&15)-DMaps[get_currdmap()].xoff)<<3)+x+10,((get_homescr()&0xF0)>>2)+y+8,linkcolor);
            }
        }
    }
}



void lifemeter(BITMAP *dest,int x,int y,int tile,bool bs_style)
{
    if(!show_subscreen_life)
    {
        return;
    }
    
    if(!bs_style)
    {
        y+=24;
    }
    
    for(int i=0; i<(game != NULL ? zc_min(game->get_maxlife(),16*24) : 1); i+=HP_PER_HEART)
    {
        if(game != NULL)
        {
            if(get_bit(quest_rules,qr_QUARTERHEART))
            {
                SpriteDefinitionRef hearts = curQuest->specialSprites().lifeMeterHearts;
                if(i+((HP_PER_HEART/4)*3)>=game->get_life()) tile=(curQuest->getSpriteDefinition(hearts).tile*4)+2;
                
                if(i+(HP_PER_HEART/2)>=game->get_life()) tile=1;
                
                if(i+((HP_PER_HEART/4)*1)>=game->get_life()) tile=(curQuest->getSpriteDefinition(hearts).tile*4)+3;
            }
            else if(i+(HP_PER_HEART/2)>=game->get_life()) tile=1;
            
            if(i>=game->get_life()) tile=4;
        }
        else
            tile=4;
            
        overtile8(dest,tile,x,y,1,0);
        x+=8;
        
        if(((i>>4)&7)==7)
        {
            x-=64;
            y+=bs_style?8:-8;
        }
    }
}

void magicmeter(BITMAP *dest,int x,int y)
{
    if(!get_bit(quest_rules,qr_ENABLEMAGIC)) return;
    
    if(game->get_maxmagic()==0) return;
    
    int tile;
    SpriteDefinitionRef mmeters = curQuest->specialSprites().magicMeter;
    int mmtile=curQuest->getSpriteDefinition(mmeters).tile;
    int mmcset=curQuest->getSpriteDefinition(mmeters).csets&15;
    overtile8(dest,(mmtile*4)+2,x-8,y,mmcset,0);
    
    if(game->get_magicdrainrate()==1)
    {
        overtile8(dest,(mmtile*4)+1,x-10,y,mmcset,0);
    }
    
    for(int i=0; i<game->get_maxmagic(); i+=MAGICPERBLOCK)
    {
        if(game->get_magic()>=i+MAGICPERBLOCK)
        {
            tile=mmtile*4;                                        //full block
        }
        else
        {
            if(i>game->get_magic())
            {
                tile=((mmtile+1)*4);                                //empty block
            }
            else
            {
                tile=((mmtile+1)*4)+((game->get_magic()-i)%MAGICPERBLOCK);
            }
        }
        
        overtile8(dest,tile,x,y,mmcset,0);
        x+=8;
    }
    
    overtile8(dest,(mmtile*4)+3,x,y,mmcset,0);
    
}

void putxnum(BITMAP *dest,int x,int y,int num,FONT *tempfont,int color,int shadowcolor,int bgcolor,int textstyle,bool usex,int digits,bool infinite,char idigit)
{
    if(!show_subscreen_numbers)
    {
        return;
    }
    
    int found_digits=5;
    
    if(num<10000)
    {
        found_digits=4;
    }
    
    if(num<1000)
    {
        found_digits=3;
    }
    
    if(num<100)
    {
        found_digits=2;
    }
    
    if(num<10)
    {
        found_digits=1;
    }
    
    if(infinite)
    {
        textprintf_styled_aligned_ex(dest,tempfont,x,y,textstyle,sstaLEFT,color,shadowcolor,bgcolor,"%s%c",usex?"X":"",idigit);
    }
    else
    {
        textprintf_styled_aligned_ex(dest,tempfont,x,y,textstyle,sstaLEFT,color,shadowcolor,bgcolor,"%s%d",(usex && found_digits<digits)?"X":"",num);
    }
}

/*
  INLINE void putdot(BITMAP *dest,int x,int y,int c)
  {
  rectfill(dest,x,y,x+2,y+2,c);
  }
  */

/****  Subscr items code  ****/

item *Bitem = NULL, *Aitem = NULL;
ItemDefinitionRef Bid;
ItemDefinitionRef Aid;

void reset_subscr_items()
{
    if(Aitem)
    {
        delete Aitem;
        Aitem = NULL;
    }
    
    if(Bitem)
    {
        delete Bitem;
        Bitem = NULL;
    }
    
    Aid = ItemDefinitionRef();
    Bid = ItemDefinitionRef();
}


void update_subscr_items()
{
    if(Bid != Bwpn)
    {
        Bid = ItemDefinitionRef();
        
        if(Bitem)
        {
            delete Bitem;
            Bitem = NULL;
        }
        
        if(curQuest->isValid(Bwpn))
        {
            Bitem = new item((fix)0, (fix)0, (fix)0, Bwpn, 0, 0);
            Bitem->dummy_bool[0]=false;
            
            switch(curQuest->getItemDefinition(Bwpn).family)
            {
            case itype_arrow:
                if(combinedBowArrowB)
                {
                    Bitem->dummy_bool[0]=true;
                }
                
                break;
            }
            
            //      Bitem = new item((fix)(zinit.subscreen<ssdtBSZELDA?124:136), (fix)24,(fix)0, Bwpn, 0, 0);
            if(Bitem != NULL)
            {
                Bid = Bwpn;
                Bitem->yofs = 0;
                Bitem->pickup |= ipDUMMY;
            }
        }
    }
    
    if(Aid != Awpn)
    {
        Aid = ItemDefinitionRef();
        
        if(Aitem)
        {
            delete Aitem;
            Aitem = NULL;
        }
        
        if(curQuest->isValid(Awpn))
        {
            Aitem = new item((fix)0, (fix)0,(fix)0,Awpn, 0, 0);
            
            switch(curQuest->getItemDefinition(Awpn).family)
            {
            case itype_arrow:
                if(combinedBowArrowA)
                {
                    Aitem->dummy_bool[0]=true;
                }
                
                break;
            }
            
            if(Aitem != NULL)
            {
                Aid = Awpn;
                Aitem->yofs = 0;
                Aitem->pickup |= ipDUMMY;
            }
        }
    }
    
    if(Bitem)
        Bitem->animate(0);
        
    if(Aitem)
        Aitem->animate(0);
}

void add_subscr_item(item *newItem)
{
    newItem->subscreenItem=true;
    Sitems.add(newItem);
}

/****/
int stripspaces(char *source, char *target, int stop)
{
    int begin, end;
    
    for(begin=0; ((begin<stop)&&(source[begin]==' ')); ++begin)
    {
        /* do nothing */
    }
    
    if(begin==stop)
    {
        return 0;
    }
    
    for(end=stop-1; ((source[end]==' ')); --end)
    {
        /* do nothing */
    }
    
    sprintf(target, "%.*s", (end-begin+1), source+begin);
    return (end-begin+1);
}

int subscreen_cset(miscQdata *misc,int c1, int c2)
{
    int ret=c1;
    
    switch(c1)
    {
    case ssctMISC:
        switch(c2)
        {
        case sscsTRIFORCECSET:
            ret=misc->colors.triforce_cset;
            break;
            
        case sscsTRIFRAMECSET:
            ret=misc->colors.triframe_cset;
            break;
            
        case sscsOVERWORLDMAPCSET:
            ret=misc->colors.overworld_map_cset;
            break;
            
        case sscsDUNGEONMAPCSET:
            ret=misc->colors.dungeon_map_cset;
            break;
            
        case sscsBLUEFRAMECSET:
            ret=misc->colors.blueframe_cset;
            break;
            
        case sscsHCPIECESCSET:
            ret=misc->colors.HCpieces_cset;
            break;
            
        case sscsSSVINECSET:
        {
            SpriteDefinitionRef vines = curQuest->specialSprites().subscreenVine;
            ret = curQuest->getSpriteDefinition(vines).csets & 15;
            break;
        }
        default:
            ret=(rand()*1000)%256;
            break;
        }
        
        break;
        
    default:
        ret=c1;
    }
    
    return ret;
}

FONT *ss_font(int fontnum)
{
    FONT *tempfont;
    
    switch(fontnum)
    {
    case ssfSMALL:
        tempfont=sfont;
        break;
        
    case ssfSMALLPROP:
        tempfont=spfont;
        break;
        
    case ssfSS1:
        tempfont=ssfont1;
        break;
        
    case ssfSS2:
        tempfont=ssfont2;
        break;
        
    case ssfSS3:
        tempfont=ssfont3;
        break;
        
    case ssfSS4:
        tempfont=ssfont4;
        break;
        
    case ssfZTIME:
        tempfont=ztfont;
        break;
        
    case ssfZELDA:
        tempfont=zfont;
        break;
        
    case ssfZ3:
        tempfont=z3font;
        break;
        
    case ssfZ3SMALL:
        tempfont=z3smallfont;
        break;
        
    case ssfGBLA:
        tempfont=gblafont;
        break;
        
    case ssfGORON:
        tempfont=goronfont;
        break;
        
    case ssfZORAN:
        tempfont=zoranfont;
        break;
        
    case ssfHYLIAN1:
        tempfont=hylian1font;
        break;
        
    case ssfHYLIAN2:
        tempfont=hylian2font;
        break;
        
    case ssfHYLIAN3:
        tempfont=hylian3font;
        break;
        
    case ssfHYLIAN4:
        tempfont=hylian4font;
        break;
        
    case ssfPROP:
        tempfont=font;
        break;
        
    case ssfGBORACLE:
        tempfont=gboraclefont;
        break;
        
    case ssfGBORACLEP:
        tempfont=gboraclepfont;
        break;
        
    case ssfDSPHANTOM:
        tempfont=dsphantomfont;
        break;
        
    case ssfDSPHANTOMP:
        tempfont=dsphantompfont;
        break;
        
    default:
        fontnum=rand()%ssfMAX;
        tempfont=ss_font(fontnum);
        break;
    }
    
    return tempfont;
}

void delete_selectors()
{
    if(sel_a)
    {
        delete sel_a;
        sel_a=NULL;
    }
    
    if(sel_b)
    {
        delete sel_b;
        sel_b=NULL;
    }
}


void animate_selectors()
{
    if(new_sel)
    {
        delete_selectors();
        new_sel = false;
    }
    
    if(!sel_a)
        sel_a = new item((fix)0, (fix)0, (fix)0, curQuest->specialItems().selectA, 0, 0);
        
    if(!sel_b)
        sel_b = new item((fix)0, (fix)0, (fix)0, curQuest->specialItems().selectA, 0, 0);
        
    sel_a->yofs=0;
    sel_a->animate(0);
    sel_b->yofs=0;
    sel_b->animate(0);
}

void show_custom_subscreen(BITMAP *dest, miscQdata *misc, subscreen_group *css, int xofs, int yofs, bool showtime, int pos2)
{
    //this is not a good place to be clearing the bitmap
    //other stuff might already have been drawn on it that needs to be kept
    //(eg the game screen when pulling down the subscreen) -DD
    //clear_to_color(dest, 0);
    color_map = &trans_table;
    set_trans_blender(0, 0, 0, 128);

    //doing animation here leads to 2x speed when drawing both active and passive subscreen -DD
    /*static item sel_a((fix)0,(fix)0,(fix)0,iSelectA,0,0);
    static item sel_b((fix)0,(fix)0,(fix)0,iSelectB,0,0);
    if (new_sel)
    {
      sel_a=item((fix)0,(fix)0,(fix)0,iSelectA,0,0);
      sel_b=item((fix)0,(fix)0,(fix)0,iSelectB,0,0);
      new_sel=false;
    }
    sel_a.yofs=0;
    sel_a.animate(0);
    sel_b.yofs=0;
    sel_b.animate(0);*/
    if (!sel_a || !sel_b)
        animate_selectors();

    for (std::vector<subscreen_object *>::iterator it = css->ss_objects.begin(); it != css->ss_objects.end(); ++it)
    {
        if (((*it)->pos & pos2) != 0)
        {
            (*it)->show(dest, *css, misc, xofs, yofs, showtime);
        }
    }
}

void buttonitem(BITMAP *dest, int button, int x, int y)
{
    switch(button)
    {
    case 0:  //A button
        if(Aitem&&show_subscreen_items)
        {
            Aitem->x=x;
            Aitem->y=y;
            
            switch(curQuest->getItemDefinition(Aitem->itemDefinition).family)
            {
            case itype_arrow:
                if(Aitem->dummy_bool[0]==true)
                {
                    if(curQuest->isValid(current_item_id(itype_bow)))
                    {
                        subscreenitemType(dest, x, y, itype_bow);
                        
                        if(((get_bit(quest_rules,qr_TRUEARROWS)&&!game->get_arrows())
                                ||(!get_bit(quest_rules,qr_TRUEARROWS)&&!game->get_rupies()&&!current_item_power(itype_wallet)))
                                &&!current_item_power(itype_quiver))
                        {
                            return;
                        }
                    }
                }
                
                break;
            }
            
            Aitem->draw(dest);
        }
        
        break;
        
    case 1:  //B button
        if(Bitem&&show_subscreen_items)
        {
            Bitem->x=x;
            Bitem->y=y;
            
            switch(curQuest->getItemDefinition(Bitem->itemDefinition).family)
            {
            case itype_arrow:
                if(Bitem->dummy_bool[0]==true)
                {
                    if(curQuest->isValid(current_item_id(itype_bow)))
                    {
                        subscreenitemType(dest, x, y, itype_bow);
                        
                        if(((get_bit(quest_rules,qr_TRUEARROWS)&&(game != NULL && !game->get_arrows()))
                                ||(!get_bit(quest_rules,qr_TRUEARROWS)&&(game != NULL && !game->get_rupies())&&!current_item_power(itype_wallet)))
                                &&!current_item_power(itype_quiver))
                        {
                            return;
                        }
                    }
                }
                
                break;
            }
            
            Bitem->draw(dest);
        }
        
        break;
        
    default:
        break;
    }
}

void defaultcounters(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, bool usex, int textstyle, int digits, char idigit)
{
    int yofs = (game==NULL || (game->get_sbombs() && curQuest->isValid(current_item_id(itype_sbomb)))) ? 8 : 0;
    
    //88, 16
    overtile8(dest,5,x,y,1,0);
    overtile8(dest,8,x,y+16-yofs,1,0);
    overtile8(dest,9,x,y+24-yofs,0,0);
    
    if(game)
    {
        bool magickey = false;
        ItemDefinitionRef itemid = current_item_id(itype_magickey);
        
        if(curQuest->isValid(itemid))
        {
            if(curQuest->getItemDefinition(itemid).flags & itemdata::IF_FLAG1)
                magickey = curQuest->getItemDefinition(itemid).power>=get_dlevel();
            else
                magickey = curQuest->getItemDefinition(itemid).power==get_dlevel();
        }
        
        putxnum(dest,x+8,y,game->get_rupies(),tempfont,color,shadowcolor,bgcolor,textstyle,usex,digits,current_item_power(itype_wallet)>0,idigit);
        putxnum(dest,x+8,y+16-yofs,game->get_keys()+game->get_lkeys(),tempfont,color,shadowcolor,bgcolor, textstyle,usex,digits,magickey,idigit);
        putxnum(dest,x+8,y+24-yofs,game->get_bombs(),tempfont,color,shadowcolor,bgcolor,textstyle,usex,digits,current_item_power(itype_bombbag)>0,idigit);
    }
    
    if(game==NULL || (game->get_sbombs() && curQuest->isValid(current_item_id(itype_sbomb))))
    {
        overtile8(dest,13,x,y+24,1,0);
        
        if(game)
        {
            ItemDefinitionRef itemid = current_item_id(itype_bombbag);
            bool superbomb = (curQuest->isValid(itemid) && curQuest->getItemDefinition(itemid).power>0 && curQuest->getItemDefinition(itemid).flags & itemdata::IF_FLAG1);
            
            putxnum(dest,x+8,y+24,game->get_sbombs(),tempfont,color,shadowcolor,bgcolor,textstyle,usex,digits,superbomb,idigit);
        }
    }
}

bool is_counter_item(const ItemDefinitionRef &itemref, int countertype)
{
    switch(countertype)
    {
    case sscBOMBS:
        if(curQuest->getItemDefinition(itemref).family==itype_bomb)
        {
            return true;
        }
        
        break;
        
    case sscSBOMBS:
        if(curQuest->getItemDefinition(itemref).family==itype_sbomb)
        {
            return true;
        }
        
        break;
        
    case sscARROWS:
        if(curQuest->getItemDefinition(itemref).family==itype_arrow)
        {
            return true;
        }
        
        break;
    }
    
    return false;
}

void counter(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, int alignment, int textstyle, int digits, char idigit, bool showzero, int itemtype1, int itemtype2, int itemtype3, const ItemDefinitionRef &infiniteitem, bool onlyselected)
{
    int value=0;
    bool infinite=false;
    
    if(game != NULL && game->get_item(infiniteitem) && !game->get_disabled_item(infiniteitem))
    {
        infinite=true;
    }
    
    char valstring[80];
    char formatstring[80];
    sprintf(valstring,"01234567890123456789");
    sprintf(formatstring, "%%0%dd", digits);
    
    if(onlyselected && !(((Bitem&&(is_counter_item(Bitem->itemDefinition,itemtype1)||is_counter_item(Bitem->itemDefinition,itemtype2)||is_counter_item(Bitem->itemDefinition,itemtype3)))||(Aitem&&(is_counter_item(Aitem->itemDefinition,itemtype1)||is_counter_item(Aitem->itemDefinition,itemtype2)||is_counter_item(Aitem->itemDefinition,itemtype3))))))
    {
        return;
    }
    
    int itemtype;
    /*
    for (int i=0; i<3; ++i)
    {
      switch (i)
      {
        case 0:
          itemtype=itemtype1;
          break;
        case 1:
          itemtype=itemtype2;
          break;
        case 2:
          itemtype=itemtype3;
          break;
      }
    */
    itemtype=itemtype1;
    /* commented out until I find out what it does - it's messing up custom subscreens now as
    itemtype2 and 3 are zero - so link's life gets added to the counters */
    
    switch(itemtype)
    {
    case sscRUPEES:
        if(current_item_power(itype_wallet))
            infinite=true;
            
        value+=game->get_rupies();
        break;
        
    case sscBOMBS:
        if(current_item_power(itype_bombbag))
            infinite=true;
            
        value+=game->get_bombs();
        break;
        
    case sscSBOMBS:
    {
        ItemDefinitionRef itemid = current_item_id(itype_bombbag);
        
        if(curQuest->isValid(itemid) && curQuest->getItemDefinition(itemid).power>0 && (curQuest->getItemDefinition(itemid).flags & itemdata::IF_FLAG1))
            infinite=true;
            
        value+=game->get_sbombs();
        break;
    }
    
    case sscARROWS:
        if((!get_bit(quest_rules,qr_TRUEARROWS) && current_item_power(itype_wallet)) || current_item_power(itype_quiver))
            infinite=true;
            
        // If Link somehow got ammunition before getting the arrow,
        // or if the arrow was disabled in a DMap,
        // we shouldn't put the value as zero.
//        if(/*current_item_id(itype_arrow)>-1*/ true)
        {
            if(get_bit(quest_rules,qr_TRUEARROWS))
            {
                value+=game->get_arrows();
            }
            else
            {
                value+=game->get_rupies();
            }
        }
        break;
        
    case sscGENKEYMAGIC:
    case sscLEVKEYMAGIC:
    case sscANYKEYMAGIC:
    {
        ItemDefinitionRef itemid = current_item_id(itype_magickey);
        
        if(curQuest->isValid(itemid) && !infinite)
        {
            if(curQuest->getItemDefinition(itemid).flags & itemdata::IF_FLAG1)
                infinite = curQuest->getItemDefinition(itemid).power>=get_dlevel();
            else
                infinite = curQuest->getItemDefinition(itemid).power==get_dlevel();
        }
    }
    
    //fall through
    case sscANYKEYNOMAGIC:
    case sscLEVKEYNOMAGIC:
    case sscGENKEYNOMAGIC:
        if(itemtype == sscGENKEYNOMAGIC || itemtype == sscANYKEYNOMAGIC
                || itemtype == sscGENKEYMAGIC || itemtype == sscANYKEYMAGIC)
            value += game->get_keys();
            
        if(itemtype == sscLEVKEYNOMAGIC || itemtype == sscANYKEYNOMAGIC
                || itemtype == sscLEVKEYMAGIC || itemtype == sscANYKEYMAGIC)
            value += game->get_lkeys();
            
        break;
        
    case sscSCRIPT1:
    case sscSCRIPT2:
    case sscSCRIPT3:
    case sscSCRIPT4:
    case sscSCRIPT5:
    case sscSCRIPT6:
    case sscSCRIPT7:
    case sscSCRIPT8:
    case sscSCRIPT9:
    case sscSCRIPT10:
    case sscSCRIPT11:
    case sscSCRIPT12:
    case sscSCRIPT13:
    case sscSCRIPT14:
    case sscSCRIPT15:
    case sscSCRIPT16:
    case sscSCRIPT17:
    case sscSCRIPT18:
    case sscSCRIPT19:
    case sscSCRIPT20:
    case sscSCRIPT21:
    case sscSCRIPT22:
    case sscSCRIPT23:
    case sscSCRIPT24:
    case sscSCRIPT25:
        value += game->get_counter(itemtype-3);
        
    default:
        break;
    }
    
    // (for loop) }
    if(!showzero&&!value&&!infinite)
    {
        return;
    }
    
    if(infinite)
    {
        sprintf(valstring, "%c", idigit);
    }
    else
    {
        sprintf(valstring, formatstring, value);
    }
    
    textout_styled_aligned_ex(dest,tempfont,valstring,x,y,textstyle,alignment,color,shadowcolor,bgcolor);
    
    //  putxnum(dest,x+8,y,game->get_rupies(),tempfont,color,shadowcolor,bgcolor,shadowed,usex,digits,0,idigit);
}

void minimaptitle(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, int alignment, int textstyle)
{
    char dmaptitlesource[2][11];
    char dmaptitle[2][11];
    sprintf(dmaptitlesource[0], "%.10s", DMaps[get_currdmap()].title);
    sprintf(dmaptitlesource[1], "%.10s", DMaps[get_currdmap()].title+10);
    
    int title_len1=stripspaces(dmaptitlesource[0], dmaptitle[0], 10);
    int title_len2=stripspaces(dmaptitlesource[1], dmaptitle[1], 10);
    
    if((title_len1>0)||(title_len2>0))
    {
        if((title_len1>0)&&(title_len2>0))
        {
            textprintf_styled_aligned_ex(dest,tempfont,x,y+8,textstyle,alignment,color,shadowcolor,bgcolor,"%s",dmaptitle[1]);
            textprintf_styled_aligned_ex(dest,tempfont,x,y,textstyle,alignment,color,shadowcolor,bgcolor,"%s",dmaptitle[0]);
        }
        else
        {
            if(title_len1>0)
            {
                textprintf_styled_aligned_ex(dest,tempfont,x,y+8,textstyle,alignment,color,shadowcolor,bgcolor,"%s",dmaptitle[0]);
            }
            else
            {
                textprintf_styled_aligned_ex(dest,tempfont,x,y+8,textstyle,alignment,color,shadowcolor,bgcolor,"%s",dmaptitle[1]);
            }
        }
    }
}

void put_passive_subscr(BITMAP *dest,miscQdata *misc,int x,int y,bool showtime,int pos2)
{
    // uncomment this?
    //  load_Sitems();
    Sitems.animate();
    update_subscr_items();
    BITMAP *subscr = create_sub_bitmap(dest,x,y,256,passive_subscreen_height);
    
    if(no_subscreen())
    {
        clear_to_color(subscr,0);
        destroy_bitmap(subscr);
        return;
    }
    
    show_custom_subscreen(subscr, misc, current_subscreen_passive, 0, 0, showtime, pos2);
    destroy_bitmap(subscr);
}

/*
  const byte tripiece[8*3] =
  {
  112,112,0, 128,112,1, 96,128,0, 144,128,1,
  112,128,2, 112,128,1, 128,128,3, 128,128,0
  };
  */

void puttriframe(BITMAP *dest, miscQdata *misc, int x, int y, int triframecolor, int numbercolor, int triframetile, int triframecset, int triforcetile, int triforcecset, bool showframe, bool showpieces, bool largepieces)
{
    if(triframetile==0)
    {
        triframetile=misc->colors.triframe_tile;
        triframecset=misc->colors.triframe_cset;
    }
    
    if(triforcetile==0)
    {
        triforcetile=misc->colors.triforce_tile;
        triforcecset=misc->colors.triforce_cset;
    }
    
    if(!largepieces)
    {
        if(showframe)
        {
            if(triframetile)
            {
                draw_block(dest,x,y,triframetile,triframecset,6,3);
            }
            else
            {
                line(dest,x+47,y,x+1,y+46,triframecolor);
                line(dest,x+48,y,x+94,y+46,triframecolor);
                _allegro_hline(dest,x,y+47,x+95,triframecolor);
                line(dest,x+47,y+7,x+15,y+39,triframecolor);
                line(dest,x+48,y+7,x+80,y+39,triframecolor);
                _allegro_hline(dest,x+15,y+40,x+80,triframecolor);
            }
        }
        
        if(showpieces)
        {
            for(int i=0; i<8; i++)
            {
                int lvl = i+1;
                
                if(get_bit(quest_rules,qr_4TRI) && lvl>4)
                    lvl -= 4;
                    
                if(has_item(itype_triforcepiece, lvl))
                {
                    int tp=0;
                    
                    for(; tp<8; tp++)
                        if(misc->triforce[tp] == i+1)
                            break;
                            
                    overtile16(dest,triforcetile,x+tripiece[0][tp][0],y+tripiece[0][tp][1],triforcecset,tripiece[0][tp][2]);
                }
            }
        }
    }
    else
    {
        if(showframe)
        {
            if(triframetile)
            {
                draw_block(dest,x,y,triframetile,triframecset,7,7);
            }
            else
            {
                if(!get_bit(quest_rules,qr_4TRI))
                {
                    //left inside vertical
                    _allegro_vline(dest,x+31,y+56,y+103,triframecolor);
                    _allegro_vline(dest,x+32,y+56,y+103,triframecolor);
                    
                    //center inside vertical top
                    _allegro_vline(dest,x+55,y+8   ,y+55,triframecolor);
                    _allegro_vline(dest,x+56,y+8   ,y+55,triframecolor);
                    
                    //right inside vertical
                    _allegro_vline(dest,x+79,y+56,y+103,triframecolor);
                    _allegro_vline(dest,x+80,y+56,y+103,triframecolor);
                    
                    if(!get_bit(quest_rules,qr_3TRI))
                    {
                        //center inside vertical bottom
                        _allegro_vline(dest,x+55,y+56,y+103,triframecolor);
                        _allegro_vline(dest,x+56,y+56,y+103,triframecolor);
                    }
                }
                
                //middle inside horizontal
                _allegro_hline(dest,x+32,y+55,x+79,triframecolor);
                _allegro_hline(dest,x+32,y+56,x+79,triframecolor);
                
                //bottom outside horizontal
                _allegro_hline(dest,x+8,y+103,x+103,triframecolor);
                
                //left outside diagonal
                line(dest,x+8,y+103,x+55,y+8,triframecolor);
                line(dest,x+9,y+102,x+55,y+9,triframecolor);
                
                //right outside diagonal
                line(dest,x+56,y+8,x+103,y+103,triframecolor);
                line(dest,x+56,y+9,x+102,y+102,triframecolor);
                
                //left inside diagonal
                line(dest,x+32,y+56,x+55,y+103,triframecolor);
                line(dest,x+32,y+57,x+54,y+102,triframecolor);
                line(dest,x+32,y+55,x+55,y+102,triframecolor);
                
                //right inside diagonal
                line(dest,x+56,y+103,x+79,y+56,triframecolor);
                line(dest,x+57,y+102,x+79,y+57,triframecolor);
                line(dest,x+56,y+102,x+79,y+55,triframecolor);
                
                for(int i=0; i<8; i++)
                {
                    int lvl = i+1;
                    
                    if(get_bit(quest_rules,qr_4TRI) && lvl>4)
                    {
                        lvl -= 4;
                    }
                    
                    int tp=0;
                    
                    for(; tp<8; tp++)
                    {
                        if(misc->triforce[tp] == i+1)
                        {
                            break;
                        }
                    }
                    
                    if(!has_item(itype_triforcepiece, lvl))
                    {
                        textprintf_ex(dest, pfont, x+tripiece[1][tp][0]+((tripiece[1][tp][2]&1)?8:14),y+tripiece[1][tp][1]+((tripiece[1][tp][2]&2)?9:32), numbercolor, -1, "%d", lvl);
                    }
                }
            }
        }
        
        if(showpieces)
        {
            for(int i=0; i<8; i++)
            {
                int lvl = i+1;
                
                if(get_bit(quest_rules,qr_4TRI) && lvl>4)
                {
                    lvl -= 4;
                }
                
                int tp=0;
                
                for(; tp<8; tp++)
                {
                    if(misc->triforce[tp] == i+1)
                    {
                        break;
                    }
                }
                
                if(has_item(itype_triforcepiece, lvl))
                {
                    draw_block_flip(dest,x+tripiece[1][tp][0]-(tripiece[1][tp][2]&1?0:8),y+tripiece[1][tp][1],triforcetile,triforcecset,2,3,tripiece[1][tp][2],1,0);
                }
            }
        }
    }
}


void puttriforce(BITMAP *dest, miscQdata *misc, int x, int y, int tile, int cset, int w, int h, int flip, bool overlay, bool trans, int trinum)
{
    if(tile==0)
    {
        tile=misc->colors.triforce_tile;
    }
    
    for(int i=0; i<8; i++)
    {
        int lvl = i+1;
        
        if(get_bit(quest_rules,qr_4TRI) && lvl>4)
            lvl -= 4;
            
        if(lvl==trinum && has_item(itype_triforcepiece, lvl))
        {
            int tp=0;
            
            for(; tp<8; tp++)
                if(misc->triforce[tp] == i+1)
                    break;
                    
            draw_block_flip(dest,x,y,tile,cset,w,h,flip,overlay,trans);
        }
    }
}

/*
  const byte bmap[16*2] = {
  0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,
  0,3,0,0,0,3,0,0,0,0,3,0,0,0,3,0 };

  const int fringe[8] = { 6,2,4,7,6,8,7,5 };

  const byte bmaptiles[8*5] = {
  0, 1, 2, 3, 2, 3, 3, 4,
  20,21,22,23,22,23,23,24,
  20,21,22,23,22,23,23,24,
  20,21,22,23,22,23,23,24,
  40,41,42,43,42,43,43,44 };
  */

void draw_block(BITMAP *dest,int x,int y,int tile,int cset,int w,int h);
void putBmap(BITMAP *dest, miscQdata *misc, int x, int y,bool showmap, bool showrooms, bool showlink, int roomcolor, int linkcolor, bool large)
{
    int si=0;
    
    int maptile=has_item(itype_map, get_dlevel())?DMaps[get_currdmap()].largemap_2_tile:DMaps[get_currdmap()].largemap_1_tile;
    int mapcset=has_item(itype_map, get_dlevel())?DMaps[get_currdmap()].largemap_2_cset:DMaps[get_currdmap()].largemap_1_cset;
    
    if(showmap)
    {
        if(maptile)
        {
            draw_block(dest,x,y,maptile,mapcset,large?9:7,5);
        }
        else if(misc->colors.dungeon_map_tile)
        {
            for(int y2=0; y2<5; y2++)
            {
                for(int x2=0; x2<(large?8:6); x2++)
                {
                    overtile16(dest,misc->colors.dungeon_map_tile+(large?bmaptiles_original[y2][x2]:bmaptiles_bs[y2][x2]),x+(x2<<4),y+(y2<<4),misc->colors.dungeon_map_cset,0);
                    //++si;
                }
            }
        }
        else
        {
            BITMAP *bmp = create_bitmap_ex(8,8,8);
            
            if(!bmp)
                return;
                
            clear_bitmap(bmp);
            
            for(int x2=0; x2<8; x2++)
            {
                for(int y2=8-fringe[x2]; y2<8; y2++)
                {
                    putpixel(bmp,x2,y2,misc->colors.bmap_bg);
                }
            }
            
            rectfill(dest,x,y,x+(large?127:95),y+79,misc->colors.bmap_bg);
            
            for(int y2=0; y2<2; ++y2)
            {
                for(int x2=0; x2<(large?16:12); ++x2)
                {
                    if((large?bmap_original[y2][x2]:bmap_bs[y2][x2]))
                    {
                        rectfill(dest,(x2<<3)+x,(y2*72)+y,(x2<<3)+x+7,(y2*72)+y+7,misc->colors.subscr_bg);
                        
                        switch((large?bmap_original[y2][x2]:bmap_bs[y2][x2]))
                        {
                        case 3:
                            draw_sprite_v_flip(dest,bmp,(x2<<3)+x,(y2*72)+y);
                            break;
                            
                        case 1:
                        default:
                            draw_sprite(dest,bmp,(x2<<3)+x,(y2*72)+y);
                            break;
                        }
                    }
                }
            }
            
            destroy_bitmap(bmp);
        }
    }
    
    if(showrooms)
    {
        if(roomcolor==-1)
        {
            roomcolor = misc->colors.bmap_fg;
        }
        
        si=(get_currdmap()-1)<<6;
        
        for(int y2=y+8; y2<y+72; y2+=8)
        {
            for(int x2=x+(large?32:16)+(maptile?8:0); x2<x+(large?96:80)+(maptile?8:0); x2+=8)
            {
                if(get_bmaps(si))
                {
                    rectfill(dest,x2+1,y2+1,x2+6,y2+6,roomcolor);
                    
                    if(get_bmaps(si)&1) _allegro_hline(dest,x2+3,y2,  x2+4,roomcolor);  //top door
                    
                    if(get_bmaps(si)&2) _allegro_hline(dest,x2+3,y2+7,x2+4,roomcolor);  //bottom door
                    
                    if(get_bmaps(si)&4) _allegro_vline(dest,x2,  y2+3,y2+4,roomcolor);  //left door
                    
                    if(get_bmaps(si)&8) _allegro_vline(dest,x2+7,y2+3,y2+4,roomcolor);  //right door
                }
                
                ++si;
            }
        }
    }
    
    if(showlink)
    {
        if(get_currscr()<MAPSCRSNORMAL)
        {
            if(linkcolor==-1)
            {
                linkcolor=misc->colors.link_dot;
            }
            
            int xoff = (((DMaps[get_currdmap()].type&dmfTYPE)==dmOVERW) ? 0 : DMaps[get_currdmap()].xoff);
            putdot(dest,(((get_homescr()&15)-xoff)<<3)+x+(large?34:18)+(maptile?8:0),((get_homescr()&0xF0)>>1)+y+11,linkcolor);
        }
    }
}


void load_Sitems(miscQdata *misc)
{
    Sitems.clear();
    
    // HC Pieces
    if(misc->colors.HCpieces_tile)
    {
        //      item *HCP = new item((fix)(inventory_x[5]-ofs),(fix)y,iMax,0,0);
        item *HCP = new item((fix)0,(fix)0,(fix)0,curQuest->specialItems().heartContainerPiece,0,0);
        
        if(HCP)
        {
            int hcpphc =  game->get_hcp_per_hc();
            HCP->tile   = misc->colors.HCpieces_tile + vbound(game->get_HCpieces(),0,hcpphc > 0 ? hcpphc-1 : 0);
            HCP->o_tile = HCP->tile;
            HCP->cs     = misc->colors.HCpieces_cset;
            HCP->frames = 0;
            add_subscr_item(HCP);
        }
    }
    
    if(has_item(itype_map, get_dlevel()))
    {
        add_subscr_item(new item((fix)0,(fix)0,(fix)0,curQuest->specialItems().map,0,0));
    }
    
    if(has_item(itype_compass, get_dlevel()))
    {
        add_subscr_item(new item((fix)0,(fix)0,(fix)0,curQuest->specialItems().compass,0,0));
    }
    
    if(has_item(itype_bosskey, get_dlevel()))
    {
        add_subscr_item(new item((fix)0,(fix)0,(fix)0,curQuest->specialItems().bossKey,0,0));
    }
    
    for(int i=0; i<itype_max; i++)
    {
        //special case: ignore the dmap-specific items processed above. -DD
        if(i == itype_map || i == itype_compass || i == itype_bosskey)
            continue;
            
        // Display the ring even if it has run out of magic.
        if(curQuest->isValid(current_item_id(i,false)))
        {
            ItemDefinitionRef j = current_item_id(i,false);
            
            if(curQuest->getItemDefinition(j).tile)
                add_subscr_item(new item((fix)0, (fix)0,(fix)0,j,0,0));
        }
    }
    
    new_sel=true;
}

void update_subscreens(int dmap)
{
    if(dmap<0)
        dmap=get_currdmap();
        
    int index=DMaps[dmap].active_subscreen;
    
    int i=-1, j=0;
    
    while(custom_subscreen[j].ss_objects.size() != 0 && custom_subscreen[j].ss_objects[0]->type!=ssoNULL && i!=index)
    {
        if(custom_subscreen[j].ss_type==sstACTIVE)
        {
            ++i;
        }
        
        ++j;
    }
    
    current_subscreen_active=&custom_subscreen[j-1];
    
    index=DMaps[dmap].passive_subscreen;
    
    i=-1, j=0;
    
    while(custom_subscreen[j].ss_objects.size() != 0 && custom_subscreen[j].ss_objects[0]->type!=ssoNULL&&i!=index)
    {
        if(custom_subscreen[j].ss_type==sstPASSIVE)
        {
            ++i;
        }
        
        ++j;
    }
    
    current_subscreen_passive=&custom_subscreen[j-1];
}

void purge_blank_subscreen_objects(subscreen_group *tempss)
{
    int objects=tempss->ss_objects.size();
    
    //filter all the ssoNONE items to the end (yeah, bubble sort; sue me)
    for(int j=0; j<objects-1; j++)
    {
        for(int k=0; k<objects-1-j; k++)
        {
            if(tempss->ss_objects[k]->type==ssoNONE)
            {
                subscreen_object *tempsso= tempss->ss_objects[k];
                tempss->ss_objects[k]=tempss->ss_objects[k+1];
                tempss->ss_objects[k+1]=tempsso;
            }
        }
    }
    
    //NULL out the ssoNONE items
    for(int j=0; j<objects; j++)
    {
        if(tempss->ss_objects[j]->type==ssoNONE)
        {
            tempss->ss_objects[j]->type=ssoNULL;
        }
    }
}


void sso_bounding_box(BITMAP *bmp, subscreen_group *tempss, int index, int color)
{
    if(index<0)
    {
        return;
    }
    
    int x = tempss->ss_objects[index]->sso_x();
    int y = tempss->ss_objects[index]->sso_y();
    int w = tempss->ss_objects[index]->sso_w();
    int h = tempss->ss_objects[index]->sso_h();
    
    switch(tempss->ss_objects[index]->get_alignment())
    {
    case sstaCENTER:
        x-=(w/2);
        break;
        
    case sstaRIGHT:
        x-=w;
        break;
        
    case sstaLEFT:
    default:
        break;
    }
    
    int c=x+w/2;
    int m=y+h/2;
    bool normalrect=true;
    
    if(color==-1)
    {
        color=jwin_pal[jcTITLER];
    }
    else if(color==-2)
    {
        color=jwin_pal[jcTITLEL];
    }
    
    //draw a bounding box around the current object
    if(normalrect)
    {
        rect(bmp, x, y, x+w-1, y+h-1, color);
    }
    else
    {
        rect(bmp, x-1,   y-1,   x+1, y+1, color);
        rect(bmp, x-1,   y+h-2, x+1, y+h, color);
        rect(bmp, x+w-2, y-1,   x+w, y+1, color);
        rect(bmp, x+w-2, y+h-2, x+w, y+h, color);
        
        if(w>=9)
        {
            rect(bmp, c-1, y-1,   c+1, y+1, color);
            rect(bmp, c-1, y+h-2, c+1, y+h, color);
            _allegro_hline(bmp, x+2,  y,     c-2,  color);
            _allegro_hline(bmp, c+2, y,     x+w-3, color);
            _allegro_hline(bmp, x+2,  y+h-1, c-2,  color);
            _allegro_hline(bmp, c+2, y+h-1, x+w-3, color);
        }
        else
        {
            _allegro_hline(bmp, x+2,  y,     x+w-3, color);
            _allegro_hline(bmp, x+2,  y+h-1, x+w-3, color);
        }
        
        if(h>=9)
        {
            rect(bmp, x-1,   m-1, x+1, m+1, color);
            rect(bmp, x+w-2, m-1, x+w, m+1, color);
            _allegro_vline(bmp, x,     y+2,  m-2,  color);
            _allegro_vline(bmp, x,     m+2, y+h-3, color);
            _allegro_vline(bmp, x+w-1, y+2,  m-2,  color);
            _allegro_vline(bmp, x+w-1, m+2, y+h-3, color);
        }
        else
        {
            _allegro_vline(bmp, x,     y+2,  y+h-3, color);
            _allegro_vline(bmp, x+w-1, y+2,  y+h-3, color);
        }
    }
}


/*** end of subscr.cc ***/

