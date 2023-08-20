//--------------------------------------------------------
//  ZQuest Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#ifndef _ZQ_SUBSCR_H_
#define _ZQ_SUBSCR_H_

//subscreen object alignment
enum { ssoaLEFT, ssoaCENTER, ssoaRIGHT, ssoaTOP, ssoaMIDDLE, ssoaBOTTOM };

//subscreen object distribution
enum { ssodLEFT, ssodCENTER, ssodRIGHT, ssodTOP, ssodMIDDLE, ssodBOTTOM };

//subscreen object snapping
enum { ssosLEFT, ssosCENTER, ssosRIGHT, ssosTOP, ssosMIDDLE, ssosBOTTOM };

bool edit_subscreen();
int32_t onEditSubscreens();

void center_zq_subscreen_dialogs();
void update_sso_name();
void update_sscpy_name();
void update_up_dn_btns();
extern bool show_new_ss;
extern DIALOG sslist_dlg[];

#endif
