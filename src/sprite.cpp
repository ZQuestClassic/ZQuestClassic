//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  sprite.cc
//
//  Sprite classes:
//   - sprite:      base class for the guys and enemies in zelda.cc
//   - movingblock: the moving block class
//   - sprite_list: main container class for different groups of sprites
//   - item:        items class
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include "zdefs.h"
#include "zsys.h"
#include "sprite.h"
#include "tiles.h"
#include "particles.h"
#include "maps.h"
extern sprite_list particles;
extern byte                quest_rules[QUESTRULES_NEW_SIZE];
extern bool get_debug();
extern bool halt;
extern bool show_sprites;
extern bool show_hitboxes;
extern bool is_zquest();
extern void debugging_box(int x1, int y1, int x2, int y2);
#include "ffscript.h"
extern FFScript FFCore;

#define degtoFix(d)     ((d)*0.7111111111111)
#define radtoFix(d)     ((d)*40.743665431525)

template<class T> inline
fixed deg_to_fixed(T d)
{
    return ftofix(degtoFix(d));
}
template<class T> inline
fixed rad_to_fixed(T d)
{
    return ftofix(radtoFix(d));
}

/**********************************/
/******* Sprite Base Class ********/
/**********************************/

sprite::sprite()
{
    uid = getNextUID();
    x=y=z=tile=shadowtile=cs=flip=c_clk=clk=xofs=yofs=zofs=hxofs=hyofs=fall=0;
    txsz=1;
    tysz=1;
    id=-1;
    hxsz=hysz=16;
    hzsz=1;
    yofs=playing_field_offset;
    dir=down;
    angular=canfreeze=false;
    drawstyle=0;
    extend=0;
    wpnsprite = 0; //wpnsprite is new for 2.6 -Z
    //memset(stack,0,sizeof(stack));
    memset(stack, 0xFFFF, sizeof(stack));
    
    /*ewpnclass=0;
    lwpnclass=0;
    guyclass=0;*/ //Not implemented
    //ewpnref=0;
    //lwpnref=0;
    //guyref=0;
    //itemref=0;
    lasthitclk=0;
    lasthit=0;
    angle=0;
    misc=0;
    pit_pulldir = -1;
	pit_pullclk = 0;
	fallclk = 0;
	fallCombo = 0;
	old_cset = 0;
	
    for(int i=0; i<10; i++)
    {
        dummy_int[i]=0;
        dummy_fix[i]=0;
        dummy_float[i]=0;
        dummy_bool[i]=false;
    }
    
    //for(int i=0;i<8;i++)
    //{
    //  if(i<2) a[i]=0;
    //  d[i]=0;
    //}
    scriptflag=0;
    //pc=0;
    //sp=0;
    //itemclass=0;
    //ffcref=0;
    scriptData.Clear(); //when we have npc scripts we'll need this again, for now not.
    doscript=1;
    waitdraw = 0;
    for(int i=0; i<32; i++) miscellaneous[i] = 0;
    
    scriptcoldet = 1;
    initialised = 0;
    
    //itemref = 0;
    //guyref = 0;
    //lwpnref = 0;
    //ewpnref = 0;
    //guyclass = 0; //Not implemented
    //lwpnclass = 0;
    //ewpnclass = 0;
    script = 0;
    weaponscript = 0;
    scripttile = -1;
    scriptflip = -1;
    do_animation = 1;
    rotation = 0;
    scale = 0;
    moveflags = 0;
    drawflags = 0;
	knockbackflags = 0;
	knockbackSpeed = 4; //default speed
	script_knockback_clk = 0;
	script_knockback_speed = 0;
	screenedge = 0;
	shadowsprite = 0;
	scriptshadowtile = -1;
    for ( int q = 0; q < 8; q++ )
    {
	    initD[q] = 0;
	    weap_initd[q] = 0;
    }
    for ( int q = 0; q < 2; q++ )
    {
	    initA[q] = 0;
	    weap_inita[q] = 0;
    }
}

sprite::sprite(sprite const & other):
    x(other.x),
    y(other.y),
    z(other.z),
    fall(other.fall),
    tile(other.tile),
    shadowtile(other.shadowtile),
    cs(other.cs),
    flip(other.flip),
    c_clk(other.c_clk),
    clk(other.clk),
    misc(other.misc),
    xofs(other.xofs),
    yofs(other.yofs),
    zofs(other.zofs),
    hxofs(other.hxofs),
    hyofs(other.hyofs),
    hxsz(other.hxsz),
    hysz(other.hysz),
    hzsz(other.hzsz),
    txsz(other.txsz),
    tysz(other.tysz),
    id(other.id),
    dir(other.dir),
    angular(other.angular),
    canfreeze(other.canfreeze),
    angle(other.angle),
    lasthit(other.lasthit),
    lasthitclk(other.lasthitclk),
    drawstyle(other.drawstyle),
    extend(other.extend),
    wpnsprite(other.wpnsprite),
    scriptData(other.scriptData),
//ffcref(other.ffcref),
//itemref(other.itemref),
//guyref(other.guyref),
//lwpnref(other.lwpnref),
//ewpnref(other.ewpnref),
//sp(other.sp),
//pc(other.pc),
scriptflag(other.scriptflag),
doscript(other.doscript),
waitdraw(other.waitdraw),
//itemclass(other.itemclass),
//guyclass(other.guyclass),
//lwpnclass(other.lwpnclass),
//ewpnclass(other.ewpnclass),
script(other.script),
weaponscript(other.weaponscript),
scripttile(other.scripttile),
scriptflip(other.scriptflip),
rotation(other.rotation),
moveflags(other.moveflags),
drawflags(other.drawflags),
knockbackflags(other.knockbackflags),
knockbackSpeed(other.knockbackSpeed),
script_knockback_clk(other.script_knockback_clk),
script_knockback_speed(other.script_knockback_speed),
scale(other.scale),
screenedge(other.screenedge),
shadowsprite(other.shadowsprite),
scriptshadowtile(other.scriptshadowtile),
pit_pulldir(other.pit_pulldir),
pit_pullclk(other.pit_pullclk),
fallclk(other.fallclk),
fallCombo(other.fallCombo),
old_cset(other.old_cset),
do_animation(other.do_animation)

{
    uid = getNextUID();
    
    for(int i=0; i<10; ++i)
    {
        dummy_int[i]=other.dummy_int[i];
        dummy_fix[i]=other.dummy_fix[i];
        dummy_float[i]=other.dummy_float[i];
        dummy_bool[i]=other.dummy_bool[i];
    }
    
    //for (int i=0; i<8; ++i)
    //{
    //  d[i]=other.d[i];
    //}
    //for (int i=0; i<2; ++i)
    //{
    //  a[i]=other.a[i];
    //}
    for(int i=0; i<32; i++) miscellaneous[i] = other.miscellaneous[i];
    
    scriptcoldet = other.scriptcoldet;
    initialised = other.initialised;
    
    
    //for ( int q = 0; q < MAX_SCRIPT_REGISTERS; q++ )
    //{
//	stack[q] = other.stack[q];
    //}
    memset(stack, 0xFFFF, sizeof(stack));
    for (int i=0; i<8; ++i)
    {
      initD[i]=other.initD[i];
      weap_initd[i]=other.weap_initd[i];
	   // al_trace("Sprite.cpp: Assigning other.initD[%d] is: %d\n",i, other.initD[i]);
    }
    for (int i=0; i<2; ++i)
    {
      initA[i]=other.initA[i];
      weap_inita[i]=other.weap_inita[i];
    }
}

sprite::sprite(zfix X,zfix Y,int T,int CS,int F,int Clk,int Yofs):
    x(X),y(Y),tile(T),cs(CS),flip(F),clk(Clk),yofs(Yofs)
{
    uid = getNextUID();
    hxsz=hysz=16;
    hxofs=hyofs=xofs=0;
    txsz=1;
    tysz=1;
    id=-1;
    dir=down;
    angular=canfreeze=false;
    extend=0;
    
    //for(int i=0;i<8;i++)
    //{
    //  if(i<2) a[i]=0;
    //  d[i]=0;
    //}
    scriptflag=0;
    //pc=0;
    //sp=0;
    //ffcref=0;
    doscript=1;
    waitdraw = 0;
    //itemclass=0;
    for(int i=0; i<32; i++) miscellaneous[i] = 0;
    
    scriptcoldet = 1;
    initialised = 0;
    scriptData.Clear();
    //ewpnclass=0;
    //lwpnclass=0;
    //guyclass=0;
    //ewpnref=0;
    //lwpnref=0;
    //guyref=0;
    //itemref=0;
    script = 0;
    weaponscript = 0;
    scripttile = -1;
    scriptflip = -1;
    rotation = 0;
    moveflags = 0;
	knockbackflags = 0;
	knockbackSpeed = 4; //default speed
	script_knockback_clk = 0;
	script_knockback_speed = 0;
    scale = 0;
    do_animation = 1;
    drawstyle=0;
    lasthitclk=0;
    lasthit=0;
    angle=0;
    hzsz=1;
    misc=0;
    c_clk=0;
    shadowtile=0;
    shadowsprite = 0;
    screenedge = 0;
    scriptshadowtile = -1;
    for ( int q = 0; q < 8; q++ ) 
    {
        initD[q] = 0;
        weap_initd[q] = 0;
    }
    for ( int q = 0; q < 2; q++ ) 
    {
        initA[q] = 0;
        weap_inita[q] = 0;
    }
    memset(stack, 0xFFFF, sizeof(stack));
}

sprite::~sprite()
{
  
}

long sprite::getNextUID()
{
    static long nextid = 0;
    return nextid++;
}

void sprite::draw2(BITMAP *)                            // top layer for special needs
{
}

void sprite::drawcloaked2(BITMAP *)                     // top layer for special needs
{
}

bool sprite::animate(int)
{
    ++c_clk;
    return false;
}
int sprite::real_x(zfix fx)
{
    int rx = fx.getInt();
    
    switch(dir)
    {
    case 9:
    case 13:
        if(fx.getDPart() != 0)
            ++rx;
            
        break;
    }
    
    return rx;
}

int sprite::real_y(zfix fy)
{
    return fy.getInt();
}

int sprite::real_z(zfix fz)
{
    return fz.getInt();
}

int sprite::get_pit() //Returns combo ID of pit that sprite WOULD fall into; no side-effects
{
	int ispitul = getpitfall(x,y);
	int ispitbl = getpitfall(x,y+15);
	int ispitur = getpitfall(x+15,y);
	int ispitbr = getpitfall(x+15,y+15);
	int ispitul_50 = getpitfall(x+8,y+8);
	int ispitbl_50 = getpitfall(x+8,y+7);
	int ispitur_50 = getpitfall(x+7,y+8);
	int ispitbr_50 = getpitfall(x+7,y+7);
	switch((ispitul?1:0) + (ispitur?1:0) + (ispitbl?1:0) + (ispitbr?1:0))
	{
		case 4:
		{
			return ispitul_50 ? ispitul_50 : ispitul;
		}
		case 3:
		{
			if(ispitul && ispitur && ispitbl) //UL_3
			{
				return ispitul_50;
			}
			else if(ispitul && ispitur && ispitbr) //UR_3
			{
				return ispitur_50;
			}
			else if(ispitul && ispitbl && ispitbr) //BL_3
			{
				return ispitbl_50;
			}
			else if(ispitbl && ispitur && ispitbr) //BR_3
			{
				return ispitbr_50;
			}
			break;
		}
		case 2:
		{
			if(ispitul && ispitur) //Up
			{
				return ispitul_50 ? ispitul_50 : ispitur_50;
			}
			if(ispitbl && ispitbr) //Down
			{
				return ispitbl_50 ? ispitbl_50 : ispitbr_50;
			}
			if(ispitul && ispitbl) //Left
			{
				return ispitul_50 ? ispitul_50 : ispitbl_50;
			}
			if(ispitur && ispitbr) //Right
			{
				return ispitur_50 ? ispitur_50 : ispitbr_50;
			}
			break;
		}
		case 1:
		{
			if(ispitul) //UL_1
			{
				return ispitul_50;
			}
			if(ispitur) //UR_1
			{
				return ispitur_50;
			}
			if(ispitbl) //BL_1
			{
				return ispitbl_50;
			}
			if(ispitbr) //BR_1
			{
				return ispitbr_50;
			}
			break;
		}
	}
	return 0;
}

int sprite::check_pits() //Returns combo ID of pit fallen into; 0 for not fallen.
{
	int safe_cnt = 0;
	bool has_fallen = false;
	int ispitul, ispitbl, ispitur, ispitbr, ispitul_50, ispitbl_50, ispitur_50, ispitbr_50;
	while(++safe_cnt < 16) //Prevent softlocks
	{
		ispitul = getpitfall(x,y);
		ispitbl = getpitfall(x,y+15);
		ispitur = getpitfall(x+15,y);
		ispitbr = getpitfall(x+15,y+15);
		ispitul_50 = getpitfall(x+8,y+8);
		ispitbl_50 = getpitfall(x+8,y+7);
		ispitur_50 = getpitfall(x+7,y+8);
		ispitbr_50 = getpitfall(x+7,y+7);
		int dir = -1;
		switch((ispitul?1:0) + (ispitur?1:0) + (ispitbl?1:0) + (ispitbr?1:0))
		{
			case 4:
			{
				fallclk = PITFALL_FALL_FRAMES; //Fall
				old_cset = cs;
				return ispitul_50 ? ispitul_50 : ispitul;
			}
			case 3:
			{
				if(ispitul && ispitur && ispitbl) //UL_3
				{
					if(ispitul_50)
					{
						dir=l_up; break;
					}
				}
				else if(ispitul && ispitur && ispitbr) //UR_3
				{
					if(ispitur_50)
					{
						dir=r_up; break;
					}
				}
				else if(ispitul && ispitbl && ispitbr) //BL_3
				{
					if(ispitbl_50)
					{
						dir=l_down; break;
					}
				}
				else if(ispitbl && ispitur && ispitbr) //BR_3
				{
					if(ispitbr_50)
					{
						dir=r_down; break;
					}
				}
				break;
			}
			case 2:
			{
				if(ispitul && ispitur) //Up
				{
					if(ispitul_50 && ispitur_50) //Straight up
					{
						dir = up; break;
					}
					else if(ispitul_50)
					{
						dir = l_up; break;
					}
					else if(ispitur_50)
					{
						dir = r_up; break;
					}
				}
				if(ispitbl && ispitbr) //Down
				{
					if(ispitbl_50 && ispitbr_50) //Straight down
					{
						dir = down; break;
					}
					else if(ispitbl_50)
					{
						dir = l_down; break;
					}
					else if(ispitbr_50)
					{
						dir = r_down; break;
					}
				}
				if(ispitul && ispitbl) //Left
				{
					if(ispitul_50 && ispitbl_50) //Straight left
					{
						dir = left; break;
					}
					else if(ispitul_50)
					{
						dir = l_up; break;
					}
					else if(ispitbl_50)
					{
						dir = l_down; break;
					}
				}
				if(ispitur && ispitbr) //Right
				{
					if(ispitur_50 && ispitbr_50) //Straight right
					{
						dir = right; break;
					}
					else if(ispitur_50)
					{
						dir = r_up; break;
					}
					else if(ispitbr_50)
					{
						dir = r_down; break;
					}
				}
				break;
			}
			case 1:
			{
				if(ispitul && ispitul_50) //UL_1
				{
					dir = l_up; break;
				}
				if(ispitur && ispitur_50) //UR_1
				{
					dir = r_up; break;
				}
				if(ispitbl && ispitbl_50) //BL_1
				{
					dir = l_down; break;
				}
				if(ispitbr && ispitbr_50) //BR_1
				{
					dir = r_down; break;
				}
				break;
			}
		}
		if(dir == -1) return 0; //Not falling
		has_fallen = true;
		switch(dir)
		{
			case l_up: case l_down: case left:
				--x; break;
			case r_up: case r_down: case right:
				++x; break;
		}
		switch(dir)
		{
			case l_up: case r_up: case up:
				--y; break;
			case l_down: case r_down: case down:
				++y; break;
		}
	}
	if(has_fallen)
	{
		int old_fall = fallclk; //sanity check
		fallclk = PITFALL_FALL_FRAMES;
		old_cset = cs;
		if(ispitul_50) return ispitul_50;
		if(ispitur_50) return ispitur_50;
		if(ispitbl_50) return ispitbl_50;
		if(ispitbr_50) return ispitbr_50;
		if(ispitul) return ispitul;
		if(ispitur) return ispitur;
		if(ispitbl) return ispitbl;
		if(ispitbr) return ispitbr;
		fall = old_fall; //sanity check
	}
	return 0;
}

bool sprite::hit(sprite *s)
{
    if(!(scriptcoldet&1) || fallclk) return false;
    
    if(id<0 || s->id<0 || clk<0) return false;
    
    if(halt)
    {
    }
    
    return hit(s->x+s->hxofs,s->y+s->hyofs,s->z+s->zofs,s->hxsz,s->hysz,s->hzsz);
}

bool sprite::hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
{
    if(!(scriptcoldet&1) || fallclk) return false;
    
    if(id<0 || clk<0) return false;
    
    return tx+txsz2>x+hxofs &&
           ty+tysz2>y+hyofs &&
           tz+tzsz2>z+zofs &&
           
           tx<x+hxofs+hxsz &&
           ty<y+hyofs+hysz &&
           tz<z+zofs+hzsz;
}

int sprite::hitdir(int tx,int ty,int txsz2,int tysz2,int dir2)
{
    if(!(scriptcoldet&1) || fallclk) return 0xFF;
    
    int cx1=x+hxofs+(hxsz>>1);
    int cy1=y+hyofs+(hysz>>1);
    int cx2=tx+(txsz2>>1);
    int cy2=ty+(tysz2>>1);
    
    if(dir2>=left && abs(cy1-cy2)<=8)
        return (cx2-cx1<0)?left:right;
        
    return (cy2-cy1<0)?up:down;
}

void sprite::move(zfix dx,zfix dy)
{
    x+=dx;
    y+=dy;
}

void sprite::move(zfix s)
{
    if(angular)
    {
        x += cos(angle)*s;
        y += sin(angle)*s;
		return;
    }
    
    switch(dir)
    {
    case 8:
    case up:
        y-=s;
        break;
        
    case 12:
    case down:
        y+=s;
        break;
        
    case 14:
    case left:
        x-=s;
        break;
        
    case 10:
    case right:
        x+=s;
        break;
        
    case 15:
    case l_up:
        x-=s;
        y-=s;
        break;
        
    case 9:
    case r_up:
        x+=s;
        y-=s;
        break;
        
    case 13:
    case l_down:
        x-=s;
        y+=s;
        break;
        
    case 11:
    case r_down:
        x+=s;
        y+=s;
        break;
        
    case -1:
        break;
    }
}

bool sprite::knockback(int time, int dir, int speed)
{
	if(knockbackflags & FLAG_NOSCRIPTKNOCKBACK) return false;
	script_knockback_clk = (time&0xFF) | ((dir&0xFF)<<8);
	script_knockback_speed = speed;
	return true;
}

bool sprite::runKnockback()
{
	return false; //Virtual; must be overridden for each class, for proper collision checking.
}
//Drawing with scripttile and scriptflip
 //sprite::draw() before adding scripttile and scriptflip

 //sprite::draw() before adding scripttile and scriptflip
//To quote Jeff Goldblum, 'That is one big pile opf shit!'. -Z (5th April, 2019)
void sprite::draw(BITMAP* dest)
{
	if(!show_sprites)
	{
		return;
	}
	if (FFCore.getQuestHeaderInfo(0) < 0x255 || ( FFCore.getQuestHeaderInfo(0) == 0x255 && FFCore.getQuestHeaderInfo(2) < 42 ))
	{
		drawzcboss(dest);
		return; //don't run the rest, use the old code
	}
	if ( get_bit(quest_rules,qr_OLDSPRITEDRAWS) || (drawflags&sprdrawflagALWAYSOLDDRAWS) ) 
	{
		drawzcboss(dest);
		return; //don't run the rest, use the old code
	}
	int sx = real_x(x+xofs);
	int sy = real_y(y+yofs)-real_z(z+zofs);
	
    
	if(id<0)
	{
		return;
        }
	BITMAP* sprBMP2 = create_bitmap_ex(8,256,256); //run after above failsafe, so that we always destroy it
	int e = extend>=3 ? 3 : extend;
	int flip_type = ((scriptflip > -1) ? scriptflip : flip);
	if(clk>=0)
	{
		switch(e)
		{
			BITMAP *temp;
            
			case 1:
				temp = create_bitmap_ex(8,16,32);
				//blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
				clear_bitmap(temp);
				if ( sprBMP2 ) clear_bitmap(sprBMP2);
            
				//Draw sprite tiles to the temp (scratch) bitmap.
				if(drawstyle==0 || drawstyle==3)
				{
					overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip));
					overtile16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
				}
            
				if(drawstyle==1)
				{
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
				}
            
				if(drawstyle==2)
				{
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,((scriptflip > -1) ? scriptflip : flip));
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),0,16,((scriptflip > -1) ? scriptflip : flip));
				}
				//Blit to the screen...
				if ( rotation )
				{	
					//First rotating and scaling as needed to a scratch-bitmap.
					if ( scale ) 
					{
						double new_scale = scale / 100.0;
						rotate_scaled_sprite(sprBMP2, temp, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
					}
					else rotate_sprite(sprBMP2, temp, 0, 0, deg_to_fixed(rotation));
					draw_sprite(dest, sprBMP2, sx, sy);
				}
				else
				{
					if ( scale ) 
					{
						double new_scale = scale / 100.0;
						rotate_scaled_sprite(sprBMP2, temp, 0, 0, deg_to_fixed(0),ftofix(new_scale));
						draw_sprite(dest, sprBMP2, sx, sy);
					}
					else masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
				}
				//clean-up
				if ( sprBMP2 ) destroy_bitmap(sprBMP2);
				destroy_bitmap(temp);
				break;
            
			case 2:
				temp = create_bitmap_ex(8,48,32);
				//blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
				clear_bitmap(temp);
				clear_bitmap(sprBMP2);
            
				if(drawstyle==0 || drawstyle==3)
				{
					overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip));
					overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,0,cs,((scriptflip > -1) ? scriptflip : flip));
					overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,0,cs,((scriptflip > -1) ? scriptflip : flip));
					overtile16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip));
					overtile16(temp,((scripttile > -1) ? scripttile : tile)-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
					overtile16(temp,((scripttile > -1) ? scripttile : tile)+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,16,cs,((scriptflip > -1) ? scriptflip : flip));
				}
            
				if(drawstyle==1)
				{
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
					overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
				}
				    
				if(drawstyle==2)
				{
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,((scriptflip > -1) ? scriptflip : flip));
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,0,((scriptflip > -1) ? scriptflip : flip));
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,0,((scriptflip > -1) ? scriptflip : flip));
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),16,16,((scriptflip > -1) ? scriptflip : flip));
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,16,((scriptflip > -1) ? scriptflip : flip));
					overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,16,((scriptflip > -1) ? scriptflip : flip));
				}
				if ( rotation )
				{
			
					if ( scale ) 
					{
						double new_scale = scale / 100.0;
						rotate_scaled_sprite(sprBMP2, temp, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
					}
					else rotate_sprite(sprBMP2, temp, 0, 0, deg_to_fixed(rotation));
					draw_sprite(dest, sprBMP2, sx, sy);
					
				}
				else
				{
					if ( scale ) 
					{
						double new_scale = scale / 100.0;
						rotate_scaled_sprite(sprBMP2, temp, 0, 0, deg_to_fixed(0),ftofix(new_scale));
						draw_sprite(dest, sprBMP2, sx, sy);
					}
					else masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
				}
					
				
				destroy_bitmap(sprBMP2);
				destroy_bitmap(temp);
				break;
            
			case 3:
			{
				int tileToDraw;
            
				switch(flip_type)
				{
					case 1:
					{
						BITMAP* sprBMP = create_bitmap_ex(8,txsz*16,tysz*16);
						//BITMAP* sprBMP2 = create_bitmap_ex(8,256,256);
						clear_bitmap(sprBMP);
						clear_bitmap(sprBMP2);
						for(int i=0; i<tysz; i++)
						{
							for(int j=txsz-1; j>=0; j--)
							{
								tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
							
								if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
									tileToDraw+=TILES_PER_ROW*(tysz-1);
							    
								if(drawstyle==0 || drawstyle==3) overtile16(sprBMP,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,((scriptflip > -1) ? scriptflip : flip));
								else if(drawstyle==1) overtiletranslucent16(sprBMP,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
								else if(drawstyle==2) overtilecloaked16(sprBMP,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,((scriptflip > -1) ? scriptflip : flip));
							}
						}
						if ( rotation )
						{
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
							}
							else rotate_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation));
							draw_sprite(dest, sprBMP2, sx, sy);
							
						}
						else
						{
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(0),ftofix(new_scale));
								draw_sprite(dest, sprBMP2, sx, sy);
							}
							else draw_sprite(dest, sprBMP, sx, sy);
						}
							
						destroy_bitmap(sprBMP);
						destroy_bitmap(sprBMP2);
					} //end extend == 3 && flip == 1
					break;
                
					case 2:
					{
						BITMAP* sprBMP = create_bitmap_ex(8,txsz*16,tysz*16);
						//BITMAP* sprBMP2 = create_bitmap_ex(8,256,256);
						clear_bitmap(sprBMP);
						clear_bitmap(sprBMP2);
						for(int i=tysz-1; i>=0; i--)
						{
							for(int j=0; j<txsz; j++)
							{
								tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
							
								if(tileToDraw%TILES_PER_ROW<j)
									tileToDraw+=TILES_PER_ROW*(tysz-1);
							    
								if(drawstyle==0 || drawstyle==3) overtile16(sprBMP,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip));
								else if(drawstyle==1) overtiletranslucent16(sprBMP,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
								else if(drawstyle==2) overtilecloaked16(sprBMP,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,((scriptflip > -1) ? scriptflip : flip));
							}
						}
						if ( rotation )
						{
							
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
							}
							else rotate_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation));
							draw_sprite(dest, sprBMP2, sx, sy);
							
						}
						else
						{
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(0),ftofix(new_scale));
								draw_sprite(dest, sprBMP2, sx, sy);
							}
							else draw_sprite(dest, sprBMP, sx, sy);
						}
							
						destroy_bitmap(sprBMP);
						destroy_bitmap(sprBMP2);
					}//end extend == 3 &7 flip == 2
					break;
                
					case 3:
					{
						BITMAP* sprBMP = create_bitmap_ex(8,txsz*16,tysz*16);
						//BITMAP* sprBMP2 = create_bitmap_ex(8,256,256);
						clear_bitmap(sprBMP);
						clear_bitmap(sprBMP2);
						for(int i=tysz-1; i>=0; i--)
						{
							for(int j=txsz-1; j>=0; j--)
							{
								tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
							
								if(tileToDraw%TILES_PER_ROW<j)
									tileToDraw+=TILES_PER_ROW*(tysz-1);
							    
								if(drawstyle==0 || drawstyle==3) overtile16(sprBMP,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip));
								else if(drawstyle==1) overtiletranslucent16(sprBMP,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
								else if(drawstyle==2) overtilecloaked16(sprBMP,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,((scriptflip > -1) ? scriptflip : flip));
							}
							    
						}
						if ( rotation )
						{
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
							}
							else rotate_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation));
							draw_sprite(dest, sprBMP2, sx, sy);
							
						}
						else
						{
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(0),ftofix(new_scale));
								draw_sprite(dest, sprBMP2, sx, sy);
							}
							else draw_sprite(dest, sprBMP, sx, sy);
						}
							
						destroy_bitmap(sprBMP);
						destroy_bitmap(sprBMP2);
					} //end extend == 3 && flip == 3
					break;
                
					case 0:
					{
						BITMAP* sprBMP = create_bitmap_ex(8,txsz*16,tysz*16);
						//BITMAP* sprBMP2 = create_bitmap_ex(8,256,256);
						clear_bitmap(sprBMP);
						clear_bitmap(sprBMP2);
					
						
						
						for(int i=0; i<tysz; i++)
						{
							for(int j=0; j<txsz; j++)
							{
								tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;

								if(tileToDraw%TILES_PER_ROW<j)
									tileToDraw+=TILES_PER_ROW*(tysz-1);

								if(drawstyle==0 || drawstyle==3) overtile16(sprBMP,tileToDraw,0+j*16,0+i*16,cs,((scriptflip > -1) ? scriptflip : flip));
								else if(drawstyle==1) overtiletranslucent16(sprBMP,tileToDraw,0+j*16,0+i*16,cs,((scriptflip > -1) ? scriptflip : flip),128);
								else if(drawstyle==2) overtilecloaked16(sprBMP,tileToDraw,0+j*16,0+i*16,((scriptflip > -1) ? scriptflip : flip));
							}
						}
						//rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, 0,ftofix(new_scale));
						if ( rotation )
						{
							
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
							}
							else rotate_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation));
							draw_sprite(dest, sprBMP2, sx, sy);
							
						}
						else
						{
							if ( scale ) 
							{
								double new_scale = scale / 100.0;
								rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(0),ftofix(new_scale));
								draw_sprite(dest, sprBMP2, sx, sy);
							}
							else draw_sprite(dest, sprBMP, sx, sy);
						}
						
						destroy_bitmap(sprBMP);
						destroy_bitmap(sprBMP2);
                
						break;
					} //end extend == 0 && flip == 3
				}
				
				//This was designed to fall-through in some cases. I tried to fix this, and it made a whopping mess.
				//so, I left it alone anc worked with what he have. -Z ( 5th April, 2019 )
				case 0:  //extend == 0
				default:
				{
					if(e) break; //Don't draw if the sprite is extended. We already drew it. 
					//Not doing this causes the UL corner of a larger sprite to draw, on top of an existing sprite. 
					//IDK why that was done, but it's not going to happen, now. -Z ( 5th April, 2019 )
					BITMAP* sprBMP = create_bitmap_ex(8,txsz*16,tysz*16);
					//BITMAP* sprBMP2 = create_bitmap_ex(8,256,256);
					clear_bitmap(sprBMP);
					if ( sprBMP2 ) clear_bitmap(sprBMP2);
					if(drawstyle==0 || drawstyle==3)
						overtile16(sprBMP,(scripttile > -1 ? scripttile : tile),0,0,cs,((scriptflip > -1) ? scriptflip : flip));
					else if(drawstyle==1)
						overtiletranslucent16(sprBMP,(scripttile > -1 ? scripttile : tile),0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
					else if(drawstyle==2)
						overtilecloaked16(sprBMP,(scripttile > -1 ? scripttile : tile),0,0,((scriptflip > -1) ? scriptflip : flip));
					
					if ( rotation )
					{
						
						if ( scale ) 
						{
							double new_scale = scale / 100.0;
							rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation),ftofix(new_scale));
						}
						else rotate_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(rotation));
						draw_sprite(dest, sprBMP2, sx, sy);
						
					}
					else
					{
						if ( scale ) 
						{
							double new_scale = scale / 100.0;
							rotate_scaled_sprite(sprBMP2, sprBMP, 0, 0, deg_to_fixed(0),ftofix(new_scale));
							draw_sprite(dest, sprBMP2, sx, sy);
						}
						else draw_sprite(dest, sprBMP, sx, sy);
					}
					if ( sprBMP ) destroy_bitmap(sprBMP);
					if ( sprBMP2 ) destroy_bitmap(sprBMP2);
					break;
				}
			} //end extend == 3, and also extend == 0. Why? Because someone was more mental, than me. -Z (5th April, 2019)
			break; //Aye, we break switch(e) here.
			if ( temp ) 
			{
				//if there is still somehow data in the temp bitmap
				destroy_bitmap(temp);
			}
		}
	} //end if(clk>=0)
	else //I'm unsure when the clk is < 0 -Z
	{
		if(e!=3) //if extend != 3 
		{
			int t  = wpnsbuf[iwSpawn].newtile;
			int cs2 = wpnsbuf[iwSpawn].csets&15;
            
			if(BSZ)
			{
				if(clk>=-10) ++t;
                
				if(clk>=-5) ++t;
			}
			else
			{
				if(clk>=-12) ++t;
                
				if(clk>=-6) ++t;
			}
            
			overtile16(dest,t,sx,sy,cs2,0);
		}
		else //extend == 3?
		{
			sprite w((zfix)sx,(zfix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
			w.xofs = xofs;
			w.yofs = yofs;
			w.zofs = zofs;
			w.txsz = txsz;
			w.tysz = tysz;
			w.extend = 3;
            
			if ( w.scripttile <= -1 ) 
			{
				if(BSZ)
				{
					if(clk>=-10)
					{
						if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
							w.tile+=txsz;
						else
							w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
					}
				
					if(clk>=-5)
					{
						if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
							w.tile+=txsz;
						else
							w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
					}
				}
				else
				{
					if(clk>=-12)
					{
						if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
							w.tile+=txsz;
						else
							w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
					}
			
					if(clk>=-6)
					{
						if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
							w.tile+=txsz;
						else
							w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
					}
				}
			}
            
			w.draw(dest);
		}
	}
    
	if(show_hitboxes && !is_zquest())
		rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));

	if ( sprBMP2 ) 
	{
		//if there is still somehow data in the scaling bitmap
		destroy_bitmap(sprBMP2);
	}
	
}


//Z1 bosses draw tiles from guys.cpp, direct to the 'dest'
void sprite::drawzcboss(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
		int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
		if ( scripttile > -1 )
		{
			overtile16(temp,vbound(scripttile-TILES_PER_ROW,0,NEWMAXTILES),0,16,cs,tmpflip);
			overtile16(temp,vbound(scripttile,0,NEWMAXTILES),0,16,cs,tmpflip);
		}
		else
		{
			overtile16(temp,tile-TILES_PER_ROW,0,0,cs,tmpflip);
			overtile16(temp,tile,0,16,cs,tmpflip);
		}
            }
            
            if(drawstyle==1)
            {
		int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
		if ( scripttile > -1 )
		{
			overtiletranslucent16(temp,vbound(scripttile-TILES_PER_ROW,0,NEWMAXTILES),0,16,cs,tmpflip,128);
			overtiletranslucent16(temp,vbound(scripttile,0,NEWMAXTILES),0,16,cs,tmpflip,128);
		}
		else
		{
			overtiletranslucent16(temp,tile-TILES_PER_ROW,0,0,cs,tmpflip,128);
			overtiletranslucent16(temp,tile,0,16,cs,tmpflip,128);
		}
            }
            
            if(drawstyle==2)
            {
		int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
		if ( scripttile > -1 )
		{
			overtilecloaked16(temp,vbound(scripttile-TILES_PER_ROW,0,NEWMAXTILES),0,16,tmpflip);
			overtilecloaked16(temp,vbound(scripttile,0,NEWMAXTILES),0,16,tmpflip);
		}
		else
		{
			overtilecloaked16(temp,tile-TILES_PER_ROW,0,0,tmpflip);
			overtilecloaked16(temp,tile,0,16,tmpflip);
		}
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
		if ( scripttile > -1 )
		{
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
			overtile16(temp,vbound(scripttile-TILES_PER_ROW,0,NEWMAXTILES),16,0,cs,tmpflip);
			overtile16(temp,vbound(scripttile-TILES_PER_ROW-(tmpflip?-1:1),0,NEWMAXTILES),0,0,cs,tmpflip);
			overtile16(temp,vbound(scripttile-TILES_PER_ROW+(tmpflip?-1:1),0,NEWMAXTILES),32,0,cs,tmpflip);
			overtile16(temp,vbound(scripttile,0,NEWMAXTILES),16,16,cs,tmpflip);
			overtile16(temp,vbound(scripttile-(tmpflip?-1:1),0,NEWMAXTILES),0,16,cs,tmpflip);
			overtile16(temp,vbound(scripttile+(tmpflip?-1:1),0,NEWMAXTILES),32,16,cs,tmpflip);
		}   
		else
		{
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
			overtile16(temp,tile-TILES_PER_ROW,16,0,cs,tmpflip);
			overtile16(temp,tile-TILES_PER_ROW-(tmpflip?-1:1),0,0,cs,tmpflip);
			overtile16(temp,tile-TILES_PER_ROW+(tmpflip?-1:1),32,0,cs,tmpflip);
			overtile16(temp,tile,16,16,cs,tmpflip);
			overtile16(temp,tile-(tmpflip?-1:1),0,16,cs,tmpflip);
			overtile16(temp,tile+(tmpflip?-1:1),32,16,cs,tmpflip);
		}
            }
            
            if(drawstyle==1)
            {
		if ( scripttile > -1 )
		{
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
			overtiletranslucent16(temp,vbound(scripttile-TILES_PER_ROW,0,NEWMAXTILES),16,0,cs,tmpflip,128);
			overtiletranslucent16(temp,vbound(scripttile-TILES_PER_ROW-(tmpflip?-1:1),0,NEWMAXTILES),0,0,cs,tmpflip,128);
			overtiletranslucent16(temp,vbound(scripttile-TILES_PER_ROW+(tmpflip?-1:1),0,NEWMAXTILES),32,0,cs,tmpflip,128);
			overtiletranslucent16(temp,vbound(scripttile,0,NEWMAXTILES),16,16,cs,tmpflip,128);
			overtiletranslucent16(temp,vbound(scripttile-(tmpflip?-1:1),0,NEWMAXTILES),0,16,cs,tmpflip,128);
			overtiletranslucent16(temp,vbound(scripttile+(tmpflip?-1:1),0,NEWMAXTILES),32,16,cs,tmpflip,128);
		}    
		else
		{
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
			overtiletranslucent16(temp,tile-TILES_PER_ROW,16,0,cs,tmpflip,128);
			overtiletranslucent16(temp,tile-TILES_PER_ROW-(tmpflip?-1:1),0,0,cs,tmpflip,128);
			overtiletranslucent16(temp,tile-TILES_PER_ROW+(tmpflip?-1:1),32,0,cs,tmpflip,128);
			overtiletranslucent16(temp,tile,16,16,cs,tmpflip,128);
			overtiletranslucent16(temp,tile-(tmpflip?-1:1),0,16,cs,tmpflip,128);
			overtiletranslucent16(temp,tile+(tmpflip?-1:1),32,16,cs,tmpflip,128);
		}
            }
            
            if(drawstyle==2)
            {
		if ( scripttile > -1 )
		{
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
			overtilecloaked16(temp,vbound(scripttile-TILES_PER_ROW,0,NEWMAXTILES),16,0,tmpflip);
			overtilecloaked16(temp,vbound(scripttile-TILES_PER_ROW-(tmpflip?-1:1),0,NEWMAXTILES),0,0,tmpflip);
			overtilecloaked16(temp,vbound(scripttile-TILES_PER_ROW+(tmpflip?-1:1),0,NEWMAXTILES),32,0,tmpflip);
			overtilecloaked16(temp,vbound(scripttile,0,NEWMAXTILES),16,16,tmpflip);
			overtilecloaked16(temp,vbound(scripttile-(tmpflip?-1:1),0,NEWMAXTILES),0,16,tmpflip);
			overtilecloaked16(temp,vbound(scripttile+(tmpflip?-1:1),0,NEWMAXTILES),32,16,tmpflip);
		}    
		else
		{
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
			overtilecloaked16(temp,tile-TILES_PER_ROW,16,0,tmpflip);
			overtilecloaked16(temp,tile-TILES_PER_ROW-(tmpflip?-1:1),0,0,tmpflip);
			overtilecloaked16(temp,tile-TILES_PER_ROW+(tmpflip?-1:1),32,0,tmpflip);
			overtilecloaked16(temp,tile,16,16,tmpflip);
			overtilecloaked16(temp,tile-(tmpflip?-1:1),0,16,tmpflip);
			overtilecloaked16(temp,tile+(tmpflip?-1:1),32,16,tmpflip);
		}
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            
            switch(flip)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw = vbound( (scripttile > -1 ) ? ( scripttile+(i*TILES_PER_ROW)+j ) : (tile+(i*TILES_PER_ROW)+j) ,0, NEWMAXTILES );
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
			
			tileToDraw = vbound(tileToDraw, 0, NEWMAXTILES);
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,tmpflip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,tmpflip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,tmpflip);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw = vbound( (scripttile > -1 ) ? ( scripttile+(i*TILES_PER_ROW)+j ) : (tile+(i*TILES_PER_ROW)+j) ,0, NEWMAXTILES );
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
			
			tileToDraw = vbound(tileToDraw, 0, NEWMAXTILES);
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,tmpflip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,tmpflip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,tmpflip);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw = vbound( (scripttile > -1 ) ? ( scripttile+(i*TILES_PER_ROW)+j ) : (tile+(i*TILES_PER_ROW)+j) ,0, NEWMAXTILES );
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
			
			tileToDraw = vbound(tileToDraw, 0, NEWMAXTILES);
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,tmpflip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,tmpflip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,tmpflip);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw = vbound( (scripttile > -1 ) ? ( scripttile+(i*TILES_PER_ROW)+j ) : (tile+(i*TILES_PER_ROW)+j) ,0, NEWMAXTILES );
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
			
			tileToDraw = vbound(tileToDraw, 0, NEWMAXTILES);
			int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,tmpflip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,tmpflip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,tmpflip);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
		{
		    int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                    overtile16(dest,vbound(((scripttile > -1) ? scripttile : tile), 0, NEWMAXTILES),sx,sy,cs,tmpflip);
		}
                else if(drawstyle==1)
		{
		    int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                    overtiletranslucent16(dest,vbound(((scripttile > -1) ? scripttile : tile), 0, NEWMAXTILES),sx,sy,cs,tmpflip,128);
		}
                else if(drawstyle==2)
		{
		    int tmpflip = ( (scriptflip > -1) ? scriptflip : flip );
                    overtilecloaked16(dest,vbound(((scripttile > -1) ? scripttile : tile), 0, NEWMAXTILES),sx,sy,tmpflip);
		}
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].newtile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((zfix)sx,(zfix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            
            if(BSZ)
            {
                if(clk>=-10)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-5)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            else
            {
                if(clk>=-12)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-6)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}



void sprite::old_draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    int flip_type = ((scriptflip > -1) ? scriptflip : flip);
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,0,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),0,16,((scriptflip > -1) ? scriptflip : flip));
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,0,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,16,cs,((scriptflip > -1) ? scriptflip : flip));
                overtile16(temp,((scripttile > -1) ? scripttile : tile)+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,16,cs,((scriptflip > -1) ? scriptflip : flip));
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,0,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile),16,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
                overtiletranslucent16(temp,((scripttile > -1) ? scripttile : tile)+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,16,cs,((scriptflip > -1) ? scriptflip : flip),128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW,16,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-TILES_PER_ROW+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,0,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile),16,16,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)-( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),0,16,((scriptflip > -1) ? scriptflip : flip));
                overtilecloaked16(temp,((scripttile > -1) ? scripttile : tile)+( ( scriptflip > -1 ) ? ( scriptflip ? -1 : 1 ) : ( flip?-1:1 ) ),32,16,((scriptflip > -1) ? scriptflip : flip));
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            switch(flip_type)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,flip_type);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,flip_type);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,flip_type);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=((scripttile > -1) ? scripttile : tile)+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip_type);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip_type,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,flip_type);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,flip_type);
                else if(drawstyle==1)
                    overtiletranslucent16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,flip_type,128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,flip_type);
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].newtile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((zfix)sx,(zfix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            if ( w.scripttile <= -1 ) 
	    {
		    if(BSZ)
		    {
			if(clk>=-10)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
			
			if(clk>=-5)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
		    }
		    else
		    {
			if(clk>=-12)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
			
			if(clk>=-6)
			{
			    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
				w.tile+=txsz;
			    else
				w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
			}
		    }
	    }   
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}

void sprite::draw8(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        switch(drawstyle)
        {
        case 0:                                               //normal
            overtile8(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,((scriptflip > -1) ? scriptflip : flip));
            break;
            
        case 1:                                               //phantom
            overtiletranslucent8(dest,((scripttile > -1) ? scripttile : tile),sx,sy,cs,((scriptflip > -1) ? scriptflip : flip),128);
            break;
        }
    }
}

void sprite::drawcloaked(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        overtilecloaked16(dest,((scripttile > -1) ? scripttile : tile),sx,sy,((scriptflip > -1) ? scriptflip : flip));
    }
    else
    {
        int t  = wpnsbuf[iwSpawn].newtile;
        int cs2 = wpnsbuf[iwSpawn].csets&15;
        
        if(BSZ)
        {
            if(clk>=-10) ++t;
            
            if(clk>=-5) ++t;
        }
        else
        {
            if(clk>=-12) ++t;
            
            if(clk>=-6) ++t;
        }
        
        overtile16(dest,t,x,sy,cs2,0);
    }
    
    if(get_debug() && key[KEY_O])
        rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(id));
}


/* //sprite::draw() before adding scripttile and scriptflip
void sprite::draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,0,0,cs,flip);
                overtile16(temp,tile,0,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,0,0,cs,flip,128);
                overtiletranslucent16(temp,tile,0,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,0,0,flip);
                overtilecloaked16(temp,tile,0,16,flip);
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,16,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip);
                overtile16(temp,tile,16,16,cs,flip);
                overtile16(temp,tile-(flip?-1:1),0,16,cs,flip);
                overtile16(temp,tile+(flip?-1:1),32,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,16,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip,128);
                overtiletranslucent16(temp,tile,16,16,cs,flip,128);
                overtiletranslucent16(temp,tile-(flip?-1:1),0,16,cs,flip,128);
                overtiletranslucent16(temp,tile+(flip?-1:1),32,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,16,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,flip);
                overtilecloaked16(temp,tile,16,16,flip);
                overtilecloaked16(temp,tile-(flip?-1:1),0,16,flip);
                overtilecloaked16(temp,tile+(flip?-1:1),32,16,flip);
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            
            switch(flip)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,flip);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,flip);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,tile,sx,sy,cs,flip);
                else if(drawstyle==1)
                    overtiletranslucent16(dest,tile,sx,sy,cs,flip,128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,tile,sx,sy,flip);
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].newtile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((zfix)sx,(zfix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            
            if(BSZ)
            {
                if(clk>=-10)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-5)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            else
            {
                if(clk>=-12)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-6)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}

void sprite::old_draw(BITMAP* dest)
{
    if(!show_sprites)
    {
        return;
    }
    
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    int e = extend>=3 ? 3 : extend;
    
    if(clk>=0)
    {
        switch(e)
        {
            BITMAP *temp;
            
        case 1:
            temp = create_bitmap_ex(8,16,32);
            blit(dest, temp, sx, sy-16, 0, 0, 16, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,0,0,cs,flip);
                overtile16(temp,tile,0,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,0,0,cs,flip,128);
                overtiletranslucent16(temp,tile,0,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,0,0,flip);
                overtilecloaked16(temp,tile,0,16,flip);
            }
            
            masked_blit(temp, dest, 0, 0, sx, sy-16, 16, 32);
            destroy_bitmap(temp);
            break;
            
        case 2:
            temp = create_bitmap_ex(8,48,32);
            blit(dest, temp, sx-16, sy-16, 0, 0, 48, 32);
            
            if(drawstyle==0 || drawstyle==3)
            {
                overtile16(temp,tile-TILES_PER_ROW,16,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip);
                overtile16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip);
                overtile16(temp,tile,16,16,cs,flip);
                overtile16(temp,tile-(flip?-1:1),0,16,cs,flip);
                overtile16(temp,tile+(flip?-1:1),32,16,cs,flip);
            }
            
            if(drawstyle==1)
            {
                overtiletranslucent16(temp,tile-TILES_PER_ROW,16,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,cs,flip,128);
                overtiletranslucent16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,cs,flip,128);
                overtiletranslucent16(temp,tile,16,16,cs,flip,128);
                overtiletranslucent16(temp,tile-(flip?-1:1),0,16,cs,flip,128);
                overtiletranslucent16(temp,tile+(flip?-1:1),32,16,cs,flip,128);
            }
            
            if(drawstyle==2)
            {
                overtilecloaked16(temp,tile-TILES_PER_ROW,16,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW-(flip?-1:1),0,0,flip);
                overtilecloaked16(temp,tile-TILES_PER_ROW+(flip?-1:1),32,0,flip);
                overtilecloaked16(temp,tile,16,16,flip);
                overtilecloaked16(temp,tile-(flip?-1:1),0,16,flip);
                overtilecloaked16(temp,tile+(flip?-1:1),32,16,flip);
            }
            
            masked_blit(temp, dest, 8, 0, sx-8, sy-16, 32, 32);
            destroy_bitmap(temp);
            break;
            
        case 3:
        {
            int tileToDraw;
            
            switch(flip)
            {
            case 1:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j) // Wrapped around
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+i*16,flip);
                    }
                }
                
                break;
                
            case 2:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 3:
                for(int i=tysz-1; i>=0; i--)
                {
                    for(int j=txsz-1; j>=0; j--)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+(txsz-j-1)*16,sy+(tysz-i-1)*16,flip);
                    }
                }
                
                break;
                
            case 0:
                for(int i=0; i<tysz; i++)
                {
                    for(int j=0; j<txsz; j++)
                    {
                        tileToDraw=tile+(i*TILES_PER_ROW)+j;
                        
                        if(tileToDraw%TILES_PER_ROW<j)
                            tileToDraw+=TILES_PER_ROW*(tysz-1);
                            
                        if(drawstyle==0 || drawstyle==3) overtile16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip);
                        else if(drawstyle==1) overtiletranslucent16(dest,tileToDraw,sx+j*16,sy+i*16,cs,flip,128);
                        else if(drawstyle==2) overtilecloaked16(dest,tileToDraw,sx+j*16,sy+i*16,flip);
                    }
                }
                
                break;
            }
            
            case 0:
            default:
                if(drawstyle==0 || drawstyle==3)
                    overtile16(dest,tile,sx,sy,cs,flip);
                else if(drawstyle==1)
                    overtiletranslucent16(dest,tile,sx,sy,cs,flip,128);
                else if(drawstyle==2)
                    overtilecloaked16(dest,tile,sx,sy,flip);
                    
                break;
            }
            break;
        }
    }
    else
    {
        if(e!=3)
        {
            int t  = wpnsbuf[iwSpawn].newtile;
            int cs2 = wpnsbuf[iwSpawn].csets&15;
            
            if(BSZ)
            {
                if(clk>=-10) ++t;
                
                if(clk>=-5) ++t;
            }
            else
            {
                if(clk>=-12) ++t;
                
                if(clk>=-6) ++t;
            }
            
            overtile16(dest,t,sx,sy,cs2,0);
        }
        else
        {
            sprite w((zfix)sx,(zfix)sy,wpnsbuf[extend].newtile,wpnsbuf[extend].csets&15,0,0,0);
            w.xofs = xofs;
            w.yofs = yofs;
            w.zofs = zofs;
            w.txsz = txsz;
            w.tysz = tysz;
            w.extend = 3;
            
            if(BSZ)
            {
                if(clk>=-10)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-5)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            else
            {
                if(clk>=-12)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
                
                if(clk>=-6)
                {
                    if(tile/TILES_PER_ROW==(tile+txsz)/TILES_PER_ROW)
                        w.tile+=txsz;
                    else
                        w.tile+=txsz+(tysz-1)*TILES_PER_ROW;
                }
            }
            
            w.draw(dest);
        }
    }
    
    if(show_hitboxes && !is_zquest())
        rect(dest,x+hxofs,y+playing_field_offset+hyofs-(z+zofs),x+hxofs+hxsz-1,(y+playing_field_offset+hyofs+hysz-(z+zofs))-1,vc((id+16)%255));
}

void sprite::draw8(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        switch(drawstyle)
        {
        case 0:                                               //normal
            overtile8(dest,tile,sx,sy,cs,flip);
            break;
            
        case 1:                                               //phantom
            overtiletranslucent8(dest,tile,sx,sy,cs,flip,128);
            break;
        }
    }
}

void sprite::drawcloaked(BITMAP* dest)
{
    int sx = real_x(x+xofs);
    int sy = real_y(y+yofs)-real_z(z+zofs);
    
    if(id<0)
        return;
        
    if(clk>=0)
    {
        overtilecloaked16(dest,tile,sx,sy,flip);
    }
    else
    {
        int t  = wpnsbuf[iwSpawn].newtile;
        int cs2 = wpnsbuf[iwSpawn].csets&15;
        
        if(BSZ)
        {
            if(clk>=-10) ++t;
            
            if(clk>=-5) ++t;
        }
        else
        {
            if(clk>=-12) ++t;
            
            if(clk>=-6) ++t;
        }
        
        overtile16(dest,t,x,sy,cs2,0);
    }
    
    if(get_debug() && zc_getkey(KEY_O))
        rectfill(dest,x+hxofs,sy+hyofs,x+hxofs+hxsz-1,sy+hyofs+hysz-1,vc(id));
}




*/

void sprite::drawshadow(BITMAP* dest,bool translucent)
{
    if(extend == 4 || shadowtile==0 || id<0)
    {
        return;
    }
    
    int sx = real_x(x+xofs)+(txsz-1)*8;
    int sy = real_y(y+yofs+(tysz-1)*16);
    //int sy1 = sx-56; //subscreen offset
    //if ( ispitfall(x+xofs, y+yofs+16) || ispitfall(x+xofs+8, y+yofs+16) || ispitfall(x+xofs+15, y+yofs+16)  ) return;
    //sWTF, why is this offset by half the screen. Can't do this right now. Sanity. -Z
    int shadowcs = wpnsbuf[iwShadow].csets & 0xFFFF;
    int shadowflip = wpnsbuf[iwShadow].misc & 0xFF;
    //if ( !ispitfall(sx,sy+4) && !ispitfall(sx+8,sy+4) )
    {
	    if(clk>=0)
	    {
		//zprint2("shadow sx: %d, sy: %d\n", sx, sy);
		//zprint2("enemy x: %d, y: %d\n", x.getInt(), y.getInt());
		if(translucent)
		{
		    overtiletranslucent16(dest,shadowtile,sx,sy,shadowcs,shadowflip,128);
		}
		else
		{
		    overtile16(dest,shadowtile,sx,sy,shadowcs,shadowflip);
		}
	    }
    }
}

int sprite::run_script(int mode)
{
	return RUNSCRIPT_OK; //Default implementation; override in subclasses
}
/***************************************************************************/

/**********************************/
/********** Sprite List ***********/
/**********************************/

//class enemy;

sprite_list::sprite_list() : count(0), active_iterator(0), max_sprites(255) {}
void sprite_list::clear()
{
    while(count>0) del(0);
    lastUIDRequested=0;
    lastSpriteRequested=0;
}

sprite *sprite_list::spr(int index)
{
    if(index<0 || index>=count)
        return NULL;
        
    return sprites[index];
}

bool sprite_list::swap(int a,int b)
{
    if(a<0 || a>=count || b<0 || b>=count)
        return false;
        
    sprite *c = sprites[a];
    sprites[a] = sprites[b];
    sprites[b] = c;
    containedUIDs[sprites[a]->getUID()] = a;
    containedUIDs[sprites[b]->getUID()] = b;
// checkConsistency();
    return true;
}

bool sprite_list::add(sprite *s)
{
    if(count>=max_sprites)
    {
        delete s;
        return false;
    }
    
    containedUIDs[s->getUID()] = count;
    sprites[count++]=s;
    //checkConsistency();
    return true;
}

bool sprite_list::remove(sprite *s)
// removes pointer from list but doesn't delete it
{
    if(s==lastSpriteRequested)
    {
        lastUIDRequested=0;
        lastSpriteRequested=0;
    }
    
    map<long, int>::iterator it = containedUIDs.find(s->getUID());
    
    if(it != containedUIDs.end())
        containedUIDs.erase(it);
        
    int j=0;
    
    for(; j<count; j++)
        if(sprites[j] == s)
            goto gotit;
            
    return false;
    
gotit:

    for(int i=j; i<count-1; i++)
    {
        sprites[i]=sprites[i+1];
        containedUIDs[sprites[i]->getUID()] = i;
    }
    
    --count;
    //checkConsistency();
    return true;
}

zfix sprite_list::getX(int j)
{
    if((j>=count)||(j<0))
    {
        return (zfix)1000000;
    }
    
    return sprites[j]->x;
}

zfix sprite_list::getY(int j)
{
    if((j>=count)||(j<0))
    {
        return (zfix)1000000;
    }
    
    return sprites[j]->y;
}

int sprite_list::getID(int j)
{
    if((j>=count)||(j<0))
    {
        return -1;
    }
    
    return sprites[j]->id;
}

int sprite_list::getMisc(int j)
{
    if((j>=count)||(j<0))
    {
        return -1;
    }
    
    return sprites[j]->misc;
}

bool sprite_list::del(int j)
{
    if(j<0||j>=count)
        return false;
        
    map<long, int>::iterator it = containedUIDs.find(sprites[j]->getUID());
    
    if(it != containedUIDs.end())
        containedUIDs.erase(it);
    
    if(sprites[j]==lastSpriteRequested)
    {
        lastUIDRequested=0;
        lastSpriteRequested=0;
    }
    
    delete sprites[j];
    
    for(int i=j; i<count-1; i++)
    {
        sprites[i]=sprites[i+1];
        containedUIDs[sprites[i]->getUID()] = i;
    }
    
    --count;
	if(j<=active_iterator) --active_iterator;
    //checkConsistency();
    return true;
}

void sprite_list::draw(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
        {
            sprites[i]->draw(dest);
        }
        
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
        {
            sprites[i]->draw(dest);
        }
        
        break;
    }
}

void sprite_list::drawshadow(BITMAP* dest,bool translucent, bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->drawshadow(dest,translucent);
            
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
            sprites[i]->drawshadow(dest,translucent);
            
        break;
    }
}

void sprite_list::draw2(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->draw2(dest);
            
        break;
        
    case false:
        for(int i=count-1; i>=0; i--)
            sprites[i]->draw2(dest);
            
        break;
    }
}

void sprite_list::drawcloaked2(BITMAP* dest,bool lowfirst)
{
    switch(lowfirst)
    {
    case true:
        for(int i=0; i<count; i++)
            sprites[i]->drawcloaked2(dest);
            
        break;
        
    case false:
    
        for(int i=count-1; i>=0; i--)
            sprites[i]->drawcloaked2(dest);
            
        break;
    }
}

void sprite_list::animate()
{
	active_iterator = 0;
	
	while(active_iterator<count)
	{
		if(!(freeze_guys && sprites[active_iterator]->canfreeze))
		{
			if(sprites[active_iterator]->animate(active_iterator))
			{
				del(active_iterator);
			}
		}
		
		++active_iterator;
	}
	active_iterator = -1;
}

void sprite_list::run_script(int mode)
{
	active_iterator = 0;
	
	while(active_iterator<count)
	{
		if(!(freeze_guys && sprites[active_iterator]->canfreeze))
		{
			sprites[active_iterator]->run_script(mode);
		}
		
		++active_iterator;
	}
	active_iterator = -1;
}

void sprite_list::check_conveyor()
{
    int i=0;
    
    while(i<count)
    {
        sprites[i]->check_conveyor();
        ++i;
    }
}

int sprite_list::Count()
{
    return count;
}

bool sprite_list::has_space(int space)
{
	return (count+space) <= max_sprites;
}

int sprite_list::hit(sprite *s)
{
    for(int i=0; i<count; i++)
        if(sprites[i]->hit(s))
            return i;
            
    return -1;
}

int sprite_list::hit(int x,int y,int z, int xsize, int ysize, int zsize)
{
    for(int i=0; i<count; i++)
        if(sprites[i]->hit(x,y,z,xsize,ysize,zsize))
            return i;
            
    return -1;
}

// returns the number of sprites with matching id
int sprite_list::idCount(int id, int mask)
{
    int c=0;
    
    for(int i=0; i<count; i++)
    {
        if(((sprites[i]->id)&mask) == (id&mask))
        {
            ++c;
        }
    }
    
    return c;
}

// returns index of first sprite with matching id, -1 if none found
int sprite_list::idFirst(int id, int mask)
{
    for(int i=0; i<count; i++)
    {
        if(((sprites[i]->id)&mask) == (id&mask))
        {
            return i;
        }
    }
    
    return -1;
}

// returns index of last sprite with matching id, -1 if none found
int sprite_list::idLast(int id, int mask)
{
    for(int i=count-1; i>=0; i--)
    {
        if(((sprites[i]->id)&mask) == (id&mask))
        {
            return i;
        }
    }
    
    return -1;
}

// returns the number of sprites with matching id
int sprite_list::idCount(int id)
{
    return idCount(id,0xFFFF);
}

// returns index of first sprite with matching id, -1 if none found
int sprite_list::idFirst(int id)
{
    return idFirst(id,0xFFFF);
}

// returns index of last sprite with matching id, -1 if none found
int sprite_list::idLast(int id)
{
    return idLast(id,0xFFFF);
}

sprite * sprite_list::getByUID(long uid)
{
    if(uid==lastUIDRequested)
        return lastSpriteRequested;
    
    map<long, int>::iterator it = containedUIDs.find(uid);
    
    if(it != containedUIDs.end())
    {
        // Only update cache if requested sprite was found
        lastUIDRequested=uid;
        lastSpriteRequested=spr(it->second);
        return lastSpriteRequested;
    }
        
    return NULL;
}

void sprite_list::checkConsistency()
{
    assert((int)containedUIDs.size() == count);
    assert(lastUIDRequested==0 || containedUIDs.find(lastUIDRequested)!=containedUIDs.end());
    
    for(int i=0; i<count; i++)
        assert(sprites[i] == getByUID(sprites[i]->getUID()));
}

void sprite::explode(int type)
{
	al_trace("Trying to explode enemy tile: %d\n",tile);
	
	/*
	tiledata *temptilebuf = NULL;
	memset(temptilebuf, 0, sizeof(temptilebuf));
	static int tempx, tempy;
	static byte linktilebuf[256];
	int ltile=0;
	int lflip=0;
	unpack_tile(temptilebuf, tile, flip, true);
	//unpack_tile(temptilebuf, tile, flip, true);
	//unpack_tile(temptilebuf, o_tile, 0, true);
	memcpy(linktilebuf, temptilebuf, 256);
	tempx=x;
	tempy=y;
	*/
	
	byte spritetilebuf[256];
	int ltile=0;
	int lflip=0;
	unpack_tile(newtilebuf, tile, flip, true);
	memcpy(spritetilebuf, unpackbuf, 256);	
	
	for(int i=0; i<16; ++i)
	{
                for(int j=0; j<16; ++j)
                {
                    if(spritetilebuf[i*16+j])
                    {
                        if(type==0)  // Twilight
                        {
                            particles.add(new pTwilight(x+j, y-z+i, 5, 0, 0, (rand()%8)+i*4));
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->step=3;
			    p->cset=cs;
			    p->color= rand()%4+1;
                        }
                        
			else if(type ==1)  // Sands of Hours
                        {
                            particles.add(new pTwilight(x+j, y-z+i, 5, 1, 2, (rand()%16)+i*2));
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->step=4;
                            
                            if(rand()%10 < 2)
                            {
                                p->color= rand()%4+1;
                                p->cset=cs;
                            }
                        }
                        else //explode
                        {
                            particles.add(new pFaroresWindDust(x+j, y-z+i, 5, 6, spritetilebuf[i*16+j], rand()%96));
                            
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->angular=true;
                            p->angle=rand();
                            p->step=(((double)j)/8);
                            p->yofs=yofs;
			    p->color= rand()%4+1;
			    p->cset=cs;
                        }
			
                    }
                }
	}
	
}



/*
void sprite::explode(int type)
{
	static int tempx, tempy;
	static byte linktilebuf[256];
	int ltile=0;
	int lflip=0;
	unpack_tile(newtilebuf, tile, flip, true);
	memcpy(linktilebuf, unpackbuf, 256);
	tempx=x;
	tempy=y;
	for(int i=0; i<16; ++i)
	{
                for(int j=0; j<16; ++j)
                {
                    if(linktilebuf[i*16+j])
                    {
                        if(type==0)  // Twilight
                        {
                            particles.add(new pTwilight(x+j, y-z+i, 5, 0, 0, (rand()%8)+i*4));
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->step=3;
                        }
                        else if(type ==1)  // Sands of Hours
                        {
                            particles.add(new pTwilight(x+j, y-z()+i, 5, 1, 2, (rand()%16)+i*2));
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->step=4;
                            
                            if(rand()%10 < 2)
                            {
                                p->color=1;
                                p->cset=0;
                            }
                        }
                        else
                        {
                            particles.add(new pFaroresWindDust(x+j, y-z+i, 5, 6, linktilebuf[i*16+j], rand()%96));
                            
                            int k=particles.Count()-1;
                            particle *p = (particle*)(particles.spr(k));
                            p->angular=true;
                            p->angle=rand();
                            p->step=(((double)j)/8);
                            p->yofs=yofs;
                        }
                    }
                }
	}
}
*/

/**********************************/
/********** Moving Block **********/
/**********************************/

movingblock::movingblock() : sprite()
{
    id=1;
}

void movingblock::draw(BITMAP *dest)
{
	if(fallclk)
	{
		int old_cs = cs;
		int old_tile = tile;
		
		wpndata& spr = wpnsbuf[QMisc.sprites[sprFALL]];
		cs = spr.csets & 0xF;
		int fr = spr.frames ? spr.frames : 1;
		int spd = spr.speed ? spr.speed : 1;
		int animclk = (PITFALL_FALL_FRAMES-fallclk);
		tile = spr.newtile + zc_min(animclk / spd, fr-1);
		sprite::draw(dest);
		
		cs = old_cs;
		tile = old_tile;
	}
    else if(clk)
    {
        //    sprite::draw(dest);
        overcombo(dest,real_x(x+xofs),real_y(y+yofs),bcombo ,cs);
    }
}

/*** end of sprite.cc ***/

