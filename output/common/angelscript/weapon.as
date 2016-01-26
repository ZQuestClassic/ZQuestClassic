class weapon: sprite
{
    __RealWeapon@ realWeapon;
    
    weapon()
    {
        @realWeapon=__getRealWeapon();
    }
    
    int power
    {
        get const { return realWeapon.power; }
        set { realWeapon.power=value; }
    }
    
    int type
    {
        get const { return realWeapon.type; }
        set { realWeapon.type=value; }
    }
    
    int parentid
    {
        get const { return realWeapon.parentid; }
        set { realWeapon.parentid=value; }
    }
    
    int parentitem
    {
        get const { return realWeapon.parentitem; }
        set { realWeapon.parentitem=value; }
    }
    
    uint16 aframe
    {
        get const { return realWeapon.aframe; }
        set { realWeapon.aframe=value; }
    }
    
    int o_tile
    {
        get const { return realWeapon.o_tile; }
        set { realWeapon.o_tile=value; }
    }
    
    int o_cset
    {
        get const { return realWeapon.o_cset; }
        set { realWeapon.o_cset=value; }
    }
    
    bool aimedBrang
    {
        get const { return realWeapon.aimedBrang; }
        set { realWeapon.aimedBrang=value; }
    }
    
    
    void adjustDraggedItem(int removed)
    {
        realWeapon.adjustDraggedItem(removed);
    }
}

