ffc script Maths
{
    void run()
    {
        // TraceS("hi");
        itemdata d = Game->LoadItemData(1);
        Trace(d->WeaponProperties);
        Trace(d->WeaponProperties->TileWidth);
        Trace(d->WeaponProperties->BurnSprites[0]);
    }
}
