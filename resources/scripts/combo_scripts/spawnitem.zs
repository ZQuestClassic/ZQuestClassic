combodata script spawnitem
{
	void run(int iid, int reg, int bit)
	{
		if ( GetScreenDBit(Game->GetCurDMap(), Game->GetCurScreen(), reg, bit) )
		{
			Quit();
		}
		for ( int q = Screen->NumItems(); q > 0; --q )
		{
			item ii = Screen->LoadItem(i);
			if ( i->ID == iid ) Quit();
		}
		item i = Screen->CreateItem(iid);
		i-X = this->PosX();
		i-Y = this->PosY();
		i->PScript = Game->GetItemScript("SetPickupBit");
		i->InitD[0] = reg;
		i->InitD[1] = bit;
	}
}

itemdata script SetPickupBit
{
	void run(int reg, int bit)
	{
		SetScreenDBit(Game->GetCurDMap(), Game->GetCurScreen(), reg, bit, true);
	}
}