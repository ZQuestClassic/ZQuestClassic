const int SWORDCHARGEFRAME = 20;
const int SWORDTAPFRAME = SWORDCHARGEFRAME-8;
const int HAMMERCHARGEFRAME = 11;
const int WANDCHARGEFRAME = 12;

class LinkClass: sprite
{
    __RealLink@ realLink;
    
    LinkClass()
    {
        @realLink=__getRealLink();
        @Link=this;
    }
    
    // Properties ==============================================================
    
    // Used in checking slashable/poundable/etc. combos to remember
    // if a combo or FFC was already hit.
    array<bool> screengrid(176);
    array<bool> ffcgrid(32);
    
    bool tapping
    {
        get const { return realLink.tapping; }
        set { realLink.tapping=value; }
    }
    
    bool stomping
    {
        get const { return realLink.stomping; }
        set { realLink.stomping=value; }
    }
    
    int ladderx
    {
        get const { return realLink.ladderx; }
        set { realLink.ladderx=value; }
    }
    
    int laddery
    {
        get const { return realLink.laddery; }
        set { realLink.laddery=value; }
    }
    
    int charging
    {
        get const { return realLink.charging; }
        set { realLink.charging=value; }
    }
    
    int spins
    {
        get const { return realLink.spins; }
        set { realLink.spins=value; }
    }
    
    int drunkclk
    {
        get const { return realLink.drunkclk; }
        set { realLink.drunkclk=value; }
    }
    
    int hoverclk
    {
        get const { return realLink.hoverclk; }
        set { realLink.hoverclk=value; }
    }
    
    int hclk
    {
        get const { return realLink.hclk; }
        set { realLink.hclk=value; }
    }
    
    int attackclk
    {
        get const { return realLink.attackclk; }
        set { realLink.attackclk=value; }
    }
    
    int attack
    {
        get const { return realLink.attack; }
        set { realLink.attack=value; }
    }
    
    int attackid
    {
        get const { return realLink.attackid; }
        set { realLink.attackid=value; }
    }
    
    int drownclk
    {
        get const { return realLink.drownclk; }
        set { realLink.drownclk=value; }
    }
    
    int slashxofs
    {
        get const { return realLink.slashxofs; }
        set { realLink.slashxofs=value; }
    }
    
    int slashyofs
    {
        get const { return realLink.slashyofs; }
        set { realLink.slashyofs=value; }
    }
    
    uint8 hopclk
    {
        get const { return realLink.hopclk; }
        set { realLink.hopclk=value; }
    }
    
    uint8 diveclk
    {
        get const { return realLink.diveclk; }
        set { realLink.diveclk=value; }
    }
    
    uint8 inlikelike
    {
        get const { return realLink.inlikelike; }
        set { realLink.inlikelike=value; }
    }
    
    int shiftdir
    {
        get const { return realLink.shiftdir; }
        set { realLink.shiftdir=value; }
    }
    
    int sdir
    {
        get const { return realLink.sdir; }
        set { realLink.sdir=value; }
    }
    
    int hopdir
    {
        get const { return realLink.hopdir; }
        set { realLink.hopdir=value; }
    }
    
    int holddir
    {
        get const { return realLink.holddir; }
        set { realLink.holddir=value; }
    }
    
    int landswim
    {
        get const { return realLink.landswim; }
        set { realLink.landswim=value; }
    }
    
    bool ilswim
    {
        get const { return realLink.ilswim; }
        set { realLink.ilswim=value; }
    }
    
    bool walkable
    {
        get const { return realLink.walkable; }
        set { realLink.walkable=value; }
    }
    
    int action
    {
        get const { return realLink.action; }
        set { realLink.action=value; }
    }
    
    fix climb_cover_x
    {
        get const { return realLink.climb_cover_x; }
        set { realLink.climb_cover_x=value; }
    }
    
    fix climb_cover_y
    {
        get const { return realLink.climb_cover_y; }
        set { realLink.climb_cover_y=value; }
    }
    
    fix falling_oldy
    {
        get const { return realLink.falling_oldy; }
        set { realLink.falling_oldy=value; }
    }
    
    bool diagonalMovement
    {
        get const { return realLink.diagonalMovement; }
        set { realLink.diagonalMovement=value; }
    }
    
    // Methods =================================================================
    
    // These are still used by other scripts.
    
    void hitlink(int hit2)
    {
        realLink.hitlink(hit2);
    }
    
    void setEaten(uint8 i)
    {
        inlikelike=i;
    }
    
    int getEaten()
    {
        return inlikelike;
    }
    
    int getAction()
    {
        if(spins > 0)
            return isspinning;
        else if(charging > 0)
            return ischarging;
        else if(diveclk > 0)
            return isdiving;
            
        return action;
    }
    
    bool isSwimming()
    {
        return ((action==swimming)||(action==swimhit)||
                (action==waterhold1)||(action==waterhold2)||
                (hopclk==0xFF));
    }
    
    bool drunk()
    {
        return ((0==(frame%((rand()%100)+1)))&&(rand()%500<drunkclk));
    }

    bool DrunkUp()
    {
        return drunk()?(rand()%2!=0)?false:!Up():Up();
    }
    
    bool DrunkDown()
    {
        return drunk()?(rand()%2!=0)?false:!Down():Down();
    }
    
    bool DrunkLeft()
    {
        return drunk()?(rand()%2!=0)?false:!Left():Left();
    }
    
    bool DrunkRight()
    {
        return drunk()?(rand()%2!=0)?false:!Right():Right();
    }
    
    bool DrunkcAbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cAbtn():cAbtn();
    }
    
    bool DrunkcBbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cBbtn():cBbtn();
    }
    
    bool DrunkcSbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cSbtn():cSbtn();
    }
    
    bool DrunkcMbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cMbtn():cMbtn();
    }
    
    bool DrunkcLbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cLbtn():cLbtn();
    }
    
    bool DrunkcRbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cRbtn():cRbtn();
    }
    
    bool DrunkcPbtn()
    {
        return drunk()?(rand()%2!=0)?false:!cPbtn():cPbtn();
    }

    bool DrunkrUp()
    {
        return drunk()?(rand()%2!=0)?false:!rUp():rUp();
    }
    
    bool DrunkrDown()
    {
        return drunk()?(rand()%2!=0)?false:!rDown():rDown();
    }
    
    bool DrunkrLeft()
    {
        return drunk()?(rand()%2!=0)?false:!rLeft():rLeft();
    }
    
    bool DrunkrRight()
    {
        return drunk()?(rand()%2!=0)?false:!rRight():rRight();
    }
    
    bool DrunkrAbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rAbtn():rAbtn();
    }
    
    bool DrunkrBbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rBbtn():rBbtn();
    }
    
    bool DrunkrSbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rSbtn():rSbtn();
    }
    
    bool DrunkrMbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rMbtn():rMbtn();
    }
    
    bool DrunkrLbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rLbtn():rLbtn();
    }
    
    bool DrunkrRbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rRbtn():rRbtn();
    }
    
    bool DrunkrPbtn()
    {
        return drunk()?(rand()%2!=0)?false:!rPbtn():rPbtn();
    }
}

// The following are only used for Link damage. Damage is in quarter hearts.
int enemy_dp(int index)
{
    return getGuy(index).dp*4;
}

int ewpn_dp(int index)
{
    return getEwpn(index).power*4;
}

int lwpn_dp(int index)
{
    return getLwpn(index).power*4;
}

void paymagiccost(int itemid)
{
    if(itemid < 0)
        return;
        
    if(getItemProp(itemid, ITEMPROP_MAGIC) <= 0)
        return;
        
    if((getItemProp(itemid, ITEMPROP_FLAGS) & ITEM_RUPEE_MAGIC)!=0)
    {
        change_dcounter(-getItemProp(itemid, ITEMPROP_MAGIC), 1);
        return;
    }
    
    if(current_item_power(itype_magicring) > 0)
        return;
    
    change_counter(-(getItemProp(itemid, ITEMPROP_MAGIC)*get_magicdrainrate()), 4);
}

fix LinkX()
{
    return Link.x;
}

fix LinkY()
{
    return Link.y;
}

fix LinkZ()
{
    return Link.z;
}

int LinkDir()
{
    return Link.dir;
}

