#include "selectors.h"
#include "jwin.h"
#include "zquest.h"
#include "zsys.h"
#include "tiles.h"
#include "gui.h"
#include "zq_misc.h"
#include "backend/AllBackends.h"
#include <algorithm>

struct module_struct
{
    std::string s;
    bool operator<(const module_struct &other) const
    {
        bool iscore = (s == std::string("CORE"));
        bool otheriscore = (other.s == std::string("CORE"));
        if (iscore && !otheriscore)
            return true;
        if (otheriscore && !iscore)
            return false;
        return s < other.s;
    }
};

static std::vector<module_struct> modules_list;

static void build_modules_list()
{
    modules_list.clear();
    std::vector<std::string> modules;
    curQuest->getModules(modules);
    for (std::vector<std::string>::iterator it = modules.begin(); it != modules.end(); ++it)
    {
        module_struct ms;
        ms.s = *it;
        modules_list.push_back(ms);
    }

    std::sort(modules_list.begin(), modules_list.end());
}

static const char *modulelist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = modules_list.size();
        return NULL;
    }

    return modules_list[index].s.c_str();
}

int d_modlist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_abclist_proc(msg,d,c);

    switch (msg)
    {
    case MSG_CLICK:
    {
        int idx = d->d1;
        ((void(*)(const std::string &))d->dp3)(modules_list[idx].s);
        return D_REDRAW_ALL;
    }
    }

    return ret;
}

int d_en_modlist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_abclist_proc(msg,d,c);

    switch(msg)
    {
    case MSG_DRAW:
    case MSG_CHAR:
    case MSG_CLICK:

        int x = d->x + d->w + 4;
        int y = d->y;
        int w = 16;
        int h = 16;

        if(is_large())
        {
            w = 32;
            h = 32;
        }

        if (d->d1 < (int)modules_list.size())
        {
            std::string module = modules_list[d->d1].s;

            uint32_t numitems = curQuest->getModule(module).itemDefTable().getNumItemDefinitions();
            uint32_t numsprites = curQuest->getModule(module).spriteDefTable().getNumSpriteDefinitions();
            uint32_t numenemies = curQuest->getModule(module).enemyDefTable().getNumEnemyDefinitions();

            textprintf_ex(screen, (is_large() ? font : pfont), x, y + 0 * (is_large() ? 2 : 1), jwin_pal[jcTEXTFG], jwin_pal[jcBOX], "Module Contents:");
            textprintf_ex(screen, (is_large() ? font : pfont), x, y + 7 * (is_large() ? 2 : 1), jwin_pal[jcTEXTFG], jwin_pal[jcBOX], "%d items   ", numitems);
            textprintf_ex(screen, (is_large() ? font : pfont), x, y + 14 * (is_large() ? 2 : 1), jwin_pal[jcTEXTFG], jwin_pal[jcBOX], "%d sprites   ", numsprites);
            textprintf_ex(screen, (is_large() ? font : pfont), x, y + 21 * (is_large() ? 2 : 1), jwin_pal[jcTEXTFG], jwin_pal[jcBOX], "%d enemies   ", numenemies);
        }
    }

    return ret;
}

static DIALOG mlist_dlg[] =
{
    // (dialog proc)        (x)         (y)         (w)                 (h)         (fg)                (bg)                (key)       (flags)         (d1)    (d2)    (dp)                (dp2)   (d3)
    { jwin_win_proc,        60 - 12,    40,         200 + 24 + 24,      148+12+25,  vc(14),             vc(1),              0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { d_timer_proc,         0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            },
    { d_en_modlist_proc,    72 - 12 - 4,60 + 4,     100 + 24 + 8,       92 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { jwin_button_proc,     60,         164,        61,                 21,         vc(14),             vc(1),              13,         D_EXIT,         0,      0,      (void *) "OK",      NULL,   NULL            },
    { jwin_button_proc,     135,        164,        61,                 21,         vc(14),             vc(1),              27,         D_EXIT,         0,      0,      (void *) "Cancel",  NULL,   NULL            },
    { jwin_button_proc,     210,        164,        61,                 21,         vc(14),             vc(1),              0,          D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { jwin_button_proc,     60+37,      164 + 25,   61,                 21,         vc(14),             vc(1),              0,          D_EXIT,         0,      0,      (void *) "Cancel",  NULL,   NULL            },
    { jwin_button_proc,     135+37,     164 + 25,   61,                 21,         vc(14),             vc(1),              0,          D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { NULL,                 0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            }
};

std::string select_module(const char *prompt, const std::string &selectedModule, bool is_editor, int &exit_status)
{
    build_modules_list();

    int modindex = 0; // defaults to CORE if module is invalid

    for (int j = 0; j < (int)modules_list.size(); j++)
    {
        if (modules_list[j].s == selectedModule)
        {
            modindex = j;
        }
    }

    mlist_dlg[0].dp = (void *)prompt;
    mlist_dlg[0].dp2 = lfont;
    mlist_dlg[2].d1 = modindex;
    ListData module_list(modulelist, &font);
    mlist_dlg[2].dp = (void *)&module_list;

    DIALOG *mlist_cpy = resizeDialog(mlist_dlg, 1.5);

    if (is_editor)
    {
        mlist_cpy[3].dp = (void *)"Done";
        mlist_cpy[4].dp = (void *)"Create";
        mlist_cpy[5].dp = (void *)"Delete";
        mlist_cpy[6].dp = (void *)"Import";
        mlist_cpy[7].dp = (void *)"Export";
        mlist_cpy[5].flags &= ~D_HIDDEN;
        mlist_cpy[6].flags &= ~D_HIDDEN;
        mlist_cpy[7].flags &= ~D_HIDDEN;
    }
    else
    {
        mlist_cpy[3].dp = (void *)"OK";
        mlist_cpy[4].dp = (void *)"Cancel";
        mlist_cpy[5].flags |= D_HIDDEN;
        mlist_cpy[6].flags |= D_HIDDEN;
        mlist_cpy[7].flags |= D_HIDDEN;
    }

    exit_status = zc_popup_dialog(mlist_cpy, 2);
    modindex = mlist_cpy[2].d1;
    delete[] mlist_cpy;
    Backend::mouse->setWheelPosition(0);
    return modindex < (int)modules_list.size() ? modules_list[modindex].s : std::string("");
}

//////// ITEMS ////////

struct item_struct
{
    std::string s;
    ItemDefinitionRef i;

    bool operator<(const item_struct &other) const
    {
        bool isnone = (s == std::string("(None)"));
        bool otherisnone = (other.s == std::string("(None)"));
        if (isnone && !otherisnone)
            return true;
        else if (otherisnone && !isnone)
            return false;
        return s < other.s;
    }
};

static std::vector<item_struct> items_list;

static void build_items_list(const std::string &module)
{
    items_list.clear();

    item_struct none;
    none.s = std::string("(None)");
    none.i = ItemDefinitionRef();
    items_list.push_back(none);

    uint32_t numitems = curQuest->getModule(module).itemDefTable().getNumItemDefinitions();

    for (uint32_t i = 0; i<numitems; i++)
    {
        item_struct is;
        is.s = curQuest->getModule(module).itemDefTable().getItemDefinition(i).name;
        is.i = ItemDefinitionRef(module, i);
        items_list.push_back(is);
    }

    std::sort(items_list.begin(), items_list.end());
}



static const char *itemlist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = items_list.size();
        return NULL;
    }

    return items_list[index].s.c_str();
}

int d_ilist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_abclist_proc(msg,d,c);

    // invalid selection, so reset it
    if (d->d1 >= (int)items_list.size())
    {
        d->d1 = 0;
        d->d2 = 0;
        ret = jwin_abclist_proc(MSG_DRAW,d,c);
    }

    switch(msg)
    {
    case MSG_DRAW:
    case MSG_CHAR:
    case MSG_CLICK:

        int tile = 0;
        int cset = 0;

        if(curQuest->isValid(items_list[d->d1].i))
        {
            tile= curQuest->getItemDefinition(items_list[d->d1].i).tile;
            cset= curQuest->getItemDefinition(items_list[d->d1].i).csets&15;
        }

        int x = d->x + d->w + 4;
        int y = d->y;
        int w = 16;
        int h = 16;

        if(is_large())
        {
            w = 32;
            h = 32;
        }

        BITMAP *buf = create_bitmap_ex(8,16,16);
        BITMAP *bigbmp = create_bitmap_ex(8,w,h);

        if(buf && bigbmp)
        {
            clear_bitmap(buf);

            if(tile)
                overtile16(buf, tile,0,0,cset,0);

            stretch_blit(buf, bigbmp, 0,0, 16, 16, 0, 0, w, h);
            destroy_bitmap(buf);
            jwin_draw_frame(screen,x,y,w+4,h+4,FR_DEEP);
            blit(bigbmp,screen,0,0,x+2,y+2,w,h);
            destroy_bitmap(bigbmp);
        }
        if(curQuest->isValid(items_list[d->d1].i))
        {
            textprintf_ex(screen,spfont,x,y+20*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"#%d  ",items_list[d->d1].i.slot);

            textprintf_ex(screen,spfont,x,y+26*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Pow:    ");
            textprintf_ex(screen,spfont,x+int(16*(is_large()?1.5:1)),y+26*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",curQuest->getItemDefinition(items_list[d->d1].i).power);
        }

        // Might be a bit confusing for new users
        /*textprintf_ex(screen,is_large?font:spfont,x,y+32*(is_large?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Cost:   ");
        textprintf_ex(screen,is_large?font:spfont,x+int(16*(is_large?1.5:1)),y+32*(is_large?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d",itemsbuf[bii[d->d1].i].magic);*/

    }

    return ret;
}

static ItemDefinitionRef copiedItem;

static MENU ilist_rclick_menu[] =
{
    { (char *)"Copy",  NULL, NULL, 0, NULL },
    { (char *)"Paste", NULL, NULL, 0, NULL },
    { NULL,            NULL, NULL, 0, NULL }
};

static void ilist_rclick_func(int index, int x, int y)
{
    if(!curQuest->isValid(items_list[index].i)) // Clicked (none)?
        return;

    if(!curQuest->isValid(copiedItem))
        ilist_rclick_menu[1].flags|=D_DISABLED;
    else
        ilist_rclick_menu[1].flags&=~D_DISABLED;

    int ret=popup_menu(ilist_rclick_menu, x, y);

    if(ret==0) // copy
        copiedItem=items_list[index].i;
    else if(ret==1) // paste
    {
        std::string oldname = curQuest->getItemDefinition(items_list[index].i).name;
        curQuest->getItemDefinition(items_list[index].i) = curQuest->getItemDefinition(copiedItem);
        curQuest->getItemDefinition(items_list[index].i).name = oldname;
        saved=false;
    }
}

static void rebuild_ilist(const std::string &module)
{
    build_items_list(module);
}

static DIALOG ilist_dlg[] =
{
    // (dialog proc)        (x)         (y)         (w)                 (h)         (fg)                (bg)                (key)       (flags)         (d1)    (d2)    (dp)                (dp2)   (d3)
    { jwin_win_proc,        60 - 12,    40,         200 + 24 + 24,      194+12,     vc(14),             vc(1),              0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { d_timer_proc,         0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            },
    { d_modlist_proc,       72 - 12 - 4,60 + 4,     176 + 24 + 8,       46 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   &rebuild_ilist  },
    { d_ilist_proc,         72 - 12 - 4,60 + 50 + 4,176 + 24 + 8,       92 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { jwin_button_proc,     60,         213,        61,                 21,         vc(14),             vc(1),              13,         D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { jwin_button_proc,     135,        213,        61,                 21,         vc(14),             vc(1),              27,         D_EXIT,         0,      0,      (void *) "Done",    NULL,   NULL            },
    { jwin_button_proc,     210,        213,        61,                 21,         vc(14),             vc(1),              0,          D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { NULL,                 0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            }
};

ItemDefinitionRef select_item(const char *prompt, const ItemDefinitionRef &item, bool is_editor, int &exit_status)
{
    build_modules_list();

    int modindex = 0; // defaults to CORE if module is invalid

    for (int j = 0; j < (int)modules_list.size(); j++)
    {
        if (modules_list[j].s == item.module)
        {
            modindex = j;
        }
    }

    build_items_list(modules_list[modindex].s);

    int itemindex = 0;

    for (int j = 0; j < (int)items_list.size(); j++)
    {
        if (items_list[j].i == item)
        {
            itemindex = j;
        }
    }

    ilist_dlg[0].dp = (void *)prompt;
    ilist_dlg[0].dp2 = lfont;
    ilist_dlg[2].d1 = modindex;
    ListData module_list(modulelist, &font);
    ilist_dlg[2].dp = (void *)&module_list;

    ilist_dlg[3].d1 = itemindex;
    ListData item_list(itemlist, &font);
    ilist_dlg[3].dp = (void *)&item_list;

    DIALOG *ilist_cpy = resizeDialog(ilist_dlg, 1.5);

    if (is_editor)
    {
        ilist_cpy[3].dp3 = (void *)&ilist_rclick_func;
        ilist_cpy[3].flags |= (D_USER << 1);
        ilist_cpy[4].dp = (void *)"Edit";
        ilist_cpy[5].dp = (void *)"Done";
        ilist_cpy[6].dp = (void *)"New";
        ilist_cpy[6].flags &= ~D_HIDDEN;
    }
    else
    {
        ilist_cpy[3].dp3 = NULL;
        ilist_cpy[3].flags &= ~(D_USER << 1);
        ilist_cpy[4].dp = (void *)"OK";
        ilist_cpy[5].dp = (void *)"Cancel";
        ilist_cpy[6].flags |= D_HIDDEN;
    }

    while (true)
    {
        exit_status = zc_popup_dialog(ilist_cpy, 2);

        if (exit_status == 0 || exit_status == 5) // Done / Cancel
        {
            Backend::mouse->setWheelPosition(0);
            copiedItem = ItemDefinitionRef();
            delete[] ilist_cpy;
            return ItemDefinitionRef();
        }

        else if (exit_status == 6) // New
        {
            modindex = ilist_cpy[2].d1;
            if (modindex >= (int)modules_list.size())
                modindex = 0;
            ilist_cpy[2].d1 = modindex;
            itemdata idata;
            idata.name = std::string("(New Item)");
            curQuest->getModule(modules_list[modindex].s).itemDefTable().addItemDefinition(idata);
            build_items_list(modules_list[modindex].s);
            saved = false;
        }

        else if (exit_status == 4) // Edit / OK
        {
            modindex = ilist_cpy[2].d1;
            itemindex = ilist_cpy[3].d1;
            delete[] ilist_cpy;
            Backend::mouse->setWheelPosition(0);
            copiedItem = ItemDefinitionRef();
            return itemindex < (int)items_list.size() ? items_list[itemindex].i : ItemDefinitionRef();
        }
    }
}

//////// SPRITES ////////


struct sprite_struct
{
    std::string s;
    SpriteDefinitionRef i;

    bool operator<(const sprite_struct &other) const
    {
        bool isnone = (s == std::string("(None)"));
        bool otherisnone = (other.s == std::string("(None)"));
        if (isnone && !otherisnone)
            return true;
        else if (otherisnone && !isnone)
            return false;
        return s < other.s;
    }
};


std::vector<sprite_struct> sprites_list;

void build_sprites_list(const std::string &module)
{
    sprites_list.clear();

    sprite_struct none;
    none.s = std::string("(None)");
    none.i = SpriteDefinitionRef();
    sprites_list.push_back(none);

    uint32_t numsprites = curQuest->getModule(module).spriteDefTable().getNumSpriteDefinitions();
    for (uint32_t i = 0; i < numsprites; i++)
    {
        sprite_struct ss;
        ss.s = curQuest->getModule(module).spriteDefTable().getSpriteDefinition(i).name;
        ss.i = SpriteDefinitionRef(module, i);
        sprites_list.push_back(ss);
    }

    std::sort(sprites_list.begin(), sprites_list.end());
}

static const char *spritelist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = sprites_list.size();
        return NULL;
    }

    return sprites_list[index].s.c_str();
}

static SpriteDefinitionRef copiedSprite;

static MENU wlist_rclick_menu[] =
{
    { (char *)"Copy",  NULL, NULL, 0, NULL },
    { (char *)"Paste", NULL, NULL, 0, NULL },
    { NULL,            NULL, NULL, 0, NULL }
};

static void wlist_rclick_func(int index, int x, int y)
{
    if(!curQuest->isValid(sprites_list[index].i)) // Clicked (none)?
        return;

    if(!curQuest->isValid(copiedSprite))
        wlist_rclick_menu[1].flags|=D_DISABLED;
    else
        wlist_rclick_menu[1].flags&=~D_DISABLED;

    int ret=popup_menu(wlist_rclick_menu, x, y);

    if(ret==0) // copy
        copiedSprite=sprites_list[index].i;
    else if(ret==1) // paste
    {
        std::string oldname = curQuest->getSpriteDefinition(sprites_list[index].i).name;
        curQuest->getSpriteDefinition(sprites_list[index].i) = curQuest->getSpriteDefinition(copiedSprite);
        curQuest->getSpriteDefinition(sprites_list[index].i).name = oldname;
        saved=false;
    }
}


static void rebuild_slist(const std::string &module)
{
    build_sprites_list(module);
}

int d_wlist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_abclist_proc(msg,d,c);
    
    // invalid selection, so reset it
    if (d->d1 >= (int)sprites_list.size())
    {
        d->d1 = 0;
        d->d2 = 0;
        ret = jwin_abclist_proc(MSG_DRAW,d,c);
    }
    
    switch (msg)
    {
    case MSG_DRAW:
    case MSG_CHAR:
    case MSG_CLICK:

        int tile = 0;
        int cset = 0;
        if (curQuest->isValid(sprites_list[d->d1].i))
        {
            tile = curQuest->getSpriteDefinition(sprites_list[d->d1].i).tile;
            cset = curQuest->getSpriteDefinition(sprites_list[d->d1].i).csets & 15;
        }
        int x = d->x + d->w + 4;
        int y = d->y;
        int w = 16;
        int h = 16;

        if (is_large())
        {
            w = 32;
            h = 32;
        }

        BITMAP *buf = create_bitmap_ex(8, 16, 16);
        BITMAP *bigbmp = create_bitmap_ex(8, w, h);

        if (buf && bigbmp)
        {
            clear_bitmap(buf);

            if (tile)
                overtile16(buf, tile, 0, 0, cset, 0);

            stretch_blit(buf, bigbmp, 0, 0, 16, 16, 0, 0, w, h);
            destroy_bitmap(buf);
            jwin_draw_frame(screen, x, y, w + 4, h + 4, FR_DEEP);
            blit(bigbmp, screen, 0, 0, x + 2, y + 2, w, h);
            destroy_bitmap(bigbmp);
        }

        if (curQuest->isValid(sprites_list[d->d1].i))
        {
            textprintf_ex(screen, is_large() ? font : spfont, x, y + 20 * (is_large() ? 2 : 1), jwin_pal[jcTEXTFG], jwin_pal[jcBOX], "#%d   ", sprites_list[d->d1].i.slot);
        }
    }
    
    return ret;
}

static DIALOG wlist_dlg[] =
{
    // (dialog proc)        (x)         (y)         (w)                 (h)         (fg)                (bg)                (key)       (flags)         (d1)    (d2)    (dp)                (dp2)   (d3)
    { jwin_win_proc,        60 - 12,    40,         200 + 24 + 24,      194+12,     vc(14),             vc(1),              0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { d_timer_proc,         0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            },
    { d_modlist_proc,       72 - 12 - 4,60 + 4,     176 + 24 + 8,       46 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   &rebuild_slist  },
    { d_wlist_proc,         72 - 12 - 4,60 + 50 + 4,176 + 24 + 8,       92 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { jwin_button_proc,     60,         213,        61,                 21,         vc(14),             vc(1),              13,         D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { jwin_button_proc,     135,        213,        61,                 21,         vc(14),             vc(1),              27,         D_EXIT,         0,      0,      (void *) "Done",    NULL,   NULL            },
    { jwin_button_proc,     210,        213,        61,                 21,         vc(14),             vc(1),              0,          D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { NULL,                 0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            }
};

SpriteDefinitionRef select_sprite(const char *prompt, const SpriteDefinitionRef &selectedSprite, bool is_editor, int &exit_status)
{
    build_modules_list();

    int modindex = 0; // defaults to CORE if module is invalid

    for (int j = 0; j < (int)modules_list.size(); j++)
    {
        if (modules_list[j].s == selectedSprite.module)
        {
            modindex = j;
        }
    }

    build_sprites_list(modules_list[modindex].s);

    int sprindex = 0;

    for (int j = 0; j < (int)sprites_list.size(); j++)
    {
        if (sprites_list[j].i == selectedSprite)
        {
            sprindex = j;
        }
    }

    wlist_dlg[0].dp = (void *)prompt;
    wlist_dlg[0].dp2 = lfont;

    wlist_dlg[2].d1 = modindex;
    ListData module_list(modulelist, &font);
    wlist_dlg[2].dp = (void *)&module_list;

    wlist_dlg[3].d1 = sprindex;
    ListData sprite_list(spritelist, &font);
    wlist_dlg[3].dp = (void *)&sprite_list;

    DIALOG *wlist_cpy = resizeDialog(wlist_dlg, 1.5);

    if (is_editor)
    {
        wlist_cpy[3].dp3 = (void *)&wlist_rclick_func;
        wlist_cpy[3].flags |= (D_USER << 1);
        wlist_cpy[4].dp = (void *)"Edit";
        wlist_cpy[5].dp = (void *)"Done";
        wlist_cpy[6].dp = (void *)"New";
        wlist_cpy[6].flags &= ~D_HIDDEN;
    }
    else
    {
        wlist_cpy[3].dp3 = NULL;
        wlist_cpy[3].flags &= ~(D_USER << 1);
        wlist_cpy[4].dp = (void *)"OK";
        wlist_cpy[5].dp = (void *)"Cancel";
        wlist_cpy[6].flags |= D_HIDDEN;
    }

    while (true)
    {
        exit_status = zc_popup_dialog(wlist_cpy, 2);

        if (exit_status == 0 || exit_status == 5) // Done / Cancel
        {
            Backend::mouse->setWheelPosition(0);
            copiedSprite = SpriteDefinitionRef();
            delete[] wlist_cpy;
            return SpriteDefinitionRef();
        }

        else if (exit_status == 6) // New
        {
            modindex = wlist_cpy[2].d1;
            if (modindex >= (int)modules_list.size())
                modindex = 0;
            wlist_cpy[2].d1 = modindex;
            wpndata wdata;
            wdata.name = std::string("(New Sprite)");
            curQuest->getModule(modules_list[modindex].s).spriteDefTable().addSpriteDefinition(wdata);
            build_sprites_list(modules_list[modindex].s);
            saved = false;
        }

        else if (exit_status == 4) // Edit / OK
        {
            modindex = wlist_cpy[2].d1;
            sprindex = wlist_cpy[3].d1;
            delete[] wlist_cpy;
            Backend::mouse->setWheelPosition(0);
            copiedItem = ItemDefinitionRef();
            return sprindex < (int)sprites_list.size() ? sprites_list[sprindex].i : SpriteDefinitionRef();
        }
    }
}

//////// ENEMIES ////////

struct enemy_struct
{
    std::string s;
    EnemyDefinitionRef i;

    bool operator<(const enemy_struct &other) const
    {
        bool isnone = (s == std::string("(None)"));
        bool otherisnone = (other.s == std::string("(None)"));
        if (isnone && !otherisnone)
            return true;
        else if (otherisnone && !isnone)
            return false;
        return s < other.s;
    }
};

static std::vector<enemy_struct> enemies_list;

static int enemyfilter=0;

static void build_enemies_list(const std::string &module)
{
    enemies_list.clear();

    enemy_struct toadd;
    toadd.s = std::string("(None)");
    toadd.i = EnemyDefinitionRef();
    enemies_list.push_back(toadd);
    
    uint32_t numenemies = curQuest->getModule(module).enemyDefTable().getNumEnemyDefinitions();

    for (uint32_t i = 0; i < numenemies; i++)
    {
        bool keep = false;
        if (enemyfilter & EnemySelectFlags::ESF_BADGUYS)
        {

            if (curQuest->getModule(module).enemyDefTable().getEnemyDefinition(i).family != eeGUY && curQuest->getModule(module).enemyDefTable().getEnemyDefinition(i).family != eeNONE)
            {
                keep = true;
            }
        }

        if (enemyfilter & EnemySelectFlags::ESF_GOODGUYS)
        {
            if (curQuest->getModule(module).enemyDefTable().getEnemyDefinition(i).family == eeGUY)
            {
                keep = true;
            }
        }

        if (keep)
        {
            enemy_struct toadd;
            toadd.s = curQuest->getModule(module).enemyDefTable().getEnemyDefinition(i).name;
            toadd.i = EnemyDefinitionRef(module, i);
            enemies_list.push_back(toadd);
        }
    }

    std::sort(enemies_list.begin(), enemies_list.end());
}

static const char *enemylist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = enemies_list.size();
        return NULL;
    }

    return enemies_list[index].s.c_str();
}

static EnemyDefinitionRef copiedGuy;

static MENU elist_rclick_menu[] =
{
    { (char *)"Copy",  NULL, NULL, 0, NULL },
    { (char *)"Paste", NULL, NULL, 0, NULL },
    { NULL,            NULL, NULL, 0, NULL }
};

static void elist_rclick_func(int index, int x, int y)
{
    if(!curQuest->isValid(enemies_list[index].i)) // Clicked (none)?
        return;

    if(!curQuest->isValid(copiedGuy))
        elist_rclick_menu[1].flags|=D_DISABLED;
    else
        elist_rclick_menu[1].flags&=~D_DISABLED;

    int ret=popup_menu(elist_rclick_menu, x, y);

    if(ret==0) // copy
        copiedGuy=enemies_list[index].i;
    else if(ret==1) // paste
    {
        curQuest->getEnemyDefinition(enemies_list[index].i)=curQuest->getEnemyDefinition(copiedGuy);
        saved=false;
    }
}


static void rebuild_elist(const std::string &module)
{
    build_enemies_list(module);
}

int d_enelist_proc(int msg,DIALOG *d,int c)
{
    int ret = jwin_abclist_proc(msg,d,c); 
                                          
    // invalid selection, so reset it
    if (d->d1 >= (int)enemies_list.size())
    {
        d->d1 = 0;
        d->d2 = 0;
        ret = jwin_abclist_proc(MSG_DRAW,d,c);
    }

    switch (msg)
    {
    case MSG_DRAW:
    case MSG_CHAR:
    case MSG_CLICK:        
        EnemyDefinitionRef id = enemies_list[d->d1].i;

        int tile = 0;
        int cset = 0;
        if (curQuest->isValid(id))
        {
            tile = get_bit(quest_rules, qr_NEWENEMYTILES) ? curQuest->getEnemyDefinition(id).e_tile
                : curQuest->getEnemyDefinition(id).tile;
            cset = curQuest->getEnemyDefinition(id).cset;
        }

        int x = d->x + int(212 * (is_large() ? 1.5:1));
        int y = d->y + int(2 * (is_large() ? 1.5:1));
        int w = 20;
        int h = 20;

        if(is_large())
        {
            w = 36;
            h = 36;
        }

        BITMAP *buf = create_bitmap_ex(8,20,20);
        BITMAP *bigbmp = create_bitmap_ex(8,w,h);

        if(buf && bigbmp)
        {
            clear_bitmap(buf);

            if(tile)
                overtile16(buf, tile+efrontfacingtile(id),2,2,cset,0);

            stretch_blit(buf, bigbmp, 2,2, 17, 17, 2, 2,w-2, h-2);
            destroy_bitmap(buf);
            jwin_draw_frame(bigbmp,0,0,w,h,FR_DEEP);
            blit(bigbmp,screen,0,0,x,y,w,h);
            destroy_bitmap(bigbmp);
        }

        /*
        rectfill(screen, x, y+20*(is_large?2:1), x+int(w*(is_large?1.5:1))-1, y+32*(is_large?2:1)-1, vc(4));
        */
        textprintf_ex(screen,is_large()?font:spfont,x,y+20*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"#%d   ",id.slot);

        textprintf_ex(screen,is_large()?font:spfont,x,y+26*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"HP :");

        int hp = 0;
        int dp = 0;
        if (curQuest->isValid(id))
        {
            hp = curQuest->getEnemyDefinition(id).hp;
            dp = curQuest->getEnemyDefinition(id).dp;
        }

        textprintf_ex(screen,is_large()?font:spfont,x+int(14*(is_large()?1.5:1)),y+26*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d   ",hp);

        textprintf_ex(screen,is_large()?font:spfont,x,y+32*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"Dmg:");
        textprintf_ex(screen,is_large()?font:spfont,x+int(14*(is_large()?1.5:1)),y+32*(is_large()?2:1),jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%d   ",dp);
    }

    return ret;
}

static DIALOG elist_dlg[] =
{
    // (dialog proc)        (x)         (y)         (w)                 (h)         (fg)                (bg)                (key)       (flags)         (d1)    (d2)    (dp)                (dp2)   (d3)
    { jwin_win_proc,        60 - 12,    40,         200 + 24 + 24,      194+12,     vc(14),             vc(1),              0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { d_timer_proc,         0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            },
    { d_modlist_proc,       72 - 12 - 4,60 + 4,     176 + 24 + 8,       46 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   &rebuild_elist  },
    { d_enelist_proc,       72 - 12 - 4,60 + 50 + 4,176 + 24 + 8,       92 + 3,     jwin_pal[jcTEXTFG], jwin_pal[jcTEXTBG], 0,          D_EXIT,         0,      0,      NULL,               NULL,   NULL            },
    { jwin_button_proc,     60,         213,        61,                 21,         vc(14),             vc(1),              13,         D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { jwin_button_proc,     135,        213,        61,                 21,         vc(14),             vc(1),              27,         D_EXIT,         0,      0,      (void *) "Done",    NULL,   NULL            },
    { jwin_button_proc,     210,        213,        61,                 21,         vc(14),             vc(1),              0,          D_EXIT,         0,      0,      (void *) "Edit",    NULL,   NULL            },
    { NULL,                 0,          0,          0,                  0,          0,                  0,                  0,          0,              0,      0,      NULL,               NULL,   NULL            }
};

EnemyDefinitionRef select_enemy(const char *prompt, const EnemyDefinitionRef &selectedEnemy, int enemySelectFlags, bool is_editor, int &exit_status)
{
    enemyfilter = enemySelectFlags;

    build_modules_list();

    int modindex = 0; // defaults to CORE if module is invalid

    for (int j = 0; j < (int)modules_list.size(); j++)
    {
        if (modules_list[j].s == selectedEnemy.module)
        {
            modindex = j;
        }
    }

    build_enemies_list(modules_list[modindex].s);

    int eneindex=0;

    for(int j=0; j<(int)enemies_list.size(); j++)
    {
        if(enemies_list[j].i == selectedEnemy)
        {
            eneindex=j;
        }
    }

    elist_dlg[0].dp=(void *)prompt;
    elist_dlg[0].dp2=lfont;

    elist_dlg[2].d1 = modindex;
    ListData mod_list(modulelist, &font);
    elist_dlg[2].dp = (void *)&mod_list;

    elist_dlg[3].d1=eneindex;
    ListData enemy_list(enemylist, &font);
    elist_dlg[3].dp=(void *) &enemy_list;

    DIALOG *elist_cpy = resizeDialog(elist_dlg, 1.5);

    if (is_editor)
    {
        elist_cpy[3].dp3 = (void *)&elist_rclick_func;
        elist_cpy[3].flags |= (D_USER << 1);
        elist_cpy[4].dp = (void *)"Edit";
        elist_cpy[5].dp = (void *)"Done";
        elist_cpy[6].dp = (void *)"New";
        elist_cpy[6].flags &= ~D_HIDDEN;
    }
    else
    {
        elist_cpy[3].dp3 = NULL;
        elist_cpy[3].flags &= ~(D_USER << 1);
        elist_cpy[4].dp = (void *)"OK";
        elist_cpy[5].dp = (void *)"Cancel";
        elist_cpy[6].flags |= D_HIDDEN;
    }

    while (true)
    {
        exit_status = zc_popup_dialog(elist_cpy, 2);

        if (exit_status == 0 || exit_status == 5) // Done / Cancel
        {
            Backend::mouse->setWheelPosition(0);
            copiedGuy = EnemyDefinitionRef();
            delete[] elist_cpy;
            return EnemyDefinitionRef();
        }

        else if (exit_status == 6) // New
        {
            modindex = elist_cpy[2].d1;
            if (modindex >= (int)modules_list.size())
                modindex = 0;
            elist_cpy[2].d1 = modindex;
            guydata gdata;
            if (enemyfilter & ESF_BADGUYS)
            {
                gdata.family = eeWALK;
                gdata.name = std::string("(New Enemy)");
            }
            else
            {
                gdata.family = eeGUY;
                gdata.name = std::string("(New Guy)");
            }
            curQuest->getModule(modules_list[modindex].s).enemyDefTable().addEnemyDefinition(gdata);
            build_enemies_list(modules_list[modindex].s);
            saved = false;
        }

        else if (exit_status == 4) // Edit / OK
        {
            modindex = elist_cpy[2].d1;
            eneindex = elist_cpy[3].d1;
            delete[] elist_cpy;
            Backend::mouse->setWheelPosition(0);
            copiedGuy = EnemyDefinitionRef();
            return eneindex < (int)enemies_list.size() ? enemies_list[eneindex].i : EnemyDefinitionRef();
        }
    }
}