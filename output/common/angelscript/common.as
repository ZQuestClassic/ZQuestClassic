LinkClass@ Link;

const double PI = 3.14159265358979323846;

const int up = 0;
const int down = 1;
const int left = 2;
const int right = 3;
const int l_up = 4;
const int r_up = 5;
const int l_down = 6;
const int r_down = 7;

const int MAXITEMS = 256;
const int MAXWPNS  = 256;
const int MAXGUYS  = 512;

const int WAV_ARROW          = 1;
const int WAV_BEAM           = 2;
const int WAV_BOMB           = 3;
const int WAV_BRANG          = 4;
const int WAV_CHIME          = 5;
const int WAV_CHINK          = 6;
const int WAV_CLEARED        = 7;
const int WAV_DODONGO        = 8;
const int WAV_DOOR           = 9;
const int WAV_EDEAD          = 10;
const int WAV_EHIT           = 11;
const int WAV_ER             = 12;
const int WAV_FIRE           = 13;
const int WAV_GANON          = 14;
const int WAV_GASP           = 15;
const int WAV_HAMMER         = 16;
const int WAV_HOOKSHOT       = 17;
const int WAV_MSG            = 18;
const int WAV_OUCH           = 19;
const int WAV_PICKUP         = 20;
const int WAV_PLACE          = 21;
const int WAV_PLINK          = 22;
const int WAV_REFILL         = 23;
const int WAV_ROAR           = 24;
const int WAV_SCALE          = 25;
const int WAV_SEA            = 26;
const int WAV_SECRET         = 27;
const int WAV_SPIRAL         = 28;
const int WAV_STAIRS         = 29;
const int WAV_SWORD          = 30;
const int WAV_VADER          = 31;
const int WAV_WAND           = 32;
const int WAV_WHISTLE        = 33;
const int WAV_ZELDA          = 34;
const int WAV_ZN1CHARGE      = 35;
const int WAV_ZN1CHARGE2     = 36;
const int WAV_ZN1DINSFIRE    = 37;
const int WAV_ZN1FALL        = 38;
const int WAV_ZN1FARORESWIND = 39;
const int WAV_ZN1FIREBALL    = 40;
const int WAV_ZN1GRASSCUT    = 41;
const int WAV_ZN1HAMMERPOST  = 42;
const int WAV_ZN1HOVER       = 43;
const int WAV_ZN1ICE         = 44;
const int WAV_ZN1JUMP        = 45;
const int WAV_ZN1LENSOFF     = 46;
const int WAV_ZN1LENSON      = 47;
const int WAV_ZN1NAYRUSLOVE1 = 48;
const int WAV_ZN1NAYRUSLOVE2 = 49;
const int WAV_ZN1PUSHBLOCK   = 50;
const int WAV_ZN1ROCK        = 51;
const int WAV_ZN1ROCKETDOWN  = 52;
const int WAV_ZN1ROCKETUP    = 53;
const int WAV_ZN1SPINATTACK  = 54;
const int WAV_ZN1SPLASH      = 55;
const int WAV_ZN1SUMMON      = 56;
const int WAV_ZN1TAP         = 57;
const int WAV_ZN1TAP2        = 58;
const int WAV_ZN1WHIRLWIND   = 59;
const int WAV_ZN2CANE        = 60;
const int Z35                = 61;

enum mapFlags
{
    mfNONE, mfPUSHUD, mfPUSH4, mfWHISTLE, mfBCANDLE, mfARROW, mfBOMB, mfFAIRY,
    mfRAFT, mfARMOS_SECRET, mfARMOS_ITEM, mfSBOMB, mfRAFT_BRANCH, mfDIVE_ITEM,
    mfLENSMARKER, mfZELDA,
    mfSECRETS01, mfSECRETS02, mfSECRETS03, mfSECRETS04,       /*16*/
    mfSECRETS05, mfSECRETS06, mfSECRETS07, mfSECRETS08,
    mfSECRETS09, mfSECRETS10, mfSECRETS11, mfSECRETS12,
    mfSECRETS13, mfSECRETS14, mfSECRETS15, mfSECRETS16,
    mfTRAP_H, mfTRAP_V, mfTRAP_4, mfTRAP_LR, mfTRAP_UD,       /*32*/
    mfENEMY0, mfENEMY1, mfENEMY2, mfENEMY3, mfENEMY4,         /*37*/
    mfENEMY5, mfENEMY6, mfENEMY7, mfENEMY8, mfENEMY9,
    mfPUSHLR, mfPUSHU, mfPUSHD, mfPUSHL, mfPUSHR,
    mfPUSHUDNS, mfPUSHLRNS, mfPUSH4NS, mfPUSHUNS,
    mfPUSHDNS, mfPUSHLNS, mfPUSHRNS, mfPUSHUDINS,
    mfPUSHLRINS, mfPUSH4INS, mfPUSHUINS,
    mfPUSHDINS, mfPUSHLINS, mfPUSHRINS,
    mfBLOCKTRIGGER, mfNOBLOCKS, mfBRANG, mfMBRANG,
    mfFBRANG, mfSARROW, mfGARROW, mfRCANDLE, mfWANDFIRE, mfDINSFIRE,
    mfWANDMAGIC, mfREFMAGIC, mfREFFIREBALL, mfSWORD, mfWSWORD,
    mfMSWORD, mfXSWORD, mfSWORDBEAM, mfWSWORDBEAM, mfMSWORDBEAM,
    mfXSWORDBEAM, mfHOOKSHOT, mfWAND, mfHAMMER, mfSTRIKE, mfBLOCKHOLE,
    mfMAGICFAIRY, mfALLFAIRY, mfSINGLE, mfSINGLE16,
    mfNOENEMY, mfNOGROUNDENEMY, mfSCRIPT1, mfSCRIPT2, mfSCRIPT3, mfSCRIPT4, mfSCRIPT5,
    mfRAFT_BOUNCE, mfMAX, mfPUSHED
};

enum questRules
{
    // 1st byte out of 20
    qr_SOLIDBLK, qr_NOTMPNORET, qr_ALWAYSRET, qr_MEANTRAPS,
    qr_BSZELDA, qr_FADE, qr_FADECS5, qr_FASTDNGN,
    // 2
    qr_NOLEVEL3FIX, qr_COOLSCROLL, qr_NOSCROLL, qr_4TRI,
    qr_EXPLICIT_RAFTING, qr_FASTFILL, qr_CAVEEXITNOSTOPMUSIC, qr_3TRI,
    // 3
    qr_TIME, qr_FREEFORM, qr_KILLALL, qr_NOFLICKER,
    qr_CONTFULL_DEP/*DEPRECATED*/, qr_RLFIX, qr_LENSHINTS, qr_LINKDUNGEONPOSFIX,
    // 4
    qr_HOLDITEMANIMATION, qr_HESITANTPUSHBLOCKS, qr_HIDECARRIEDITEMS, qr_FFCSCROLL,
    qr_RAFTLENS, qr_SMOOTHVERTICALSCROLLING, qr_WHIRLWINDMIRROR, qr_NOFLASHDEATH,
    // 5
    qr_HOLDNOSTOPMUSIC, qr_FIREPROOFLINK, qr_OUCHBOMBS, qr_NOCLOCKS_DEP/*DEPRECATED*/,
    qr_TEMPCLOCKS_DEP/*DEPRECATED*/, qr_BRKBLSHLDS_DEP/*DEPRECATED*/, qr_BRKNSHLDTILES, qr_MEANPLACEDTRAPS,
    // 6
    qr_PHANTOMPLACEDTRAPS, qr_ALLOWFASTMSG, qr_LINKEDCOMBOS/*DEPRECATED*/, qr_NOGUYFIRES,
    qr_HEARTRINGFIX, qr_NOHEARTRING, qr_OVERWORLDTUNIC, qr_SWORDWANDFLIPFIX,
    // 7
    qr_ENABLEMAGIC, qr_MAGICWAND_DEP/*DEPRECATED*/, qr_MAGICCANDLE_DEP/*DEPRECATED*/, qr_MAGICBOOTS_DEP/*DEPRECATED*/,
    qr_NONBUBBLEMEDICINE, qr_NONBUBBLEFAIRIES, qr_NONBUBBLETRIFORCE, qr_NEWENEMYTILES,
    // 8
    qr_NOROPE2FLASH_DEP/*DEPRECATED*/, qr_NOBUBBLEFLASH_DEP/*DEPRECATED*/, qr_GHINI2BLINK_DEP, qr_WPNANIMFIX /* UNIMPLEMENTED */,
    qr_PHANTOMGHINI2_DEP/*DEPRECATED*/, qr_Z3BRANG_HSHOT, qr_NOITEMMELEE, qr_SHADOWS,
    // 9
    qr_TRANSSHADOWS, qr_QUICKSWORD, qr_BOMBHOLDFIX, qr_EXPANDEDLTM,
    qr_NOPOTIONCOMBINE_DEP/*DEPRECATED*/, qr_LINKFLICKER, qr_SHADOWSFLICKER, qr_WALLFLIERS,
    // 10
    qr_NOBOMBPALFLASH, qr_HEARTSREQUIREDFIX, qr_PUSHBLOCKCSETFIX, qr_TRANSLUCENTNAYRUSLOVEROCKET_DEP/*DEPRECATED*/,
    qr_FLICKERINGNAYRUSLOVEROCKET_DEP/*DEPRECATED*/, qr_CMBCYCLELAYERS, qr_DMGCOMBOPRI, qr_WARPSIGNOREARRIVALPOINT,
    // 11
    qr_LTTPCOLLISION, qr_LTTPWALK, qr_SLOWENEMYANIM_DEP/*DEPRECATED*/, qr_TRUEARROWS,
    qr_NOSAVE, qr_NOCONTINUE, qr_QUARTERHEART, qr_NOARRIVALPOINT,
    // 12
    qr_NOGUYPOOF, qr_ALLOWMSGBYPASS, qr_NODIVING, qr_LAYER12UNDERCAVE,
    qr_NOSCROLLCONTINUE, qr_SMARTSCREENSCROLL, qr_RINGAFFECTDAMAGE, qr_ALLOW10RUPEEDROPS_DEP/*DEPRECATED*/,
    // 13
    qr_TRAPPOSFIX, qr_TEMPCANDLELIGHT, qr_REDPOTIONONCE_DEP/*DEPRECATED*/, qr_OLDSTYLEWARP,
    qr_NOBORDER, qr_OLDTRIBBLES_DEP/*DEPRECATED*/, qr_REFLECTROCKS /*DEPRECATED*/, qr_OLDPICKUP,
    // 14
    qr_ENEMIESZAXIS, qr_SAFEENEMYFADE, qr_MORESOUNDS, qr_BRANGPICKUP,
    qr_HEARTPIECEINCR_DEP/*DEPRECATED*/, qr_ITEMSONEDGES, qr_EATSMALLSHIELD_DEP/*DEPRECATED*/, qr_MSGFREEZE,
    // 15
    qr_SLASHFLIPFIX, qr_FIREMAGICSPRITE_DEP/*DEPRECATED*/, qr_SLOWCHARGINGWALK, qr_NOWANDMELEE,
    qr_SLOWBOMBFUSES_DEP/*DEPRECATED*/, qr_SWORDMIRROR, qr_SELECTAWPN, qr_LENSSEESENEMIES,
    // 16
    qr_INSTABURNFLAGS, qr_DROWN, qr_MSGDISAPPEAR, qr_SUBSCREENOVERSPRITES,
    qr_BOMBDARKNUTFIX, qr_LONGBOMBBOOM_DEP/*DEPRECATED*/, qr_OFFSETEWPNCOLLISIONFIX, qr_DMGCOMBOLAYERFIX,
    // 17
    qr_ITEMSINPASSAGEWAYS, qr_LOG, qr_FIREPROOFLINK2, qr_NOITEMOFFSET,
    qr_ITEMBUBBLE, qr_GOTOLESSNOTEQUAL /* Compatibility */, qr_LADDERANYWHERE, qr_HOOKSHOTLAYERFIX,
    // 18
    qr_REPLACEOPENDOORS /* Compatibility */, qr_OLDLENSORDER /* Compatibility */, qr_NOFAIRYGUYFIRES /* Compatibility */, qr_SCRIPTERRLOG,
    qr_TRIGGERSREPEAT /* Compatibility */, qr_ENEMIESFLICKER, qr_OVALWIPE, qr_TRIANGLEWIPE,
    // 19
    qr_SMASWIPE, qr_NOSOLIDDAMAGECOMBOS /* Compatibility */, qr_SHOPCHEAT, qr_HOOKSHOTDOWNBUG /* Compatibility */,
    qr_OLDHOOKSHOTGRAB /* Compatibility */, qr_PEAHATCLOCKVULN /* Compatibility */, qr_VERYFASTSCROLLING, qr_OFFSCREENWEAPONS /* Compatibility */,
    qr_MAX
};

enum comboTypes
{
    cNONE, cSTAIR, cCAVE, cWATER, cARMOS, cGRAVE, cDOCK,
    cUNDEF, cPUSH_WAIT, cPUSH_HEAVY, cPUSH_HW, cL_STATUE, cR_STATUE,
    cWALKSLOW, cCVUP, cCVDOWN, cCVLEFT, cCVRIGHT, cSWIMWARP, cDIVEWARP,
    cLADDERHOOKSHOT, cTRIGNOFLAG, cTRIGFLAG, cZELDA, cSLASH, cSLASHITEM,
    cPUSH_HEAVY2, cPUSH_HW2, cPOUND, cHSGRAB, cHSBRIDGE, cDAMAGE1,
    cDAMAGE2, cDAMAGE3, cDAMAGE4, cC_STATUE, cTRAP_H, cTRAP_V, cTRAP_4,
    cTRAP_LR, cTRAP_UD, cPIT, cHOOKSHOTONLY, cOVERHEAD, cNOFLYZONE, cMIRROR,
    cMIRRORSLASH, cMIRRORBACKSLASH, cMAGICPRISM, cMAGICPRISM4,
    cMAGICSPONGE, cCAVE2, cEYEBALL_A, cEYEBALL_B, cNOJUMPZONE, cBUSH,
    cFLOWERS, cTALLGRASS, cSHALLOWWATER, cLOCKBLOCK, cLOCKBLOCK2,
    cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2, cLADDERONLY, cBSGRAVE,
    cCHEST, cCHEST2, cLOCKEDCHEST, cLOCKEDCHEST2, cBOSSCHEST, cBOSSCHEST2,
    cRESET, cSAVE, cSAVE2, /*cVERTICAL,*/ cCAVEB, cCAVEC, cCAVED,
    cSTAIRB, cSTAIRC, cSTAIRD, cPITB, cPITC, cPITD,
    cCAVE2B, cCAVE2C, cCAVE2D, cSWIMWARPB, cSWIMWARPC, cSWIMWARPD,
    cDIVEWARPB, cDIVEWARPC, cDIVEWARPD, cSTAIRR, cPITR,
    cAWARPA, cAWARPB, cAWARPC, cAWARPD, cAWARPR,
    cSWARPA, cSWARPB, cSWARPC, cSWARPD, cSWARPR, cSTRIGNOFLAG, cSTRIGFLAG,
    cSTEP, cSTEPSAME, cSTEPALL, cSTEPCOPY, cNOENEMY, cBLOCKARROW1, cBLOCKARROW2,
    cBLOCKARROW3, cBLOCKBRANG1, cBLOCKBRANG2, cBLOCKBRANG3, cBLOCKSBEAM, cBLOCKALL,
    cBLOCKFIREBALL, cDAMAGE5, cDAMAGE6, cDAMAGE7, cCHANGE/**DEPRECATED**/, cSPINTILE1, cSPINTILE2,
    cSCREENFREEZE, cSCREENFREEZEFF, cNOGROUNDENEMY, cSLASHNEXT, cSLASHNEXTITEM, cBUSHNEXT,
    cSLASHTOUCHY, cSLASHITEMTOUCHY, cBUSHTOUCHY, cFLOWERSTOUCHY, cTALLGRASSTOUCHY,
    cSLASHNEXTTOUCHY, cSLASHNEXTITEMTOUCHY, cBUSHNEXTTOUCHY, cEYEBALL_4, cTALLGRASSNEXT,
    cSCRIPT1, cSCRIPT2, cSCRIPT3, cSCRIPT4, cSCRIPT5, cMAX
};

enum specialWalkFlags
{
    spw_none, spw_door, spw_clipright, spw_floater, spw_trap, spw_halfstep,
    spw_water, spw_wizzrobe, spw_clipbottomright
};

// game.maps[] flags
const int mDOOR_UP       =     1;                                 // only dungeons use this
const int mDOOR_DOWN     =     2;                                 //        ''
const int mDOOR_LEFT     =     4;                                 //        ''
const int mDOOR_RIGHT    =     8;                                 //        ''
const int mITEM          =    16;                                 // item (main screen)
const int mBELOW         =    32;                                 // special item (underground)
const int mNEVERRET      =    64;                                 // enemy never returns
const int mTMPNORET      =   128;                                 // enemies don't return until you leave the dungeon
const int mLOCKBLOCK     =   256;                                 // if the lockblock on the screen has been triggered
const int mBOSSLOCKBLOCK =   512;                                 // if the bosslockblock on the screen has been triggered
const int mCHEST         =  1024;                                 // if the unlocked check on this screen has been opened
const int mLOCKEDCHEST   =  2048;                                 // if the locked chest on this screen has been opened
const int mBOSSCHEST     =  4096;                                 // if the boss chest on this screen has been opened
const int mSECRET        =  8192;                                 // only overworld and caves use this
const int mVISITED       = 16384;                                 // only overworld uses this

// Screen flags
// flags2
const int wfUP         = 0x101;
const int wfDOWN       = 0x102;
const int wfLEFT       = 0x104;
const int wfRIGHT      = 0x108;
const int fSECRET      = 0x110; // play "secret" sfx upon entering this screen
const int fAIRCOMBOS   = 0x120;
const int fFLOATTRAPS  = 0x140;
const int fCLEARSECRET = 0x180; // clear all enemies to trigger secret entrance (Enemies->Secret)

// flags3
const int fHOLDITEM        = 0x201;
const int fCYCLEONINIT     = 0x202;
const int fINVISROOM       = 0x204;
const int fINVISLINK       = 0x208;
const int fNOSUBSCR        = 0x210;
const int fIWARPFULLSCREEN = 0x220;                                // instawarps affect all sprites
const int fNOSUBSCROFFSET  = 0x240;                                // don't offset the screen when fNOSUBSCR is true
const int fENEMIESRETURN   = 0x280;                                // enemies always return

//flags7
const int fLAYER3BG     = 0x601;
const int fLAYER2BG     = 0x602;
const int fITEMFALLS    = 0x604;
const int fSIDEVIEW     = 0x608;
const int fNOLINKMARK   = 0x610;
const int fSPECITEMMARK = 0x620;
const int fWHISTLEPAL   = 0x640;
const int fWHISTLEWATER = 0x680;

enum itemType
{
    // 0
    itype_sword, itype_brang, itype_arrow, itype_candle, itype_whistle,
    itype_bait, itype_letter, itype_potion, itype_wand, itype_ring,
    itype_wallet, itype_amulet, itype_shield, itype_bow, itype_raft,
    itype_ladder, itype_book, itype_magickey, itype_bracelet, itype_flippers,
    // 20
    itype_boots, itype_hookshot, itype_lens, itype_hammer, itype_dinsfire,
    itype_faroreswind, itype_nayruslove, itype_bomb, itype_sbomb, itype_clock,
    itype_key, itype_magiccontainer, itype_triforcepiece, itype_map, itype_compass,
    itype_bosskey, itype_quiver, itype_lkey, itype_cbyrna, itype_rupee,
    // 40
    itype_arrowammo, itype_fairy, itype_magic, itype_heart, itype_heartcontainer,
    itype_heartpiece, itype_killem, itype_bombammo, itype_bombbag, itype_rocs,
    itype_hoverboots, itype_spinscroll,itype_crossscroll, itype_quakescroll,itype_whispring,
    itype_chargering, itype_perilscroll, itype_wealthmedal,itype_heartring,itype_magicring,
    // 60
    itype_spinscroll2, itype_quakescroll2, itype_agony, itype_stompboots, itype_whimsicalring,
    itype_perilring, itype_misc,
    // 67
    itype_custom1, itype_custom2, itype_custom3, itype_custom4, itype_custom5,
    itype_custom6, itype_custom7, itype_custom8, itype_custom9, itype_custom10,
    itype_custom11, itype_custom12, itype_custom13, itype_custom14, itype_custom15,
    itype_custom16, itype_custom17, itype_custom18, itype_custom19, itype_custom20,
    // 87
    itype_bowandarrow, itype_letterpotion,
    itype_last, itype_max=255
};

enum weaponType
{
    // 0
    wNone,wSword,wBeam,wBrang,
    wBomb,wSBomb,wLitBomb,wLitSBomb,
    // 8
    wArrow,wFire,wWhistle,wBait,
    wWand,wMagic,wCatching,wWind,
    // 16
    wRefMagic,wRefFireball,wRefRock, wHammer,
    wHookshot, wHSHandle, wHSChain, wSSparkle,
    // 24
    wFSparkle, wSmack, wPhantom, wCByrna,
    wRefBeam, wStomp,
    lwMax,
    // Dummy weapons - must be between lwMax and wEnemyWeapons!
    wScript1, wScript2, wScript3, wScript4,
    wScript5, wScript6, wScript7, wScript8,
    wScript9, wScript10,
    // Enemy weapons
    wEnemyWeapons=128,
    //129
    ewFireball,ewArrow,ewBrang,ewSword,
    ewRock,ewMagic,ewBomb,ewSBomb,
    //137
    ewLitBomb,ewLitSBomb,ewFireTrail,ewFlame,
    ewWind,ewFlame2,ewFlame2Trail,
    //145
    ewIce,ewFireball2,
    wMax
};

enum weapons // Also sprites
{
    wSWORD, wWSWORD, wMSWORD, wXSWORD, wBRANG,
    wMBRANG, wFBRANG, wBOMB, wSBOMB, wBOOM,
// 10
    wARROW, wSARROW, wFIRE, wWIND, wBAIT,
    wWAND, wMAGIC, ewFIREBALL, ewROCK, ewARROW,
// 20
    ewSWORD, ewMAGIC, iwSpawn, iwDeath, iwSwim,
    wHAMMER, wHSHEAD, wHSCHAIN_H, wHSHANDLE, wSSPARKLE,
// 30
    wGSPARKLE, wMSPARKLE, wFSPARKLE, iwHammerSmack, wGARROW,
    ewFLAME, ewWIND, iwMMeter, wDINSFIRE1A, wDINSFIRE1B,
// 40
    wDINSFIRES1A, wDINSFIRES1B, wHSCHAIN_V, iwMore, iwBossMarker,
    iwLinkSlash, wSWORDSLASH, wWSWORDSLASH, wMSWORDSLASH, wXSWORDSLASH,
// 50
    iwShadow, iwLargeShadow, iwBushLeaves, iwFlowerClippings, iwGrassClippings,
    iwTallGrass, iwRipples, iwNPCs, wNAYRUSLOVE1A, wNAYRUSLOVE1B,
// 60
    wNAYRUSLOVES1A, wNAYRUSLOVES1B, wNAYRUSLOVE2A, wNAYRUSLOVE2B, wNAYRUSLOVES2A,
    wNAYRUSLOVES2B, iwNayrusLoveShieldFront, iwNayrusLoveShieldBack, iwSubscreenVine, wCBYRNA,
// 70
    wCBYRNASLASH, wLSHEAD, wLSCHAIN_H, wLSHANDLE, wLSCHAIN_V,
    wSBOOM, ewBOMB, ewSBOMB, ewBOOM, ewSBOOM,
// 80
    ewFIRETRAIL, ewFLAME2, ewFLAME2TRAIL, ewICE, iwHover,
    wFIREMAGIC, iwQuarterHearts, wCBYRNAORB, //iwSideLadder, iwSideRaft,
    
    wLast, wMAX=256
};

enum enemies // Also guys
{
    eNONE=0,
    gNONE=0, gABEI, gAMA, gDUDE, gMOBLIN,
    gFIRE, gFAIRY, gGORIYA, gZELDA, gABEI2,
    //10
    gEMPTY, gDUMMY1, gDUMMY2, gDUMMY3, gDUMMY4,
    gDUMMY5, gDUMMY6, gDUMMY7, gDUMMY8, gDUMMY9,
    //20
    eSTART=20,
    eOCTO1S=20, eOCTO2S, eOCTO1F, eOCTO2F, eTEK1,
    eTEK2, eLEV1, eLEV2, eMOBLIN1, eMOBLIN2,
    //30
    eLYNEL1, eLYNEL2, ePEAHAT, eZORA, eROCK,
    eGHINI1, eGHINI2, eARMOS, eKEESE1, eKEESE2,
    //40
    eKEESE3, eSTALFOS, eGEL, eZOL, eROPE,
    eGORIYA1, eGORIYA2, eTRAP, eWALLM, eDKNUT1,
    //50
    eDKNUT2, eBUBBLEST, eVIRE, eLIKE, eGIBDO,
    ePOLSV, eWIZ1, eWIZ2, eRAQUAM, eMOLDORM,
    //60
    eDODONGO, eMANHAN, eGLEEOK1, eGLEEOK2, eGLEEOK3,
    eGLEEOK4, eDIG1, eDIG3, eDIGPUP1, eDIGPUP2,
    //70
    eDIGPUP3, eDIGPUP4, eGOHMA1, eGOHMA2, eCENT1,
    eCENT2, ePATRA1, ePATRA2, eGANON, eSTALFOS2,
    //80
    eROPE2, eBUBBLESP, eBUBBLESR, eSHOOTFBALL, eITEMFAIRY,
    eFIRE, eOCTO5, eDKNUT5, eGELTRIB, eZOLTRIB,
    //90
    eKEESETRIB, eVIRETRIB, eDKNUT3, eLAQUAM, eMANHAN2,
    eTRAP_H, eTRAP_V, eTRAP_LR, eTRAP_UD, eFWIZ,
    //100
    eWWIZ, eCEILINGM, eFLOORM, ePATRABS, ePATRAL2,
    ePATRAL3, eBAT, eBATROBE, eBATROBEKING, eGLEEOK1F,
    //110
    eGLEEOK2F, eGLEEOK3F, eGLEEOK4F, eMWIZ, eDODONGOBS,
    eDODONGOF, eTRIGGER, eBUBBLEIT, eBUBBLEIP, eBUBBLEIR,
    //120
    eSTALFOS3, eGOHMA3, eGOHMA4, eNPCSTAND1, eNPCSTAND2,
    eNPCSTAND3, eNPCSTAND4, eNPCSTAND5, eNPCSTAND6, eNPCWALK1,
    //130
    eNPCWALK2, eNPCWALK3, eNPCWALK4, eNPCWALK5, eNPCWALK6,
    eBOULDER, eGORIYA3, eLEV3, eOCTO3S, eOCTO3F,
    //140
    eOCTO4S, eOCTO4F, eTRAP_8WAY, eTRAP_DIAGONAL, eTRAP_SLASH_C,
    eTRAP_SLASH_LOS, eTRAP_BACKSLASH_C, eTRAP_BACKSLASH_LOS, eTRAP_CW_C, eTRAP_CW_LOS,
    //150
    eTRAP_CCW_C, eTRAP_CCW_LOS, eSUMMONER, eIWIZ, eSHOOTMAGIC,
    eSHOOTROCK, eSHOOTSPEAR, eSHOOTSWORD, eSHOOTFLAME, eSHOOTFLAME2,
    //160
    eBOMBCHU, eFGEL, eFZOL, eFGELTRIB, eFZOLTRIB,
    eTEK3, eSPINTILE1, eSPINTILE2, eLYNEL3, eFPEAHAT,
    //170
    eMPOLSV, eWPOLSV, eDKNUT4, eFGHINI, eMGHINI,
    eGRAPBUGHP, eGRAPBUGMP, e177,
    
    eMAXGUYS = MAXGUYS
};

enum enemyFamily
{
    eeGUY=0, eeWALK,
    eeSHOOT/*DEPRECATED*/,
    eeTEK, eeLEV, eePEAHAT, eeZORA, eeROCK,
    eeGHINI, eeARMOS/*DEPRECATED*/, eeKEESE, eeGEL/*DEPRECATED*/, eeZOL/*DEPRECATED*/, eeROPE/*DEPRECATED*/, eeGORIYA/*DEPRECATED*/, eeTRAP,
    eeWALLM, eeBUBBLE/*DEPRECATED*/, eeVIRE/*DEPRECATED*/, eeLIKE/*DEPRECATED*/, eePOLSV/*DEPRECATED*/, eeWIZZ, eeAQUA, eeMOLD,
    eeDONGO, eeMANHAN, eeGLEEOK, eeDIG, eeGHOMA, eeLANM, eePATRA, eeGANON,
    eePROJECTILE, eeGELTRIB/*DEPRECATED*/, eeZOLTRIB/*DEPRECATED*/, eeVIRETRIB/*DEPRECATED*/, eeKEESETRIB/*DEPRECATED*/, eeSPINTILE, eeNONE,
    eeFAIRY, eeFIRE, eeOTHER,
    eeMAX
};

enum enemyMisc1
{
    e1tNORMAL, e1tEACHTILE, e1tCONSTANT, e1tHOMINGBRANG=2,
    e1tFAST, e1tSLANT, e1t3SHOTS, e1t4SHOTS,
    e1t5SHOTS, e1t3SHOTSFAST, e1tFIREOCTO, e1t8SHOTS,
    e1tSUMMON, e1tSUMMONLAYER, e1tLAST
};

enum enemyMisc2
{
    e2tNORMAL, e2tSPLITHIT, e2tKEESETRIB=1, e2tSPLIT,
    e2tFIREOCTO, e2tBOMBCHU, e2tTRIBBLE, e2tLAST
};

enum enemyMisc7
{
    e7tNORMAL, e7tTEMPJINX, e7tPERMJINX, e7tUNJINX, e7tTAKEMAGIC, e7tTAKERUPEES, e7tDRUNK,
// all from this point involve engulfing
    e7tEATITEMS, e7tEATMAGIC, e7tEATRUPEES, e7tEATHURT,
// all from this point involve dragging
    e7tWALLMASTER, e7tLAST
};

enum enemyMisc9
{
    e9tNORMAL, e9tROPE, e9tVIRE, e9tPOLSVOICE, e9tARMOS,
    // remainder unimplemented
    e9tLEEVER, e9tZ3LEEVER, e9tZ3WALK, e9tZ3STALFOS, e9tLAST
};

enum enemyAnim
{
    aNONE, aFLIP, aUNUSED1, a2FRM, aUNUSED2,
    aOCTO, aTEK, aLEV, aWALK, aZORA,
    aNEWZORA, aGHINI, aARMOS, aROPE, aWALLM,
    aNEWWALLM, aDWALK, aVIRE, a3FRM, aWIZZ,
    aAQUA, aDONGO, aMANHAN, aGLEEOK, aDIG,
    aGHOMA, aLANM, a2FRMPOS, a4FRM4EYE,a4FRM8EYE,
    a4FRM4DIRF, a4FRM4DIR, a4FRM8DIRF, aARMOS4, a4FRMPOS4DIR,
    a4FRMPOS8DIR, aUNUSED3, a4FRM8DIRB, aNEWTEK, a3FRM4DIR,
    a2FRM4DIR, aNEWLEV, a2FRM4EYE, aNEWWIZZ, aNEWDONGO,
    aDONGOBS, a4FRMPOS8DIRF, a4FRMPOS4DIRF, a4FRMNODIR, aGANON, a2FRMB, aMAX
};
// deprecated styles
const int aFLIPSLOW = 1;
const int a2FRMSLOW = 3;
const int aNEWDWALK = 42;
const int aNEWPOLV = 39;
const int a4FRM3TRAP = 36;

// Enemy flags
const uint guy_bhit        = 0x00000001;
const uint guy_invisible   = 0x00000002;
const uint guy_neverret    = 0x00000004;
const uint guy_doesntcount = 0x00000008;
const uint guy_fadeflicker = 0x00000010;
const uint guy_fadeinstant = 0x00000020;
const uint inv_front       = 0x01000000;
const uint inv_left        = 0x02000000;
const uint inv_right       = 0x04000000;
const uint inv_back        = 0x08000000;
const uint guy_bkshield    = 0x10000000; // Shield can't be broken
const uint lens_only       = 0x80000000;

// Enemy flags2
const uint guy_flashing    = 0x00000001;
const uint eneflag_zora    = 0x00000002;
const uint eneflag_rock    = 0x00000004;
const uint eneflag_trap    = 0x00000008;
const uint cmbflag_trph    = 0x00000010;
const uint cmbflag_trpv    = 0x00000020;
const uint cmbflag_trp4    = 0x00000040;
const uint cmbflag_trplr   = 0x00000080;
const uint cmbflag_trpud   = 0x00000100;
const uint eneflag_trp2    = 0x00000200;
const uint eneflag_fire    = 0x00000400;
const uint cmbflag_armos   = 0x00000800;
const uint cmbflag_ghini   = 0x00001000;
const uint eneflag_ganon   = 0x00002000;
const uint guy_blinking    = 0x00004000;
const uint guy_transparent = 0x00008000;

enum fadeEffects
{
    fade_none, fade_flicker, fade_invisible, fade_flash_die, fade_blue_poof
};

enum itemSets
{
    isNONE, isDEFAULT, isBOMBS, isMONEY, isLIFE, isBOMB100, isSBOMB100,
    isMAGIC, isMAGICBOMBS, isMAGICMONEY, isMAGICLIFE, isMAGIC2, isTALLGRASS, isMAX
};

enum items // value matters because of current quest file data
{
    iRupy, i5Rupies, iHeart, iBombs, iClock,
    iSword, iWSword, iMSword, iShield, iKey,
    // 10
    iBCandle, iRCandle, iLetter, iArrow, iSArrow,
    iBow, iBait, iBRing, iRRing, iBracelet,
    // 20
    iTriforce, iMap, iCompass, iBrang, iMBrang,
    iWand, iRaft,iLadder,iHeartC, iBPotion,
    // 30
    iRPotion, iWhistle,iBook, iMKey, iFairyMoving,
    iFBrang, iXSword, iMShield, i20Rupies, i50Rupies,
    // 40
    i200Rupies, iWallet500, iWallet999, iPile, iBigTri,
    iSelectA, iMisc1, iMisc2, iSBomb, iHCPiece,
    // 50
    iAmulet, iFlippers, iHookshot, iLens, iHammer,
    iBoots, iL2Bracelet, iGArrow, iMagicC, iSMagic,
    // 60
    iLMagic, iGRing, iKillAll, iL2Amulet, iDinsFire,
    iFaroresWind, iNayrusLove, iBossKey, iBow2, iFairyStill,
    // 70
    i1ArrowAmmo, i5ArrowAmmo, i10ArrowAmmo, i30ArrowAmmo, iQuiver,
    iQuiverL2, iQuiverL3, i1BombAmmo, i4BombAmmo, i8BombAmmo,
    // 80
    i30BombAmmo, iBombBag, iBombBagL2, iBombBagL3, iLevelKey,
    iSelectB, i10Rupies, i100Rupies, iCByrna, iLongshot,
    // 90
    iLetterUsed,iRocsFeather,iHoverBoots,iSShield,iSpinScroll,
    iCrossScroll,iQuakeScroll,iL2QuakeScroll,iL2SpinScroll,iWhispRing,
    // 100
    iL2WhispRing, iChargeRing, iL2ChargeRing, iPerilScroll, iWalletL3,
    iQuiverL4, iBombBagL4, iOldGlove, iL2Ladder,iWealthMedal,
    // 110
    iL2WealthMedal, iL3WealthMedal,iHeartRing, iL2HeartRing, iL3HeartRing,
    iMagicRing, iL2MagicRing, iL3MagicRing, iL4MagicRing, iStoneAgony,
    // 120
    iStompBoots, iPerilRing, iWhimsicalRing,
    iCustom1, iCustom2, iCustom3, iCustom4, iCustom5,
    iCustom6, iCustom7, iCustom8, iCustom9, iCustom10,
    iCustom11, iCustom12, iCustom13, iCustom14, iCustom15,
    iCustom16, iCustom17, iCustom18, iCustom19, iCustom20,
    iLast,
    iMax=256
};

// Item flags
const int ITEM_GAMEDATA    = 0x0001;  // Whether this item sets the corresponding gamedata value or not
const int ITEM_EDIBLE      = 0x0002;  // can be eaten by Like Like
const int ITEM_COMBINE     = 0x0004;  // blue potion + blue potion = red potion
const int ITEM_DOWNGRADE   = 0x0008;
const int ITEM_FLAG1       = 0x0010;
const int ITEM_FLAG2       = 0x0020;
const int ITEM_KEEPOLD     = 0x0040;
const int ITEM_RUPEE_MAGIC = 0x0080;
const int ITEM_UNUSED      = 0x0100;
const int ITEM_GAINOLD     = 0x0200;
const int ITEM_FLAG3       = 0x0400;
const int ITEM_FLAG4       = 0x0800;
const int ITEM_FLAG5       = 0x1000;

// Item pickup flags
const int ipBIGRANGE = 1;    // Collision rectangle is large
const int ipHOLDUP   = 2;    // Link holds up item when he gets it
const int ipONETIME  = 4;    // Getting this item sets mITEM
const int ipDUMMY    = 8;    // Dummy item.  Can't get this.
const int ipCHECK    = 16;   // Check restrictions (money in a shop, etc.)
const int ipMONEY    = 32;   // This item causes money loss (bomb upgrade, swindle room, etc.)
const int ipFADE     = 64;   // Blinks if fadeclk>0
const int ipENEMY    = 128;  // Enemy is carrying it around
const int ipTIMER    = 256;  // Disappears after a while
const int ipBIGTRI   = 512;  // Large collision rectangle (used for large triforce)
const int ipNODRAW   = 1024; // Don't draw this (for underwater items)
const int ipONETIME2 = 2048; // Getting this item sets mBELOW
const int ipSPECIAL  = 4096; // This is the item in a Special Item room


// lvlitems flags
const int liTRIFORCE = 1;
const int liMAP      = 2;
const int liCOMPASS  = 4;
const int liBOSS     = 8;
const int liBOSSKEY  = 16;


enum spritePalettes
{
    spAQUA, spGLEEOK, spDIG, spGANON, spBROWN, spPILE, spBLUE, spRED,
    spGOLD, spICON1, spICON2, spICON3, spICON4, spGLEEOKF, spFROZEN
};

enum secrets
{
    sBCANDLE, sARROW, sBOMB, sSTAIRS, sSECRET01, sSECRET02, sSECRET03,
    sSECRET04, sSECRET05, sSECRET06, sSECRET07, sSECRET08, sSECRET09,
    sSECRET10, sSECRET11, sSECRET12, sSECRET13, sSECRET14, sSECRET15,
    sSECRET16, sRCANDLE, sWANDFIRE, sDINSFIRE, sSARROW, sGARROW,
    sSBOMB, sBRANG, sMBRANG, sFBRANG, sWANDMAGIC, sREFMAGIC, sREFFIREBALL,
    sSWORD, sWSWORD, sMSWORD, sXSWORD, sSWORDBEAM, sWSWORDBEAM,
    sMSWORDBEAM, sXSWORDBEAM, sHOOKSHOT, sWAND, sHAMMER, sSTRIKE
};

enum decorations
{
    dBUSHLEAVES, dFLOWERCLIPPINGS, dGRASSCLIPPINGS, dHAMMERSMACK,
    dTALLGRASS, dRIPPLES, dNAYRUSLOVESHIELD, dHOVER, dMAXDECORATIONS
}

enum scriptTypes
{
 SCRIPT_GLOBAL, SCRIPT_FFC, SCRIPT_SCREEN, SCRIPT_LINK, SCRIPT_ITEM,
 SCRIPT_LWPN, SCRIPT_NPC, SCRIPT_SUBSCREEN, SCRIPT_EWPN
}

// Miscellaneous
const int DAMAGE_MULTIPLIER = 2;
const int csBOSS = 14;
const int TILES_PER_ROW = 20;


// Special enemy flags
const int asf_ALWAYSDIE    = 0x0001; // Invincibility doesn't prevent death by ringleader
const int asf_PATRADEFENSE = 0x0002; // Center eye is invincible

// Arguments to getGuyProp()
const int GUYPROP_FAMILY  = 0;
const int GUYPROP_HP      = 1;
const int GUYPROP_STEP    = 2;
const int GUYPROP_MISC2   = 3;
const int GUYPROP_MISC9   = 4;
const int GUYPROP_MISC10  = 5;
const int GUYPROP_FLAGS2  = 6;
const int GUYPROP_BOSSPAL = 7;

// Arguments to getItemProp()
enum itemProp
{
    ITEMPROP_FAMILY, ITEMPROP_LEVEL, ITEMPROP_POWER, ITEMPROP_MAGIC,
    ITEMPROP_TILE, ITEMPROP_CSETS, ITEMPROP_SOUND, ITEMPROP_FLAGS,
    ITEMPROP_MISC1, ITEMPROP_WPN2, ITEMPROP_PICKUP_SCRIPT
}

// Arguments to getWeaponProp()
enum weaponProp
{
    WPNPROP_TILE, WPNPROP_CSETS, WPNPROP_FRAMES
}

enum comboProp
{
    COMBOPROP_TYPE
}

// Arguments to getComboClassProp()
enum comboClassProp
{
    CCPROP_BLOCK_ENEMIES
}

// Link's actions
enum actiontype
{
    none, walking, attacking, freeze, landhold1, landhold2,
    rafting, gothit, inwind, scrolling, won, swimming, hopping,
    swimhit, waterhold1, waterhold2, casting, climbcovertop,
    climbcoverbottom, dying, drowning, climbing,
    // Fake actiontypes: used by ZScripts
    ischarging, isspinning, isdiving
}

int vbound(int num, int min, int max)
{
    if(num<min)
        return min;
    if(num>max)
        return max;
    return num;
}

float vbound(float num, float min, float max)
{
    if(num<min)
        return min;
    if(num>max)
        return max;
    return num;
}

fix vbound(fix num, fix min, fix max)
{
    if(num<min)
        return min;
    if(num>max)
        return max;
    return num;
}

int zc_min(int a, int b)
{
    return (a<b) ? a : b;
}

fix zc_min(fix a, fix b)
{
    return (a<b) ? a : b;
}

int zc_max(int a, int b)
{
    return (a>b) ? a : b;
}

fix zc_max(fix a, fix b)
{
    return (a>b) ? a : b;
}

fix distance(int x1, int y1, int x2, int y2)
{
    return fix(sqrt(pow(abs(x1-x2),2)+pow(abs(y1-y2),2)));
}

int wpnsfx(int wpn)
{
    switch(wpn)
    {
    case ewFireTrail:
    case ewFlame:
    case ewFlame2Trail:
    case ewFlame2:
        return WAV_FIRE;
        
    case ewWind:
    case ewMagic:
        return WAV_WAND;
        
    case ewIce:
        return WAV_ZN1ICE;
        
    case ewRock:
        if(questRuleEnabled(qr_MORESOUNDS)) return WAV_ZN1ROCK;
        
    case ewFireball2:
    case ewFireball:
        if(questRuleEnabled(qr_MORESOUNDS)) return WAV_ZN1FIREBALL;
    }
    
    return -1;
}

int addenemy(int x, int y, int id, int clk)
{
    return addenemy(x, y, 0, id, clk);
}

void never_return(int index)
{
    if(!questRuleEnabled(qr_KILLALL))
    {
        setmapflag(mNEVERRET);
        return;
    }
        
    for(int i=0; i<numGuys(); i++)
    {
        enemy@ en=getGuy(i);
        if((en.flags&guy_neverret)!=0 && i!=index)
        {
            return;
        }
    }
    
    setmapflag(mNEVERRET);
}

enemy@ getGuy(int index)
{
    return cast<enemy>(__getGuy(index));
}

item@ loadItem(int index)
{
    return cast<item>(__getItem(index));
}

weapon@ getEwpn(int index)
{
    return cast<weapon>(__getEwpn(index));
}

int numEwpnsWithID(int id)
{
    int count=0;
    for(int i=numEwpns()-1; i>=0; i--)
    {
        if(getEwpn(i).id==id)
            count++;
    }
    return count;
}

weapon@ getLwpn(int index)
{
    return cast<weapon>(__getLwpn(index));
}

int idOfFirstLwpnOfType(int type)
{
    for(int i=0; i<numLwpns(); i++)
    {
        weapon@ w=cast<weapon>(__getLwpn(i));
        if(w.type==type)
            return i;
    }
    
    return -1;
}

void additem(int x, int y, int id, int pickup, int clk=0)
{
    if(questRuleEnabled(qr_NOITEMOFFSET))
        y--;
    placeitem(x, y, id, pickup, clk);
}


bool tooclose(int x,int y,int d)
{
    return (abs(int(LinkX())-x)<d && abs(int(LinkY())-y)<d);
}

// Get an extra sprite CSet
int pSprite(int x)
{
    return x+6671; // x+newerpoSPRITE
}

bool isGrassType(int type)
{
    switch(type)
    {
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cTALLGRASSTOUCHY:
        return true;
    }
    
    return false;
}

bool isFlowersType(int type)
{
    switch(type)
    {
    case cFLOWERS:
    case cFLOWERSTOUCHY:
        return true;
    }
    
    return false;
}

bool isBushType(int type)
{
    switch(type)
    {
    case cBUSH:
    case cBUSHNEXT:
    case cBUSHTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isSlashType(int type)
{
    switch(type)
    {
    case cSLASH:
    case cSLASHITEM:
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableNextType(int type)
{
    switch(type)
    {
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cTALLGRASSNEXT:
    case cBUSHNEXT:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isTouchyType(int type)
{
    switch(type)
    {
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableType(int type)
{
    switch(type)
    {
    case cSLASH:
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cSLASHNEXT:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:
    
    case cSLASHTOUCHY:
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

bool isCuttableItemType(int type)
{
    switch(type)
    {
    case cSLASHITEM:
    case cBUSH:
    case cFLOWERS:
    case cTALLGRASS:
    case cTALLGRASSNEXT:
    case cSLASHNEXTITEM:
    case cBUSHNEXT:
    
    case cSLASHITEMTOUCHY:
    case cBUSHTOUCHY:
    case cFLOWERSTOUCHY:
    case cTALLGRASSTOUCHY:
    case cSLASHNEXTITEMTOUCHY:
    case cBUSHNEXTTOUCHY:
        return true;
    }
    
    return false;
}

int get_magicdrainrate()
{
    return get_generic(1);
}

bool get_canslash()
{
    return get_generic(2)!=0;
}

