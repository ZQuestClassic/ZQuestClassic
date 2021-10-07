#include "unicorn.h"
#include "../ffscript.h"
#include "../zelda.h"

eAquamentus::eAquamentus(enemy const & other, bool new_script_uid, bool clear_parent_script_UID):
	 //Struct Element			Type		Purpose
	//sprite(other),
	enemy(other),
	walkTimer(((eAquamentus&)other).walkTimer),
    shotTimer(clk3)
{

	//arrays

	//stack(other.stack),			//int
	//scriptData(other.scriptData)			//int
	memset(stack, 0xFFFF, MAX_SCRIPT_REGISTERS * sizeof(long));
	memcpy(stack, other.stack, MAX_SCRIPT_REGISTERS * sizeof(long));

	scriptData = other.scriptData;
	//memset((refInfo)scriptData, 0xFFFF, sizeof(refInfo));
	//memset((refInfo)scriptData, other.scriptData, sizeof(refInfo));

	for(int i=0; i<edefLAST255; i++)
		defense[i]=other.defense[i];
	for ( int q = 0; q < 10; q++ ) frozenmisc[q] = other.frozenmisc[q];
	for ( int q = 0; q < NUM_HIT_TYPES_USED; q++ ) hitby[q] = other.hitby[q];

	if(new_script_uid)
	{
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs.
	}
	if(clear_parent_script_UID)
	{
		parent_script_UID = 0;
	}
	for ( int q = 0; q < 32; q++ ) movement[q] = other.movement[q];
	for ( int q = 0; q < 32; q++ ) new_weapon[q] = other.new_weapon[q];

	for ( int q = 0; q < 8; q++ )
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for ( int q = 0; q < 2; q++ )
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

eAquamentus::eAquamentus(zfix X, zfix Y, int Id, int Clk):
    enemy(X, Y, Id, Clk),
    facingDir(dmisc1 ? Facing::right : Facing::left),
    walkTimer(Clk),
    shotTimer(clk3)
{
    clk2=0;
    shotTimer=32;
    dir=left;
    yofs=playing_field_offset+1;

	if((editorflags&ENEMY_FLAG5)==0)
	{
		x=facingDir==Facing::right ? 64 : 176;
		y=64;
	}
	else
    {
        x=X;
        y=Y;
    }

	if(facingDir==Facing::right && !get_bit(quest_rules,qr_NEWENEMYTILES))
		flip=1;

    SIZEflags=d->SIZEflags;
    if(SIZEflags&guyflagOVERRIDE_TILE_WIDTH && txsz>0 )
    {
        txsz = txsz;
        if(txsz>1)
            extend = 3;
    }

    if(SIZEflags&guyflagOVERRIDE_TILE_HEIGHT && tysz>0)
    {
        tysz=d->tysz;
        if(tysz>1)
            extend = 3;
    }
    if(SIZEflags&guyflagOVERRIDE_HIT_WIDTH && hxsz>=0)
        hxsz=d->hxsz;
    if(SIZEflags&guyflagOVERRIDE_HIT_HEIGHT && hysz>=0)
        hysz=d->hysz;
    if(SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT && hzsz>=0)
        hzsz=d->hzsz;
    if(SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET)
        hxofs = d->hxofs;
    if(SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET)
        hyofs = d->hyofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET)
        xofs = d->xofs;
    if(SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET)
    {
        yofs=d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
        yofs+=56 ; //this offset fixes yofs not plaing properly. -Z
    }

    if(SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET)
        zofs=d->zofs;
}

bool eAquamentus::animate(int index)
{
    if(dying)
        return Dead(index);
    if(clk==0)
        removearmos(x,y);

    shotTimer--;
    if(shotTimer==0)
        eWpn.fire();
    // shotTimer affects animation - 32 frames of firing face
    // before actually firing
    if(shotTimer<-80 && (rand()&63)==0)
        shotTimer=32;

    // Every 64 frames, maybe turn around.
    if(((walkTimer+1)&63)==0)
    {
        if(tooFarLeft())
            dir=right;
        else if(tooFarRight())
            dir=left;
        else
        {
            int d2=(rand()%3)+1;
            if(d2>=left)
                dir=d2;
        }
    }

    // Walk one pixel every 8 frames. walkTimer is negative while spawning.
    if(walkTimer>=-1 && ((walkTimer+1)&7)==0)
    {
        if(dir==left)
            x-=1;
        else
            x+=1;
    }

    walkTimer=(walkTimer+1)%256;

    return enemy::animate(index);
}

void eAquamentus::draw(BITMAP *dest)
{
    if(get_bit(quest_rules,qr_NEWENEMYTILES))
    {
        xofs=(facingDir==Facing::right ? -16 : 0);
        if(do_animation)
            tile=o_tile+((clk&24)>>2)+(shotTimer>-32?(shotTimer>0?40:80):0);

        if(dying)
        {
            xofs=0;
            enemy::draw(dest);
        }
        else
            drawblock(dest, 15);
    }
    else
    {
        int xblockofs=(facingDir==Facing::right ? -16 : 16);
        xofs=0;

        if(clk<0 || dying)
        {
            enemy::draw(dest);
            return;
        }
        if ( do_animation )
        {
            // face (0=firing, 2=resting)
            tile=o_tile+((clk3>0)?0:2);
            enemy::draw(dest);
            // tail (
            tile=o_tile+((clk&16)?1:3);
            xofs=xblockofs;
            enemy::draw(dest);
            // body
            yofs+=16;
            xofs=0;
            tile=o_tile+((clk&16)?20:22);
            enemy::draw(dest);
            xofs=xblockofs;
            tile=o_tile+((clk&16)?21:23);
            enemy::draw(dest);
            yofs-=16;
        }
        else
            enemy::draw(dest);
    }
}

bool eAquamentus::hit(weapon *w)
{
	if((w->scriptcoldet&1)==0 || w->fallclk || w->drownclk)
        return false;

	switch(w->id)
	{
	case wBeam:
	case wRefBeam:
	case wMagic:
		hysz=32;
	}

	bool ret=(dying || hclk>0) ? false : sprite::hit(w);
	hysz=16;
	return ret;

}
