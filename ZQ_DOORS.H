//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zquest.cc
//
//  Main code for the quest editor.
//
//--------------------------------------------------------

/*
char *doors_string[8]={"Wall","Passage","Locked","Shutter","Bombable","Walk-through","1-Way Shutter","Boss"};
char *doorslist(int index, int *list_size)
void edit_door(int side)
char *doorcombosetlist(int index, int *list_size);
static DIALOG door_select_dlg[] =
DoorComboSet working_dcs;
void fix_dcs(int dir)
static int door_north_list[] =
static int door_south_list[] =
static int door_east_list[] =
static int door_west_list[] =
static TABPANEL door_tabs[] =
int dcs_auto_button_proc(int msg, DIALOG *d, int c);
static DIALOG doorcomboset_dlg[] =
void fill_dcs_dlg()
void extract_dcs_dlg()
int dcs_auto_button_proc(int msg, DIALOG *d, int c)
int edit_dcs(int index)
void editdoorcomboset(int index)
char *doorcombosetlist(int index, int *list_size)
int doorcombosetlist_del();
int copydcs();
int pastedcs();
int replacedcs();
static DIALOG doorcombosetlist_dlg[] =
void reset_doorcomboset(int index);
void fix_door_combo_set(word &door_combo_set, byte index);
int doorcombosetlist_del()
int copydcs()
int pastedcs()
int replacedcs()
void fix_door_combo_set(word &door_combo_set, byte index)
void reset_doorcomboset(int index)
void init_doorcombosets()
*/
extern int onDoors();
extern int onDoorCombos();
extern void center_zq_door_dialogs();
