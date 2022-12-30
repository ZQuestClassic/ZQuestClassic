#include "base/zdefs.h"
#include "base/zsys.h"
#include "ffc.h"
#include "tiles.h"
#include "sprite.h"
extern sprite_list Lwpns;
extern byte quest_rules[QUESTRULES_NEW_SIZE];

#ifdef IS_PLAYER
#include "combos.h"
#include "maps.h"
#include "hero.h"
extern newcombo *combobuf;
// extern mapscr tmpscr[2]; // TODO z3 ! delete me
extern int16_t lensclk;
extern HeroClass Hero;
void screen_ffc_modify_postroutine(word index);
#endif

ffcdata::ffcdata() : solid_object(), parent(nullptr), mapscr_index(0)
{
	clear();
	loaded = false;
}

ffcdata::~ffcdata()
{}

void ffcdata::copy(ffcdata const& other)
{
	solid_object::copy(other);
	ax = other.ax;
	ay = other.ay;
	flags = other.flags;
	setData(other.data);
	delay = other.delay;
	cset = other.cset;
	link = other.link;
	txsz = other.txsz;
	tysz = other.tysz;
	initialized = other.initialized;
	script = other.script;
	memcpy(initd, other.initd, sizeof(initd));
	memcpy(inita, other.inita, sizeof(inita));
	updateSolid();
}

void ffcdata::changerCopy(ffcdata& other, int32_t i, int32_t j)
{
#ifdef IS_PLAYER
	if(other.flags&ffCHANGETHIS)
	{
		setData(other.data);
		cset = other.cset;
	}
	
	if(other.flags&ffCHANGENEXT)
		incData(1);
	
	if(other.flags&ffCHANGEPREV)
		incData(-1);
	
	delay=other.delay;
	x=other.x;
	y=other.y;
	vx=other.vx;
	vy=other.vy;
	ax=other.ax;
	ay=other.ay;
	link=other.link;
	hxsz=other.hxsz;
	hysz=other.hysz;
	txsz=other.txsz;
	tysz=other.tysz;
	
	if(flags&ffCARRYOVER)
		flags=other.flags|ffCARRYOVER;
	else
		flags=other.flags;
	
	flags&=~ffCHANGER;
	
	if(combobuf[other.data].flag>15 && combobuf[other.data].flag<32)
		other.setData(tmpscr.secretcombo[combobuf[other.data].flag-16+4]);
	
	if(i > -1 && j > -1)
	{
		ffposx[i]=(other.x.getInt());
		ffposy[i]=(other.y.getInt());
		if((other.flags&ffSWAPNEXT)||(other.flags&ffSWAPPREV))
		{
			int32_t k=0;
			
			if(other.flags&ffSWAPNEXT)
				k=j<(MAXFFCS-1)?j+1:0;
				
			if(other.flags&ffSWAPPREV)
				k=j>0?j-1:(MAXFFCS-1);
			ffcdata& ffck = tmpscr.ffcs[k];
			auto w = ffck.data;
			ffck.setData(other.data);
			other.setData(w);
			zc_swap(other.cset,ffck.cset);
			zc_swap(other.delay,ffck.delay);
			zc_swap(other.vx,ffck.vx);
			zc_swap(other.vy,ffck.vy);
			zc_swap(other.ax,ffck.ax);
			zc_swap(other.ay,ffck.ay);
			zc_swap(other.link,ffck.link);
			zc_swap(other.hxsz,ffck.hxsz);
			zc_swap(other.hysz,ffck.hysz);
			zc_swap(other.txsz,ffck.txsz);
			zc_swap(other.tysz,ffck.tysz);
			zc_swap(other.flags,ffck.flags);
		}
	}
	updateSolid();
	solid_update(false);
#endif
}

ffcdata::ffcdata(ffcdata const& other)
{
	copy(other);
}

ffcdata& ffcdata::operator=(ffcdata const& other)
{
	copy(other);
	return *this;
}

void ffcdata::clear()
{
	x = y = vx = vy = ax = ay = 0;
	flags = 0;
	setData(0);
	delay = 0;
	cset = link = 0;
	txsz = tysz = 1;
	hxsz = hysz = 16;
	initialized = false;
	hooked = false;
	script = 0;
	memset(initd, 0, sizeof(initd));
	memset(inita, 0, sizeof(inita));
	updateSolid();
}

void ffcdata::setData(word newdata)
{
	data = newdata;
	if(parent)
		parent->update_ffc_data(mapscr_index, data!=0);
#if IS_PLAYER
	if(parent == &tmpscr)
		screen_ffc_modify_postroutine(mapscr_index);
#endif
}
void ffcdata::incData(int32_t inc)
{
	setData(data+inc);
}

void ffcdata::draw(BITMAP* dest, int32_t xofs, int32_t yofs, bool overlay)
{
	if (!data) return;
	if (flags&ffCHANGER) return;
	#ifdef IS_PLAYER
	if ((flags&ffLENSINVIS) && lensclk) return; //If lens is active and ffc is invis to lens, don't draw
	if ((flags&ffLENSVIS) && !lensclk) return; //If FFC does not require lens, or lens is active, draw
	
	if (switch_hooked)
	{
		switch(Hero.switchhookstyle)
		{
			default: case swPOOF:
				break;
			case swFLICKER:
			{
				if (abs(Hero.switchhookclk-33)&0b1000)
					break;
				return;
			}
			case swRISE:
				yofs -= 8-(abs(Hero.switchhookclk-32)/4);
				break;
		}
	}
	#endif
	
	if(!(flags&ffOVERLAY) == !overlay) //force cast both of these to boolean. They're both not, so same as if they weren't not.
	{
		int32_t tx = x + xofs;
		int32_t ty = y + yofs;
		
		if(flags&ffTRANS)
		{
			overcomboblocktranslucent(dest, tx, ty, data, cset, txsz, tysz,128);
		}
		else
		{
			overcomboblock(dest, tx, ty, data, cset, txsz, tysz);
		}
	}
}

bool ffcdata::setSolid(bool set) //exists so that ffcs can do special handling for whether to make something solid or not.
{
	bool actual = set && !(flags&ffCHANGER) && loaded;
	bool ret = solid_object::setSolid(actual);
	solid = set;
	return ret;
}
void ffcdata::updateSolid()
{
	if(setSolid(flags&ffSOLID))
		solid_update(false);
}

void ffcdata::solid_update(bool push)
{
#ifdef IS_PLAYER
	if (hooked && push)
	{
		if (Lwpns.idFirst(wHookshot) > -1)
		{
			zfix dx = (x - old_x);
			zfix dy = (y - old_y);
			if (dx) 
				Hero.setXfix(Hero.getX() + dx);
			if (dy)
				Hero.setYfix(Hero.getY() + dy);
		}
		else
			hooked = false;
	}
#endif
	solid_object::solid_update(push);
}

void ffcdata::setLoaded(bool set)
{
	if(loaded==set) return;
	loaded = set;
	updateSolid();
}
bool ffcdata::getLoaded() const
{
	return loaded;
}

void ffcdata::doContactDamage(int32_t hdir)
{
#ifdef IS_PLAYER
	if(flags & (ffCHANGER | ffETHEREAL))
		return; //Changer or ethereal; has no type
	newcombo const& cmb = combobuf[data];
	if(data && isdamage_type(cmb.type))
		trigger_damage_combo(data, hdir, true);
#endif
}

void mapscr::zero_memory()
{
	//oh joy, this will be fun...
	valid=0;
	guy=0;
	str=0;
	room=0;
	item=0;
	hasitem=0;
	tilewarpoverlayflags=0;
	door_combo_set=0;
	warpreturnc=0;
	stairx=0;
	stairy=0;
	itemx=0;
	itemy=0;
	color=0;
	enemyflags=0;
	lastffc = 0;
	
	exitdir=0;
	pattern=0;
	sidewarpoverlayflags=0;
	warparrivalx=0;
	warparrivaly=0;
	
	sidewarpindex=0;
	undercombo=0;
	undercset=0;
	catchall=0;
	flags=0;
	flags2=0;
	flags3=0;
	flags4=0;
	flags5=0;
	flags6=0;
	flags7=0;
	flags8=0;
	flags9=0;
	flags10=0;
	csensitive=1;
	noreset=0;
	nocarry=0;
	timedwarptics=0;
	nextmap=0;
	nextscr=0;
	
	entry_x = 0;
	entry_y = 0;
	
	old_cpage = 0;
	screen_midi = -1;
	
	for(int32_t i(0); i<4; i++)
	{
		door[i]=0;
		tilewarpdmap[i]=0;
		tilewarpscr[i]=0;
		tilewarptype[i]=0;
		warpreturnx[i]=0;
		warpreturny[i]=0;
		path[i]=0;
		sidewarpscr[i]=0;
		sidewarpdmap[i]=0;
		sidewarptype[i]=0;
	}
	
	for(int32_t i(0); i<10; i++)
		enemy[i]=0;
		
	for(int32_t i(0); i<128; i++)
	{
		secretcombo[i]=0;
		secretcset[i]=0;
		secretflag[i]=0;
	}
	
	for(int32_t i(0); i<6; i++)
	{
		layermap[i]=0;
		layerscreen[i]=0;
		layeropacity[i]=255;
	}
	
	for(int32_t i(0); i<MAXFFCS; i++)
	{
		ffcs[i].clear();
		ffcs[i].parent = this;
		ffcs[i].mapscr_index = i;
	}
	
	script_entry=0;
	script_occupancy=0;
	script_exit=0;
	oceansfx=0;
	bosssfx=0;
	secretsfx=27;
	holdupsfx=20;
	lens_layer=0;

	for ( int32_t q = 0; q < 10; q++ ) npcstrings[q] = 0;
	for ( int32_t q = 0; q < 10; q++ ) new_items[q] = 0;
	for ( int32_t q = 0; q < 10; q++ ) new_item_x[q] = 0;
	for ( int32_t q = 0; q < 10; q++ ) new_item_y[q] = 0;

	script = 0;
	doscript = 0;
	for ( int32_t q = 0; q < 8; q++) screeninitd[q] = 0;
	preloadscript = 0;
	
	screen_waitdraw = 0;
	ffcswaitdraw = 0;
	screendatascriptInitialised = 0;
	hidelayers = 0;
	hidescriptlayers = 0;
	memset(data, 0, sizeof(data));
	memset(sflag, 0, sizeof(sflag));
	memset(cset, 0, sizeof(cset));
}

mapscr::mapscr()
{
	zero_memory();
}

void mapscr::copy(mapscr const& other)
{
	valid=other.valid;
	guy=other.guy;
	str=other.str;
	room=other.room;
	item=other.item;
	hasitem=other.hasitem;
	tilewarpoverlayflags=other.tilewarpoverlayflags;
	door_combo_set=other.door_combo_set;
	warpreturnc=other.warpreturnc;
	stairx=other.stairx;
	stairy=other.stairy;
	itemx=other.itemx;
	itemy=other.itemy;
	color=other.color;
	enemyflags=other.enemyflags;
	lastffc = other.lastffc;
	
	exitdir=other.exitdir;
	pattern=other.pattern;
	sidewarpoverlayflags=other.sidewarpoverlayflags;
	warparrivalx=other.warparrivalx;
	warparrivaly=other.warparrivaly;
	
	sidewarpindex=other.sidewarpindex;
	undercombo=other.undercombo;
	undercset=other.undercset;
	catchall=other.catchall;
	flags=other.flags;
	flags2=other.flags2;
	flags3=other.flags3;
	flags4=other.flags4;
	flags5=other.flags5;
	flags6=other.flags6;
	flags7=other.flags7;
	flags8=other.flags8;
	flags9=other.flags9;
	flags10=other.flags10;
	csensitive=other.csensitive;
	noreset=other.noreset;
	nocarry=other.nocarry;
	timedwarptics=other.timedwarptics;
	nextmap=other.nextmap;
	nextscr=other.nextscr;
	
	
	entry_x = other.entry_x;
	entry_y = other.entry_y;
	
	old_cpage = other.old_cpage;
	screen_midi = other.screen_midi;
	
	for(int32_t i(0); i<4; i++)
	{
		door[i]=other.door[i];
		tilewarpdmap[i]=other.tilewarpdmap[i];
		tilewarpscr[i]=other.tilewarpscr[i];
		tilewarptype[i]=other.tilewarptype[i];
		warpreturnx[i]=other.warpreturnx[i];
		warpreturny[i]=other.warpreturny[i];
		path[i]=other.path[i];
		sidewarpscr[i]=other.sidewarpscr[i];
		sidewarpdmap[i]=other.sidewarpdmap[i];
		sidewarptype[i]=other.sidewarptype[i];
	}
	
	for(int32_t i(0); i<10; i++)
		enemy[i]=other.enemy[i];
		
	for(int32_t i(0); i<128; i++)
	{
		secretcombo[i]=other.secretcombo[i];
		secretcset[i]=other.secretcset[i];
		secretflag[i]=other.secretflag[i];
	}
	
	for(int32_t i(0); i<6; i++)
	{
		layermap[i]=other.layermap[i];
		layerscreen[i]=other.layerscreen[i];
		layeropacity[i]=other.layeropacity[i];
	}
	
	word c = other.numFFC();
	for(word i = 0; i<c; ++i)
		ffcs[i] = other.ffcs[i];
	for(word i = c; i<MAXFFCS; ++i)
		ffcs[i].clear();
	
	script_entry=other.script_entry;
	script_occupancy=other.script_occupancy;
	script_exit=other.script_exit;
	oceansfx=other.oceansfx;
	bosssfx=other.bosssfx;
	secretsfx=other.secretsfx;
	holdupsfx=other.holdupsfx;
	lens_layer=other.lens_layer;

	for ( int32_t q = 0; q < 10; q++ ) npcstrings[q] = other.npcstrings[q];
	for ( int32_t q = 0; q < 10; q++ ) new_items[q] = other.new_items[q];
	for ( int32_t q = 0; q < 10; q++ ) new_item_x[q] = other.new_item_x[q];
	for ( int32_t q = 0; q < 10; q++ ) new_item_y[q] = other.new_item_y[q];

	script = other.script;
	doscript = other.doscript;
	for ( int32_t q = 0; q < 8; q++) screeninitd[q] = other.screeninitd[q];
	preloadscript = other.preloadscript;
	
	screen_waitdraw = other.screen_waitdraw;
	ffcswaitdraw = other.ffcswaitdraw;
	screendatascriptInitialised = other.screendatascriptInitialised;
	hidelayers = other.hidelayers;
	hidescriptlayers = other.hidescriptlayers;
	
	memcpy(data, other.data, sizeof(data));
	memcpy(sflag, other.sflag, sizeof(sflag));
	memcpy(cset, other.cset, sizeof(cset));
}

mapscr::mapscr(mapscr const& other)
{
	copy(other);
}

mapscr& mapscr::operator=(mapscr const& other)
{
	copy(other);
	return *this;
}

void mapscr::update_ffc_count(word spos)
{
	if(spos < lastffc || spos > MAXFFCS)
		spos = lastffc;
	lastffc = 0;
	for(word w = spos; w > 0; --w)
	{
		if(ffcs[w].data)
		{
			lastffc = w;
			break;
		}
	}
	if (lastffc < 31 && get_bit(quest_rules, qr_OLD_FFC_FUNCTIONALITY))
		lastffc = 31;
}

