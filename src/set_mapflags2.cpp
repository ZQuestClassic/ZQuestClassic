enum mapflagtype
{
	// Room Types
	MSF_INTERIOR, MSF_DUNGEON, MSF_SIDEVIEW,
	
	// View
	MSF_INVISLINK, MSF_NOLINKMARKER, MSF_NOSUBSCREEN, MSF_NOOFFSET,
	MSF_LAYER2BG, MSF_LAYER3BG, MSF_DARKROOM,
	
	// Secrets
	MSF_BLOCKSHUT, MSF_TEMPSECRETS, MSF_TRIGPERM, MSF_ALLTRIGFLAGS,
	
	// Warp
	MSF_AUTODIRECT, MSF_SENDSIRECT, MSF_MAZEPATHS, MSF_MAZEOVERRIDE,
	MSF_SPRITECARRY, MSF_DIRECTTIMEDWARPS, MSF_SECRETSISABLETIMEWRP, MSF_RANDOMTIMEDWARP,
	
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
	
	//t.b.a, enemy flags (EFlags)
	
	/*
	case 0:
		f = m->enemyflags&0x1F;
		break;
		
	case 1:
		f = ornextflag(m->enemyflags&32) | ornextflag(m->enemyflags&64) | ornextflag(m->flags3&4)
			| ornextflag(m->enemyflags&128)| ornextflag((m->flags2>>4)&4);
		break;
		
	case 2:
		f = ornextflag(m->flags3&128)    | ornextflag(m->flags&2)       | ornextflag((m->flags2>>4)&8)
			| ornextflag(m->flags4&16);
	*/

	
}

void set_mapscreenflag_state(mapscr *m, int flag, bool state)
{
	switch(flag)
	{
		// Room Types
		case MSF_INTERIOR: 
			if ( state )
				m->flags6 |= 1;
			else flags6 &= ~1;
			break;
		case MSF_DUNGEON: 
			if ( state )
				m->flags6 |= 2;
			else flags6 &= ~2;
			break;
		case MSF_SIDEVIEW: 
			if ( state )
				m->flags7 |= 8;
			else flags6 &= ~8;
			break;
		
		// View
		case MSF_INVISLINK: 
			if ( state )
				m->flags3 |= 8;
			else flags3 &= ~8;
			break;
		case MSF_NOLINKMARKER: 
			if ( state )
				m->flags7 |= 16;
			else flags7 &= ~16;
			break;
			
		case MSF_NOSUBSCREEN: 
			if ( state )
				m->flags3 |= 16;
			else flags3 &= ~16;
			break;
		case MSF_NOOFFSET: 
			if ( state )
				m->flags3 |= 64;
			else flags3 &= ~64;
			break;
		
		case MSF_LAYER2BG: 
			if ( state )
				m->flags7 |= 2;
			else flags7 &= ~2;
			break;
		case MSF_LAYER3BG: 
			if ( state )
				m->flags7 |= 1;
			else flags7 &= ~1;
			break;
		case MSF_DARKROOM: 
			if ( state )
				m->flags |= 4;
			else flags &= ~4;
			break;
	
		// Secrets
		case MSF_BLOCKSHUT: 
			if ( state )
				m->flags |= 1;
			else flags &= ~1;
			break;
		case MSF_TEMPSECRETS:
			if ( state )
				m->flags5 |= 16;
			else flags5 &= ~16;
			break;
			
		case MSF_TRIGPERM: 
			if ( state )
				m->flags6 |= 4;
			else flags6 &= ~4;
			break;
		case MSF_ALLTRIGFLAGS: 
			if ( state )
				m->flags6 |= 32;
			else flags6 &= ~32;
			break;
		// Warp
		case MSF_AUTODIRECT: 
			if ( state )
				m->flags5 |= 4;
			else flags5 &= ~4;
			break;
		case MSF_SENDSIRECT: 
			if ( state )
				m->flags5 |= 8;
			else flags5 &= ~8;
			break;
		case MSF_MAZEPATHS: 
			if ( state )
				m->flags |= 64;
			else flags &= ~64;
			break;
			
		case MSF_MAZEOVERRIDE: 
			if ( state )
				m->flags8 |= 64;
			else flags8 &= ~64;
			break;
		case MSF_SPRITECARRY: 
			if ( state )
				m->flags3 |= 32;
			else flags3 &= ~32;
			break;
			
			//TODO
		case MSF_DIRECTTIMEDWARPS:
		case MSF_SECRETSISABLETIMEWRP:
		case MSF_RANDOMTIMEDWARP:
		
			//TODO 
			break;
		
		// Item
		case MSF_HOLDUP: 
			if ( state )
				m->flags3 |= 1;
			else flags3 &= ~1;
			break;
		case MSF_FALLS: 
			if ( state )
				m->flags7 |= 64;
			else flags7 &= ~64;
			break;
			
			//TODO
		case MSF_HOLDUP:
		case MSF_FALLS:
			//TODO 
			break;
			
		// Combo
		case MSF_MIDAIR: 
			if ( state )
				m->(flags2>>4) |= 2;
			else (flags2>>4) &= ~2;
			break;
		case MSF_CYCLEINIT: 
			if ( state )
				m->flags3 |= 2;
			else flags3 &= ~2;
			break;
		case MSF_IGNOREBOOTS: 
			if ( state )
				m->flags5 |= 2;
			else flags5 &= ~2;
			break;
		case MSF_TOGGLERINGS: 
			if ( state )
				m->flags6 |= 64;
			else flags6 &= ~64;
			break;
		// Save
		case MSF_SAVECONTHERE: 
			if ( state )
				m->flags4 |= 64;
			else flags4 &= ~64;
			break;
		case MSF_SAVEONENTRY:
			if ( state )
				m->flags4 |= 128;
			else flags4 &= ~128;
			break;			
			
		case MSF_CONTHERE: 
			if ( state )
				m->flags6 |= 8;
			else flags6 &= ~8;
			break;
			
		case MSF_NOCONTINUEWARP:  
			if ( state )
				m->flags6 |= 16;
			else flags6 &= ~16;
			break;
	
		// FFC
		case MSF_WRAPFFC: 
			if ( state )
				m->flags6 |= 128;
			else flags6 &= ~128;
			break;
			
		case MSF_NOCARRYOVERFFC: 
			if ( state )
				m->flags5 |= 128;
			else flags5 &= ~128;
			break;
	
		// Whistle
		case MSF_STAIRS: 
			if ( state )
				m->flags |= 16;
			else flags &= ~16;
			break;
		case MSF_PALCHANGE: 
			if ( state )
				m->flags7 |= 64;
			else flags7 &= ~64;
			break;
		case MSF_DRYLAKE:  
			if ( state )
				m->flags7 |= 128;
			else flags7 &= ~128;
			break;
			
		// Enemies
			//TODO
		case MSF_INVISIBLEENEMIES:
		case MSF_TRAPS_IGNORE_SOLID:
		case MSF_EMELIESALWAYSRETURN:
		case MSF_ENEMIES_ITEM:
		case MSF_ENEMEIS_SECRET:
		case MSF_ENEMIES_SECRET_PERM:
			//TODO
			break;
			
		// Misc
		case MSF_ALLOW_LADDER: 
			if ( state )
				m->flags |= 32;
			else flags &= ~32;
			break;
		case MSF_NO_DIVING: 
			if ( state )
				m->flags5 |= 64;
			else flags5 &= ~64;
			break;
			
		case MSF_LENSEFFECT:
			if ( state )
				m->flags8 |= 32;
			else flags8 &= ~32;
			break;
		
		//TODO
		case MSF_SFXONENTRY:
			break;
		//TODO
			
		// Custom / Script
		case MSF_SCRIPT1: 
		case MSF_CUSTOM1:
			if ( state )
				m->flags8 |= 1;
			else flags8 &= ~1;
			break;
		case MSF_SCRIPT2: 
		case MSF_CUSTOM2:
			if ( state )
				m->flags8 |= 2;
			else flags8 &= ~2;
			break;
		case MSF_SCRIPT3: 
		case MSF_CUSTOM3:
			if ( state )
				m->flags8 |= 4;
			else flags8 &= ~4;
			break;
		case MSF_SCRIPT4:
		case MSF_CUSTOM4:
			if ( state )
				m->flags8 |= 8;
			else flags8 &= ~8;
			break;
		case MSF_SCRIPT5:
		case MSF_CUSTOM5:
			if ( state )
				m->flags8 |= 16;
			else flags8 &= ~16;
			break;
				
		
		default: Z_scripterrlog("Illegal flag value (%d) passed to SetScreenFlags", flag);
	}
}
				
void get_mapscreenflag_state(mapscr *m, int flag)
{
	switch(flag)
	{
		// Room Types
		case MSF_INTERIOR: 
			return (m->flags6&1) ? 1 : 0;
		case MSF_DUNGEON: 
			return (m->flags6&2) ? 1 : 0;
		case MSF_SIDEVIEW: 
			return (m->flags7&8) ? 1 : 0;
		
		// View
		case MSF_INVISLINK: 
			return (m->flags3&8) ? 1 : 0;
		case MSF_NOLINKMARKER: 
			return (m->flags7&16) ? 1 : 0;
			
		case MSF_NOSUBSCREEN: 
			return (m->flags3&16) ? 1 : 0;
		case MSF_NOOFFSET: 
			return (m->flags3&64) ? 1 : 0;
		
		case MSF_LAYER2BG: 
			return (m->flags7&2) ? 1 : 0;
		case MSF_LAYER3BG: 
			return (m->flags7&1) ? 1 : 0;
		case MSF_DARKROOM: 
			return (m->flags&4) ? 1 : 0;
	
		// Secrets
		case MSF_BLOCKSHUT: 
			return (m->flags&1) ? 1 : 0;
		case MSF_TEMPSECRETS:
			return (m->flags5&16) ? 1 : 0;
		case MSF_TRIGPERM: 
			return (m->flags6&4) ? 1 : 0;
		case MSF_ALLTRIGFLAGS: 
			return (m->flags6&32) ? 1 : 0;
		
		// Warp
		case MSF_AUTODIRECT: 
			return (m->flags5&4) ? 1 : 0;
		case MSF_SENDSIRECT: 
			return (m->flags5&8) ? 1 : 0;
		case MSF_MAZEPATHS: 
			return (m->flags&64) ? 1 : 0;
			
		case MSF_MAZEOVERRIDE: 
			return (m->flags8&64) ? 1 : 0;
		case MSF_SPRITECARRY: 
			return (m->flags3&32) ? 1 : 0;
		
		//TODO
		case MSF_DIRECTTIMEDWARPS:
		case MSF_SECRETSISABLETIMEWRP:
		case MSF_RANDOMTIMEDWARP:
			return -2;
		//TODO
			
		// Item
		case MSF_HOLDUP: 
			return (m->flags3&1) ? 1 : 0;
		case MSF_FALLS: 
			return (m->flags7&64) ? 1 : 0;
		
		// Combo
		case MSF_MIDAIR: 
			return (m->(flags2>>4)&2) ? 1 : 0;
		case MSF_CYCLEINIT: 
			return (m->flags3&2) ? 1 : 0;
		case MSF_IGNOREBOOTS: 
			return (m->flags5&2) ? 1 : 0;
		case MSF_TOGGLERINGS: 
			return (m->flags6&64) ? 1 : 0;
		// Save
		case MSF_SAVECONTHERE: 
			return (m->flags4&64) ? 1 : 0;
		case MSF_SAVEONENTRY:
			return (m->flags4&128) ? 1 : 0;		
			
		case MSF_CONTHERE: 
			return (m->flags6&8) ? 1 : 0;
			
		case MSF_NOCONTINUEWARP:  
			return (m->flags6&16) ? 1 : 0;
	
		// FFC
		case MSF_WRAPFFC: 
			return (m->flags6&128) ? 1 : 0;
			
		case MSF_NOCARRYOVERFFC: 
			return (m->flags5&128) ? 1 : 0;
	
		// Whistle
		case MSF_STAIRS: 
			return (m->flags&16) ? 1 : 0;
		case MSF_PALCHANGE: 
			return (m->flags7&64) ? 1 : 0;
		case MSF_DRYLAKE:  
			return (m->flagsy7&128) ? 1 : 0;
		
		// Enemies
			//TODO
		case MSF_INVISIBLEENEMIES:
		case MSF_TRAPS_IGNORE_SOLID:
		case MSF_EMELIESALWAYSRETURN:
		case MSF_ENEMIES_ITEM:
		case MSF_ENEMEIS_SECRET:
		case MSF_ENEMIES_SECRET_PERM:
			return -2;
			//TODO
		
		// Misc
		case MSF_ALLOW_LADDER: 
			return (m->flags&32) ? 1 : 0;
		case MSF_NO_DIVING: 
			return (m->flags5&64) ? 1 : 0;
		
		case MSF_LENSEFFECT:
			return (m->flags8&32) ? 1 : 0;
		
		//TODO
		case MSF_SFXONENTRY:
			return -2;
		//TODO
		
		//Custom / Script 
		case MSF_SCRIPT1: 
		case MSF_CUSTOM1:
			return (m->flags8&1) ? 1 : 0;
		case MSF_SCRIPT2: 
		case MSF_CUSTOM2:
			return (m->flags8&2) ? 1 : 0;
		case MSF_SCRIPT3: 
		case MSF_CUSTOM3:
			return (m->flags8&4) ? 1 : 0;
		case MSF_SCRIPT4:
		case MSF_CUSTOM4:
			return (m->flags8&8) ? 1 : 0;
		case MSF_SCRIPT5:
		case MSF_CUSTOM5:
			return (m->flags8&16) ? 1 : 0;
				
		
		default: 
		{
			Z_scripterrlog("Illegal flag value (%d) passed to SetScreenFlags", flag);
			return -1;
		}
	}
}


/* LIST OF MAP FLAGS AND ENEMY FLAGS
	//List of map flags



// Room Types
MSF_INTERIOR
flags6&1
MSF_DUNGEON
flags6&2
MSF_SIDEVIEW
flags7&8

// View
MSF_INVISLINK
flags3&8
MSF_NOLINKMARKER
flags7&16
MSF_NOSUBSCREEN
flags3&16
MSF_NOOFFSET
flags3&64
MSF_LAYER2BG
lags7&2
MSF_LAYER3BG
flags7&1
MSF_DARKROOM
flags&4

// Secrets
MSF_BLOCKSHUT
flags&1
MSF_TEMPSECRETS
flags5&16
MSF_TRIGPERM
flags6&4
MSF_ALLTRIGFLAGS
flags6&32

// Warp
MSF_AUTODIRECT
flags5&4
MSF_SENDSIRECT
flags5&8
MSF_MAZEPATHS
flags&64
MSF_MAZEOVERRIDE
flags8&64
MSF_SPRITECARRY
flags3&32

// Item
MSF_HOLDUP
flags3&1
MSF_FALLS
flags7&4

// Combo
MSF_MIDAIR
(flags2>>4)&2
MSF_CYCLEINIT
flags3&2
MSF_IGNOREBOOTS
flags5&2
MSF_TOGGLERINGS
flags6&64

// Save
MSF_MIDAIR
flags4&64
MSF_CYCLEINIT
flags4&128
MSF_IGNOREBOOTS
flags6&8
MSF_TOGGLERINGS
flags6&16

// FFC
MSF_WRAPFFC
flags6&128
MSF_NOCARRYOVERFFC
flags5&128

// Whistle
MSF_STAIRS
flags&16
MSF_PALCHANGE
flags7&64
MSF_DRYLAKE
flags7&128

// Misc
MSF_ALLOW_LADDER
flags&32
MSF_NO_DIVING
flags5&64
MSF_SCRIPT1
flags8<<2;
MSF_SCRIPT2
MSF_SCRIPT3
MSF_SCRIPT4
MSF_SCRIPT5


*/