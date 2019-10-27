#include "std.zh"

    eweapon script laz
    {
    	void run(int speed, int colour, int layer, int fizzlesfx, int fizzlesprite, int shotsfx, int maxDist)
    	{
    		int startX = CenterX(this);
    		int startY = CenterY(this);
    		this->Angular = true;
    		Audio->PlaySound(shotsfx); //because enemy editor shot sound isn't variable yet
    		this->Angle = ArcTan(Link->X-this->X, Link->Y-this->Y);
    		this->Step = speed;
    		//this->Damage = Game->LoadNPCData(this->Parent)->WeaponDamage; //should get bthis from the enemy editor
    		//this->DrawYOffset = -32768; //make wpn sprite invisible
    		this->DrawYOffset = -32768;
	
		while(1)
    		{
    			if ( Screen->isSolid(CenterX(this), CenterY(this)) /*|| ( Distance(startX, startY, CenterX(this), CenterY(this)) > maxDist )*/ )
    			{
    				this->DeadState = WDS_DEAD;
    				Audio->PlaySound(fizzlesfx);
    				if(fizzlesprite)
    				{
    					
    					lweapon fiz = Screen->CreateLWeapon(LW_SPARKLE);
    					fiz->UseSprite(fizzlesprite);
    					fiz->CollDetection = false;
    					fiz->X = this->X;
    					fiz->Y = this->Y;
    				}
    			}
    			else
    			{
				//printf("parent->X: %d\n", Screen->LoadNPCByUID(this->ParentUID)->X);
    				Screen->Line(layer, Screen->LoadNPCByUID(this->ParentUID)->DrawXOffset+Screen->LoadNPCByUID(this->ParentUID)->X+((Screen->LoadNPCByUID(this->ParentUID)->TileWidth)*8),
					Screen->LoadNPCByUID(this->ParentUID)->DrawYOffset+Screen->LoadNPCByUID(this->ParentUID)->Y+((Screen->LoadNPCByUID(this->ParentUID)->TileHeight)*8), CenterX(this), CenterY(this), colour, 1, 0, 0, 0, 128);
    			}
    			Waitframe();
    		}
    	}
    }