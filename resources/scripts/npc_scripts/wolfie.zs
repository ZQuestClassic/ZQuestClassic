// Walking 2x1 Enemy
// v1.0
// 13th December, 2019


npc script wolfie
{
	void run()
	{
		int clks[1];
		lweapon bite = Screen->CreateLWeapon(EW_SCRIPT1);
		bite->X = this->X;
		bite->Y = this->Y;
		bite->HitHeight = 8;
		bite->HitWidth = 16;
		bite->DrawYOffset = -32768;
		bite->Damage = 2;
		while(1)
		{
			this->ConstantWalk({this->Rate, this->Homing, this->Hunger});
			unless(bite->isValid()) 
			{
				bite = Screen->CreateLWeapon(EW_SCRIPT1);
				bite->DrawYOffset = -32768;
				bite->Damage = 2;
			}
			if(bite->isValid()) 
			{
				wolfnme::positionbite(this, bite);
				wolfnme::transformbite(this, bite);
				
			}
			Waitdraw();
			wolfnme::transform2x1(this,clks);
			Waitframe();
		}
	}
}

namespace wolfnme
{		
	void positionbite(npc n, lweapon bite)
	{
		switch(n->Dir)
		{
			case DIR_UP:
				bite->X = n->X; bite->Y = n->Y -1; bite->HitYOffset = -2; bite->HitXOffset = 0; break;
			case DIR_DOWN:
				bite->X = n->X; bite->Y = n->Y + 1; bite->HitYOffset = 24; bite->HitXOffset = 0; break;
			case DIR_LEFT:
				bite->X = n->X - 1; bite->Y = ((n->Y > 0) ? n->Y : n->Y + 1); bite->HitYOffset = 0; bite->HitXOffset = -2; break;
			case DIR_RIGHT:
				bite->X = n->X + 1; bite->Y = ((n->Y > 0) ? n->Y : n->Y + 1); bite->HitYOffset = 0; bite->HitXOffset = 25; break;
		}
	}
	void transformbite(npc n, lweapon l)
	{
		switch(n->Dir)
		{
			case DIR_DOWN:
			case DIR_UP:
			{
				l->HitWidth = 16;
				l->HitHeight = 8;
				break;
			}
			case DIR_RIGHT:
			case DIR_LEFT:
			{
				l->HitWidth = 8;
				l->HitHeight = 16;
				break;
			}
		}
	}

	void transform2x1(npc n, int clk)
	{
		++clk[0];
		int f4 = Floor(clk[0]/(n->ASpeed/4));
		if( f4 >= 4 ) { f4 = 0; clk[0] = 0;}
		switch(n->Dir)
		{
			case DIR_DOWN:
			{
				n->Tile = (Game->LoadNPCData(n->ID)->Tile) + 4 + f4;
				n->OriginalTile = n->Tile;
				n->TileWidth = 1;
				n->TileHeight = 2;
				n->HitWidth = 16;
				n->HitHeight = 32;
				break;
			}
			case DIR_RIGHT:
			{
				n->Tile = (Game->LoadNPCData(n->ID)->Tile) + 60 + (f4 * 2);
				n->OriginalTile = n->Tile;
				n->TileWidth = 2;
				n->TileHeight = 1;
				n->HitWidth = 32;
				n->HitHeight = 16;
				break;
			}
			case DIR_LEFT:
			{
				n->Tile = (Game->LoadNPCData(n->ID)->Tile) + 40 + (f4 *2);
				n->OriginalTile = n->Tile;
				n->TileWidth = 2;
				n->TileHeight = 1;
				n->HitWidth = 32;
				n->HitHeight = 16;
				break;
			}
			case DIR_UP:
			{
				n->Tile = (Game->LoadNPCData(n->ID)->Tile) + f4;
				n->OriginalTile = n->Tile;
				n->TileWidth = 1;
				n->TileHeight = 2;
				n->HitWidth = 16;
				n->HitHeight = 32;
				break;
			}
		}
	}
}