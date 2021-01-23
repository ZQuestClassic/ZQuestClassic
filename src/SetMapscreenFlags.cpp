//Helper functions to set or get mapdata->Flags[]

enum mapscreenflags
{
	// Room Types
	MSF_INTERIOR, MSF_DUNGEON, MSF_SIDEVIEW,
	
	// View
	MSF_INVISLINK, MSF_NOLINKMARKER, MSF_NOSUBSCREEN, MSF_NOOFFSET, MSF_LAYER3BG,
	MSF_LAYER2BG, MSF_DARKROOM,
	
	// Secrets
	MSF_BLOCKSHUT, MSF_TEMPSECRETS, MSF_TRIGPERM, MSF_ALLTRIGFLAGS,
	
	// Warp
	MSF_AUTODIRECT, MSF_SENDSIRECT, MSF_MAZEPATHS, MSF_MAZEOVERRIDE, MSF_SPRITECARRY,
	MSF_DIRECTTIMEDWARPS, MSF_SECRETSISABLETIMEWRP, MSF_RANDOMTIMEDWARP,
	
	// Item
	MSF_HOLDUP, MSF_FALLS,
	
	// Combo
	MSF_MIDAIR, MSF_CYCLEINIT, MSF_IGNOREBOOTS, MSF_TOGGLERINGS,
	
	// Save
	MSF_SAVECONTHERE, MSF_SAVEONENTRY, MSF_CONTHERE, MSF_NOCONTINUEWARP,
	
	// FFC
	MSF_WRAPFFC, MSF_NOCARRYOVERFFC, 
	
	// Whistle 
	MSF_STAIRS, MSF_PALCHANGE, MSF_DRYLAKE,
	
	// Enemies
	MSF_INVISIBLEENEMIES, MSF_TRAPS_IGNORE_SOLID, MSF_EMELIESALWAYSRETURN, MSF_ENEMIES_ITEM, MSF_ENEMEIS_SECRET,
	MSF_ENEMIES_SECRET_PERM,
	
	// Misc
	MSF_ALLOW_LADDER, MSF_NO_DIVING, MSF_SFXONENTRY, MSF_LENSEFFECT,
		 
	//Custom / Script 
	MSF_SCRIPT1,
	MSF_CUSTOM1 = MSF_SCRIPT1,
	MSF_SCRIPT2,
	MSF_CUSTOM2 = MSF_SCRIPT2,
	MSF_SCRIPT3,
	MSF_CUSTOM3 = MSF_SCRIPT3,
	MSF_SCRIPT4,
	MSF_CUSTOM4 = MSF_SCRIPT4,
	MSF_SCRIPT5,
	MSF_CUSTOM5 = MSF_SCRIPT5,
	MSF_LAST
};
		 

int GetMapscreenFlag(mapdata m, mapscreenflags flagid)
{
	switch(flagid)
	{
		//0, flags
		case MSF_BLOCKSHUT:
			return (m->Flags[0]&1);
		case MSF_ENEMIES_ITEM:
			return (m->Flags[0]&2);  
		case MSF_DARKROOM:
			return (m->Flags[0]&4);
		//8 IS A DUMMY PROC
		case MSF_STAIRS:
			return (m->Flags[0]&16);
		case MSF_ALLOW_LADDER:
			return (m->Flags[0]&32);
		case MSF_MAZEPATHS:
			return (m->Flags[0]&64);
		case MSF_SFXONENTRY:
			return (m->Flags[0]&128);
		
		//1, flags2
		//what, no 1?
		case MSF_MIDAIR:
			return ((m->Flags[1]>>4)&2);
		case MSF_TRAPS_IGNORE_SOLID:
			return ((m->Flags[1]>>4)&4);
		case MSF_ENEMEIS_SECRET:
			return ((m->Flags[1]>>4)&8);
		
		//2, flags3
		case MSF_HOLDUP:
			return (m->Flags[2]&1);
		case MSF_CYCLEINIT:
			return (m->Flags[2]&2);
		case MSF_INVISIBLEENEMIES:
			return (m->Flags[2]&4);
		case MSF_INVISLINK:
			return (m->Flags[2]&8);
		case MSF_NOSUBSCREEN:
			return (m->Flags[2]&16);
		case MSF_SPRITECARRY:
			return (m->Flags[2]&32);
		case MSF_NOOFFSET:
			return (m->Flags[2]&64);
		case MSF_EMELIESALWAYSRETURN:
			return (m->Flags[2]&128);
		
		//3, flags4
		//1 AND 2 ARE APPARENTLY UNUSED?
		case MSF_DIRECTTIMEDWARPS:
			return (m->Flags[3]&4);
		case MSF_SECRETSISABLETIMEWRP:
			return (m->Flags[3]&8);
		case MSF_ENEMIES_SECRET_PERM:
			return (m->Flags[3]&16);
		//32 IS NOT USED
		case MSF_SAVECONTHERE:
			return (m->Flags[3]&64);
		case MSF_SAVEONENTRY:
			return (m->Flags[3]&128);
		
		//4, flags5
		//no 1?
		case MSF_RANDOMTIMEDWARP:
			return (m->Flags[4]&1);
		case MSF_IGNOREBOOTS:
			return (m->Flags[4]&2);
		case MSF_AUTODIRECT:
			return (m->Flags[4]&4);
		case MSF_SENDSIRECT:
			return (m->Flags[4]&8);
		case MSF_TEMPSECRETS:
			return (m->Flags[4]&16);
		//no 32?
		case MSF_NO_DIVING:
			return (m->Flags[4]&64);
		case MSF_NOCARRYOVERFFC:
			return (m->Flags[4]&128);
		
		//5, flags6
		case MSF_INTERIOR:
			return (m->Flags[5]&1);
		case MSF_DUNGEON:
			return (m->Flags[5]&2);
		case MSF_TRIGPERM:
			return (m->Flags[5]&4);
		case MSF_CONTHERE:
			return (m->Flags[5]&8);
		case MSF_NOCONTINUEWARP:
			return (m->Flags[5]&16);
		case MSF_ALLTRIGFLAGS:
			return (m->Flags[5]&32);
		case MSF_TOGGLERINGS:
			return (m->Flags[5]&64);
		case MSF_WRAPFFC:
			return (m->Flags[5]&128);
		
		//6, flags7
		case MSF_LAYER3BG:
			return (m->Flags[6]&1);
		case MSF_LAYER2BG:
			return (m->Flags[6]&2);
		case MSF_FALLS:
			return (m->Flags[6]&4);
		case MSF_SIDEVIEW:
			return (m->Flags[6]&8);
		case MSF_NOLINKMARKER:
			return (m->Flags[6]&16);
		case MSF_PALCHANGE:
			return (m->Flags[6]&64);
		case MSF_DRYLAKE:
			return (m->Flags[6]&128);
		
		//7, flags8
		case MSF_SCRIPT1:
			return (m->Flags[7]&1);
		case MSF_SCRIPT2:
			return (m->Flags[7]&2);
		case MSF_SCRIPT3:
			return (m->Flags[7]&4);
		case MSF_SCRIPT4:
			return (m->Flags[7]&8);
		case MSF_SCRIPT5:
			return (m->Flags[7]&16);
		case MSF_LENSEFFECT:
			return (m->Flags[7]&32);
		case MSF_MAZEOVERRIDE:
			return (m->Flags[7]&64);
		
		default: 
		{
			printf("Invalid flag id (%d) passed to GetMapscreenFlag()\n", flagid); return 0;
		}
	}
}


int GetMapscreenFlag(int mapid, int scr, mapscreenflags flagid)
{
	mapdata m = Game->LoadMapData(mapid, scr);
	switch(flagid)
	{
		//0, flags
		case MSF_BLOCKSHUT:
			return (m->Flags[0]&1);
		case MSF_ENEMIES_ITEM:
			return (m->Flags[0]&2);  
		case MSF_DARKROOM:
			return (m->Flags[0]&4);
		//8 IS A DUMMY PROC
		case MSF_STAIRS:
			return (m->Flags[0]&16);
		case MSF_ALLOW_LADDER:
			return (m->Flags[0]&32);
		case MSF_MAZEPATHS:
			return (m->Flags[0]&64);
		case MSF_SFXONENTRY:
			return (m->Flags[0]&128);
		
		//1, flags2
		//what, no 1?
		case MSF_MIDAIR:
			return ((m->Flags[1]>>4)&2);
		case MSF_TRAPS_IGNORE_SOLID:
			return ((m->Flags[1]>>4)&4);
		case MSF_ENEMEIS_SECRET:
			return ((m->Flags[1]>>4)&8);
		
		//2, flags3
		case MSF_HOLDUP:
			return (m->Flags[2]&1);
		case MSF_CYCLEINIT:
			return (m->Flags[2]&2);
		case MSF_INVISIBLEENEMIES:
			return (m->Flags[2]&4);
		case MSF_INVISLINK:
			return (m->Flags[2]&8);
		case MSF_NOSUBSCREEN:
			return (m->Flags[2]&16);
		case MSF_SPRITECARRY:
			return (m->Flags[2]&32);
		case MSF_NOOFFSET:
			return (m->Flags[2]&64);
		case MSF_EMELIESALWAYSRETURN:
			return (m->Flags[2]&128);
		
		//3, flags4
		//1 AND 2 ARE APPARENTLY UNUSED?
		case MSF_DIRECTTIMEDWARPS:
			return (m->Flags[3]&4);
		case MSF_SECRETSISABLETIMEWRP:
			return (m->Flags[3]&8);
		case MSF_ENEMIES_SECRET_PERM:
			return (m->Flags[3]&16);
		//32 IS NOT USED
		case MSF_SAVECONTHERE:
			return (m->Flags[3]&64);
		case MSF_SAVEONENTRY:
			return (m->Flags[3]&128);
		
		//4, flags5
		//no 1?
		case MSF_RANDOMTIMEDWARP:
			return (m->Flags[4]&1);
		case MSF_IGNOREBOOTS:
			return (m->Flags[4]&2);
		case MSF_AUTODIRECT:
			return (m->Flags[4]&4);
		case MSF_SENDSIRECT:
			return (m->Flags[4]&8);
		case MSF_TEMPSECRETS:
			return (m->Flags[4]&16);
		//no 32?
		case MSF_NO_DIVING:
			return (m->Flags[4]&64);
		case MSF_NOCARRYOVERFFC:
			return (m->Flags[4]&128);
		
		//5, flags6
		case MSF_INTERIOR:
			return (m->Flags[5]&1);
		case MSF_DUNGEON:
			return (m->Flags[5]&2);
		case MSF_TRIGPERM:
			return (m->Flags[5]&4);
		case MSF_CONTHERE:
			return (m->Flags[5]&8);
		case MSF_NOCONTINUEWARP:
			return (m->Flags[5]&16);
		case MSF_ALLTRIGFLAGS:
			return (m->Flags[5]&32);
		case MSF_TOGGLERINGS:
			return (m->Flags[5]&64);
		case MSF_WRAPFFC:
			return (m->Flags[5]&128);
		
		//6, flags7
		case MSF_LAYER3BG:
			return (m->Flags[6]&1);
		case MSF_LAYER2BG:
			return (m->Flags[6]&2);
		case MSF_FALLS:
			return (m->Flags[6]&4);
		case MSF_SIDEVIEW:
			return (m->Flags[6]&8);
		case MSF_NOLINKMARKER:
			return (m->Flags[6]&16);
		case MSF_PALCHANGE:
			return (m->Flags[6]&64);
		case MSF_DRYLAKE:
			return (m->Flags[6]&128);
		
		//7, flags8
		case MSF_SCRIPT1:
			return (m->Flags[7]&1);
		case MSF_SCRIPT2:
			return (m->Flags[7]&2);
		case MSF_SCRIPT3:
			return (m->Flags[7]&4);
		case MSF_SCRIPT4:
			return (m->Flags[7]&8);
		case MSF_SCRIPT5:
			return (m->Flags[7]&16);
		case MSF_LENSEFFECT:
			return (m->Flags[7]&32);
		case MSF_MAZEOVERRIDE:
			return (m->Flags[7]&64);
		
		default: 
		{
			printf("Invalid flag id (%d) passed to GetMapscreenFlag()\n", flagid); return 0;
		}
	}
}

void SetMapscreenFlag(mapdata m, mapscreenflags flagid, bool state)
{
	switch(flagid)
	{
		//0, flags
		case MSF_BLOCKSHUT:
			m->Flags[0] |= (state) 1 : ~1; break;
		case MSF_ENEMIES_ITEM:
			m->Flags[0] |= (state) 2 : ~2; break;
		case MSF_DARKROOM:
			m->Flags[0] |= (state) 4 : ~4; break;
		//8 IS A DUMMY PROC
		case MSF_STAIRS:
			m->Flags[0] |= (state) 15 : ~16; break;
		case MSF_ALLOW_LADDER:
			m->Flags[0] |= (state) 32 : ~32; break;
		case MSF_MAZEPATHS:
			m->Flags[0] |= (state) 64 : ~64; break;
		case MSF_SFXONENTRY:
			m->Flags[0] |= (state) 128 : ~128; break;
		
		//1, flags2
		//what, no 1?
		case MSF_MIDAIR:
			m->Flags[1] |= (state) 2>>4 : ~2>>4; break;  
		case MSF_TRAPS_IGNORE_SOLID:
			m->Flags[1] |= (state) 4>>4 : ~4>>4; break;  
		case MSF_ENEMEIS_SECRET:
			m->Flags[1] |= (state) 8>>4 : ~8>>4; break;  
		
		//2, flags3
		case MSF_HOLDUP:
			m->Flags[2] |= (state) 1 : ~1; break;
		case MSF_CYCLEINIT:
			m->Flags[2] |= (state) 2 : ~2; break;
		case MSF_INVISIBLEENEMIES:
			m->Flags[2] |= (state) 4 : ~4; break;
		case MSF_INVISLINK:
			m->Flags[2] |= (state) 8 : ~8; break;
		case MSF_NOSUBSCREEN:
			m->Flags[2] |= (state) 16 : ~16; break;
		case MSF_SPRITECARRY:
			m->Flags[2] |= (state) 32 : ~32; break;
		case MSF_NOOFFSET:
			m->Flags[2] |= (state) 64 : ~64; break;
		case MSF_EMELIESALWAYSRETURN:
			m->Flags[2] |= (state) 128 : ~128; break;
		
		//3, flags4
		//1 AND 2 ARE APPARENTLY UNUSED?
		case MSF_DIRECTTIMEDWARPS:
			m->Flags[3] |= (state) 4 : ~4; break;
		case MSF_SECRETSISABLETIMEWRP:
			m->Flags[3] |= (state) 8 : ~8; break;
		case MSF_ENEMIES_SECRET_PERM:
			m->Flags[3] |= (state) 16 : ~16; break;
		//32 IS NOT USED
		case MSF_SAVECONTHERE:
			m->Flags[3] |= (state) 64 : ~64; break;
		case MSF_SAVEONENTRY:
			m->Flags[3] |= (state) 128 : ~128; break;
		
		//4, flags5
		//no 1?
		case MSF_RANDOMTIMEDWARP:
			m->Flags[4] |= (state) 1 : ~1; break;
		case MSF_IGNOREBOOTS:
			m->Flags[4] |= (state) 2 : ~2; break;
		case MSF_AUTODIRECT:
			m->Flags[4] |= (state) 4 : ~4; break;
		case MSF_SENDSIRECT:
			m->Flags[4] |= (state) 8 : ~8; break;
		case MSF_TEMPSECRETS:
			m->Flags[4] |= (state) 16 : ~16; break;
		//no 32?
		case MSF_NO_DIVING:
			m->Flags[4] |= (state) 64 : ~64; break;
		case MSF_NOCARRYOVERFFC:
			m->Flags[4] |= (state) 128 : ~128; break;
		
		
		//5, flags6
		case MSF_INTERIOR:
			m->Flags[5] |= (state) 1 : ~1; break;
		case MSF_DUNGEON:
			m->Flags[5] |= (state) 2 : ~2; break;
		case MSF_TRIGPERM:
			m->Flags[5] |= (state) 4 : ~4; break;
		case MSF_CONTHERE:
			m->Flags[5] |= (state) 8 : ~8; break;
		case MSF_NOCONTINUEWARP:
			m->Flags[5] |= (state) 16 : ~16; break;
		case MSF_ALLTRIGFLAGS:
			m->Flags[5] |= (state) 32 : ~32; break;
		case MSF_TOGGLERINGS:
			m->Flags[5] |= (state) 64 : ~64; break;
		case MSF_WRAPFFC:
			m->Flags[5] |= (state) 128 : ~128; break;
		
		//6, flags7
		case MSF_LAYER3BG:
			m->Flags[6] |= (state) 1 : ~1; break;
		case MSF_LAYER2BG:
			m->Flags[6] |= (state) 2 : ~2; break;
		case MSF_FALLS:
			m->Flags[6] |= (state) 4 : ~4; break;
		case MSF_SIDEVIEW:
			m->Flags[6] |= (state) 8 : ~8; break;
		case MSF_NOLINKMARKER:
			m->Flags[6] |= (state) 16 : ~16; break;
		//32 IS UNUSED
		case MSF_PALCHANGE:
			m->Flags[6] |= (state) 64 : ~64; break;
		case MSF_DRYLAKE:
			m->Flags[6] |= (state) 128 : ~128; break;
		
		//7, flags8
		case MSF_SCRIPT1:
			m->Flags[7] |= (state) 1 : ~1; break;
		case MSF_SCRIPT2:
			m->Flags[7] |= (state) 2 : ~2; break;
		case MSF_SCRIPT3:
			m->Flags[7] |= (state) 4 : ~4; break;
		case MSF_SCRIPT4:
			m->Flags[7] |= (state) 8 : ~8; break;
		case MSF_SCRIPT5:
			m->Flags[7] |= (state) 16 : ~16; break;
		case MSF_LENSEFFECT:
			m->Flags[7] |= (state) 32 : ~32; break;
		case MSF_MAZEOVERRIDE:
			m->Flags[7] |= (state) 64 : ~64; break;
		
		
		default: 
		{
			printf("Invalid flag id (%d) passed to SetMapscreenFlag()\n", flagid);  break;
		}
	}
}

void SetMapscreenFlag(int mapid, int scr, mapscreenflags flagid, bool state)
{
	mapdata m = Game->LoadMapData(mapid, scr);
	switch(flagid)
	{
		//0, flags
		case MSF_BLOCKSHUT:
			m->Flags[0] |= (state) 1 : ~1; break;
		case MSF_ENEMIES_ITEM:
			m->Flags[0] |= (state) 2 : ~2; break;
		case MSF_DARKROOM:
			m->Flags[0] |= (state) 4 : ~4; break;
		//8 IS A DUMMY PROC
		case MSF_STAIRS:
			m->Flags[0] |= (state) 15 : ~16; break;
		case MSF_ALLOW_LADDER:
			m->Flags[0] |= (state) 32 : ~32; break;
		case MSF_MAZEPATHS:
			m->Flags[0] |= (state) 64 : ~64; break;
		case MSF_SFXONENTRY:
			m->Flags[0] |= (state) 128 : ~128; break;
		
		//1, flags2
		//what, no 1?
		case MSF_MIDAIR:
			m->Flags[1] |= (state) 2>>4 : ~2>>4; break;  
		case MSF_TRAPS_IGNORE_SOLID:
			m->Flags[1] |= (state) 4>>4 : ~4>>4; break;  
		case MSF_ENEMEIS_SECRET:
			m->Flags[1] |= (state) 8>>4 : ~8>>4; break;  
		
		//2, flags3
		case MSF_HOLDUP:
			m->Flags[2] |= (state) 1 : ~1; break;
		case MSF_CYCLEINIT:
			m->Flags[2] |= (state) 2 : ~2; break;
		case MSF_INVISIBLEENEMIES:
			m->Flags[2] |= (state) 4 : ~4; break;
		case MSF_INVISLINK:
			m->Flags[2] |= (state) 8 : ~8; break;
		case MSF_NOSUBSCREEN:
			m->Flags[2] |= (state) 16 : ~16; break;
		case MSF_SPRITECARRY:
			m->Flags[2] |= (state) 32 : ~32; break;
		case MSF_NOOFFSET:
			m->Flags[2] |= (state) 64 : ~64; break;
		case MSF_EMELIESALWAYSRETURN:
			m->Flags[2] |= (state) 128 : ~128; break;
		
		//3, flags4
		//1 AND 2 ARE APPARENTLY UNUSED?
		case MSF_DIRECTTIMEDWARPS:
			m->Flags[3] |= (state) 4 : ~4; break;
		case MSF_SECRETSISABLETIMEWRP:
			m->Flags[3] |= (state) 8 : ~8; break;
		case MSF_ENEMIES_SECRET_PERM:
			m->Flags[3] |= (state) 16 : ~16; break;
		//32 IS NOT USED
		case MSF_SAVECONTHERE:
			m->Flags[3] |= (state) 64 : ~64; break;
		case MSF_SAVEONENTRY:
			m->Flags[3] |= (state) 128 : ~128; break;
		
		//4, flags5
		//no 1?
		case MSF_RANDOMTIMEDWARP:
			m->Flags[4] |= (state) 1 : ~1; break;
		case MSF_IGNOREBOOTS:
			m->Flags[4] |= (state) 2 : ~2; break;
		case MSF_AUTODIRECT:
			m->Flags[4] |= (state) 4 : ~4; break;
		case MSF_SENDSIRECT:
			m->Flags[4] |= (state) 8 : ~8; break;
		case MSF_TEMPSECRETS:
			m->Flags[4] |= (state) 16 : ~16; break;
		//no 32?
		case MSF_NO_DIVING:
			m->Flags[4] |= (state) 64 : ~64; break;
		case MSF_NOCARRYOVERFFC:
			m->Flags[4] |= (state) 128 : ~128; break;
		
		
		//5, flags6
		case MSF_INTERIOR:
			m->Flags[5] |= (state) 1 : ~1; break;
		case MSF_DUNGEON:
			m->Flags[5] |= (state) 2 : ~2; break;
		case MSF_TRIGPERM:
			m->Flags[5] |= (state) 4 : ~4; break;
		case MSF_CONTHERE:
			m->Flags[5] |= (state) 8 : ~8; break;
		case MSF_NOCONTINUEWARP:
			m->Flags[5] |= (state) 16 : ~16; break;
		case MSF_ALLTRIGFLAGS:
			m->Flags[5] |= (state) 32 : ~32; break;
		case MSF_TOGGLERINGS:
			m->Flags[5] |= (state) 64 : ~64; break;
		case MSF_WRAPFFC:
			m->Flags[5] |= (state) 128 : ~128; break;
		
		//6, flags7
		case MSF_LAYER3BG:
			m->Flags[6] |= (state) 1 : ~1; break;
		case MSF_LAYER2BG:
			m->Flags[6] |= (state) 2 : ~2; break;
		case MSF_FALLS:
			m->Flags[6] |= (state) 4 : ~4; break;
		case MSF_SIDEVIEW:
			m->Flags[6] |= (state) 8 : ~8; break;
		case MSF_NOLINKMARKER:
			m->Flags[6] |= (state) 16 : ~16; break;
		//32 IS UNUSED
		case MSF_PALCHANGE:
			m->Flags[6] |= (state) 64 : ~64; break;
		case MSF_DRYLAKE:
			m->Flags[6] |= (state) 128 : ~128; break;
		
		//7, flags8
		case MSF_SCRIPT1:
			m->Flags[7] |= (state) 1 : ~1; break;
		case MSF_SCRIPT2:
			m->Flags[7] |= (state) 2 : ~2; break;
		case MSF_SCRIPT3:
			m->Flags[7] |= (state) 4 : ~4; break;
		case MSF_SCRIPT4:
			m->Flags[7] |= (state) 8 : ~8; break;
		case MSF_SCRIPT5:
			m->Flags[7] |= (state) 16 : ~16; break;
		case MSF_LENSEFFECT:
			m->Flags[7] |= (state) 32 : ~32; break;
		case MSF_MAZEOVERRIDE:
			m->Flags[7] |= (state) 64 : ~64; break;
		
		
		default: 
		{
			printf("Invalid flag id (%d) passed to SetMapscreenFlag()\n", flagid);  break;
		}
	}
}
