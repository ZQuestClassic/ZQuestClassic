// ghost.zh
// Version 3.0.0 - Alpha 5

// Demo Scripts, v.3.0


namespace ghost3 //Ghost is now in a namespace. You now have three options when it comes to making ghost scipts:
{                //This is the first one. Including the script in namespace ghost3 will let you access ghost's
                 //functions and constants without prefixes (see third method).
		 
		 
	@Author("Dimi") //This is a handy way of marking a script's author. Good practice!
	ffc script cheatson
	{
		void run()
		{
			Game->Cheat = 4;
		}
	}

	@Author("Dimi")
	npc script G3_Example1
	{
		//using namespace ghost3; //This is the second method. Putting this line at the top of your script will also allow you to access 
		//ghost's functions and etc w/o prefix. You can also put this line outside of any scripts to allow usage anywhere.
		//It is commented out because this script is already in a namespace.
		//Keep in mind that the "using namespace" method may cause compatibility issues with Ghost 2.8 if you are mix and matching.
		//Either upgrade 2.8 to 2.9 (if it's released), or don't mix n match if you want to use this method.
		void run()
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			
			eweapon fakeparticle = CreateDummyEWeapon(EW_SCRIPT1, 0, 0, 32, 0, EWF_NO_COLLISION);
			SetEWeaponLifespan(fakeparticle, EWL_TIMER, 15);
			SetEWeaponDeathEffect(fakeparticle, EWD_VANISH, -1);
			SetEWeaponSparkleFrequency(fakeparticle, 4);
			fakeparticle->Behind = true;
			int shotcounter;
			this->NoSlide = true;
			while(true)
			{
				this->CanMove({this->Dir,1,SPW_WIZZROBE});
				this->CanMove({this->Dir,1,SPW_WIZZROBE});
				UpdateKnockback(this, 16, 4);
				++shotcounter;
				shotcounter%=((90*4)+(30*4)+30);
				if (shotcounter <= (90*4)+30)
				{
					this->ConstantWalk({this->Rate, this->Homing, 0});
					if (shotcounter % 90 == 0 && shotcounter > 0)
					{
						eweapon e = FireAimedEWeapon(EW_FIREBALL, this->X, this->Y, 0, 200, this->WeaponDamage, -1, -1, 0);
						SetEWeaponMovement(e, EWM_HOMING_REAIM, 1, 45);
						SetEWeaponLifespan(e, EWL_TIMER, 90);
						SetEWeaponDeathEffect(e, EWD_4_FIREBALLS_DIAG, -1);
						SetEWeaponSparkle(e, fakeparticle);
						SetEWeaponAttribute(e, GWI_PARTICLEOFFSET, 2);
					}
				}
				else
				{
					if ((shotcounter % 30) == 0)
					{
						int angle = Angle(this->X, this->Y, Hero->X, Hero->Y);
						this->Dir = AngleDir4(angle);
						angle = DegtoRad(angle);
						for(int i = 0; i < 5; ++i)
						{
							eweapon e = FireEWeapon(EW_FIREBALL, this->X, this->Y, angle, 300, this->WeaponDamage, -1, -1, 0);
							SetEWeaponMovement(e, EWM_SINE_WAVE, 8, 6);
							eweapon e2 = FireEWeapon(EW_FIREBALL, this->X, this->Y, angle, 300, this->WeaponDamage, -1, -1, 0);
							SetEWeaponMovement(e2, EWM_SINE_WAVE, 8, 6);
							SetEWeaponAttribute(e2, GWI_MISC1, 120);
							eweapon e3 = FireEWeapon(EW_FIREBALL, this->X, this->Y, angle, 300, this->WeaponDamage, -1, -1, 0);
							SetEWeaponMovement(e3, EWM_SINE_WAVE, 8, 6);
							SetEWeaponAttribute(e3, GWI_MISC1, 240);
							repeat(5) Ghost_Waitframe(this, GHD_EXPLODE, true);
						}
					}
				}
				Ghost_Waitframe(this, GHD_EXPLODE, true);
			}
		}
	}
}

@Author("Dimi")
npc script G3_Example2
{
	//This script uses the third method of ghost scripts. It doesn't have the "using" line at the beginning;
	//Instead ghost's functions and constants and etc are accessed via ghost3::(thingname)
	//This is the most annoying method, but Zoria wanted to show it off for whatever reason?
	//Personally I think it's dumb. You should do either the method shown above or the method mentioned above
	void run()
	{
		untyped data[ghost3::DATA_SIZE]; //Always first
		ghost3::init(this, data); //Always second
		
		eweapon fakeparticle = ghost3::CreateDummyEWeapon(EW_SCRIPT1, 0, 0, 97, 0, ghost3::EWF_NO_COLLISION);
		ghost3::SetEWeaponLifespan(fakeparticle, ghost3::EWL_TIMER, 20);
		ghost3::SetEWeaponDeathEffect(fakeparticle, ghost3::EWD_VANISH, -1);
		ghost3::SetEWeaponSparkleFrequency(fakeparticle, 4);
		fakeparticle->Behind = true;
		int shotcounter;
		while(true)
		{
			ghost3::MoveTowardLink(this, this->Step/100, 2);
			++shotcounter;
			if (shotcounter % 420 == 30)
			{
				eweapon e = ghost3::FireAimedEWeapon(EW_SCRIPT1, this->X, this->Y, 0, 200, this->WeaponDamage, 96, -1, 0);
				ghost3::SetEWeaponMovement(e, ghost3::EWM_THROWCHASE, 3, 4);
				ghost3::SetEWeaponDeathEffect(e, ghost3::EWD_FIRE, -1);
				ghost3::SetEWeaponSparkle(e, fakeparticle);
			}
			ghost3::Ghost_Waitframe(this, ghost3::GHD_EXPLODE, true);
		}
	}
}

namespace ghost3
{

	@Author("Dimi")
	npc script G3_Example3
	{
		//using namespace ghost3;
		void run()
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			
			eweapon fakeparticle = CreateDummyEWeapon(EW_SCRIPT1, 0, this->WeaponDamage, 87, 0, 0);
			SetEWeaponLifespan(fakeparticle, EWL_TIMER, 30);
			SetEWeaponDeathEffect(fakeparticle, EWD_AIM_AT_LINK, 0);
			SetEWeaponSparkleFrequency(fakeparticle, 4);
			fakeparticle->Behind = true;
			int shotcounter;
			while(true)
			{
				this->ConstantWalk8({this->Rate, this->Homing, 0});
				++shotcounter;
				if (shotcounter % 180 == 60)
				{
					eweapon e = FireAimedEWeapon(EW_SCRIPT1, this->X, this->Y, 0, 300, this->WeaponDamage, 98, -1, 0);
					SetEWeaponSparkle(e, fakeparticle);
				}
				Ghost_Waitframe(this, GHD_SHRINK, true);
			}
		}
	}

	@Author("Dimi")
	npc script G3_Example4
	{
		//using namespace ghost3;
		void run()
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			
			int shotcounter;
			while(true)
			{
				this->ConstantWalk8({this->Rate, this->Homing, 0});
				++shotcounter;
				if (shotcounter % 60 == 30)
				{
					eweapon e = FireAimedEWeapon(EW_SCRIPT1, this->X, this->Y, DegtoRad(180)+Rand(-0.4, 0.4), 200, this->WeaponDamage, 13, -1, 0);
					SetEWeaponMovement(e, EWM_VEER, AngleDir8(this->X, this->Y, Hero->X, Hero->Y), 0.07);
				}
				Ghost_Waitframe(this, GHD_NONE, true);
			}
		}
	}
}
	
@Author("Dimi")
npc script G3_Example5
{
	//Another script that uses the tedious method. *sigh*
	void run()
	{
		untyped data[ghost3::DATA_SIZE]; //Always first
		ghost3::init(this, data); //Always second
		
		int shotcounter;
		while(true)
		{
			this->HaltingWalk({this->Rate, this->Homing, 0, this->Haltrate, 60});
			if (this-> Halt == 20)
			{
				eweapon e = ghost3::FireAimedEWeapon(EW_SCRIPT1, this->X, this->Y, 0, this->Attributes[6], this->WeaponDamage, this->WeaponSprite, -1, 0);
				int flags = ghost3::EWMF_DIE;
				if (this->Attributes[9] == 1 || this->Attributes[9] == 3) flags |= ghost3::EWMF_SPEEDBOUNCE;
				if (this->Attributes[9] == 2 || this->Attributes[9] == 3) flags |= ghost3::EWMF_SLOWBOUNCE;
				if (this->Attributes[8] == 1) flags |= ghost3::EWMF_SCALEUP;
				if (this->Attributes[8] == 2) flags |= ghost3::EWMF_SCALEDOWN;
				ghost3::SetEWeaponMovement(e, ghost3::EWM_WALLBOUNCE, this->Attributes[7], flags);
				ghost3::SetEWeaponDeathEffect(e, ghost3::EWD_SBOMB_EXPLODE, this->WeaponDamage*2);
			}
			ghost3::Ghost_Waitframe(this, ghost3::GHD_NONE, true);
		}
	}
}

namespace ghost3
{
	//WizzrobeTeleport(npc this, int x, int y, int outframes, int inframes, int waittime, bool collflicker, DeathAnim deathtype, bool quitondeath)
	npc script G3_Example6
	{
		//using namespace ghost3;
		void run()
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			while(true)
			{
				if (DoWizzrobeStuff(this, this->Step, this->Step*2, this->Homing, this->Homing, this->Rate, this->Rate, this->Haltrate, 0, 8))
				{
					this->Dir = NormalizeDir(this->Dir);
					this->OriginalTile += 40;
					this->Tile += 40;
					int angle = DirAngle(this->Dir) - 45;
					eweapon e = FireEWeapon(EW_SCRIPT1, this->X+VectorX(14, angle), this->Y+VectorY(14, angle), DegtoRad(angle), 0, this->WeaponDamage, 100, 30, EWF_ROTATE_360|EWF_UNBLOCKABLE);
					for (int i = 0; i < 90; i+=6)
					{
						e->X = this->X+VectorX(14, angle+i);
						e->Y = this->Y+VectorY(14, angle+i);
						e->Angle = DegtoRad(angle+i);
						MoveAtAngle(this, DirAngle(this->Dir), this->Step/33, 2);
						Ghost_Waitframe(this, GHD_NONE, true);
					}
					Remove(e);
					this->OriginalTile -= 40;
					this->Tile -= 40;
				}
				Ghost_Waitframe(this, GHD_NONE, true);
			}
		}
	}
	npc script G3_Example7
	{
		//using namespace ghost3;
		void run()
		{
			RemoveSpawnPoof(this);
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			WizzrobeTeleport(this, 0, 0, 64, 24, false, GHD_NONE, true);
			while(true)
			{
				Ghost_Waitframes(this, 19);
				this->Attack();
				Ghost_Waitframes(this, 36);
				WizzrobeTeleport(this, 0);
			}
		}
	}
	npc script G3_Example8
	{
		//using namespace ghost3;
		void run()
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			int segx[201];
			int segy[201];
			memset(segx, this->X, 201);
			memset(segy, this->Y, 201);
			this->DrawXOffset = -1000;
			SetFlag(this, GHF_SET_DIRECTION);
			int i; int hitclk; int cset = this->CSet;
			eweapon e; int sinclk;
			while(true)
			{
				if (hitclk > 0) 
				{
					this->CSet = 6 + ((hitclk>>1)%4);
					--hitclk;
				}
				else this->CSet = cset;
				if (GotHit(this)) hitclk = 32;
				ArrayShift(segx, this->X, 1);
				ArrayShift(segy, this->Y, 1);
				ghost3::MoveAtAngle(this, Angle(this->X, this->Y, Hero->X, Hero->Y)+12*Sin(sinclk), this->Step/100, 2);
				sinclk+=8;
				i = 10;
				e = FireEWeapon(EW_SCRIPT1, segx[i], segy[i], 0, 0, this->Damage, -1, 0, EWF_UNBLOCKABLE);
				e->DrawXOffset = -1000;
				SetEWeaponLifespan(e, EWL_TIMER, 2);
				SetEWeaponDeathEffect(e, EWD_VANISH, -1);
				Screen->DrawCombo(2, segx[i], segy[i], this->InitD[0]+1, 1, 1, this->CSet, -1, -1, segx[i], segy[i], Angle(segx[i], segy[i], segx[i-10], segy[i-10]), -1, 0, true, OP_OPAQUE);
				Screen->DrawCombo(2, this->X, this->Y, this->InitD[0], 1, 1, this->CSet, -1, -1, this->X, this->Y, Angle(this->X, this->Y, Hero->X, Hero->Y)+8*Sin(sinclk), -1, 0, true, OP_OPAQUE);
				for (i = 20; i <= 190; i+=10)
				{
					e = FireEWeapon(EW_SCRIPT1, segx[i], segy[i], 0, 0, this->Damage, -1, 0, EWF_UNBLOCKABLE);
					e->DrawXOffset = -1000;
					SetEWeaponLifespan(e, EWL_TIMER, 2);
					SetEWeaponDeathEffect(e, EWD_VANISH, -1);
					Screen->DrawCombo(2, segx[i], segy[i], this->InitD[0]+1, 1, 1, this->CSet, -1, -1, segx[i], segy[i], Angle(segx[i], segy[i], segx[i-10], segy[i-10]), -1, 0, true, OP_OPAQUE);
				}
				i = 200;
				e = FireEWeapon(EW_SCRIPT1, segx[i], segy[i], 0, 0, this->Damage, -1, 0, EWF_UNBLOCKABLE);
				e->DrawXOffset = -1000;
				SetEWeaponLifespan(e, EWL_TIMER, 2);
				SetEWeaponDeathEffect(e, EWD_VANISH, -1);
				Screen->DrawCombo(2, segx[i], segy[i], this->InitD[0]+2, 1, 1, this->CSet, -1, -1, segx[i], segy[i], Angle(segx[i], segy[i], segx[i-10], segy[i-10]), -1, 0, true, OP_OPAQUE);
				Ghost_Waitframe(this, GHD_NONE, true);
			}
		}
	}
	
	npc script G3_Example9
	{
		//using namespace ghost3;
		void run()
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			SetFlag(this, GHF_SET_DIRECTION);
			SetTileAnimation(this, GH_STA_4DIR, {39008, 39180, 39040, 39048, 0, 4});
			SetTrail(this, 2, 16, 3);
			eweapon e;
			while(true)
			{
				SetTileAnimation(this, GH_STA_4DIR, {39008, 39000, 39040, 39048, 0, 4});
				this->Dir = AngleDir4(CenterX(this), CenterY(this), Hero->X+8, Hero->Y+8);
				TeleportAtAngle(this, Angle(CenterX(this), CenterY(this), Hero->X+8, Hero->Y+8), 96, 4, true, true, false, 32);
				for (int i = 0; i < 20; ++i)
				{
					Ghost_Waitframe(this, GHD_NONE, true);
				}
				SetTileAnimation(this, GH_STA_4DIR, {39008, 39000, 39040, 39048, 4, 4}, true, true);
				for (int i = 0; i < 12; ++i)
				{
					this->Dir = AngleDir4(CenterX(this), CenterY(this), Hero->X+8, Hero->Y+8);
					Ghost_Waitframe(this, GHD_NONE, true);
				}
				for (int q = -0.8; q <= 0.8; q+=0.8)
				{
					switch(this->Dir)
					{
						case DIR_RIGHT:
							e = FireAimedEWeapon(EW_FIREBALL, this->X+20, this->Y+12, q, 200, this->WeaponDamage, -1, -1, 0);
							break;
						case DIR_LEFT:
							e = FireAimedEWeapon(EW_FIREBALL, this->X-4, this->Y+12, q, 200, this->WeaponDamage, -1, -1, 0);
							break;
						case DIR_DOWN:
							e = FireAimedEWeapon(EW_FIREBALL, this->X+8, this->Y+12, q, 200, this->WeaponDamage, -1, -1, 0);
							break;
						default:
							e = FireAimedEWeapon(EW_FIREBALL, this->X+8, this->Y, q, 200, this->WeaponDamage, -1, -1, 0);
							break;
					}
					SetEWeaponLifespan(e, EWL_TIMER, 30);
					SetEWeaponDeathEffect(e, EWD_AIM_AT_LINK, 44);
					for (int i = 0; i < 8; ++i)
					{
						Ghost_Waitframe(this, GHD_NONE, true);
					}
				}
				SetTileAnimation(this, GH_STA_4DIR, {39008, 39000, 39040, 39048, 0, 4});
				for (int i = 0; i < 50; ++i)
				{
					Ghost_Waitframe(this, GHD_NONE, true);
				}
				
			}
		}
	}
	
	npc script G3_Patra
	{
		//using namespace ghost3;
		const int DEFAULT_TILE = 0;
		const int DEFAULT_FRAME = 0;
		const int DEFAULT_ASPEED = 0;
		const int DEFAULT_DIST = 0;
		void run(int tile, int frames, int aspeed, int dirs, int anglespeed, int spawnid, int spawnnum, int spawndist) //int fuckmylife why the fuck am I doing this
		{
			untyped data[DATA_SIZE]; //Always first
			init(this, data); //Always second
			
			tile = (tile > 0) ? tile : DEFAULT_TILE;
			frames = (frames > 0) ? frames : DEFAULT_FRAME;
			aspeed = (aspeed > 0) ? aspeed : DEFAULT_ASPEED;
			
			SetFlag(this, GHF_SET_DIRECTION);
			if (dirs >= 2) SetFlag(this, GHF_8WAY);
			if (dirs == 1) SetFlag(this, GHF_4WAY);
			int arr[10];
			arr[0] = tile;
			for (int i = 1; i < 8; ++i)
			{
				arr[i] = arr[i-1]+(Max(1,this->TileWidth)*Max(1,frames));
				if (this->TileHeight > 1 && ((arr[i]/20)<<0) > ((arr[i-1]/20)<<0))
				{
					arr[i] += 20*(((arr[i]/20)<<0) - ((arr[i-1]/20)<<0))*(this->TileHeight-1);
				}
			}
			arr[8] = aspeed;
			arr[9] = frames;
			int dist;
			int angle;
			npc n = Screen->LoadNPCByUID(this->ParentUID);
			if (n->isValid()) 
			{
				dist = Distance(CenterX(n), CenterY(n), CenterX(this), CenterY(this));
				angle = Angle(CenterX(n), CenterY(n), CenterX(this), CenterY(this));
			}
			if (dirs <= 0) SetTileAnimation(this, GH_STA_1DIR, arr);
			if (dirs == 1) SetTileAnimation(this, GH_STA_4DIR, arr);
			if (dirs >= 2) SetTileAnimation(this, GH_STA_8DIR, arr);
			eweapon e;
			if (spawnid > 0 && spawnnum > 0)
			{
				int initangle = Rand(360);
				int offset = 360 / spawnnum;
				for (int i = 0; i < spawnnum; ++i)
				{
					npc spawn = Screen->CreateNPC(spawnid);
					spawn->ParentUID = this->UID;
					spawn->X = CenterX(this)-8+VectorX(spawndist, initangle+(offset*i));
					spawn->Y = CenterY(this)-8+VectorY(spawndist, initangle+(offset*i));
				}
			}
			
			while(true)
			{
				if (n->isValid())
				{
					OrbitNPC(this, Screen->LoadNPCByUID(this->ParentUID), dist, anglespeed, true);
				}
				else
				{
					data[GHI_AX] = Sign(Hero->X-this->X)*(this->Homing/1000);
					data[GHI_AY] = Sign(Hero->Y-this->Y)*(this->Homing/1000);
					data[GHI_VX] = Clamp(data[GHI_VX], -(this->Step/100), (this->Step/100));
					data[GHI_VY] = Clamp(data[GHI_VY], -(this->Step/100), (this->Step/100));
				}
				Ghost_Waitframe(this, GHD_NONE, true);
			} 
		}
	}
	
	void RemoveSpawnPoof(npc this)
	{
		int drawxoff = this->DrawXOffset;
		this->DrawXOffset = -1000;
		Waitframes(4);
		this->DrawXOffset = drawxoff;
	}
		
// Temp Functions
	void UpdateKnockback(npc this, int frames, int speed)
	{
		this->NoSlide = true;
		if (GotHit(this))
		{
			//this->Knockback(frames, AngleDir4(Hero->X, Hero->Y, this->X, this->Y), speed);
			if (this->HitBy[6])
			{
				lweapon hitby = Screen->LoadLWeaponByUID(this->HitBy[6]);
				if ((hitby->Dir & 2) == (this->Dir & 2))
				{
					this->Knockback(frames, hitby->Dir, speed);
				}
			}
		}
	}
	//Shifts an array in a direction and sets the last one to a value.
	void ArrayShift(untyped ptr, untyped value, int shifts)
	{
		unless(IsValidArray(ptr))
		{
			printf("Invalid array passred to std::arrayshift(untyped,untyped,int)\n");
			return;
		}
		unless (SizeOfArray(ptr) > Abs(shifts))
		{
			printf("Shifts must be smaller than array size passed to std::arrayshift(untyped,untyped,int)\n");
			return;
		}
		if (shifts == 0)
		{
			printf("Shifts must be greater than 0; std::arrayshift(untyped,untyped,int)\n");
			return;
		}
		int q; 
		if (shifts > 0)
		{
			for ( q = (SizeOfArray(ptr)-1); q >= shifts; --q) 
			{
				ptr[q] = ptr[q-shifts];
			}
			for (q = shifts-1; q >= 0; --q)
			{
				ptr[q] = value;
			}
		}
		else
		{
			shifts = Abs(shifts);
			int arrsize = (SizeOfArray(ptr)-1);
			for (q = 0; q <= arrsize-shifts; ++q) 
			{
				ptr[q] = ptr[q+shifts];
			}
			for (q = arrsize-shifts+1; q <= arrsize; ++q)
			{
				ptr[q] = value;
			}
		}
	}

	//FireEWeapon(int weaponID, int x, int y, float angle, int step, int damage, int sprite, int sound, int flags);
}