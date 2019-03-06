static int itemdata_weaponscript_list[] =
{
    // dialog control number
	275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,
	292,
	-1
};



static TABPANEL itemdata_tabs[] =
{
    // (text)
    { (char *)"Data",         D_SELECTED,    itemdata_flags_list,         0, NULL },
    { (char *)"Flags",        0,    itemdata_flags2_list,         0, NULL },
    { (char *)"GFX",          0,             itemdata_gfx_list,           0, NULL },
    { (char *)"Pickup",       0,             itemdata_pickup_list,        0, NULL },
    { (char *)"Action",       0,             itemdata_action_list,        0, NULL },
    { (char *)"Scripts",      0,             itemdata_scriptargs_list,    0, NULL },
    { (char *)"W. Script",      0,             itemdata_weaponscript_list,    0, NULL },
    { (char *)"Size",      0,             itemdata_itemsize_list,    0, NULL },
   //  { (char *)"Weapon",      0,             itemdata_weaponargs_list,    0, NULL },
    { (char *)"Weapon Size",      0,             itemdata_weaponsize_list,    0, NULL },
    { NULL,                   0,             NULL,                        0, NULL }
};

const char *eweaponscriptdroplist(int index, int *list_size)
{
    if(index<0)
    {
        *list_size = bieweapons_cnt;
        return NULL;
    }
    
    return bieweapons[index].first.c_str();
}


//droplist like the dialog proc, naming scheme for this stuff is awful...
static ListData eweaponscript_list(eweaponscriptdroplist, &pfont);
