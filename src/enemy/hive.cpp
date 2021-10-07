#include "hive.h"
#include "../link.h"

// This should probably go somewhere else...
static direction angleToDir(double angle)
{
    if(angle<=-5*PI/8 && angle>-7*PI/8)
        return l_down;
    else if(angle<=-3*PI/8 && angle>-5*PI/8)
        return left;
    else if(angle<=-1*PI/8 && angle>-3*PI/8)
        return l_up;
    else if(angle<=1*PI/8 && angle>-1*PI/8)
        return up;
    else if(angle<=3*PI/8 && angle>1*PI/8)
        return r_up;
    else if(angle<=5*PI/8 && angle>3*PI/8)
        return right;
    else if(angle<=7*PI/8 && angle>5*PI/8)
        return r_down;
    else
        return down;
}

ePatra::ePatra(zfix X, zfix Y, int Id, int Clk):
    enemy(X, Y, Id, Clk)
{
    if((editorflags&ENEMY_FLAG5)==0)
    {
        x=128;
        y=48;
    }
    else
    {
        x=X;
        y=Y;
    }

    adjusted=false;
    dir=(rand()&7)+8;
    flycnt=dmisc1;
    flycnt2=dmisc2;
    loopcnt=0;
    if(dmisc6<1)
        dmisc6=1; // ratio cannot be 0!
    SIZEflags = d->SIZEflags;
    if(SIZEflags&guyflagOVERRIDE_TILE_WIDTH && txsz>0)
    {
        txsz=txsz;
        if(txsz>1)
            extend=3;
    }
    if(SIZEflags&guyflagOVERRIDE_TILE_HEIGHT && tysz>0)
    {
        tysz=tysz;
        if(tysz>1)
            extend = 3;
    }
    if(SIZEflags&guyflagOVERRIDE_HIT_WIDTH && hxsz>=0)
        hxsz=hxsz;
    if(SIZEflags&guyflagOVERRIDE_HIT_HEIGHT && hysz>=0)
        hysz=hysz;
    if(SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT && hzsz>=0)
        hzsz=hzsz;
    if(SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET)
        hxofs=hxofs;
    if(SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET)
        hyofs=hyofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET)
        xofs=xofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET)
    {
        yofs=yofs; //This seems to be setting to +48 or something with any value set?! -Z
        yofs+=56 ; //this offset fixes yofs not plaing properly. -Z
    }
    if(SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET)
        zofs=zofs;
}

bool ePatra::animate(int index)
{
    if(dying)
    {
        for(int i=index+1; i<index+flycnt+flycnt2+1; i++)
            ((enemy*)guys.spr(i))->hp=-1000;

        return Dead(index);
    }

    if(clk==0)
        removearmos(x, y);

    variable_walk_8(rate, homing, hrate, spw_floater);

    clk2++;
    if(clk2>84)
    {
        clk2=0;

        if(loopcnt)
            loopcnt--;
        else
        {
            if(misc%dmisc6==0)
                loopcnt=dmisc7;
        }

        misc++;
    }

    double size=1;

    for(int i=index+1; i<index+flycnt+1; i++)
    {
        enemy* orbiter=(enemy*)guys.spr(i);
        //outside ring
        if(!adjusted)
        {
            if(get_bit(quest_rules,qr_NEWENEMYTILES))
            {
                orbiter->o_tile=o_tile+dmisc8;
                orbiter->parent_script_UID= this->script_UID;
            }
            else
            {
                orbiter->o_tile=o_tile+1;
                orbiter->parent_script_UID=this->script_UID;
            }

            orbiter->cs=dmisc9;
            orbiter->hp=dmisc3;
        }

        if(orbiter->hp<=0)
        {
            for(int j=i; j<index+flycnt+flycnt2; j++)
                guys.swap(j,j+1);

                --flycnt;
        }
        else
        {
            int pos2 = orbiter->misc;
            double a2 = (clk2-pos2*84.0/(dmisc1 == 0 ? 1 : dmisc1))*PI/42;

            if(!dmisc4)
            {
                //maybe playing_field_offset here?
                if(loopcnt>0)
                {
                    orbiter->x =  cos(a2+PI/2)*56*size - sin(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1))*28*size;
                    orbiter->y = -sin(a2+PI/2)*56*size + cos(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1))*28*size;
                }
                else
                {
                    orbiter->x =  cos(a2+PI/2)*28*size;
                    orbiter->y = -sin(a2+PI/2)*28*size;
                }

                temp_x=orbiter->x;
                temp_y=orbiter->y;
            }
            else
            {
                circle_x=cos(a2+PI/2)*42;
                circle_y=-sin(a2+PI/2)*42;

                if(loopcnt>0)
                {
                    orbiter->x=cos(a2+PI/2)*42;
                    orbiter->y=(-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc1 == 0 ? 1 : dmisc1)))*21;
                }
                else
                {
                    orbiter->x=circle_x;
                    orbiter->y=circle_y;
                }

                temp_x=circle_x;
                temp_y=circle_y;
            }

            orbiter->dir=angleToDir(atan2(double(temp_y), double(temp_x)));

            orbiter->x+=x;
            orbiter->y+=y;
        }
    }

    if(dmisc5==1)
    {
        if((rand()&127)==0)
        {
            addEwpn(x, y, z, wpn, 3, wdp, dir, getUID());
            sfx(wpnsfx(wpn),pan(int(x)));
        }
    }

    size=.5;

    if(flycnt2)
    {
        for(int i=index+flycnt+1; i<index+flycnt+flycnt2+1; i++)//inner ring
        {
            enemy* orbiter=(enemy*)guys.spr(i);
            if(!adjusted)
            {
                orbiter->hp=12*DAMAGE_MULTIPLIER;

                if(get_bit(quest_rules,qr_NEWENEMYTILES))
                {
                    switch(dmisc5)
                    {
                    // Center eye shoots projectiles; make room for its firing tiles
                    case 1:
                        orbiter->o_tile=o_tile+120;
                        break;

                    // Center eyes does not shoot; use tiles two rows below for inner eyes.
                    default:
                    case 2:
                        orbiter->o_tile=o_tile+40;
                        break;
                    }
                }
                else
                    orbiter->o_tile=o_tile+1;

                orbiter->cs=dmisc9;
            }

            if(flycnt>0)
                orbiter->superman=true;
            else
                orbiter->superman=false;

            if(orbiter->hp <= 0)
            {
                for(int j=i; j<index+flycnt+flycnt2; j++)
                    guys.swap(j, j+1);

                flycnt2--;
            }
            else
            {
                if(dmisc5==2)
                {
                    if((rand()&127)==0)
                    {
                        addEwpn(guys.spr(i)->x, guys.spr(i)->y, guys.spr(i)->z, wpn, 3, wdp, dir, getUID());
                        sfx(wpnsfx(wpn), pan(int(x)));
                    }
                }

                int pos2 = ((enemy*)guys.spr(i))->misc;
                double a2 = ((clk2-pos2*84/(dmisc2==0 ? 1 : dmisc2))*PI/(42));

                if(dmisc4==0)
                {
                    if(loopcnt>0)
                    {
                        orbiter->x=cos(a2+PI/2)*56*size-sin(pos2*PI*2/(dmisc2==0 ? 1 : dmisc2))*28*size;
                        orbiter->y=-sin(a2+PI/2)*56*size+cos(pos2*PI*2/(dmisc2==0 ? 1 : dmisc2))*28*size;
                    }
                    else
                    {
                        orbiter->x=cos(a2+PI/2)*28*size;
                        orbiter->y=-sin(a2+PI/2)*28*size;
                    }

                    temp_x=orbiter->x;
                    temp_y=orbiter->y;
                }
                else
                {
                    circle_x=cos(a2+PI/2)*42*size;
                    circle_y=-sin(a2+PI/2)*42*size;

                    if(loopcnt>0)
                    {
                        orbiter->x=cos(a2+PI/2)*42*size;
                        orbiter->y=(-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc2 == 0 ? 1 : dmisc2)))*21*size;
                    }
                    else
                    {
                        orbiter->x=circle_x;
                        orbiter->y=circle_y;
                    }

                    temp_x=circle_x;
                    temp_y=circle_y;
                }

                orbiter->dir=angleToDir(atan2(double(temp_y),double(temp_x)));

                orbiter->x+=x;
                orbiter->y=y-orbiter->y;

            }
        }
    }

    adjusted=true;
    return enemy::animate(index);
}

void ePatra::draw(BITMAP *dest)
{
	tile=o_tile;
	update_enemy_frame();
	enemy::draw(dest);
}

int ePatra::defend(int wpnId, int *power, int edef)
{
	int ret=enemy::defend(wpnId, power, edef);

	if(ret<0 && (flycnt || flycnt2))
		return 0;

	return ret;
}

int ePatra::defendNew(int wpnId, int *power, int edef)
{
	int ret=enemy::defendNew(wpnId, power, edef);

	if(ret<0 && (flycnt || flycnt2))
		return 0;

	return ret;
}

esPatra::esPatra(zfix X, zfix Y, int Id, int Clk, sprite* prnt):
    enemy(X, Y, Id, Clk),
    parent(prnt)
{
    item_set=0;
    misc=clk;
    clk=-((misc*21)>>1)-1;
    yofs=playing_field_offset;
    enemy* prntenemy=(enemy*)prnt;
    int prntSIZEflags=prntenemy->SIZEflags;
    if(SIZEflags&guyflagOVERRIDE_TILE_WIDTH && txsz>0)
    {
        txsz=prntenemy->txsz;
        if(txsz>1)
            extend=3;
    }
    if(SIZEflags&guyflagOVERRIDE_TILE_HEIGHT && tysz>0)
    {
        tysz=prntenemy->tysz;
        if(tysz>1)
            extend=3;
    }
    if(SIZEflags&guyflagOVERRIDE_HIT_WIDTH && hxsz>=0)
        hxsz=prntenemy->hxsz;
    else
        hxsz=12;
    if(SIZEflags&guyflagOVERRIDE_HIT_HEIGHT && hysz>=0)
        hysz=prntenemy->hysz;
    else
        hysz=12;
    if(SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT && hzsz>=0)
        hzsz=prntenemy->hzsz;
    if(SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET)
        hxofs=prntenemy->hxofs;
    else
        hxofs=2;
    if(SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET)
        hyofs=prntenemy->hyofs;
    else
        hyofs=2;
    if(SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET)
        xofs=prntenemy->xofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET)
        yofs=prntenemy->yofs; //This seems to be setting to +48 or something with any value set?! -Z

    if(SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET)
        zofs=prntenemy->zofs;

    mainguy=count_enemy=false;
    bgsfx=-1;
    flags&=~guy_neverret;
    deadsfx=WAV_EDEAD;
    hitsfx=WAV_EHIT;
    isCore=false;
}

bool esPatra::animate(int index)
{
	if(dying)
		return Dead(index);

	return enemy::animate(index);
}

void esPatra::draw(BITMAP *dest)
{
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		tile=o_tile+(clk&3);
        flip=0;
		switch(dir)
		{
		case up:
			break;

		case down:
			tile+=4;
			break;

		case left:
			tile+=8;
			break;

		case right:
			tile+=12;
			break;

		case l_up:
			tile+=20;
			break;

		case r_up:
			tile+=24;
			break;

		case l_down:
			tile+=28;
			break;

		case r_down:
			tile+=32;
			break;
		}
	}
	else
		tile=o_tile+((clk&2)>>1);

	if(clk>=0)
		enemy::draw(dest);
}


ePatraBS::ePatraBS(zfix, zfix, int Id, int Clk):
    enemy(128_x, 48_x, Id, Clk)
{
    adjusted=false;
    dir=(rand()&7)+8;
    step=0.25;
    //flycnt=6; flycnt2=0;
    flycnt=dmisc1;
    flycnt2=0; // PatraBS doesn't have inner rings!
    loopcnt=0;

    SIZEflags = d->SIZEflags;
    if(SIZEflags&guyflagOVERRIDE_TILE_WIDTH && txsz>0)
    {
        txsz=d->txsz;
        if(txsz>1)
            extend=3;
    }
    if(SIZEflags&guyflagOVERRIDE_TILE_HEIGHT && tysz>0)
    {
        tysz=d->tysz;
        if(tysz>1)
            extend = 3;
    }
    if(SIZEflags&guyflagOVERRIDE_HIT_WIDTH && hxsz>=0)
        hxsz=d->hxsz;
    else
        hxsz=32;
    if(SIZEflags&guyflagOVERRIDE_HIT_HEIGHT && hysz>=0)
        hysz=d->hysz;
    if(SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT && hzsz>=0)
        hzsz=d->hzsz;
    if(SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET)
        hxofs=d->hxofs;
    else
        hxofs=-8;
    if(SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET)
        hyofs=d->hyofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET)
        xofs=d->xofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET)
    {
        yofs=d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
        yofs+=56 ; //this offset fixes yofs not plaing properly. -Z
    }
    if(SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET)
        zofs=d->zofs;
    if(dmisc6<1)
        dmisc6=1; // ratio cannot be 0!
}

bool ePatraBS::animate(int index)
{
	if(dying)
		return Dead(index);

	if(clk==0)
		removearmos(x,y);

	variable_walk_8(rate, homing, hrate, spw_floater);

    clk2++;
	if(clk2>90)
	{
		clk2=0;

		if(loopcnt)
			loopcnt--;
		else
		{
			if(misc%dmisc6==0)
				loopcnt=dmisc7;
		}

		misc++;
	}

	for(int i=index+1; i<index+flycnt+1; i++)
	{
        enemy* orbiter=(enemy*)guys.spr(i);

		if(!adjusted)
		{
			orbiter->hp=dmisc3;

			if(get_bit(quest_rules,qr_NEWENEMYTILES))
				orbiter->o_tile=o_tile+dmisc8;
			else
				orbiter->o_tile=o_tile+1;

			orbiter->cs=dmisc9;
		}

		if(orbiter->hp<=0)
		{
			for(int j=i; j<index+flycnt+flycnt2; j++)
				guys.swap(j,j+1);

			flycnt--;
		}
		else
		{
			int pos2=orbiter->misc;
			double a2=(clk2-pos2*90/(dmisc1==0?1:dmisc1))*PI/45;
			temp_x=cos(a2+PI/2)*45;
			temp_y=-sin(a2+PI/2)*45;

			if(loopcnt>0)
			{
				orbiter->x=cos(a2+PI/2)*45;
				orbiter->y=(-sin(a2+PI/2)-cos(pos2*PI*2/(dmisc1==0 ? 1 : dmisc1)))*22.5;
			}
			else
			{
				orbiter->x=temp_x;
				orbiter->y=temp_y;
			}

			orbiter->x+=x;
			orbiter->y+=y;
            orbiter->dir=angleToDir(atan2(double(temp_y), double(temp_x)));
		}
	}

	adjusted=true;
	return enemy::animate(index);
}

void ePatraBS::draw(BITMAP *dest)
{
	tile=o_tile;

	if(get_bit(quest_rules, qr_NEWENEMYTILES))
	{
        flip=0; // XXX Would this ever be set to something nonzero?
		switch(angleToDir(atan2(double(y-(Link.y)),double(Link.x-x))))
		{
		case up:
			break;

		case down:
			tile+=8;
			break;

		case left:
			tile+=40;
			break;

		case right:
			tile+=48;
			break;

		case l_up:
			tile+=80;
			break;

		case r_up:
			tile+=88;
			break;

		case l_down:
			tile+=120;
			break;

		case r_down:
			tile+=128;
			break;
		}

		tile+=(2*(clk&3));
		xofs-=8;
		yofs-=8;
		drawblock(dest,15);
		xofs+=8;
		yofs+=8;
	}
	else
	{
		flip=(clk&1);
		xofs-=8;
		yofs-=8;
		enemy::draw(dest);
		xofs+=16;
		enemy::draw(dest);
		yofs+=16;
		enemy::draw(dest);
		xofs-=16;
		enemy::draw(dest);
		xofs+=8;
		yofs-=8;
	}
}

int ePatraBS::defend(int wpnId, int *power, int edef)
{
	int ret=enemy::defend(wpnId, power, edef);

	if(ret<0 && (flycnt || flycnt2))
		return 0;

	return ret;
}

int ePatraBS::defendNew(int wpnId, int *power, int edef)
{
	int ret = enemy::defendNew(wpnId, power, edef);

	if(ret<0 && (flycnt || flycnt2))
		return 0;

	return ret;
}

esPatraBS::esPatraBS(zfix X, zfix Y, int Id, int Clk, sprite* prnt):
    enemy(X, Y, Id, Clk),
    parent(prnt)
{
    item_set=0;
    misc=clk;
    clk=-((misc*21)>>1)-1;

    enemy* prntenemy=(enemy*)prnt;
    int prntSIZEflags=prntenemy->SIZEflags;
    if(prntSIZEflags&guyflagOVERRIDE_TILE_WIDTH && txsz>0)
    {
        txsz=prntenemy->txsz;
        if(txsz>1)
            extend=3;
    }
    if(prntSIZEflags&guyflagOVERRIDE_TILE_HEIGHT && tysz>0)
    {
        tysz=prntenemy->tysz;
        if(tysz>1)
            extend=3;
    }
    if(prntSIZEflags&guyflagOVERRIDE_HIT_WIDTH && hxsz>=0)
        hxsz=prntenemy->hxsz;
    else
        hxsz=16;
    if(prntSIZEflags&guyflagOVERRIDE_HIT_HEIGHT && hysz>=0)
        hysz=prntenemy->hysz;
    else
        hysz=16;
    if(prntSIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT && hzsz>=0)
        hzsz=prntenemy->hzsz;
    if(prntSIZEflags&guyflagOVERRIDE_HIT_X_OFFSET)
        hxofs=prntenemy->hxofs;
    if(prntSIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET)
        hyofs=prntenemy->hyofs;
    else
        hyofs=2;
    if(prntSIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET)
        xofs=prntenemy->xofs;
    if(prntSIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET)
        yofs=prntenemy->yofs;
    else
        yofs=playing_field_offset;
    if(prntSIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET)
        prntenemy->zofs=zofs;

    bgsfx=-1;
    mainguy=count_enemy=false;
    deadsfx=WAV_EDEAD;
    hitsfx=WAV_EHIT;
    flags&=~guy_neverret;
    isCore=false;
}

bool esPatraBS::animate(int index)
{
	if(dying)
		return Dead(index);

	return enemy::animate(index);
}

void esPatraBS::draw(BITMAP *dest)
{
	tile=o_tile;

	if(get_bit(quest_rules, qr_NEWENEMYTILES))
	{
        flip=0;
		switch(dir)
		{
		case up:
			break;

		case down:
			tile+=4;
			break;

		case left:
			tile+=8;
			break;

		case right:
			tile+=12;
			break;

		case l_up:
			tile+=20;
			break;

		case r_up:
			tile+=24;
			break;

		case l_down:
			tile+=28;
			break;

		case r_down:
			tile+=32;
			break;
		}

		tile+=(clk&6)>>1;
	}
	else
		tile+=(clk&4) ? 1 : 0;

	if(clk>=0)
		enemy::draw(dest);
}
