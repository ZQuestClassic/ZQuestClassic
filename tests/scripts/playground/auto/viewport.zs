// https://www.youtube.com/watch?v=JLaQevEaMUE

#include "std.zh"
#include "auto/test_runner.zs"

void fillScreen(mapdata scr)
{
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 11; y++)
		{
			scr->ComboD[x + y*16] = 12 + (x + y) % 8;
		}
	}
}

lweapon script seeking_arrow
{
	void run()
	{
		Viewport->Target = this;

		Input->DisableButton[CB_UP] = true;
		Input->DisableButton[CB_DOWN] = true;
		Input->DisableButton[CB_LEFT] = true;
		Input->DisableButton[CB_RIGHT] = true;

		int rotation = 0;
		int time = 0;
		bool explode = false;

		while (this->DeadState != WDS_DEAD)
		{
			if (time % 60 == 0)
				Audio->PlaySound(SFX_WHIRLWIND);

			if (this->Vx)
			{
				if (Input->Button[CB_UP])
				{
					this->Y -= 1;
					rotation -= 1;
				}
				if (Input->Button[CB_DOWN])
				{
					this->Y += 1;
					rotation += 1;
				}
			}
			else
			{
				if (Input->Button[CB_LEFT])
				{
					this->X -= 1;
					rotation += 1;
				}
				if (Input->Button[CB_RIGHT])
				{
					this->X += 1;
					rotation -= 1;
				}
			}

			if (Input->Button[CB_A])
			{
				explode = true;
				break;
			}

			this->Rotation = rotation + 10*Cos(time++ * 15);

			Waitframe();
		}

		if (explode)
		{
			lweapon blast = Screen->CreateLWeapon(LW_BOMBBLAST);
			blast->X = this->X;
			blast->Y = this->Y;

			Audio->PlaySound(SFX_BOMB);

			this->DrawStyle = DS_PHANTOM;
			this->Vx = 0;
			this->Vy = 0;

			Waitframes(30);
		}

		Input->DisableButton[CB_UP] = false;
		Input->DisableButton[CB_DOWN] = false;
		Input->DisableButton[CB_LEFT] = false;
		Input->DisableButton[CB_RIGHT] = false;

		this->Remove();
	}
}

void press(int button)
{
	WaitTo(SCR_TIMING_POST_PLAYER_ACTIVE);
	Input->Button[button] = true;
}

generic script viewport
{
	void run()
	{
		int dmap = 5;
		int screen = 10;
		int map = Game->LoadDMapData(dmap)->Map;
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				mapdata scr = Game->LoadMapData(map, screen + x + y*16);
				scr->Valid = 1;
				scr->RegionID = 1;
				fillScreen(scr);
			}
		}

		Test::Init(dmap, 27);

		Viewport->Mode = VIEW_MODE_SCRIPT;

		int r = 20;
        int angle = 0;
        int da = 5;

		for (int i = 0; i < 60 * 3; i++)
		{
			Viewport->X = Hero->X - Viewport->Width / 2 + r * Cos(angle);
            Viewport->Y = Hero->Y - Viewport->Height / 2 + r * Sin(angle);

            angle += da;
            if (angle < -360) angle += 360;
            else if (angle > 360) angle -= 360;

			Waitframe();
		}

		Viewport->Mode = VIEW_MODE_CENTER_AND_BOUND;

		Hero->Item[Game->LoadItemData(I_BOW1)->ID] = true;
		Hero->Item[Game->LoadItemData(I_ARROW1)->ID] = true;
		Game->Counter[CR_ARROWS] = 10;
		Game->MCounter[CR_ARROWS] = 10;
		Game->LoadItemData(I_ARROW1)->WeaponScript = Game->GetLWeaponScript("seeking_arrow");

		Waitframes(30);
		press(CB_B);
		Waitframes(30);
		press(CB_A);
		Waitframes(60);

		Hero->Dir = DIR_RIGHT;
		Waitframes(30);
		press(CB_B);
		Waitframes(60);
		press(CB_A);
		Waitframes(60);

		Hero->Dir = DIR_UP;
		Waitframes(30);
		press(CB_B);
		Waitframes(30);
		press(CB_A);
		Waitframes(60);

		Test::End();
	}
}
