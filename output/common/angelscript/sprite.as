abstract class sprite: spriteIF
{
    __RealSprite@ realSprite;
    
    sprite()
    {
        @realSprite=__getRealSprite();
    }
    
    // Properties ==============================================================
    
    int uid
    {
        get const { return realSprite.uid; }
        set { realSprite.uid=value; }
    }
    
    fix x
    {
        get const { return realSprite.x; }
        set { realSprite.x=value; }
    }
    
    fix y
    {
        get const { return realSprite.y; }
        set { realSprite.y=value; }
    }
    
    fix z
    {
        get const { return realSprite.z; }
        set { realSprite.z=value; }
    }
    
    fix fall
    {
        get const { return realSprite.fall; }
        set { realSprite.fall=value; }
    }
    
    int tile
    {
        get const { return realSprite.tile; }
        set { realSprite.tile=value; }
    }
    
    int shadowtile
    {
        get const { return realSprite.shadowtile; }
        set { realSprite.shadowtile=value; }
    }
    
    int cs
    {
        get const { return realSprite.cs; }
        set { realSprite.cs=value; }
    }
    
    int flip
    {
        get const { return realSprite.flip; }
        set { realSprite.flip=value; }
    }
    
    int c_clk
    {
        get const { return realSprite.c_clk; }
        set { realSprite.c_clk=value; }
    }
    
    int clk
    {
        get const { return realSprite.clk; }
        set { realSprite.clk=value; }
    }
    
    int misc
    {
        get const { return realSprite.misc; }
        set { realSprite.misc=value; }
    }
    
    fix xofs
    {
        get const { return realSprite.xofs; }
        set { realSprite.xofs=value; }
    }
    
    fix yofs
    {
        get const { return realSprite.yofs; }
        set { realSprite.yofs=value; }
    }
    
    fix zofs // Both hit and draw, because why not
    {
        get const { return realSprite.zofs; }
        set { realSprite.zofs=value; }
    }
    
    int hxofs
    {
        get const { return realSprite.hxofs; }
        set { realSprite.hxofs=value; }
    }
    
    int hyofs
    {
        get const { return realSprite.hyofs; }
        set { realSprite.hyofs=value; }
    }
    
    int hxsz
    {
        get const { return realSprite.hxsz; }
        set { realSprite.hxsz=value; }
    }
    
    int hysz
    {
        get const { return realSprite.hysz; }
        set { realSprite.hysz=value; }
    }
    
    int hzsz
    {
        get const { return realSprite.hzsz; }
        set { realSprite.hzsz=value; }
    }
    
    int txsz
    {
        get const { return realSprite.txsz; }
        set { realSprite.txsz=value; }
    }
    
    int tysz
    {
        get const { return realSprite.tysz; }
        set { realSprite.tysz=value; }
    }
    
    int id
    {
        get const { return realSprite.id; }
        set { realSprite.id=value; }
    }
    
    int dir
    {
        get const { return realSprite.dir; }
        set { realSprite.dir=value; }
    }
    
    bool angular
    {
        get const { return realSprite.angular; }
        set { realSprite.angular=value; }
    }
    
    bool canfreeze
    {
        get const { return realSprite.canfreeze; }
        set { realSprite.canfreeze=value; }
    }
    
    double angle
    {
        get const { return realSprite.angle; }
        set { realSprite.angle=value; }
    }
    
    int lasthitclk
    {
        get const { return realSprite.lasthitclk; }
        set { realSprite.lasthitclk=value; }
    }
    
    int lasthit
    {
        get const { return realSprite.lasthit; }
        set { realSprite.lasthit=value; }
    }
    
    int drawstyle
    {
        get const { return realSprite.drawstyle; }
        set { realSprite.drawstyle=value; }
    }
    
    int extend
    {
        get const { return realSprite.extend; }
        set { realSprite.extend=value; }
    }
    
    uint8 scriptcoldet
    {
        get const { return realSprite.scriptcoldet; }
        set { realSprite.scriptcoldet=value; }
    }
    
    // Methods =================================================================
    
    int real_x(fix fx)
    {
        int rx=fx.getV()>>16;
        
        switch(dir)
        {
        case 9:
        case 13:
            if((fx.getV()&0xFFFF)!=0)
                ++rx;
                
            break;
        }
        
        return rx;
    }

    int real_y(fix fy)
    {
        return fy.getV()>>16;
    }
    
    void draw()
    {
        realSprite.draw();
    }
    
    void drawcloaked()
    {
        realSprite.drawcloaked();
    }
    
    bool hit(sprite@ s)
    {
        if((scriptcoldet&1)==0)
            return false;
        
        if(id<0 || s.id<0 || clk<0)
            return false;
        
        return hit(s.x+s.hxofs, s.y+s.hyofs, s.z+s.zofs, s.hxsz, s.hysz, s.hzsz);
    }

    bool hit(int tx,int ty,int tz,int txsz2,int tysz2,int tzsz2)
    {
        if((scriptcoldet&1)==0)
            return false;
        
        if(id<0 || clk<0)
            return false;
        
        return tx+txsz2>x+hxofs &&
               ty+tysz2>y+hyofs &&
               tz+tzsz2>z+zofs &&
               
               tx<x+hxofs+hxsz &&
               ty<y+hyofs+hysz &&
               tz<z+zofs+hzsz;
    }
    
    void move(fix dx,fix dy)
    {
        x+=dx;
        y+=dy;
    }

    void move(fix s)
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
    
    bool ON_SIDEPLATFORM()
    {
        return (_walkflag(x+4,y+16,0) || (y>=160 && currscr>=0x70 && !screenFlagSet(wfDOWN)));
    }
}

