//2.54 -Z

//Global

//Game

    //These may be Link-> at present, but they could equally be Game->
    case GAMESETA: return "GAMESETA";
    case GAMESETB: return "GAMESETB";
    
    //Button press/Input arrays[]
    case BUTTONPRESS: return "BUTTONPRESS";
    case BUTTONINPUT: return "BUTTONINPUT";
    case BUTTONHELD: return "BUTTONHELD";
    
    //Keyboard handling 
    case KEYPRESS: return "KEYPRESS";
    case READKEY: return "READKEY";
    case JOYPADPRESS: return "JOYPADPRESS"; //Check if a button press was by joypad, not keyboard. 
					//useful for keyboard handling. 
	//! We need a way to prevent keypresses from being registered as 'Link buttons' so that
	//! workarounds in scripts are easier. This is mostly for start, and map, but also
	//! for when the cheat menu is active. 
	//! Because Saffith's fix, I believe, only affects controllers?
    
    
    //I want to add ways to read any scrween attribute from anywhere in the game. 
    case SETSCREENDOOR: return "SETSCREENDOOR";
    case SETSCREENENEMY: return "SETSCREENENEMY";

    case GAMEMAXMAPS: return "GAMEMAXMAPS";

    case ZELDAVERSION: return "ZELDAVERSION";
    
    case SCREENFLAG: return "SCREENFLAG";
    case DMAPLEVELPAL: return "DMAPLEVELPAL";
    //Need to add direct palettew handling, to change csets by array, or individual index. 
    
    //Prevent the active subscreen from falling. 
    case NOACTIVESUBSC: return "NOACTIVESUBSC";
    
    case ADJUSTSFX: return "ADJUSTSFX";
    
    case DISABLEDITEM: return "DISABLEDITEM"; //items disabled on a dmap, or by the user. 
    
    //I would like to retain these, for use by quest developers working with beta releases. 
    //They can go under Debug-> if you want. I want to add a Debug-> pointer for some things
    //that should be used by people working with betas and ZC Source dev. 
    case ZELDABUILD: return "ZELDABUILD";
    case ZELDABETA: return "ZELDABETA";
    
    //I will change this at a function level, to prevent uncapping, instead of returning it,
    //This differs int hat you do not need to return an engine state, but rather,
    //prevent a user input that can affect it. The engine state itself is free to do what it wishes, otherwise. 
    case GAMETHROTTLE: return "GAMETHROTTLE";

//Link

    //Warp
    case LINKRETSQUARE: return "LINKRETSQUARE";
    case LINKWARPSOUND: return "LINKWARPSOUND";
    case SIDEWARPSFX: return "SIDEWARPSFX";
    case PITWARPSFX: return "PITWARPSFX"; 
    case SIDEWARPVISUAL: return "SIDEWARPVISUAL";
    case PITWARPVISUAL: return "PITWARPVISUAL"; 
    case PLAYPITWARPSFX: return "PLAYPITWARPSFX";
    case WARPEFFECT: return "WARPEFFECT"; 
    
    //Link Items, vars, states.
    case SETITEMSLOT: return "SETITEMSLOT";
    case LINKEATEN: return "LINKEATEN";
    case LINKITEMB: return "LINKITEMB";
    case LINKITEMA: return "LINKITEMA";
    case LINKEXTEND: return "LINKEXTEND";
    case LINKSWIMSPD: return "LINKSWIMSPD";
    case LINKWALKANMSPD: return "LINKWALKANMSPD";
    case LINKANIMTYPE: return "LINKANIMTYPE";
    case LINKINVFRAME: return "LINKINVFRAME"; //This is extremely useful. 
    case LINKCANFLICKER: return "LINKCANFLICKER"; //As is this. 
    case LINKHURTSFX: return "LINKHURTSFX"; //and this. Setting this overrides the normal sounbds, and works
						//though a clean var used only by ZScript. 
    case SETLINKEXTEND: return "SETLINKEXTEND";
    case SETLINKTILE: return "SETLINKTILE";
        
    case LINKUSINGITEM: return "LINKUSINGITEM"; //Returns the item used this frame. also very useful.

    //Planned to deprecate.
    case LINKUSINGITEMA: return "LINKUSINGITEMA";
    case LINKUSINGITEMB: return "LINKUSINGITEMB";
    
    //Deprecated, preserved until we're done.
    
    case LINKWALKTILE: return "LINKWALKTILE";
    case LINKFLOATTILE: return "LINKFLOATTILE";
    case LINKSWIMTILE: return "LINKSWIMTILE";
    case LINKDIVETILE: return "LINKDIVETILE";
    case LINKSLASHTILE: return "LINKSLASHTILE";
    case LINKJUMPTILE: return "LINKJUMPTILE";
    case LINKCHARGETILE: return "LINKCHARGETILE";
    case LINKSTABTILE: return "LINKSTABTILE";
    case LINKCASTTILE: return "LINKCASTTILE";
    case LINKHOLD1LTILE: return "LINKHOLD1LTILE";
    case LINKHOLD2LTILE: return "LINKHOLD2LTILE";
    case LINKHOLD1WTILE: return "LINKHOLD1WTILE";
    case LINKHOLD2WTILE: return "LINKHOLD2WTILE";
    case LINKPOUNDTILE: return "LINKPOUNDTILE";
    case PLAYWARPSOUND: return "PLAYWARPSOUND";
    case LINKINVINC: return "LINKINVINC";
    case LINKBIGHITBOX: return "LINKBIGHITBOX";
    case LINKDIAG: return "LINKDIAG";
    

//itemclass / itemdata

    case IDATAFRAME: return "IDATAFRAME";
    case ITEMCLASSID: return "IDATAID";
    case IDATALTM: return "IDATALTM";
    case IDATAPSCRIPT: return "IDATAPSCRIPT"
    case IDATASCRIPT: return "IDATASCRIPT";
    case IDATAMAGCOST: return "IDATAMAGCOST";
    case IDATAMINHEARTS: return "IDATAMINHEARTS";
    case IDATATILE: return "IDATATILE";
    case IDATAMISC: return "IDATAMISC";    
    case IDATACSET: return "IDATACSET";
    case IDATAFRAMES: return "IDATAFRAMES";
    case IDATAASPEED: return "IDATAASPEED";
    case IDATADELAY: return "IDATADELAY"; 
    case IDATACOMBINE: return "IDATACOMBINE";
    case IDATADOWNGRADE: return "IDATADOWNGRADE";
    case IDATAKEEPOLD: return "IDATAKEEPOLD";
    case IDATARUPEECOST: return "IDATARUPEECOST";
    case IDATAEDIBLE: return "IDATAEDIBLE";
    case IDATAFLAGUNUSED: return "IDATAFLAGUNUSED";
    case IDATAGAINLOWER: return "IDATAGAINLOWER";
    
    //idata arrays
    case IDATAATTRIB: return "IDATAATTRIB";
    case IDATAFLAGS: return "IDATAFLAGS";
    case IDATASPRITE: return "IDATASPRITE";

    //Deprecated by IDATAFLAGS
    case IDATAFLAG1: return "IDATAFLAG1";
    case IDATAFLAG3: return "IDATAFLAG3";
    case IDATAFLAG4: return "IDATAFLAG4";
    case IDATAFLAG5: return "IDATAFLAG5";
    case IDATAFLAG2: return "IDATAFLAG2";

    //Deprecated by IDATAATTRIB
    case IDATAMISC1: return "IDATAMISC1";
    case IDATAMISC2: return "IDATAMISC2";
    case IDATAMISC3: return "IDATAMISC3";
    case IDATAMISC4: return "IDATAMISC4";
    case IDATAMISC5: return "IDATAMISC5";
    case IDATAMISC6: return "IDATAMISC6";
    case IDATAMISC7: return "IDATAMISC7";
    case IDATAMISC8: return "IDATAMISC8";
    case IDATAMISC9: return "IDATAMISC9";
    case IDATAMISC10: return "IDATAMISC10";

    //Deprecated by IDATASPRITE
    case IDATAWPN: return "IDATAWPN";
    case IDATAWPN1: return "IDATAWPN1";
    case IDATAWPN2: return "IDATAWPN2";
    case IDATAWPN3: return "IDATAWPN3";
    case IDATAWPN4: return "IDATAWPN4";
    case IDATAWPN5: return "IDATAWPN5";
    case IDATAWPN6: return "IDATAWPN6";
    case IDATAWPN7: return "IDATAWPN7";
    case IDATAWPN8: return "IDATAWPN8";
    case IDATAWPN9: return "IDATAWPN9";
    case IDATAWPN10: return "IDATAWPN10";
     
//Item
    case ITEMACLK: return "ITEMACLK";

//Weapon
    /* 	Screen->CreatLWeaponDX(int type, int item)
	Makes a lweapon forwarding attributes from a specified item. Extremely useful tool, that
	also prevents some problems with setting sprites, and sounds for some weapon types.
	I would like to add an EWeapon version too, despite eweapons not using items, as it could be fun. */
    case CREATELWPNDX: return "CREATELWPNDX";
    
    /* 	The Range/Diuration/Length attribute for boomerange, arrows, and hookshots, respectively. 
	Used only by ZScript, and can override editor settings *if* the user desires. 
    I modified weapon handling to ensure that the proper value has precidence. */
    case LWPNRANGE: return "LWPNRANGE";

//NPC
    case NPCINVINC: return "NPCINVINC";
    case NPCSUPERMAN: return "NPCSUPERMAN";
    case NPCHASITEM: return "NPCHASITEM";
    case NPCRINGLEAD: return "NPCRINGLEAD";

//Pointers, arrays, UIDs
    case REFFFC: return "REFFFC";
    case REFITEM: return "REFITEM";
    case ITEMINDEX: return "ITEMINDEX";
    case LWPNINDEX: return "LWPNINDEX";
    case EWPNINDEX: return "EWPNINDEX";
    case NPCINDEX: return "NPCINDEX";
    case ITEMPTR: return "ITEMPTR";
    case NPCPTR: return "NPCPTR";
    case LWPNPTR: return "LWPNPTR";
    case EWPNPTR: return "EWPNPTR";



//New 2.54

//Global

string OOverlayTileRegister::toString()
{
    return "OVERLAYTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

//Array size for non-float arrays. 

string OArraySizeF::toString()
{
    return "ARRAYSIZEF " + getArgument()->toString();
}
string OArraySizeN::toString()
{
    return "ARRAYSIZEN " + getArgument()->toString();
}
string OArraySizeE::toString()
{
    return "ARRAYSIZEE " + getArgument()->toString();
}
string OArraySizeL::toString()
{
    return "ARRAYSIZEL " + getArgument()->toString();
}
string OArraySizeB::toString()
{
    return "ARRAYSIZEB " + getArgument()->toString();
}
string OArraySizeI::toString()
{
    return "ARRAYSIZEI " + getArgument()->toString();
}
string OArraySizeID::toString()
{
    return "ARRAYSIZEID " + getArgument()->toString();
}

//Game

string OEndSoundRegister::toString()
{
    return "ENDSOUNDR " + getArgument()->toString();
}
string OGetScreenEnemy::toString()
{
    return "GETSCREENENEMY " + getArgument()->toString();
}


string OSetDMapName::toString()
{
    return "SETDMAPNAME " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetDMapTitle::toString()
{
    return "SETDMAPTITLE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetDMapIntro::toString()
{
    return "SETDMAPINTRO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSetMessage::toString()
{
    return "SETMESSAGE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OContinueSFX::toString()
{
    return "CONTINUESFX " + getArgument()->toString();
}

string OPauseSoundRegister::toString()
{
    return "PAUSESOUNDR " + getArgument()->toString();
}

string OPauseMusic::toString()
{
    return "PAUSEMUSIC";
}
string OResumeMusic::toString()
{
    return "RESUMEMUSIC";
}

string OResumeSoundRegister::toString()
{
    return "RESUMESOUNDR " + getArgument()->toString();
}

string OGetScreenDoor::toString()
{
    return "GETSCREENDOOR " + getArgument()->toString();
}

string OGetBoolPointer::toString()
{
    return "BOOLARRPTR " + getArgument()->toString();
}

string OSetBoolPointer::toString()
{
    return "BOOLARRPTR2 " + getArgument()->toString();
}

//Screen

//Drawing
string ODrawBitmapExRegister::toString()
{
    return "BITMAPEXR";
}

//Deprecating, but here for now

string OBitmapQuadRegister::toString()
{
    return "QUAD2R";
}

//Link

//Weapon


string OGetLWeaponPointer::toString()
{
    return "LWPNARRPTR " + getArgument()->toString();
}

string OSetLWeaponPointer::toString()
{
    return "LWPNARRPTR2 " + getArgument()->toString();
}

string OGetEWeaponPointer::toString()
{
    return "EWPNARRPTR " + getArgument()->toString();
}

string OSetEWeaponPointer::toString()
{
    return "EWPNARRPTR2 " + getArgument()->toString();
}

//FFC


string OGetFFCPointer::toString()
{
    return "FFCARRPTR " + getArgument()->toString();
}

string OSetFFCPointer::toString()
{
    return "FFCARRPTR2 " + getArgument()->toString();
}

//NPC


string OGetNPCPointer::toString()
{
    return "NPCARRPTR " + getArgument()->toString();
}

string OSetNPCPointer::toString()
{
    return "NPCARRPTR2 " + getArgument()->toString();
}


//item
string OGetItemPointer::toString()
{
    return "ITEMARRPTR " + getArgument()->toString();
}

string OSetItemPointer::toString()
{
    return "ITEMARRPTR2 " + getArgument()->toString();
}


//itemclass

string OGetItemDataPointer::toString()
{
    return "IDATAARRPTR " + getArgument()->toString();
}

string OSetItemDataPointer::toString()
{
    return "IDATAARRPTR2 " + getArgument()->toString();
}

//Visual Effects

string OGreyscaleOn::toString()
{
    return "GREYSCALEON";
}

string OGreyscaleOff::toString()
{
    return "GREYSCALEOFF";
}

string OZapIn::toString()
{
    return "ZAPIN";
}

string OZapOut::toString()
{
    return "ZAPOUT";
}

//These need to be unary opcodes that accept bool linkvisible. 
string OWavyIn::toString()
{
    return "WAVYIN";
}

string OWavyOut::toString()
{
    return "WAVYOUT";
}

string OOpenWipe::toString()
{
    return "OPENWIPE";
}



