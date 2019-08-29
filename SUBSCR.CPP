/*
  switch (textstyle)
  {
  case sstsNORMAL:
  break;
  case sstsSHADOW:
  break;
  case sstsSHADOWU:
  break;
  case sstsOUTLINE8:
  break;
  case sstsOUTLINEPLUS:
  break;
  case sstsOUTLINEX:
  break;
  case sstsSHADOWED:
  break;
  case sstsSHADOWEDU:
  break;
  case sstsOUTLINED8:
  break;
  case sstsOUTLINEDPLUS:
  break;
  case sstsOUTLINEDX:
  break;
  }

  */

//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  subscr.cc
//
//  Subscreen code for zelda.cc
//
//--------------------------------------------------------

#include "subscr.h"
//#include "guys.h"
#include "zelda.h"
#include "tiles.h"
#include "zsys.h"
#include "guys.h"
#include "link.h"
#include "gamedata.h"
#include <stdio.h>
#include <string.h>

bool show_subscreen_dmap_dots=true;
bool show_subscreen_numbers=true;
bool show_subscreen_items=true;
bool show_subscreen_life=true;
bool new_sel=false;

extern sprite_list  guys, items, Ewpns, Lwpns, Sitems, chainlinks, decorations;
extern LinkClass   Link;

subscreen_group *current_subscreen_active;
subscreen_group *current_subscreen_passive;

subscreen_group custom_subscreen[MAXCUSTOMSUBSCREENS];

subscreen_object z3_active[200]=
{
  //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                frames    speed    delay    frame     dp1
  { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,                 8,      0,     19,      12,       9,                    0,                            0,                    0,                            0,                    0,                        14089,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,                 8,     96,     19,       9,       9,                    0,                            0,                    0,                            0,                    0,                        14089,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,               168,      0,     10,       6,       9,                    0,                            0,                    0,                            0,                    0,                        14089,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,               168,     48,     10,       7,       9,                    0,                            0,                    0,                            0,                    0,                        14089,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,               168,    104,     10,       8,       9,                    0,                            0,                    0,                            0,                    0,                        14089,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { sso2X2FRAME,                    0,                 8,      0,     19,      12,       9,                    0,                            0,                    0,                            0,                    0,                        14087,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,                 8,     96,     19,       9,       8,                    0,                            0,                    0,                            0,                    0,                        14085,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,               168,      0,     10,       6,       9,                    0,                            0,                    0,                            0,                    0,                        14087,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,               168,     48,     10,       7,       8,                    0,                            0,                    0,                            0,                    0,                        14083,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,               168,    104,     10,       8,       8,                    0,                            0,                    0,                            0,                    0,                        14083,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoCURRENTITEM,                 0,               176,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSWORD,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               200,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               224,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               200,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoCURRENTITEM,                 0,                28,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                52,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                76,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               100,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               124,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoCURRENTITEM,                 0,                28,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                52,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                76,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               100,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               124,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiQUIVER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoCURRENTITEM,                 0,                32,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOWANDARROW,    1,                 0,                10,                 5,                14,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                44,     26,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               0,                 sscARROWS,         0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                56,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 1,                11,                 6,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                80,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 2,                12,                 7,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               104,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 3,                13,                 8,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,               116,     26,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               0,                 sscBOMBS,          0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               128,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 4,                14,                 9,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoCURRENTITEM,                 0,                32,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 5,                 0,                10,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                56,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 6,                 1,                11,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                68,     50,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               0,                 sscSBOMBS,         0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                80,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiDINSFIRE,       1,                 7,                 2,                12,                 6,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               104,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiNAYRUSLOVE,     1,                 8,                 3,                13,                 7,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               128,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFARORESWIND,    1,                 9,                 4,                14,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoCURRENTITEM,                 0,                32,     64,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                10,                 5,                 0,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                56,     64,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 6,                 1,                10,                12,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,                80,     64,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                12,                 7,                 2,                11,                13,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               104,     64,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                13,                 8,                 3,                12,                14,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCURRENTITEM,                 0,               128,     64,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                14,                 9,                 4,                13,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoLINE,                        0,               177,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               181,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               185,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               189,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               193,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               197,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               201,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               205,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               209,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoLINE,                        0,               213,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               217,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               221,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               225,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               229,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               233,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,               237,    139,      2,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoBUTTONITEM,                  0,               200,      8,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,               212,     18,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               2,                 sscARROWS,         0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,               212,     18,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               2,                 sscBOMBS,          0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,               212,     18,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               2,                 sscSBOMBS,         0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoSELECTEDITEMNAME,            0,               173,     24,     70,      16,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfPROP,           sstaCENTER,        sstsSHADOWED,      1,                 4,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoRECT,                        0,                24,      1,     16,       6,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTEXT,                        0,                24,      1,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ITEM" },
  { ssoTILEBLOCK,                   0,                16,      8,      1,       1,       9,                    0,                            0,                    0,                            0,                    0,                            14206,             0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTEXT,                        0,                17,    105,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
  { ssoTEXT,                        0,               177,     57,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
  { ssoTEXT,                        0,               177,    113,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "EQUIPMENT" },

  { ssoTRIFORCE,                    0,               192,     64,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               208,     64,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 1,                 1,                 0,                 2,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               184,     80,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 3,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               216,     80,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 1,                 1,                 0,                 4,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               192,     80,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 2,                 1,                 0,                 5,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               200,     80,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 1,                 1,                 0,                 6,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               208,     80,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 3,                 1,                 0,                 7,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTRIFORCE,                    0,               200,     80,      1,       1,       ssctMISC,             sscsTRIFORCECSET,             0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 8,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
};

subscreen_object z3_passive[200]=
{
  //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                frames    speed    delay    frame     dp1
  { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMINITILE,                    0,                 72,     6,      0,       0,       1,                    0,                            0,                    0,                            0,                    0,                            56325,             0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMINITILE,                    0,                100,     6,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            56327,             0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMINITILE,                    0,                120,     6,      0,       0,       1,                    0,                            0,                    0,                            0,                    0,                            56328,             0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMINITILE,                    0,                128,     6,      0,       0,       1,                    0,                            0,                    0,                            0,                    0,                            56329,             0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMINITILE,                    0,                144,     6,      0,       0,       1,                    0,                            0,                    0,                            0,                    0,                            56326,             0,                 0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  //                                                                                       notlast_cset          last_cset                     cap_cset              aftercap_cset                                                                     container          notlast_tile       last_tile          cap_tile           aftercap_tile      frames             speed              delay              show               unique_last&notlast_mod&last_mod&cap_mod&aftercap_mod
  //  left top cap
  { ssoMAGICGAUGE,                  0,                 20,      9,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            9,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     13,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            8,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     17,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            7,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     21,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            6,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     25,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            5,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     29,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            4,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     33,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            3,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     37,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            2,                 56643,             56643,             56804,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  // left gauge
  { ssoMAGICGAUGE,                  0,                 20,     14,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            8,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     18,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            7,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     22,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            6,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     26,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            5,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     30,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            4,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     34,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            3,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     38,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            2,                 56644,             56644,             56647,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     42,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            1,                 56644,             56644,             56643,             56643,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     46,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            0,                 56646,             56643,             56643,             56643,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  //  right top cap
  { ssoMAGICGAUGE,                  0,                 28,      9,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            9,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     13,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            8,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     17,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            7,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     21,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            6,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     25,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            5,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     29,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            4,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     33,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            3,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     37,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            2,                 56723,             56723,             56805,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  // right gauge
  { ssoMAGICGAUGE,                  0,                 28,     14,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            8,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     18,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            7,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     22,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            6,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     26,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            5,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     30,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            4,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     34,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            3,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     38,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            2,                 56724,             56724,             56727,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     42,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            1,                 56724,             56724,             56723,             56723,             1,                 0,                 0,                 0,                 19,                0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     46,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            0,                 56726,             56723,             56723,             56723,             1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  // left "1/2"
  { ssoMAGICGAUGE,                  0,                 20,      6,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            9,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     10,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            8,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     14,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            7,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     18,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            6,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     22,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            5,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     26,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            4,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     30,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            3,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 20,     34,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            2,                 56643,             56643,             56645,             56643,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  // right "1/2"
  { ssoMAGICGAUGE,                  0,                 28,      6,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            9,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     10,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            8,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     14,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            7,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     18,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            6,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     22,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            5,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     26,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            4,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     30,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            3,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },
  { ssoMAGICGAUGE,                  0,                 28,     34,     0,       0,       9,                    9,                            9,                    9,                            0,                    0,                            2,                 56723,             56723,             56725,             56723,             1,                 0,                 0,                 1,                 0,                 0,        0,       0,       0,        NULL },

  { sso2X2FRAME,                    0,                 32,      6,     4,       4,       9,                    0,                            0,                    0,                            0,                    0,                        14131,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { sso2X2FRAME,                    0,                 32,      6,     4,       4,       1,                    0,                            0,                    0,                            0,                    0,                        14040,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoBUTTONITEM,                  0,                 40,     14,     0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                 52,     24,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               2,                 sscARROWS,         0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                 52,     24,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               2,                 sscBOMBS,          0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                 52,     24,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfZ3SMALL,        sstaCENTER,        sstsOUTLINEDPLUS,  2,                 '~',               2,                 sscSBOMBS,         0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoTEXT,                        0,                200,      6,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfSS4,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "LIFE" },
  { ssoRECT,                        0,                178,      8,    12,       2,       0,                    15,                           0,                    15,                           0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,                179,      8,    10,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoRECT,                        0,                210,      8,    12,       2,       0,                    15,                           0,                    15,                           0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoLINE,                        0,                211,      8,    10,       1,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoLIFEGAUGE,                   0,                169,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            1,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                177,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            2,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                185,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            3,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                193,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            4,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                201,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            5,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                209,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            6,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                217,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            7,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                225,     15,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            8,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                169,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                            9,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                177,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           10,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                185,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           11,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                193,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           12,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                201,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           13,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                209,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           14,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                217,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           15,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                225,     23,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           16,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                169,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           17,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                177,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           18,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                185,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           19,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                193,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           20,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                201,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           21,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                209,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           22,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                217,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           23,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoLIFEGAUGE,                   0,                225,     31,     0,       0,       1,                    1,                            1,                    1,                            0,                    0,                           24,                 56884,             56884,             56885,             56885,             1,                 0,                 0,                 0,                 3,                0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                 77,     15,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfSS4,            sstaCENTER,        sstsSHADOWED,      3,                 '~',               1,                 sscRUPEES,         0,                 0,                 0,                0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                105,     15,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfSS4,            sstaCENTER,        sstsSHADOWED,      2,                 '~',               1,                 sscBOMBS,          0,                 0,                 0,                0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                129,     15,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfSS4,            sstaCENTER,        sstsSHADOWED,      2,                 '~',               1,                 sscARROWS,         0,                 0,                 0,                0,        0,       0,       0,        NULL },
  { ssoCOUNTER,                     0,                148,     15,     0,       0,       ssctMISC,             ssctTEXT,                     0,                    15,                           ssctSYSTEM,           -1,                           ssfSS4,            sstaCENTER,        sstsSHADOWED,      1,                 '~',               1,                 sscGENKEYMAGIC,    0,                 0,                 0,                0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
  { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

  { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
};

//enum { ssdtOLD, ssdtNEWSUBSCR, ssdtREV2, ssdtBSZELDA, ssdtBSZELDAMODIFIED, ssdtBSZELDAENHANCED, ssdtBSZELDACOMPLETE, ssdtMAX };
subscreen_object default_subscreen_active[ssdtMAX][2][66]=//[zinit.subscreen][dungeon]
{
  { //ssdtOLD
    { //ssdtOLD, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                68,     18,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                56,     34,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                64,     66,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "USE B BUTTON" },
      { ssoTEXT,                        0,                64,     74,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "FOR THIS" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               128,    154,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,                80,     98,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     34,     13,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                64,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               128,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               148,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               160,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               176,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               204,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               172,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               180,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               128,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 4,                 4,                 7,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               152,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 5,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               176,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                 6,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               200,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                 7,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               128,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 0,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               152,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 1,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               176,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                 2,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               200,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                 3,                 6,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtOLD, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                68,     18,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                56,     34,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                64,     66,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "USE B BUTTON" },
      { ssoTEXT,                        0,                64,     74,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "FOR THIS" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,                96,     82,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 1,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                52,     90,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "MAP" },
      { ssoTEXT,                        0,                52,    130,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "COMPASS" },
      { ssoCURRENTITEM,                 0,                44,    106,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                44,    146,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     34,     13,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                64,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               128,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               148,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               160,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               176,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               204,     18,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               172,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               180,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               128,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 4,                 4,                 7,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               152,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 5,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               176,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                 6,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               200,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                 7,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               128,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 0,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               152,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 1,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               176,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                 2,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               200,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                 3,                 6,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtNEWSUBSCR
    { //ssdtNEWSUBSCR, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                68,     18,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                12,     34,     14,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               128,    154,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,                80,     98,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               136,     18,     13,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                20,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                36,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                52,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                68,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               100,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                20,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                36,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                52,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                68,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               100,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               188,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               196,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               144,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                11,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               216,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               144,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               216,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               144,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                 7,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               216,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtNEWSUBSCR, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                68,     18,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                12,     34,     14,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,                96,     82,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 1,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                52,     90,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "MAP" },
      { ssoTEXT,                        0,                52,    130,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "COMPASS" },
      { ssoCURRENTITEM,                 0,                44,    106,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                44,    146,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               136,     18,     13,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                20,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                36,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                52,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                68,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               100,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                20,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                36,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                52,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                68,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               100,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               188,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               196,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               144,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                11,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               216,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               144,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               216,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               144,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                 7,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               192,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               216,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtREV2
    { //ssdtREV2, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                62,     18,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                 6,     34,     14,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               128,    154,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,                80,     98,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               118,     18,     16,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                14,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                30,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                46,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                78,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                94,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                14,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                30,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                46,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                78,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                94,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               170,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               178,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               126,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                14,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               150,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               174,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               198,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                12,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               222,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiDINSFIRE,       1,                12,                14,                13,                 3,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               126,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                12,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               150,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               174,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               198,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                13,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               222,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFARORESWIND,    1,                13,                12,                14,                 7,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               126,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                13,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               150,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               174,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               198,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                14,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               222,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiNAYRUSLOVE,     1,                14,                13,                12,                11,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtREV2, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                62,     18,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                 6,     34,     14,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { sso2X2FRAME,                    0,                16,     98,      9,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,                96,     82,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 1,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,    106,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                44,    106,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                64,    106,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOSSKEY,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               118,     18,     16,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                14,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                30,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                46,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                78,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                94,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                14,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                30,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                46,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                78,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                94,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               170,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               178,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               126,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                14,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               150,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               174,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               198,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                12,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               222,     26,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiDINSFIRE,       1,                12,                14,                13,                 3,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               126,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                12,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               150,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               174,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               198,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                13,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               222,     42,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFARORESWIND,    1,                13,                12,                14,                 7,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,               126,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                13,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               150,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               174,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               198,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                14,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               222,     58,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiNAYRUSLOVE,     1,                14,                13,                12,                11,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDA
    { //ssdtBSZELDA, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,     24,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     80,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     80,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     96,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                20,     40,     13,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               136,     48,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,               136,     56,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                20,    112,     14,       7,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     88,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                48,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                96,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                32,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                64,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                76,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 4,                 4,                 7,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 5,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                 6,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                 7,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 0,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 1,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                 2,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                 3,                 6,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      2,               208,     25,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,     25,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,     25,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     32,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     26,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDA, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,     24,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     80,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     80,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     96,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                20,     40,     13,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { sso2X2FRAME,                    0,               160,     40,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               192,     40,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,               144,     88,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               176,     72,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "Compass" },
      { ssoTEXT,                        0,               208,     72,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "Map" },
      { ssoCURRENTITEM,                 0,               200,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                20,    112,     14,       7,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     88,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                48,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                96,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                32,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                64,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                76,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 4,                 4,                 7,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 5,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                 6,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                 7,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 0,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 1,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                 2,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                 3,                 6,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      2,               208,     25,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,     25,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,     25,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     32,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     26,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDAMODIFIED
    { //ssdtBSZELDAMODIFIED, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,     24,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     80,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     80,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     96,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                12,     40,     15,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               136,     48,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,               136,     56,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                20,    112,     14,       7,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     88,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                76,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 4,                 4,                 7,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 5,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                 6,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                 7,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 0,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 1,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                 2,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                 3,                 6,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      2,               208,     25,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,     25,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,     25,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     32,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     26,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDAMODIFIED, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,     24,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,     24,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     80,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     80,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     96,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                12,     40,     15,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { sso2X2FRAME,                    0,               160,     40,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               192,     40,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,               144,     88,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               176,     72,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "Compass" },
      { ssoTEXT,                        0,               208,     72,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "Map" },
      { ssoCURRENTITEM,                 0,               200,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                20,    112,     14,       7,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     88,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                76,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 4,                 4,                 7,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 5,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                 6,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                 7,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 0,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 1,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                 2,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                 3,                 6,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      2,               208,     25,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,     25,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,     25,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     32,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     26,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDAENHANCED
    { //ssdtBSZELDAENHANCED, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     72,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     72,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     80,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                12,     20,     15,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               136,     48,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,               136,     56,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                20,    104,     14,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     80,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                76,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                11,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                 7,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      2,               208,      9,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,      9,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,      9,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     16,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     10,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDAENHANCED, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     72,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     72,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     80,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                12,     20,     15,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { sso2X2FRAME,                    0,               160,     40,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               192,     40,      4,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,               144,     88,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               176,     72,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "Compass" },
      { ssoTEXT,                        0,               208,     72,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "Map" },
      { ssoCURRENTITEM,                 0,               200,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               168,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                20,    104,     14,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     80,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                76,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                84,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                11,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    120,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                 3,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                32,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                 7,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                80,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      2,               208,      9,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,      9,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,      9,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     16,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     10,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDACOMPLETE
    { //ssdtBSZELDACOMPLETE, overworld
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     72,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     72,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     80,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                12,     20,     15,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoTEXT,                        0,               136,     48,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "TRIFORCE" },
      { ssoTRIFRAME,                    0,               136,     56,      0,       0,       ssctMISC,             ssctTRIFRAMECOLOR,            ssctMISC,             ssctCAPTION,                  0,                    0,                            0,                 0,                 0,                 0,                 1,                 1,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                 4,    104,     16,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     80,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                58,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                68,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                14,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                14,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                38,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                86,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                12,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               110,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiDINSFIRE,       1,                12,                14,                13,                 3,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                14,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                12,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                38,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                86,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                13,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               110,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFARORESWIND,    1,                13,                12,                14,                 7,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                14,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                13,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                38,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                86,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                14,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               110,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiNAYRUSLOVE,     1,                14,                13,                12,                11,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoBSTIME,                      2,               208,      9,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,      9,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,      9,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     16,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     10,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDACOMPLETE, dungeon
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoRECT,                        0,                 0,      0,    256,     168,       ssctMISC,             ssctSUBSCRBG,                 ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 0,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                 8,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   1,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                88,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   2,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,                96,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   3,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               104,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   4,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               112,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   5,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               120,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   6,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               128,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   7,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               136,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   8,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               144,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,   9,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               152,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  10,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINITILE,                    0,               248,      8,      0,       0,       ssctMISC,             sscsSSVINECSET,               0,                    0,                            0,                    0,                            -1,                ssmstSSVINETILE,  11,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      //
      { ssoTEXT,                        0,                51,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "INVENTORY" },
      { sso2X2FRAME,                    0,                28,     72,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,                40,     72,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,                56,     80,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B Button" },
      { ssoTEXT,                        0,                56,     88,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfSS2,            sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "to use item" },
      { sso2X2FRAME,                    0,                12,     20,     15,       6,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               152,     40,     10,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLARGEMAP,                    0,               144,     88,      0,       0,       ssctMISC,             ssctBMAPFG,                   ssctMISC,             ssctLINKDOT,                  0,                    0,                            1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               160,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAP,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               184,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCOMPASS,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               208,     48,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOSSKEY,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,                 4,    104,     16,       8,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,                32,     80,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRAFT,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOK,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiRING,           1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLADDER,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiMAGICKEY,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     28,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRACELET,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                24,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSHIELD,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                40,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWALLET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                56,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiAMULET,         1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                72,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFLIPPERS,       1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                88,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOOTS,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               104,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHCPIECE,        1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoSELECTOR2,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                58,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                68,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOW,            1,                 -1,                0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                14,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBRANG,          1,                 0,                 8,                 4,                14,                 1,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                38,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBOMB,           1,                 1,                 9,                 5,                 0,                 2,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiARROW,          0,                 2,                10,                 6,                 1,                 3,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                86,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiCANDLE,         1,                 3,                11,                 7,                 2,                12,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               110,    112,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiDINSFIRE,       1,                12,                14,                13,                 3,                 4,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                14,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWHISTLE,        1,                 4,                 0,                 8,                12,                 5,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                38,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiBAIT,           1,                 5,                 1,                 9,                 4,                 6,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLETTERPOTION,   1,                 6,                 2,                10,                 5,                 7,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                86,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiWAND,           1,                 7,                 3,                11,                 6,                13,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               110,    128,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiFARORESWIND,    1,                13,                12,                14,                 7,                 8,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoCURRENTITEM,                 0,                14,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHOOKSHOT,       1,                 8,                 4,                 0,                13,                 9,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                38,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiSBOMB,          1,                 9,                 5,                 1,                 8,                10,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                62,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiLENS,           1,                10,                 6,                 2,                 9,                11,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,                86,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiHAMMER,         1,                11,                 7,                 3,                10,                14,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCURRENTITEM,                 0,               110,    144,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            ssiNAYRUSLOVE,     1,                14,                13,                12,                11,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },

      { ssoBSTIME,                      2,               208,      9,       0,      0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        2,               160,      9,      44,      7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               204,      9,       1,      8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        2,               160,     16,      45,      1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        2,               161,     10,       0,      0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoSELECTOR1,                   0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  }
};


subscreen_object default_subscreen_passive[ssdtMAX][2][21]= //[zinit.subscreen][no magic]
{
  { //ssdtOLD
    { //ssdtOLD, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          1,                 sstsNORMAL,        3,                 'A',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               144,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               124,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               148,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               132,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               156,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               208,     12,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-LIFE-" },
      { ssoLIFEMETER,                   0,               176,     12,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               176,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtOLD, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          1,                 sstsNORMAL,        3,                 'A',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               144,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               124,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               148,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               132,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               156,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               208,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-LIFE-" },
      { ssoLIFEMETER,                   0,               176,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtNEWSUBSCR
    { //ssdtNEWSUBSCR, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          1,                 sstsNORMAL,        3,                 'A',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               144,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               124,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               148,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               132,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               156,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               208,     12,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-LIFE-" },
      { ssoLIFEMETER,                   0,               176,     12,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               176,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtNEWSUBSCR, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          1,                 sstsNORMAL,        3,                 'A',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               144,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               124,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               148,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               132,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               156,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               208,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-LIFE-" },
      { ssoLIFEMETER,                   0,               176,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtREV2
    { //ssdtREV2, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          1,                 sstsNORMAL,        3,                 'A',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               144,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               124,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               148,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               132,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               156,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               208,     12,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-LIFE-" },
      { ssoLIFEMETER,                   0,               176,     12,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               176,     44,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtREV2, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            0,                    0,                            ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          1,                 sstsNORMAL,        3,                 'A',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               120,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               144,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               124,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               148,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               132,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               156,     16,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctMISC,             ssctSUBSCRBG,                 ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               208,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaCENTER,        sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-LIFE-" },
      { ssoLIFEMETER,                   0,               176,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDA
    { //ssdtBSZELDA, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               220,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               188,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               188,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDA, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               216,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               184,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDAMODIFIED
    { //ssdtBSZELDAMODIFIED, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               220,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               188,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               188,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDAMODIFIED, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               216,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               184,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDAENHANCED
    { //ssdtBSZELDAENHANCED, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               220,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               188,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               188,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDAENHANCED, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               216,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               184,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    }
  },
  { //ssdtBSZELDACOMPLETE
    { //ssdtBSZELDACOMPLETE, magic
    //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               220,      8,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               188,     16,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMAGICMETER,                  0,               188,     40,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL }
    },
    { //ssdtBSZELDACOMPLETE, no magic
      //object                          position         x        y        w        h        color type 1          color 1                       color type 2          color 2                       color type 3          color 3                       d1                 d2                 d3                 d4                 d5                 d6                 d7                 d8                 d9                 d10                 frames    speed    delay    frame     dp1
      { ssoCLEAR,                       0,                 0,      0,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAP,                     0,                 8,      8,      0,       0,       ssctMISC,             ssctLINKDOT,                  ssctMISC,             ssctCOMPASSLT,                ssctMISC,             ssctCOMPASSDK,                1,                 1,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoMINIMAPTITLE,                0,                48,      0,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS3,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoCOUNTERS,                    0,                88,     16,     32,      32,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            1,                 sstsSHADOWED,      4,                 '~',               0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoSSTIME,                      0,               240,      0,      0,       0,       ssctMISC,             ssctTEXT,                     0,                    0,                            ssctSYSTEM,           -1,                           ssfZELDA,          sstaRIGHT,         sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               132,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { sso2X2FRAME,                    0,               156,     16,      3,       4,       ssctMISC,             sscsBLUEFRAMECSET,            0,                    0,                            0,                    0,                            0,                 0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               136,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBUTTONITEM,                  0,               160,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               144,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWEDU,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "B" },
      { ssoTEXT,                        0,               168,     16,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsOUTLINED8,     0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "A" },
      { ssoLINE,                        0,               163,     15,    174,       1,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        0,               216,     16,      0,       0,       ssctMISC,             ssctCAPTION,                  ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaCENTER,        sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "-  life  -" },
      { ssoLIFEMETER,                   0,               184,     24,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoNONE,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoBSTIME,                      1,               208,     48,      0,       0,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctSUBSCRSHADOW,             ssctSYSTEM,           -1,                           ssfSS1,            sstaLEFT,          sstsSHADOWED,      0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoRECT,                        1,               160,     48,     44,       7,       ssctMISC,             ssctTEXT,                     ssctMISC,             ssctTEXT,                     0,                    0,                            1,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               204,     48,      1,       8,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoLINE,                        1,               160,     55,     45,       1,       ssctMISC,             ssctSUBSCRSHADOW,             0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
      { ssoTEXT,                        1,               161,     49,      0,       0,       ssctMISC,             ssctSUBSCRBG,                 0,                    0,                            ssctSYSTEM,           -1,                           ssfZTIME,          sstaLEFT,          sstsNORMAL,        0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        (void *) "ZELDA TIME" },
      { ssoNULL,                        0,                 0,      0,      0,       0,       0,                    0,                            0,                    0,                            0,                    0,                            0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,                 0,        0,       0,       0,        NULL },
    }
  }
};


void textout_styled_aligned_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int textstyle, int alignment, int color, int shadow, int bg)
{
  switch (alignment)
  {
    case sstaRIGHT:
    switch (textstyle)
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
    switch (textstyle)
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
    switch (textstyle)
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


void draw_textbox(BITMAP *dest, int x, int y, int w, int h, FONT *tempfont, char *thetext, bool wword, int tabsize, int alignment, int textstyle, int color, int shadowcolor, int backcolor)
{
  int y1 = y;
  int x1;
  int len;
  char s[16];
  char text[16];
  char space[16];
  char *printed = text;
  char *scanned = text;
  char *oldscan = text;
  char *ignore = NULL;
  char *tmp, *ptmp;
  int width;
  int i = 0;
  int noignore;

  usetc(s+usetc(s, '.'), 0);
  usetc(text+usetc(text, ' '), 0);
  usetc(space+usetc(space, ' '), 0);

  /* find the correct text */
  if (thetext != NULL)
  {
    printed = thetext;
    scanned = thetext;
  }

  /* loop over the entire string */
  for(;;)
  {
    width = 0;

    /* find the next break */
    while (ugetc(scanned))
    {
      /* check for a forced break */
      if (ugetc(scanned) == '\n')
      {
        scanned += uwidth(scanned);
        /* we are done parsing the line end */
        break;
      }

      /* the next character length */
      usetc(s+usetc(s, ugetc(scanned)), 0);
      len = text_length(tempfont, s);

      /* modify length if its a tab */
      if (ugetc(s) == '\t')
      {
        len = tabsize * text_length(tempfont, space);
      }

      /* check for the end of a line by excess width of next char */
      if (width+len >= w)
      {
        /* we have reached end of line do we go back to find start */
        if (wword)
        {
          /* remember where we were */
          oldscan = scanned;
          noignore = FALSE;

          /* go backwards looking for start of word */
          while (!uisspace(ugetc(scanned)))
          {
            /* don't wrap too far */
            if (scanned == printed)
            {
              /* the whole line is filled, so stop here */
              tmp = ptmp = scanned;
              while (ptmp != oldscan)
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
            while (tmp < scanned)
            {
              ptmp = tmp;
              tmp += uwidth(tmp);
            }
            scanned = ptmp;
          }
          /* put the space at the end of the line */
          if (!noignore)
          {
            ignore = scanned;
            scanned += uwidth(scanned);
          }
          else
          {
            ignore = NULL;
          }

          /* check for endline at the convenient place */
          if (ugetc(scanned) == '\n')
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
    if ((y1+text_height(tempfont) <= (y+h)))
    {
      int tempw=0;
      char *tempprinted=printed;

      /* print up to the marked character */
      while (printed != scanned)
      {
        /* do special stuff for each charater */
        switch (ugetc(printed))
        {
          case '\r':
          case '\n':
          /* don't print endlines in the text */
          break;

          /* possibly expand the tabs */
          case '\t':
          for (i=0; i<tabsize; i++)
          {
            usetc(s+usetc(s, ' '), 0);
            tempw += text_length(tempfont, s);
          }
          break;

          /* print a normal character */
          default:
          if (printed != ignore)
          {
            usetc(s+usetc(s, ugetc(printed)), 0);
            tempw += text_length(tempfont, s);
          }
        }

        /* goto the next character */
        printed += uwidth(printed);
      }

      printed=tempprinted;
      switch (alignment)
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
      while (printed != scanned)
      {
        /* do special stuff for each charater */
        switch (ugetc(printed))
        {

          case '\r':
          case '\n':
          /* don't print endlines in the text */
          break;

          /* possibly expand the tabs */
          case '\t':
          for (i=0; i<tabsize; i++)
          {
            usetc(s+usetc(s, ' '), 0);
            textout_styled_aligned_ex(dest, tempfont, s, x1, y1, textstyle, sstaLEFT, color, shadowcolor, backcolor);
            x1 += text_length(tempfont, s);
          }
          break;

          /* print a normal character */
          default:
          if (printed != ignore)
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
    if (!ugetc(printed))
    {
      return;
    }
  }
}



//void frame2x2(BITMAP *dest,miscQdata *misc,int x,int y,int xsize,int ysize,int t,int c)
//draw_block_flip(dest,x,y,css->objects[i].d1,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].w,css->objects[i].h,css->objects[i].d2,css->objects[i].d3,css->objects[i].d4);
void frame2x2(BITMAP *dest,miscQdata *misc,int x,int y,int tile,int cset,int w,int h,int flip,bool overlay,bool trans)
{
  //these are here to bypass compiler warnings about unused arguments
  flip=flip;

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
  if (tile==0)
  {
    tile=misc->colors.blueframe_tile;
  }
  int t8 = tile<<2;

  if (overlay)
  {
    if (trans)
    {
      for(int dx=0; dx<w; dx++)
      {
        int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4 ) : ((dx==0) ? t8    : t8+1 );
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
        int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4 ) : ((dx==0) ? t8    : t8+1 );
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
    if (trans)
    {
      for(int dx=0; dx<w; dx++)
      {
        int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4 ) : ((dx==0) ? t8    : t8+1 );
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
        int top    = (dx >= (w>>1)) ? ((dx==w-1) ? t8+5  : t8+4 ) : ((dx==0) ? t8    : t8+1 );
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
  if (overlay)
  {
    switch (flip&3)
    {
      case 0:
        if (trans)
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
        if (trans)
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
        if (trans)
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
        if (trans)
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
    switch (flip&3)
    {
      case 0:
        if (trans)
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
        if (trans)
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
        if (trans)
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
        if (trans)
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

  if (showmap)
  {
    switch(type)
    {
      case dmOVERW:
      case dmBSOVERW:

      if (DMaps[get_currdmap()].minimap_1_tile)
      {
        draw_block(dest,x,y,DMaps[get_currdmap()].minimap_1_tile,DMaps[get_currdmap()].minimap_1_cset,5,3);
      }
      else if (c.overworld_map_tile)
      {
        draw_block(dest,x,y,c.overworld_map_tile,c.overworld_map_cset,5,3);
      }
      else
      {
        rectfill(dest,x+8,y+8,x+71,y+39,c.overw_bg);
      }

      if (!DMaps[get_currdmap()].minimap_1_tile && ((DMaps[get_currdmap()].type&dmfTYPE) == dmBSOVERW))
      {
        drawgrid(dest,16,16,c.bs_goal,c.bs_dk);
      }
      break;

      case dmDNGN:
      case dmCAVE:
      int maptile=has_item(itype_map, get_dlevel())?DMaps[get_currdmap()].minimap_2_tile:DMaps[get_currdmap()].minimap_1_tile;
      int mapcset=has_item(itype_map, get_dlevel())?DMaps[get_currdmap()].minimap_2_cset:DMaps[get_currdmap()].minimap_1_cset;

      if (maptile)
      {
        draw_block(dest,x,y,maptile,mapcset,5,3);
      }
      else if (c.dungeon_map_tile)
        {
          draw_block(dest,x,y,c.dungeon_map_tile,c.dungeon_map_cset,5,3);
        }
        else
        {
          rectfill(dest,x+8,y+8,x+71,y+39,c.dngn_bg);
        }

      if (!DMaps[get_currdmap()].minimap_2_tile && has_item(itype_map, get_dlevel()))
      {
        if ((DMaps[get_currdmap()].type&dmfTYPE) == dmCAVE)
        {
          drawgrid(dest,16,16,c.cave_fg,-1);
        }
        else
        {
          drawgrid(dest,16,16,c.dngn_fg,-1);
        }
      }
      break;
    }
  }
  if (showcompass)
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
  if (showlink)
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
  if (!show_subscreen_life)
  {
    return;
  }
  if (!bs_style)
  {
    y+=24;
  }
  for(int i=0; i<(game != NULL ? game->get_maxlife() : 1); i+=HP_PER_HEART)
  {
    if(game != NULL)
	{
		if(get_bit(quest_rules,qr_QUARTERHEART))
		{
			if(i+((HP_PER_HEART/4)*3)>=game->get_life()) tile=6;
			if(i+(HP_PER_HEART/2)>=game->get_life()) tile=1;
			if(i+((HP_PER_HEART/4)*1)>=game->get_life()) tile=7;
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

void magicgauge(BITMAP *dest,int x,int y, int container, int notlast_tile, int notlast_cset, bool notlast_mod, int last_tile, int last_cset, bool last_mod,
                int cap_tile, int cap_cset, bool cap_mod, int aftercap_tile, int aftercap_cset, bool aftercap_mod, int frames, int speed, int delay, bool unique_last, int show)
{
  //these are here to bypass compiler warnings about unused arguments
  frames=frames;
  speed=speed;
  delay=delay;

  if (((game->get_magicdrainrate()==1)&&show==2)||
      ((game->get_magicdrainrate()==2)&&show==1))
  {
    return;
  }
  int containers=game->get_maxmagic()/MAGICPERBLOCK;
  int tile=rand()%32767, cset=rand()%15;
  bool mod_value=(rand()%2)!=0;

  if (container<containers)
  {
    tile=notlast_tile;
    cset=notlast_cset;
    mod_value=notlast_mod;
  }
  else if (container==containers)
    {
      tile=last_tile;
      cset=last_cset;
      mod_value=last_mod;
    }
    else if (container==containers+1)
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

  if (mod_value)
  {
    if (game->get_magic()>=container*MAGICPERBLOCK)
    {
      //tile=tile;                                        //full block
      if (game->get_magic()==container*MAGICPERBLOCK)
      {
        if (unique_last)
        {
          tile+=35;
        }
      }
    }
    else
    {
      if (((container-1)*MAGICPERBLOCK)>game->get_magic())
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

  if (container<containers)
  {
    tile=notlast_tile;
    cset=notlast_cset;
    mod_value=notlast_mod;
  }
  else if (container==containers)
    {
      tile=last_tile;
      cset=last_cset;
      mod_value=last_mod;
    }
    else if (container==containers+1)
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

  if (mod_value)
  {
    if (game->get_life()>=container*HP_PER_HEART)
    {
      //tile=tile;                                        //full block
      if (game->get_life()==container*HP_PER_HEART)
      {
        if (unique_last)
        {
          tile+=19;
        }
      }
    }
    else
    {
      if (((container-1)*HP_PER_HEART)>game->get_life())
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


void magicmeter(BITMAP *dest,int x,int y)
{
  if (!get_bit(quest_rules,qr_ENABLEMAGIC)) return;
  if (game->get_maxmagic()==0) return;
  int tile;
  int mmtile=wpnsbuf[iwMMeter].tile;
  int mmcset=wpnsbuf[iwMMeter].csets&15;
  overtile8(dest,(mmtile*4)+2,x-8,y,mmcset,0);
  if (game->get_magicdrainrate()==1)
  {
    overtile8(dest,(mmtile*4)+1,x-10,y,mmcset,0);
  }
  for(int i=0; i<game->get_maxmagic(); i+=MAGICPERBLOCK)
  {
    if (game->get_magic()>=i+MAGICPERBLOCK)
    {
      tile=mmtile*4;                                        //full block
    }
    else
    {
      if (i>game->get_magic())
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
  if (!show_subscreen_numbers)
  {
    return;
  }
  int found_digits=5;
  if(num<10000)
  {
    found_digits=4;
  }
  if (num<1000)
  {
    found_digits=3;
  }
  if (num<100)
  {
    found_digits=2;
  }
  if (num<10)
  {
    found_digits=1;
  }
  if (infinite)
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
int   Bid = 0, Aid = 0;

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
  Aid = Bid = 0;
}

void update_subscr_items()
{
  if(Bid != Bwpn)
  {
    Bid = 0;
    if(Bitem)
    {
      delete Bitem;
      Bitem = NULL;
    }

    if(Bwpn > 0)
    {
      Bitem = new item((fix)0, (fix)0, (fix)0, Bwpn&0x0FFF, 0, 0);
      Bitem->dummy_bool[0]=false;
      switch (Bwpn&0x0FFF)
      {
        case iArrow:
        case iSArrow:
        case iGArrow:
        if ((Bwpn&0xF000)==0xF000)
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
      }
    }
  }

  if(Aid != Awpn)
  {
    Aid = 0;
    if(Aitem)
    {
      delete Aitem;
      Aitem = NULL;
    }

    if(Awpn > 0)
    {
      Aitem = new item((fix)0, (fix)0,(fix)0,Awpn&0x0FFF, 0, 0);
      switch (Awpn&0x0FFF)
      {
        case iArrow:
        case iSArrow:
        case iGArrow:
        if ((Awpn&0xF000)==0xF000)
        {
          Aitem->dummy_bool[0]=true;
        }
        break;
      }
      if(Aitem != NULL)
      {
        Aid = Awpn;
        Aitem->yofs = 0;
      }
    }
  }

  if(Bitem)
    Bitem->animate(0);
  if(Aitem)
    Aitem->animate(0);
}

/****/
int stripspaces(char *source, char *target, int stop)
{
  int begin, end;
  for (begin=0; ((begin<stop)&&(source[begin]==' ')); begin++);
  if (begin==stop)
  {
    return 0;
  }
  for (end=stop-1; ((source[end]==' ')); end--);
  sprintf(target, "%.*s", (end-begin+1), source+begin);
  return (end-begin+1);
}

char *subscreen_item_class_names[ssiMAX]=
{ "bomb", "sword", "shield", "candle", "letter", "potion", "letter/potion", "bow", "arrow", "bait", "ring", "bracelet", "map", "compass", "boss key", "magic key",
  "boomerang", "wand", "raft", "ladder", "whistle", "book", "wallet", "super bomb", "heart container pieces", "amulet", "flippers", "hookshot", "lens", "hammer", "boots",
  "din's fire", "farore's wind", "nayru's love", "quiver", "bomb bag", "cane of byrna", "roc items", "hover boots", "spin attack scroll", "cross beams scroll", "quake hammer scroll", "whisp ring", "charge ring", "peril beam scroll", "wealth medals" };

char *subscreen_item_names[iMax] =
{
  "Rupee", "5 Rupees", "Heart", "Bombs", "Clock",
  "Wooden Sword", "White Sword", "Magic Sword", "Magic Shield", "Key",
  "Blue Candle", "Red Candle", "Letter", "Wooden Arrows", "Silver Arrows",
  "Short Bow", "Bait", "Blue Ring", "Red Ring", "Power Bracelet",
  "Triforce Fragment", "Map", "Compass", "Wooden Boomerang", "Magic Boomerang",
  "Wand", "Raft", "Ladder", "Heart Container", "Blue Potion",
  "Red Potion", "Whistle", "Magic Book", "Magic Key", "Fairy (Moving)",
  "Fire Boomerang", "Master Sword", "Mirror Shield", "20 Rupees", "50 Rupee",
  "200 Rupees", "Small Wallet", "Large Wallet", "Dust Pile",
  "Triforce", "Selection", "Misc 1", "Misc 2", "Super Bomb","Heart Container Piece",
  "Amulet", "Flippers", "Hookshot", "Lens of Truth", "Hammer", "Boots",
  "Power Glove", "Golden Arrows", "Magic Container", "Small Magic Jar",
  "Large Magic Jar", "Golden Ring", "Kill All Enemies", "Large Amulet",
  "Din's Fire", "Farore's Wind", "Nayru's Love", "Boss Key", "Long Bow", "Fairy (Stationary)",
  "Arrow Ammunition (1)", "Arrow Ammunition (5)", "Arrow Ammunition (10)", "Arrow Ammunition (30)",
  "Small Quiver", "Medium Quiver", "Large Quiver", "Bomb Ammunition (1)", "Bomb Ammunition (4)", "Bomb Ammunition (8)", "Bomb Ammunition (30)",
  "Small Bomb Bag", "Medium Bomb Bag", "Large Bomb Bag", "10 Rupees", "100 Rupees", "Cane of Byrna", "Letter (used)", "Roc's Feather", "Hover Boots","Small Shield","Scroll: Spin Attack","Scroll: Cross Beams","Scroll: Quake Hammer", "Scroll: Super Quake", "Scroll: Hurricane Spin", "Whisp Ring", "Large Whisp Ring", "Charge Ring", "Large Charge Ring", "Scroll: Peril Beam", "Magic Wallet", "Magic Quiver", "Magic Bomb Bag", "Worn-Out Glove", "Four-Way Ladder", "Bronze Wealth Medal", "Silver Wealth Medal", "Golden Wealth Medal"
};

void subscreenitem(BITMAP *dest, int x, int y, int itemtype)
{
  switch (itemtype)
  {
    case ssiBOMB:
    case ssiSWORD:
    case ssiSHIELD:
    case ssiCANDLE:
    case ssiLETTER:
    case ssiPOTION:
    case ssiLETTERPOTION:
    case ssiBOW:
    case ssiARROW:
    case ssiBOWANDARROW:
    case ssiBAIT:
    case ssiRING:
    case ssiBRACELET:
    case ssiMAP:
    case ssiCOMPASS:
    case ssiBOSSKEY:
    case ssiMAGICKEY:
    case ssiBRANG:
    case ssiWAND:
    case ssiRAFT:
    case ssiLADDER:
    case ssiWHISTLE:
    case ssiBOOK:
    case ssiWALLET:
    case ssiSBOMB:
    case ssiHCPIECE:
    case ssiAMULET:
    case ssiFLIPPERS:
    case ssiHOOKSHOT:
    case ssiLENS:
    case ssiHAMMER:
    case ssiBOOTS:
    case ssiDINSFIRE:
    case ssiFARORESWIND:
    case ssiNAYRUSLOVE:
    case ssiQUIVER:
    case ssiCBYRNA:
    case ssiBOMBBAG:
    case ssiROCS:
    case ssiHOVERBOOTS:
    case ssiSPINSCROLL:
    case ssiCROSSSCROLL:
    case ssiQUAKESCROLL:
    case ssiWHISPRING:
    case ssiCHARGERING:
    case ssiPERILSCROLL:
    case ssiWEALTHMEDAL:
      for(int i=0; i<Sitems.Count(); i++)
      {
        int d=-1;
        switch (Sitems.spr(i)->id)
        {
          case iBombs:
            d=ssiBOMB;
            break;

          case iSBomb:
            d=ssiSBOMB;
            break;

          case iSword:
          case iWSword:
          case iMSword:
          case iXSword:
            d=ssiSWORD;
            break;

          case iSShield:
          case iShield:
          case iMShield:
            d=ssiSHIELD;
            break;

          case iBCandle:
          case iRCandle:
            d=ssiCANDLE;
            break;

          case iLetter:
            d=ssiLETTER;
            break;

          case iBPotion:
          case iRPotion:
            d=ssiPOTION;
            break;

          case iArrow:
          case iSArrow:
          case iGArrow:
            d=ssiARROW;
            break;

          case iBow:
          case iBow2:
            d=ssiBOW;
            break;

          case iBait:
            d=ssiBAIT;
            break;

          case iBRing:
          case iRRing:
          case iGRing:
            d=ssiRING;
            break;

	  case iOldGlove:
          case iBracelet:
          case iL2Bracelet:
            d=ssiBRACELET;
            break;

          case iMap:
            d=ssiMAP;
            break;

          case iCompass:
            d=ssiCOMPASS;
            break;

          case iBossKey:
            d=ssiBOSSKEY;
            break;

          case iBrang:
          case iMBrang:
          case iFBrang:
            d=ssiBRANG;
            break;

          case iWand:
            d=ssiWAND;
            break;

          case iRaft:
            d=ssiRAFT;
            break;

          case iLadder:
          case iL2Ladder:
            d=ssiLADDER;
            break;

          case iWhistle:
            d=ssiWHISTLE;
            break;

          case iBook:
            d=ssiBOOK;
            break;

          case iMKey:
            d=ssiMAGICKEY;
            break;

          case iWallet500:
          case iWallet999:
          case iWalletL3:
            d=ssiWALLET;
            break;

          case iWealthMedal:
          case iL2WealthMedal:
          case iL3WealthMedal:
            d=ssiWEALTHMEDAL;
            break;

          case iHCPiece:
            d=ssiHCPIECE;
            break;

          case iAmulet:
          case iL2Amulet:
            d=ssiAMULET;
            break;

          case iFlippers:
            d=ssiFLIPPERS;
            break;

          case iHookshot:
          case iLongshot:
            d=ssiHOOKSHOT;
            break;

          case iLens:
            d=ssiLENS;
            break;

          case iHammer:
            d=ssiHAMMER;
            break;

          case iBoots:
            d=ssiBOOTS;
            break;

          case iDinsFire:
            d=ssiDINSFIRE;
            break;

          case iFaroresWind:
            d=ssiFARORESWIND;
            break;

          case iNayrusLove:
            d=ssiNAYRUSLOVE;
            break;

          case iQuiver:
          case iQuiverL2:
          case iQuiverL3:
          case iQuiverL4:
            d=ssiQUIVER;
            break;

          case iCByrna:
            d=ssiCBYRNA;
            break;

          case iBombBag:
          case iBombBagL2:
          case iBombBagL3:
          case iBombBagL4:
            d=ssiBOMBBAG;
            break;

          case iRocsFeather:
            d=ssiROCS;
            break;

          case iHoverBoots:
            d=ssiHOVERBOOTS;
            break;

          case iSpinScroll:
          case iL2SpinScroll:
            d=ssiSPINSCROLL;
            break;

          case iWhispRing:
          case iL2WhispRing:
            d=ssiWHISPRING;
            break;

          case iCrossScroll:
            d=ssiCROSSSCROLL;
            break;

          case iQuakeScroll:
          case iL2QuakeScroll:
            d=ssiQUAKESCROLL;
            break;

          case iPerilScroll:
            d=ssiPERILSCROLL;
            break;

	  case iChargeRing:
	  case iL2ChargeRing:
	    d=ssiCHARGERING;
	    break;

          case iRupy:
          case i5Rupies:
          case iHeart:
          case iClock:
          case iKey:
          case iTriforce:
          case iHeartC:
          case iFairyMoving:
          case i20Rupies:
          case i50Rupies:
          case i200Rupies:
          case iPile:
          case iBigTri:
          case iSelectA:
          case iSelectB:
          case iMisc1:
          case iMisc2:
          case iMagicC:
          case iSMagic:
          case iLMagic:
          case iKillAll:
          case iFairyStill:
          case i10Rupies:
          case i100Rupies:
          default:
            d=-1; //these are unused in the subscreen
            break;

        }
        if ((d==itemtype)||
            (itemtype==ssiLETTERPOTION&&(d==ssiLETTER||d==ssiPOTION))||
            (itemtype==ssiBOWANDARROW&&(d==ssiBOW||d==ssiARROW)))
        {
          Sitems.spr(i)->x = x;
          Sitems.spr(i)->y = y;
          Sitems.spr(i)->yofs=0;
          if (itemtype!=ssiBOWANDARROW||d!=ssiARROW||((get_bit(quest_rules,qr_TRUEARROWS)&&game->get_arrows())||(!get_bit(quest_rules,qr_TRUEARROWS)&&game->get_rupies())))
          {
            Sitems.spr(i)->draw(dest);
          }
          if (itemtype!=ssiBOWANDARROW)
          {
            return;
          }
        }
      }
      break;
    default:
      break;
  }
}

int subscreen_color(miscQdata *misc, int c1, int c2)
{
  int ret;
  switch (c1)
  {
    case ssctSYSTEM:
      ret=(c2==-1)?c2:vc(c2);
      break;
    case ssctMISC:
      switch (c2)
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



int subscreen_cset(miscQdata *misc,int c1, int c2)
{
  int ret=c1;
  switch (c1)
  {
    case ssctMISC:
    switch (c2)
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
        ret=wpnsbuf[iwSubscreenVine].csets&15;
        break;
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
  switch (fontnum)
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
    case ssfGBZELDA:
      tempfont=gbzfont;
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
    default:
      fontnum=rand()%ssfMAX;
      tempfont=ss_font(fontnum);
      break;
  }
  return tempfont;
}

void show_custom_subscreen(BITMAP *dest, miscQdata *misc, subscreen_group *css, int xofs, int yofs, bool showtime, bool pulled_down)
{
  color_map=&trans_table;
  set_trans_blender(0, 0, 0, 128);
  static item sel_a((fix)0,(fix)0,(fix)0,iSelectA,0,0);
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
  sel_b.animate(0);
  for (int i=0; i<MAXSUBSCREENITEMS&&css->objects[i].type>ssoNULL; ++i)
  {
    if (css->objects[i].pos==0||((css->objects[i].pos==1)&&!pulled_down)||((css->objects[i].pos==2)&&pulled_down))
    {
      FONT *tempfont=zfont;
      int fontnum=css->objects[i].d1;
      tempfont=ss_font(fontnum);
      int x=css->objects[i].x+xofs;
      int y=css->objects[i].y+yofs;
      switch (css->objects[i].type)
      {
        case sso2X2FRAME:
        {
          frame2x2(dest, misc, x, y, css->objects[i].d1, subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1), css->objects[i].w, css->objects[i].h, css->objects[i].d2, css->objects[i].d3 != 0, css->objects[i].d4 != 0);
        }
        break;
        case ssoBSTIME:
        {
			char *ts;
			if(game)
				ts = time_str_short2(game->get_time());
			else
				ts = time_str_short2(0);
//          textout_shadowed_ex(dest,tempfont,ts,x,y,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
          textout_styled_aligned_ex(dest,tempfont,ts,x,y,css->objects[i].d3,css->objects[i].d2,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
        }
        break;
        case ssoSSTIME:
        case ssoTIME:
        {
          if(showtime||css->objects[i].type==ssoTIME)
          {
			  char *ts;
			  if(game)
				 ts = time_str_med(game->get_time());
			  else
				 ts = time_str_med(0);
//            textout_right_ex(dest,tempfont,ts,x,y,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2));
            textout_styled_aligned_ex(dest,tempfont,ts,x,y,css->objects[i].d3,css->objects[i].d2,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
          }
        }
        break;
        case ssoTEXT:
        {
        //textout_styled_aligned_ex(bmp, f,       s,                          x,y,textstyle,         alignment,         color,                                                                    shadow,                                                                   bg)
          textout_styled_aligned_ex(dest,tempfont,(char *)css->objects[i].dp1,x,y,css->objects[i].d3,css->objects[i].d2,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
        }
        break;
        case ssoLINE:
        {
          if (css->objects[i].d4)
          {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
          }
          line(dest, x, y, x+css->objects[i].w-1, y+css->objects[i].h-1, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1));
          if (css->objects[i].d4)
          {
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
        }
        break;
        case ssoRECT:
        {
          if (css->objects[i].d2)
          {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
          }
          if (css->objects[i].d1!=0)
          {
            rectfill(dest, x, y, x+css->objects[i].w-1, y+css->objects[i].h-1, subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2));
          }
          rect(dest, x, y, x+css->objects[i].w-1, y+css->objects[i].h-1, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1));
          if (css->objects[i].d2)
          {
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
        }
        break;
        case ssoMAGICMETER:
        {
          magicmeter(dest, x, y);
        }
        break;
        case ssoLIFEMETER:
        {
          lifemeter(dest, x, y, css->objects[i].d1, css->objects[i].d2 != 0);
        }
        break;
        case ssoBUTTONITEM:
        {
          if (css->objects[i].d2)
          {
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
          }
          buttonitem(dest, css->objects[i].d1, x, y);
          if (css->objects[i].d2)
          {
            drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
          }
        }
        break;
        case ssoCOUNTERS:
        {
          defaultcounters(dest, x, y, tempfont,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3),css->objects[i].d2 != 0,css->objects[i].d3,css->objects[i].d4,css->objects[i].d5);
        }
        break;
        case ssoCOUNTER:
        {
        //counter(dest, x, y, tempfont,color,                                                                    shadowcolor,                                                              bgcolor,                                                                  alignment,         textstyle,         digits,            idigit,            showzero,           itemtype,           onlyselected)
          counter(dest, x, y, tempfont,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3),css->objects[i].d2,css->objects[i].d3,css->objects[i].d4,css->objects[i].d5,css->objects[i].d6&1, css->objects[i].d7, css->objects[i].d8, css->objects[i].d9, css->objects[i].d10, (css->objects[i].d6&2)!=0);
//defaultcounters(dest, x, y, tempfont,color,                                                                    shadowcolor,                                                              bgcolor,                                                                  usex,              textstyle,         digits,            idigit )
//defaultcounters(dest, x, y, tempfont,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3),css->objects[i].d2,css->objects[i].d3,css->objects[i].d4,css->objects[i].d5);
        }
        break;
        case ssoMINIMAPTITLE:
        {
          minimaptitle(dest, x, y, tempfont, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1),subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2),subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3), css->objects[i].d2, css->objects[i].d3);
        }
        break;
        case ssoMINIMAP:
        {
        //drawdmap(dest, misc, x, y, showmap,                 showlink,           showcompass,        linkcolor,                                                                 lccolor,                                                                   dccolor)
          drawdmap(dest, misc, x, y, css->objects[i].d1 != 0, css->objects[i].d2 && !(TheMaps[(DMaps[get_currdmap()].map*MAPSCRS)+get_homescr()].flags7&fNOLINKMARK),
 css->objects[i].d3 && !(DMaps[get_currdmap()].flags&dmfNOCOMPASS), subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
        }
        break;
        case ssoLARGEMAP:
        {
        //putBmap(dest, misc, x, y, showmap,                 showlink,              showrooms,             roomcolor,                                                                 linkcolor,                                                                 large)
          putBmap(dest, misc, x, y, css->objects[i].d1 != 0, css->objects[i].d2!=0, css->objects[i].d3!=0, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), css->objects[i].d10 != 0);
        }
        break;
        case ssoCLEAR:
        {
          clear_to_color(dest,subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1));
        }
        break;
        case ssoCURRENTITEM:
        {
          if (css->objects[i].d2)
          {
            subscreenitem(dest, x, y, css->objects[i].d1);
          }
        }
        break;
        case ssoTEXTBOX:
        {
        //draw_textbox(dest, x, y, w,                 h,                 tempfont, thetext,                     wword,                 tabsize,            alignment,          textstyle,          color,                                                                     shadowcolor,                                                               backcolor)
          draw_textbox(dest, x, y, css->objects[i].w, css->objects[i].h, tempfont, (char *)css->objects[i].dp1, css->objects[i].d4!=0, css->objects[i].d5, css->objects[i].d2, css->objects[i].d3, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
//          draw_textbox(dest, 0, 0, 200, 50, sfont, "This is a test", 1, 4, 0, 0, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
        }
        break;
        case ssoSELECTEDITEMNAME:
        {
			if(Bitem==NULL)
				break;
		  item *Bitem2 = new item((fix)0, (fix)0, (fix)0,get_bit(quest_rules,qr_SELECTAWPN) ? Bweapon(Bpos)&0xFFF : Bitem->id, 0, 0);
          if (!Bitem2)

          {
            break;
          }
          char itemname[80];
          sprintf(itemname, "%s", subscreen_item_names[Bitem2->id]);

          switch (Bitem2->id)
          {
            case iArrow:
            case iSArrow:
            case iGArrow:
            if (Bitem->dummy_bool[0]==true) //if we also have a bow
            {
              if(current_item(itype_bow))
              {
                bool hasarrows=((get_bit(quest_rules,qr_TRUEARROWS)&&(game==NULL || game->get_arrows()))||(!get_bit(quest_rules,qr_TRUEARROWS)&&(game==NULL || game->get_rupies())));
                switch (current_item(itype_bow))
                {
                  case 1:
                  sprintf(itemname, "Short Bow%s%s", hasarrows?" & ":"",hasarrows?subscreen_item_names[Bitem->id]:"");
                  break;
                  case 2:
                  sprintf(itemname, "Long Bow%s%s", hasarrows?" & ":"",hasarrows?subscreen_item_names[Bitem->id]:"");
                  break;
                }
              }
            }
            break;
          }
          draw_textbox(dest, x, y, css->objects[i].w, css->objects[i].h, tempfont, itemname, css->objects[i].d4!=0, css->objects[i].d5, css->objects[i].d2, css->objects[i].d3, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), subscreen_color(misc, css->objects[i].colortype3, css->objects[i].color3));
       // draw_textbox(dest, x, y, w,                 h,                 tempfont, thetext,  wword,                 tabsize,            alignment,          textstyle,          color,                                             shadowcolor,                                       backcolor);
        }
        break;
        case ssoTRIFRAME:
        {
//        puttriframe(dest, misc, x, y, triframecolor,                                                             numbercolor,                                                               triframetile,       triframecset,       triforcetile,       triforcecset,       showframe,             showpieces,            largepieces)
          puttriframe(dest, misc, x, y, subscreen_color(misc, css->objects[i].colortype1, css->objects[i].color1), subscreen_color(misc, css->objects[i].colortype2, css->objects[i].color2), css->objects[i].d1, css->objects[i].d2, css->objects[i].d3, css->objects[i].d4, css->objects[i].d5!=0, css->objects[i].d6!=0, css->objects[i].d7!=0);
        }
        break;
        case ssoTRIFORCE:
        {
          puttriforce(dest,misc,x,y,css->objects[i].d1,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].w,css->objects[i].h,css->objects[i].d2,css->objects[i].d3!=0,css->objects[i].d4!=0,css->objects[i].d5);
        }
        break;
        case ssoTILEBLOCK:
        {
          draw_block_flip(dest,x,y,css->objects[i].d1,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].w,css->objects[i].h,css->objects[i].d2,css->objects[i].d3!=0,css->objects[i].d4!=0);
        }
        break;
        case ssoMINITILE:
        {
          int t=css->objects[i].d1;
          if (t==-1)
          {
            switch (css->objects[i].d2)
            {
              case ssmstSSVINETILE:
              t=wpnsbuf[iwSubscreenVine].tile*4;
              break;
              case ssmstMAGICMETER:
              t=wpnsbuf[iwMMeter].tile*4;
              break;
              default:
              t=(rand()*100000)%32767;
              break;
            }
          }
          t+=css->objects[i].d3;
          if (css->objects[i].d5)
          {
            if (css->objects[i].d6)
            {
              overtiletranslucent8(dest,t,x,y,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].d4,128);
            }
            else
            {
              overtile8(dest,t,x,y,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].d4);
            }
          }
          else
          {
            if (css->objects[i].d6)
            {
              puttiletranslucent8(dest,t,x,y,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].d4,128);
            }
            else
            {
              oldputtile8(dest,t,x,y,subscreen_cset(misc, css->objects[i].colortype1, css->objects[i].color1),css->objects[i].d4);
            }
          }
        }
        break;
        case ssoSELECTOR1:
        case ssoSELECTOR2:
        {
          int p=-1;
          for (int j=0; j<MAXSUBSCREENITEMS&&css->objects[j].type!=ssoNULL; ++j)
          {
            if (css->objects[j].type==ssoCURRENTITEM)
            {
              if (css->objects[j].d3==(int)Bpos)
              {
                p=j;
                break;
              }
            }
          }
          bool big_sel=css->objects[i].d5 != 0;
          item *tempsel=(css->objects[i].type==ssoSELECTOR1)?&sel_a:&sel_b;
          int temptile=tempsel->tile;
          tempsel->drawstyle=0;
          if (css->objects[i].d4)
          {
            tempsel->drawstyle=1;
          }
          for (int j=0; j<4; ++j)
          {
            if (p!=-1)
            {
              tempsel->x=css->objects[p].x+xofs+(big_sel?(j%2?8:-8):0);
              tempsel->y=css->objects[p].y+yofs+(big_sel?(j>1?8:-8):0);
              tempsel->tile+=(max(itemsbuf[tempsel->id].frames,1)*j);
              if (temptile)
              {
                tempsel->draw(dest);
              }
              tempsel->tile=temptile;
              if (!big_sel)
              {
                break;
              }
            }
          }
        }
        break;
        case ssoMAGICGAUGE:
        {
          //          void magicgauge(BITMAP *dest,int x,int y, int container, int notlast_tile, int notlast_cset, bool notlast_mod, int last_tile, int last_cset, bool last_mod,
          //                          int cap_tile, int cap_cset, bool cap_mod, int aftercap_tile, int aftercap_cset, bool aftercap_mod, int frames, int speed, int delay, bool unique_last,
          //                          int show)
          magicgauge(dest,x,y, css->objects[i].d1, css->objects[i].d2, css->objects[i].colortype1, ((css->objects[i].d10&1)?1:0), css->objects[i].d3, css->objects[i].color1, ((css->objects[i].d10&2)?1:0),
                     css->objects[i].d4, css->objects[i].colortype2, ((css->objects[i].d10&4)?1:0), css->objects[i].d5, css->objects[i].color2, ((css->objects[i].d10&8)?1:0), css->objects[i].d6, css->objects[i].d7, css->objects[i].d8, ((css->objects[i].d10&16)?1:0),
                     css->objects[i].d9);
        }
        break;
        case ssoLIFEGAUGE:
        {
          lifegauge(dest,x,y, css->objects[i].d1, css->objects[i].d2, css->objects[i].colortype1, ((css->objects[i].d10&1)?1:0), css->objects[i].d3, css->objects[i].color1, ((css->objects[i].d10&2)?1:0),
                    css->objects[i].d4, css->objects[i].colortype2, ((css->objects[i].d10&4)?1:0), css->objects[i].d5, css->objects[i].color2, ((css->objects[i].d10&8)?1:0), css->objects[i].d6, css->objects[i].d7, css->objects[i].d8, ((css->objects[i].d10&16)?1:0));
        }
        break;
        default:
        {
        }
        break;
      }
      //sso_bounding_box(dest, css, i, vc(15));
    }
  }
}


void buttonitem(BITMAP *dest, int button, int x, int y)
{
  switch (button)
  {
    case 0:  //A button
    if(Aitem&&show_subscreen_items)
    {
      Aitem->x=x;
      Aitem->y=y;
      switch (Aitem->id)
      {
        case iArrow:
        case iSArrow:
        case iGArrow:
        if (Aitem->dummy_bool[0]==true)
        {
          if(current_item(itype_bow))
          {
            subscreenitem(dest, x, y, ssiBOW);
            if (((get_bit(quest_rules,qr_TRUEARROWS)&&!game->get_arrows())
	      ||(!get_bit(quest_rules,qr_TRUEARROWS)&&!game->get_rupies()&&!can_use_item(itype_wallet,i_mwallet)))
		&&!can_use_item(itype_quiver,i_quiverl4))
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
      switch (Bitem->id)
      {
        case iArrow:
        case iSArrow:
        case iGArrow:
        if (Bitem->dummy_bool[0]==true)
        {
          if(current_item(itype_bow))
          {
            subscreenitem(dest, x, y, ssiBOW);
            if (((get_bit(quest_rules,qr_TRUEARROWS)&&(game != NULL && !game->get_arrows()))
	      ||(!get_bit(quest_rules,qr_TRUEARROWS)&&(game != NULL && !game->get_rupies())&&!can_use_item(itype_wallet,i_mwallet)))
		&&!can_use_item(itype_quiver,i_quiverl4))
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
  int yofs = (game==NULL || game->get_sbombs()) ? 8 : 0;

  //88, 16
  overtile8(dest,5,x,y,1,0);
  overtile8(dest,8,x,y+16-yofs,1,0);
  overtile8(dest,9,x,y+24-yofs,0,0);
  if(game)
  {
	putxnum(dest,x+8,y,game->get_rupies(),tempfont,color,shadowcolor,bgcolor,textstyle,usex,digits,can_use_item(itype_wallet,i_mwallet),idigit);
	putxnum(dest,x+8,y+16-yofs,game->get_keys()+game->get_lkeys(),tempfont,color,shadowcolor,bgcolor, textstyle,usex,digits,can_use_item(itype_magickey,i_magickey),idigit);
	putxnum(dest,x+8,y+24-yofs,game->get_bombs(),tempfont,color,shadowcolor,bgcolor,textstyle,usex,digits,can_use_item(itype_bombbag,i_bombbag4),idigit);
  }

  if(game==NULL || game->get_sbombs())
  {
    overtile8(dest,13,x,y+24,1,0);
	if(game)
		putxnum(dest,x+8,y+24,game->get_sbombs(),tempfont,color,shadowcolor,bgcolor,textstyle,usex,digits,0,idigit);
  }
}

bool is_counter_item(int itemtype, int countertype)
{
  switch (countertype)
  {
    case sscBOMBS:
      if (itemtype==iBombs)
      {
        return true;
      }
      break;
    case sscSBOMBS:
      if (itemtype==iSBomb)
      {
        return true;
      }
      break;
    case sscARROWS:
      if (itemtype==iArrow||itemtype==iSArrow||itemtype==iGArrow)
      {
        return true;
      }
      break;
  }
  return false;
}

void counter(BITMAP *dest, int x, int y, FONT *tempfont, int color, int shadowcolor, int bgcolor, int alignment, int textstyle, int digits, char idigit, bool showzero, int itemtype1, int itemtype2, int itemtype3, int infiniteitem, bool onlyselected)
{
  int value=0;
  bool infinite=false;
  if (infiniteitem)
  {
    infinite=true;
  }
  char valstring[80];
  char formatstring[80];
  sprintf(valstring,"01234567890123456789");
  sprintf(formatstring, "%%0%dd", digits);
  if (onlyselected && !(((Bitem&&(is_counter_item(Bitem->id,itemtype1)||is_counter_item(Bitem->id,itemtype2)||is_counter_item(Bitem->id,itemtype3)))||(Aitem&&(is_counter_item(Aitem->id,itemtype1)||is_counter_item(Aitem->id,itemtype2)||is_counter_item(Aitem->id,itemtype3))))))
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
	
    switch (itemtype)
    {
      case sscRUPEES:
	if (can_use_item(itype_wallet,i_mwallet))
	  infinite=true;
        value+=game->get_rupies();
        break;
      case sscBOMBS:
	if (can_use_item(itype_bombbag,i_bombbag4))
	  infinite=true;
        value+=game->get_bombs();
        break;
      case sscSBOMBS:
        value+=game->get_sbombs();
        break;
      case sscARROWS:
	if ((!get_bit(quest_rules,qr_TRUEARROWS) && can_use_item(itype_wallet,i_mwallet)) || can_use_item(itype_quiver,i_quiverl4))
	  infinite=true;
        if(current_item(itype_arrow))
        {
          if (get_bit(quest_rules,qr_TRUEARROWS))
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
        if (can_use_item(itype_magickey,i_magickey))
        {
          infinite=true;
        }
        //fall through
      case sscGENKEYNOMAGIC:
        value+=game->get_keys();
        break;
      case sscLEVKEYMAGIC:
        if (can_use_item(itype_magickey,i_magickey))
        {
          infinite=true;
        }
        //fall through
      case sscLEVKEYNOMAGIC:
        value+=game->get_lkeys();
        break;
      case sscANYKEYMAGIC:
        if (can_use_item(itype_magickey,i_magickey))
        {
          infinite=true;
        }
        //fall through
      case sscANYKEYNOMAGIC:
        value+=game->get_keys()+game->get_lkeys();
        break;
      default:
        break;
    }
  // (for loop) }
  if (!showzero&&!value&&!infinite)
  {
    return;
  }
  if (infinite)
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

  if ((title_len1>0)||(title_len2>0))
  {
    if ((title_len1>0)&&(title_len2>0))
    {
      textprintf_styled_aligned_ex(dest,tempfont,x,y+8,textstyle,alignment,color,shadowcolor,bgcolor,dmaptitle[1]);
      textprintf_styled_aligned_ex(dest,tempfont,x,y,textstyle,alignment,color,shadowcolor,bgcolor,dmaptitle[0]);
    }
    else
    {
      if (title_len1>0)
      {
        textprintf_styled_aligned_ex(dest,tempfont,x,y+8,textstyle,alignment,color,shadowcolor,bgcolor,dmaptitle[0]);
      }
      else
      {
        textprintf_styled_aligned_ex(dest,tempfont,x,y+8,textstyle,alignment,color,shadowcolor,bgcolor,dmaptitle[1]);
      }
    }
  }
}

void put_passive_subscr(BITMAP *dest,miscQdata *misc,int x,int y,bool showtime,bool pulled_down)
{
  // uncomment this?
  //  load_Sitems();
  update_subscr_items();
  BITMAP *subscr = create_sub_bitmap(dest,x,y,256,passive_subscreen_height);
  if (no_subscreen())
  {
    clear_to_color(subscr,0);
	destroy_bitmap(subscr);
    return;
  }
  show_custom_subscreen(subscr, misc, current_subscreen_passive, 0, 0, showtime, pulled_down);
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
  if (triframetile==0)
  {
    triframetile=misc->colors.triframe_tile;
    triframecset=misc->colors.triframe_cset;
  }
  if (triforcetile==0)
  {
    triforcetile=misc->colors.triforce_tile;
    triforcecset=misc->colors.triforce_cset;
  }

  if (!largepieces)
  {
    if (showframe)
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

    if (showpieces)
    {
      for(int i=0; i<8; i++)
      {
        int lvl = i+1;
        if(get_bit(quest_rules,qr_4TRI) && lvl>4)
          lvl -= 4;

        if(has_item(itype_triforcepiece, lvl))
        {
          int tp=0;
          for( ; tp<8; tp++)
            if(misc->triforce[tp] == i+1)
              break;

            overtile16(dest,triforcetile,x+tripiece[0][tp][0],y+tripiece[0][tp][1],triforcecset,tripiece[0][tp][2]);
        }
      }
    }
  }
  else
  {
    if (showframe)
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
          for( ; tp<8; tp++)
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

    if (showpieces)
    {
      for(int i=0; i<8; i++)
      {
        int lvl = i+1;
        if(get_bit(quest_rules,qr_4TRI) && lvl>4)
        {
          lvl -= 4;
        }

        int tp=0;
        for( ; tp<8; tp++)
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
  if (tile==0)
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
      for( ; tp<8; tp++)
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

  if (showmap)
  {
    if (maptile)
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
              case 3:  draw_sprite_v_flip(dest,bmp,(x2<<3)+x,(y2*72)+y); break;
              case 1:
              default: draw_sprite(dest,bmp,(x2<<3)+x,(y2*72)+y); break;
            }
          }
        }
      }
      destroy_bitmap(bmp);
    }
  }
  if (showrooms)
  {
    if (roomcolor==-1)
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
          if(get_bmaps(si)&1) _allegro_hline(dest,x2+3,y2,x2+4,roomcolor);
          if(get_bmaps(si)&2) _allegro_hline(dest,x2+3,y2+7,x2+4,roomcolor);
          if(get_bmaps(si)&4) _allegro_vline(dest,x2,y2+3,y2+4,roomcolor);
          if(get_bmaps(si)&8) _allegro_vline(dest,x2+7,y2+3,y2+4,roomcolor);
        }
        ++si;
      }
    }
  }

  if (showlink)
  {
    if(get_currscr()<128)
    {
      if (linkcolor==-1)
      {
        linkcolor=misc->colors.link_dot;
      }
      putdot(dest,(((get_homescr()&15)-DMaps[get_currdmap()].xoff)<<3)+x+(large?34:18)+(maptile?8:0),((get_homescr()&0xF0)>>1)+y+11,linkcolor);
    }
  }
}

void load_Sitems(miscQdata *misc)
{
  Sitems.clear();
/*
  if(Awpn > 0)
  {
    Sitems.add(new item((fix)0, (fix)0,(fix)0, Awpn, 0, 0));
  }
*/
  if(current_item(itype_raft)>=i_raft)
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iRaft,0,0));
  }
  if(current_item(itype_book)>=i_book)
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iBook,0,0));
  }
  if(current_item(itype_ring))
  {
    if (current_item(itype_ring)>=3)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iGRing,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_ring)-1+iBRing,0,0));
    }
  }
  if(current_item(itype_ladder))
  {
    if (current_item(itype_ladder)>=i_ladder2)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iL2Ladder,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iLadder,0,0));
    }
  }
  if(current_item(itype_magickey)>=i_magickey)
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iMKey,0,0));
  }
  if(current_item(itype_bracelet))
  {
    if(current_item(itype_bracelet)>=i_bracelet2)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iL2Bracelet,0,0));
    }
    else if(current_item(itype_bracelet)>=i_bracelet1)
      {
        Sitems.add(new item((fix)0,(fix)0,(fix)0,iBracelet,0,0));
      }
    else
      {
	Sitems.add(new item((fix)0,(fix)0,(fix)0,iOldGlove,0,0));
      }
  }

  if(current_item(itype_shield))
  {
    if(current_item(itype_shield)>=i_mirrorshield)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iMShield,0,0));
    }
    else if(current_item(itype_shield)>=i_largeshield)
      {
        Sitems.add(new item((fix)0,(fix)0,(fix)0,iShield,0,0));
      }
    else if(current_item(itype_shield)>=i_smallshield)
      {
        Sitems.add(new item((fix)0,(fix)0,(fix)0,iSShield,0,0));
      }
  }
  if(current_item(itype_sword))
  {
    if(current_item(itype_shield)>=i_xsword)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iXSword,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iSword-1+current_item(itype_sword),0,0));
    }
  }
  if(current_item(itype_wallet))
  {
    if(current_item(itype_wallet)==3)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iWalletL3,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iWallet500-1+current_item(itype_wallet),0,0));
    }
  }
  if(current_item(itype_amulet))
  {
    if(current_item(itype_amulet)>=i_amulet2)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iL2Amulet,0,0));
    }
    else if (current_item(itype_amulet)>=i_amulet1)
      {
        Sitems.add(new item((fix)0,(fix)0,(fix)0,iAmulet,0,0));
      }
  }
  if(current_item(itype_flippers)>=i_flippers)
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iFlippers,0,0));
  }
  if(current_item(itype_boots)>=i_boots)
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iBoots,0,0));
  }
  // HC Pieces
  if(misc->colors.HCpieces_tile)
  {
    //      item *HCP = new item((fix)(inventory_x[5]-ofs),(fix)y,iMax,0,0);
    item *HCP = new item((fix)0,(fix)0,(fix)0,iHCPiece,0,0);
    if(HCP)
    {
		//this method is called in the subscreen code to set up the item colors
		//as a result this HACK is necessary to prevent ZQ from crashing, as ZQ has
		//no valid game pointer.
		//This haphazardly intermingled ZQ and ZC code makes me very sad :-/ -DD
		if(game != NULL)
			HCP->tile   = misc->colors.HCpieces_tile + vbound(game->get_HCpieces(),0,3);
      HCP->o_tile = HCP->tile;
      HCP->cs     = misc->colors.HCpieces_cset;
      HCP->frames = 0;
      Sitems.add(HCP);
    }
  }
  if(current_item(itype_brang))
  {
    if (current_item(itype_brang)<i_fbrang)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_brang)-1+iBrang,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iFBrang,0,0));
    }
  }
  //yet another ZQ problem... sigh
  if(game == NULL || game->get_bombs())
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iBombs,0,0));
  }
  if(current_item(itype_bow))
  {
    if(current_item(itype_bow)==2)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iBow2,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iBow,0,0));
    }
  }
  if(current_item(itype_arrow))
  {
    if (current_item(itype_arrow)<i_garrow)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_arrow)-1+iArrow,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iGArrow,0,0));
    }
  }
  if(current_item(itype_candle))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_candle)-1+iBCandle,0,0));
  }
  if(current_item(itype_whistle))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iWhistle,0,0));
  }
  if(current_item(itype_bait))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iBait,0,0));
  }
  if(current_item(itype_potion))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_potion)-1+iBPotion,0,0));
  }
  if(current_item(itype_letter))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iLetter,0,0));
  }
  if(current_item(itype_wand))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iWand,0,0));
  }
  if(current_item(itype_hookshot))
  {
   if(current_item(itype_hookshot)==2)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iLongshot,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iHookshot,0,0));
    }
  }
  if(game == NULL || game->get_sbombs())
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iSBomb,0,0));
  }
  if(current_item(itype_lens))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iLens,0,0));
  }
  if(current_item(itype_hammer))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iHammer,0,0));
  }
  if(current_item(itype_dinsfire))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iDinsFire,0,0));
  }
  if(current_item(itype_faroreswind))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iFaroresWind,0,0));
  }
  if(current_item(itype_nayruslove))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iNayrusLove,0,0));
  }
  if(current_item(itype_quiver))
  {
    if(current_item(itype_quiver)==4)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iQuiverL4,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_quiver)-1+iQuiver,0,0));
    }
  }
  if(current_item(itype_cbyrna))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iCByrna,0,0));
  }
  if(current_item(itype_rocs))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_rocs)-1+iRocsFeather,0,0));
  }
  if(current_item(itype_hoverboots))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iHoverBoots,0,0));
  }
  if(current_item(itype_spinscroll))
  {
   if(current_item(itype_spinscroll)==2)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iL2SpinScroll,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iSpinScroll,0,0));
    }
  }
  if(current_item(itype_crossscroll))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iCrossScroll,0,0));
  }
  if(current_item(itype_quakescroll))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_quakescroll)-1+iQuakeScroll,0,0));
  }
  if(current_item(itype_whispring))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_whispring)-1+iWhispRing,0,0));
  }
  if(current_item(itype_chargering))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_chargering)-1+iChargeRing,0,0));
  }
  if(current_item(itype_perilscroll))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iPerilScroll,0,0));
  }
  if(current_item(itype_wealthmedal))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_wealthmedal)-1+iWealthMedal,0,0));
  }
  if(current_item(itype_wealthmedal))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_wealthmedal)-1+iWealthMedal,0,0));
  }
  if(current_item(itype_bombbag))
  {
    if(current_item(itype_bombbag)==4)
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,iBombBagL4,0,0));
    }
    else
    {
      Sitems.add(new item((fix)0,(fix)0,(fix)0,current_item(itype_bombbag)-1+iBombBag,0,0));
    }
  }
  if(has_item(itype_map, get_dlevel()))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iMap,0,0));
  }
  if(has_item(itype_compass, get_dlevel()))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iCompass,0,0));
  }
  if(has_item(itype_bosskey, get_dlevel()))
  {
    Sitems.add(new item((fix)0,(fix)0,(fix)0,iBossKey,0,0));
  }
  new_sel=true;
}

void update_subscreens()
{
  int index=DMaps[get_currdmap()].active_subscreen;

  int i=-1, j=0;
  while (custom_subscreen[j].objects[0].type!=ssoNULL&&i!=index)
  {
    if (custom_subscreen[j].ss_type==sstACTIVE)
    {
      ++i;
    }
    ++j;
  }
  current_subscreen_active=&custom_subscreen[j-1];


  index=DMaps[get_currdmap()].passive_subscreen;

  i=-1, j=0;
  while (custom_subscreen[j].objects[0].type!=ssoNULL&&i!=index)
  {
    if (custom_subscreen[j].ss_type==sstPASSIVE)
    {
      ++i;
    }
    ++j;
  }
  current_subscreen_passive=&custom_subscreen[j-1];
}

int ss_objects(subscreen_group *tempss)
{
  int i=0;
  while (i<MAXSUBSCREENITEMS&&tempss->objects[i].type!=ssoNULL)
  {
    ++i;
  }
  return i;
}


void purge_blank_subscreen_objects(subscreen_group *tempss)
{
  int objects=ss_objects(tempss);
  subscreen_object tempsso;
  //filter all the ssoNONE items to the end (yeah, bubble sort; sue me)
  for (int j=0; j<objects-1; j++)
  {
    for (int k=0; k<objects-1-j; k++)
    {
      if (tempss->objects[k].type==ssoNONE)
      {
        tempsso=tempss->objects[k];
        tempss->objects[k]=tempss->objects[k+1];
        tempss->objects[k+1]=tempsso;
      }
    }
  }
  //NULL out the ssoNONE items
  for (int j=0; j<objects; j++)
  {
    if (tempss->objects[j].type==ssoNONE)
    {
      tempss->objects[j].type=ssoNULL;
    }
  }
}



int get_alignment(subscreen_object *tempsso)
{
  switch (tempsso->type)
  {
    case ssoTEXT:
    return tempsso->d2;
    break;

    case ssoBSTIME:
    return sstaLEFT;
    break;

    case ssoTIME:
    return sstaRIGHT;
    break;

    case ssoSSTIME:
    return sstaRIGHT;
    break;

    case ssoCOUNTER:
    return tempsso->d2;
    break;

    case ssoMINIMAPTITLE:
    return tempsso->d2;
    break;

    case ssoCURRENTITEMTEXT:
    case ssoCURRENTITEMNAME:
    case ssoSELECTEDITEMNAME:
    case ssoCURRENTITEMCLASSTEXT:
    case ssoCURRENTITEMCLASSNAME:
    case ssoSELECTEDITEMCLASSNAME:
    return sstaLEFT;
    break;

    case ssoNONE:
    case sso2X2FRAME:
    case ssoLINE:
    case ssoRECT:
    case ssoMAGICMETER:
    case ssoLIFEMETER:
    case ssoBUTTONITEM:
    case ssoICON:
    case ssoCOUNTERS:
    case ssoMINIMAP:
    case ssoLARGEMAP:
    case ssoCLEAR:
    case ssoCURRENTITEM:
    case ssoITEM:
    case ssoTRIFRAME:
    case ssoTRIFORCE:
    case ssoTILEBLOCK:
    case ssoMINITILE:
    case ssoSELECTOR1:
    case ssoSELECTOR2:
    case ssoMAGICGAUGE:
    case ssoLIFEGAUGE:
    case ssoTEXTBOX:
    case ssoCURRENTITEMTILE:
    case ssoSELECTEDITEMTILE:
    default:
    return sstaLEFT;
    break;
  }
//  return false;//UNREACHABLE
}

int sso_y(subscreen_object *tempsso)
{
  int y=0;
  switch (tempsso->type)
  {
    case ssoLIFEMETER:
    y=tempsso->y+(tempsso->d2?0:(get_bit(quest_rules, qr_24HC)?8:16));
    break;

    case ssoNONE:
    case sso2X2FRAME:
    case ssoTEXT:
    case ssoLINE:
    case ssoRECT:
    case ssoBSTIME:
    case ssoTIME:
    case ssoSSTIME:
    case ssoMAGICMETER:
    case ssoBUTTONITEM:
    case ssoICON:
    case ssoCOUNTER:
    case ssoCOUNTERS:
    case ssoMINIMAPTITLE:
    case ssoMINIMAP:
    case ssoLARGEMAP:
    case ssoCLEAR:
    case ssoCURRENTITEM:
    case ssoITEM:
    case ssoTRIFRAME:
    case ssoTRIFORCE:
    case ssoTILEBLOCK:
    case ssoMINITILE:
    case ssoSELECTOR1:
    case ssoSELECTOR2:
    case ssoMAGICGAUGE:
    case ssoLIFEGAUGE:
    case ssoTEXTBOX:
    case ssoCURRENTITEMTILE:
    case ssoSELECTEDITEMTILE:
    case ssoCURRENTITEMTEXT:
    case ssoCURRENTITEMNAME:
    case ssoSELECTEDITEMNAME:
    case ssoCURRENTITEMCLASSTEXT:
    case ssoCURRENTITEMCLASSNAME:
    case ssoSELECTEDITEMCLASSNAME:
    y=tempsso->y;
    break;

    default:
    y=(rand()*1000)%64;
    break;
  }
  return y;
}

int sso_x(subscreen_object *tempsso)
{
  int x=0;
  switch (tempsso->type)
  {
    case ssoMAGICMETER:
      x=tempsso->x-10;
      break;
    case ssoNONE:
    case sso2X2FRAME:
    case ssoTEXT:
    case ssoLINE:
    case ssoRECT:
    case ssoBSTIME:
    case ssoTIME:
    case ssoSSTIME:
    case ssoLIFEMETER:
    case ssoBUTTONITEM:
    case ssoICON:
    case ssoCOUNTER:
    case ssoCOUNTERS:
    case ssoMINIMAPTITLE:
    case ssoMINIMAP:
    case ssoLARGEMAP:
    case ssoCLEAR:
    case ssoCURRENTITEM:
    case ssoITEM:
    case ssoTRIFRAME:
    case ssoTRIFORCE:
    case ssoTILEBLOCK:
    case ssoMINITILE:
    case ssoSELECTOR1:
    case ssoSELECTOR2:
    case ssoMAGICGAUGE:
    case ssoLIFEGAUGE:
    case ssoTEXTBOX:
    case ssoCURRENTITEMTILE:
    case ssoSELECTEDITEMTILE:
    case ssoCURRENTITEMTEXT:
    case ssoCURRENTITEMNAME:
    case ssoSELECTEDITEMNAME:
    case ssoCURRENTITEMCLASSTEXT:
    case ssoCURRENTITEMCLASSNAME:
    case ssoSELECTEDITEMCLASSNAME:
    x=tempsso->x;
    break;

    default:
    x=(rand()*1000)%64;
    break;
  }
  return x;
}



int sso_h(subscreen_object *tempsso)
{
  int h=0;
  switch (tempsso->type)
  {
    case ssoNONE:
    h=5;
    break;

    case sso2X2FRAME:
    h=tempsso->h*8;
    break;

    case ssoTEXT:
    h=text_height(ss_font(tempsso->d1));
    break;

    case ssoLINE:
    h=tempsso->h;
    break;

    case ssoRECT:
    h=tempsso->h;
    break;

    case ssoBSTIME:
    h=text_height(ss_font(tempsso->d1));
    break;

    case ssoTIME:
    h=text_height(ss_font(tempsso->d1));
    break;

    case ssoSSTIME:
    h=text_height(ss_font(tempsso->d1));
    break;

    case ssoMAGICMETER:
    h=8;
    break;

    case ssoLIFEMETER:
    h=get_bit(quest_rules, qr_24HC)?24:16;
    break;

    case ssoBUTTONITEM:
    h=16;
    break;

    case ssoICON:
    h=8;
    break;

    case ssoCOUNTER:
    h=text_height(ss_font(tempsso->d1));
    break;

    case ssoCOUNTERS:
    h=32;
    break;

    case ssoMINIMAPTITLE:
    h=16;
    break;

    case ssoMINIMAP:
    h=48;
    break;

    case ssoLARGEMAP:
    h=80;
    break;

    case ssoCLEAR:
    h=5;
    break;

    case ssoCURRENTITEM:
    h=16;
    break;

    case ssoITEM:
    h=16;
    break;

    case ssoTRIFRAME:
    h=16*((tempsso->d7)?7:3);
    break;

    case ssoTRIFORCE:
    h=16;
    break;

    case ssoTILEBLOCK:
    h=16*(tempsso->h);
    break;

    case ssoMINITILE:
    h=8;
    break;

    case ssoSELECTOR1:
    case ssoSELECTOR2:
    h=tempsso->d5?32:16;
    break;

    case ssoMAGICGAUGE:
    h=8;
    break;

    case ssoLIFEGAUGE:
    h=8;
    break;

    case ssoTEXTBOX:
    h=tempsso->h;
    break;

    case ssoCURRENTITEMTILE:
    h=16;
    break;

    case ssoSELECTEDITEMTILE:
    h=16;
    break;

    case ssoCURRENTITEMTEXT:
    h=16;
    break;

    case ssoCURRENTITEMNAME:
    h=tempsso->h;
    break;

    case ssoSELECTEDITEMNAME:
    h=tempsso->h;
    break;

    case ssoCURRENTITEMCLASSTEXT:
    h=tempsso->h;
    break;

    case ssoCURRENTITEMCLASSNAME:
    h=tempsso->h;
    break;

    case ssoSELECTEDITEMCLASSNAME:
    h=tempsso->h;
    break;

    default:
    h=(rand()*1000)%64;
    break;
  }
  return h;
}

int sso_w(subscreen_object *tempsso)
{
  int w=0;
  switch (tempsso->type)
  {
    case ssoNONE:
    w=5;
    break;

    case sso2X2FRAME:
    w=tempsso->w*8;
    break;

    case ssoTEXT:
    w=text_length(ss_font(tempsso->d1), (char *)tempsso->dp1);
    break;

    case ssoLINE:
    w=tempsso->w;
    break;

    case ssoRECT:
    w=tempsso->w;
    break;

    case ssoBSTIME:
    w=text_length(ss_font(tempsso->d1), time_str_short2(game->get_time()));
    break;

    case ssoTIME:
    w=text_length(ss_font(tempsso->d1), time_str_med(game->get_time()));
    break;

    case ssoSSTIME:
    w=text_length(ss_font(tempsso->d1), time_str_med(game->get_time()));
    break;

    case ssoMAGICMETER:
    w=82;
    break;

    case ssoLIFEMETER:
    w=64;
    break;

    case ssoBUTTONITEM:
    w=16;
    break;

    case ssoICON:
    w=8;
    break;

    case ssoCOUNTER:
    w=text_length(ss_font(tempsso->d1), "0")*tempsso->d4;
    break;

    case ssoCOUNTERS:
    w=32;
    break;

    case ssoMINIMAPTITLE:
    w=80;
    break;

    case ssoMINIMAP:
    w=80;
    break;

    case ssoLARGEMAP:
    w=16*((tempsso->d4)?9:7);
    break;

    case ssoCLEAR:
    w=5;
    break;

    case ssoCURRENTITEM:
    w=16;
    break;

    case ssoITEM:
    w=16;
    break;

    case ssoTRIFRAME:
    w=16*((tempsso->d7)?7:6);
    break;

    case ssoTRIFORCE:
    w=16;
    break;

    case ssoTILEBLOCK:
    w=16*(tempsso->w);
    break;

    case ssoMINITILE:
    w=8;
    break;

    case ssoSELECTOR1:
    case ssoSELECTOR2:
    w=tempsso->d5?32:16;
    break;

    case ssoMAGICGAUGE:
    w=8;
    break;

    case ssoLIFEGAUGE:
    w=8;
    break;

    case ssoTEXTBOX:
    w=tempsso->w;
    break;

    case ssoCURRENTITEMTILE:
    w=16;
    break;

    case ssoSELECTEDITEMTILE:
    w=16;
    break;

    case ssoCURRENTITEMTEXT:
    w=16;
    break;

    case ssoCURRENTITEMNAME:
    w=tempsso->w;
    break;

    case ssoSELECTEDITEMNAME:
    w=tempsso->w;
    break;

    case ssoCURRENTITEMCLASSTEXT:
    w=tempsso->w;
    break;

    case ssoCURRENTITEMCLASSNAME:
    w=tempsso->w;
    break;

    case ssoSELECTEDITEMCLASSNAME:
    w=tempsso->w;
    break;

    default:
    w=(rand()*1000)%64;
    break;
  }
  return w;
}


void sso_bounding_box(BITMAP *bmp, subscreen_group *tempss, int index, int color)
{
  if (index<0)
  {
    return;
  }
  int x=sso_x(&tempss->objects[index]);
  int y=sso_y(&tempss->objects[index]);
  int w=sso_w(&tempss->objects[index]);
  int h=sso_h(&tempss->objects[index]);
  switch (get_alignment(&tempss->objects[index]))
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
  if (color==-1)
  {
    color=jwin_pal[jcTITLER];
  }
  else if (color==-2)
  {
    color=jwin_pal[jcTITLEL];
  }
  //draw a bounding box around the current object
  if (normalrect)
  {
    rect(bmp, x, y, x+w-1, y+h-1, color);
  }
  else
  {
    rect(bmp, x-1,   y-1,   x+1, y+1, color);
    rect(bmp, x-1,   y+h-2, x+1, y+h, color);
    rect(bmp, x+w-2, y-1,   x+w, y+1, color);
    rect(bmp, x+w-2, y+h-2, x+w, y+h, color);
    if (w>=9)
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
    if (h>=9)
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
