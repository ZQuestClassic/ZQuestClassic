enum wfiFlags
{
    UNWALKABLE = 1,
    SETILSWIM = 2,
    CLEARILSWIM = 4,
    SETHOPCLK = 8,
    SETDIR = 16,
    CLEARCHARGEATTACK = 32,
    SETHOPDIR = 64
};

class WalkflagInfo
{
    int flags;
    int newhopclk;
    int newhopdir;
    int newdir;
    int newladderdir;
    int newladderx;
    int newladdery;
    int newladderstart;
    
    WalkflagInfo()
    {
        flags=0;
        newhopclk=0;
        newhopdir=0;
        newdir=0;
        newladderdir=0;
        newladderx=0;
        newladdery=0;
        newladderstart=0;
    }
    
    int getHopClk() const { return newhopclk; }
    int getHopDir() const { return newhopdir; }
    int getDir() const { return newdir; }
    int getLadderdir() const { return newladderdir; }
    int getLadderx() const { return newladderx; }
    int getLaddery() const { return newladdery; }
    int getLadderstart() const { return newladderstart; }
    
    void setUnwalkable(bool val)
    {
        if(val) flags |= UNWALKABLE;
        else flags &= ~UNWALKABLE;
    }
    
    void setIlswim(bool val)
    {
        if(val) flags |= SETILSWIM;
        else flags |= CLEARILSWIM;
    }
    
    void setHopClk(int val)
    {
        flags |= SETHOPCLK;
        newhopclk = val;
    }
    
    void setHopDir(int val)
    {
        flags |= SETHOPDIR;
        newhopdir = val;
    }
    
    void setDir(int val)
    {
        flags |= SETDIR;
        newdir = val;
    }
    
    void setChargeAttack()
    {
        flags |= CLEARCHARGEATTACK;
    }
    
    int getFlags()
    {
        return flags;
    }
    
    bool isUnwalkable()
    {
        return (flags & UNWALKABLE) != 0;
    }
    
    // AngelScript doesn't allow overriding &&, ||, or !,
    // so they've been replaced by &, |, and ~.
    // I don't know yet if it's necessary to include variants of these
    // such as opAnd_r and opOrAssign.
    
    WalkflagInfo@ opOr(WalkflagInfo@ other)
    {
        WalkflagInfo ret;
        ret.newhopclk = newhopclk;
        ret.newdir = newdir;
        ret.newhopdir = (other.newhopdir >-1 ? other.newhopdir : newhopdir);
        
        int flags1 = (flags & ~UNWALKABLE) & (other.flags & ~UNWALKABLE);
        int flags2 = (flags & UNWALKABLE) | (other.flags & UNWALKABLE);
        ret.flags = flags1 | flags2;
        return ret;
    }

    WalkflagInfo@ opAnd(WalkflagInfo@ other)
    {
        WalkflagInfo ret;
        ret.newhopclk = newhopclk;
        ret.newdir = newdir;
        ret.newhopdir = (other.newhopdir >-1 ? other.newhopdir : newhopdir);
        
        ret.flags = flags & other.flags;
        return ret;
    }

    WalkflagInfo@ opCom()
    {
        WalkflagInfo ret;
        ret.newhopclk = newhopclk;
        ret.newdir = newdir;
        ret.newhopdir = newhopdir;
        
        ret.flags = flags ^ UNWALKABLE;
        return ret;
    }
}

