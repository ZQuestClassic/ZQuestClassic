class weapon: sprite
{
    __RealWeapon@ realWeapon;
    
    weapon()
    {
        @realWeapon=__getRealWeapon();
    }
	
    // Properties ==============================================================
	
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
	
	bool dead
	{
		get const { return realWeapon.dead; }
		set { realWeapon.dead=value; }
	}
	
	int clk2
	{
		get const { return realWeapon.clk2; }
		set {realWeapon.clk2=value; }
	}
	
	int misc2
	{
		get const {return realWeapon.misc2; }
		set {realWeapon.misc2=value; }
	}
	
	int ignoreCombo
	{
		get const { return realWeapon.ignoreCombo; }
		set {realWeapon.ignoreCombo=value; }
	}
	
	bool isLit
	{
		get const {return realWeapon.isLit; }
		set { realWeapon.isLit=value; }
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
    
	int dragging
	{
		get { realWeapon.dragging; }
		set { realWeapon.dragging=value; }
	}
	
	fix step
    {
        get const { return realWeapon.step; }
        set { realWeapon.step=value; }
    }
	
	bool bounce
	{
		get const { return realWeapon.bounce; }
		set { realWeapon.bounce=value; }
	}
	
	bool ignoreLink
	{
		get const { return realWeapon.ignoreLink; }
		set { realWeapon.ignoreLink=value; }
	}
	
	uint16 flash
	{
		get const { return realWeapon.flash;}
		set { realWeapon.flash=value; }
	}

	uint16 wid //IDs and Types are two different things remember that folks!
	{
		get const { realWeapon.wid; }
		set { realWeapon.wID=value; }
	}
	
    uint16 aframe
    {
        get const { return realWeapon.aframe; }
        set { realWeapon.aframe=value; }
    }
	
	uint16  csclk
	{
		get const { return realWeapon.csclk; }
		set { realWeapon.csclk=value; }
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

	int o_speed
	{
		get const { return realWeapon.o_speed; }
		set { realWeapon.o_speed=value}
	}
	
	int o_type
	{
		get const { return realWeapon.o_type; }
		set { realWeapon.o_type; }
	}
	
	int frames
	{
		get const { return realWeapon.frames; }
		set { realWeapon.frames=value; }
	}
	
	int o_flip
	{
		get const { return realWeapon.o_flip}
		set {realWeapon.o_flip=value; }
	}
    
	int temp1
	{
		get const { return realWeapon.temp1; }
		set { realWeapon.temp1=value; }
	}
	
	bool behind
	{
		get const { return realWeapon.behind; }
		set { realWeapon.behind=value; }
	}
	
	private int sfxLoop
	{
		get const { realWeapon.sfxLoop; }
		set { realWeapon.sfxLoop=value; }
	}
	
	private int minX
	{
		get { realWeapon.minX; }
		set { realWeapon.minX=value; }
	}
	
	private int maxX
	{
		get const{ realWeapon.maxX; }
		set { realWeapon.maxX=value; }
	}
	
	private int minY
	{
		get const { realWeapon.minY; }
		set { realWeapon.minY=value; }
	}
	
	private int maxY
	{
		get { realWeapon.maxY; }
		set { realWeapon.maxY=value; }
	}
	
    bool aimedBrang
    {
        get const { return realWeapon.aimedBrang; }
        set { realWeapon.aimedBrang=value; }
    }
	
	private bool sparkleFunc;
	
    // Methods =================================================================
	
    void adjustDraggedItem(int removed)
    {
        realWeapon.adjustDraggedItem(removed);
    }
	void setSparkleFunc(const &weapon) //I am confident this is correct
	{
	    realWeapon.sparkleFunc=true;
	}
}
