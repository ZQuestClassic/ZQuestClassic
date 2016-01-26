class item: sprite
{
    __RealItem@ realItem;
    
    item()
    {
        @realItem=__getRealItem();
    }
    
    int pickup
    {
        get const { return realItem.pickup; }
        set { realItem.pickup=value; }
    }
    
    int clk2
    {
        get const { return realItem.clk2; }
        set { realItem.clk2=value; }
    }
}

