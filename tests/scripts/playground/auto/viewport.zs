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

void walk(int button, int frames)
{
	for (int i = 0; i < frames; i++)
	{
		press(button);
		Waitframe();
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
		Test::Init();

		int screen = 0;
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				mapdata scr = Game->LoadMapData(map, screen + x + y*16);
				fillScreen(scr);
			}
		}
		Test::loadRegion(screen, 4);
		Player->Warp(Test::TestingDmap, 18);
		Waitframe();

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
		Game->LoadItemData(I_ARROW1)->WeaponData->Script = Game->GetLWeaponScript("seeking_arrow");

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

		// The seeking_arrow script disabled the directional buttons; restore them so the
		// hero can walk.
		Input->DisableButton[CB_UP] = false;
		Input->DisableButton[CB_DOWN] = false;
		Input->DisableButton[CB_LEFT] = false;
		Input->DisableButton[CB_RIGHT] = false;

		// Deadzone: while the hero stays within the box, the viewport must not move; once past
		// the box's edge, the hero stays pinned to it.
		Test::AssertEqual(Viewport->DeadzoneWidth, 0);
		Viewport->DeadzoneWidth = 64;
		Viewport->DeadzoneHeight = 48;
		Waitframe();

		int start_x = Viewport->X;
		int start_y = Viewport->Y;
		int hero_start_x = Hero->X;

		// A short walk (well under half the box width) must not move the viewport.
		for (int i = 0; i < 12; i++)
		{
			press(CB_RIGHT);
			Waitframe();
		}
		Test::AssertEqual(Hero->X > hero_start_x, true, "hero didn't walk");
		Test::AssertEqual(Viewport->X, start_x, "viewport moved inside deadzone");
		Test::AssertEqual(Viewport->Y, start_y, "viewport moved inside deadzone");

		// A long walk pushes the hero to the box's edge: their center stays exactly
		// half the box width ahead of the viewport's focus.
		for (int i = 0; i < 60; i++)
		{
			press(CB_RIGHT);
			Waitframe();
		}
		int focus_x = Viewport->X + Viewport->Width / 2;
		Test::AssertEqual(Hero->X + 8 - focus_x, Viewport->DeadzoneWidth / 2, "hero not pinned to deadzone edge");

		// Walking back the same distance re-crosses the box before the viewport moves again.
		int came_from_x = Viewport->X;
		for (int i = 0; i < 12; i++)
		{
			press(CB_LEFT);
			Waitframe();
		}
		Test::AssertEqual(Viewport->X, came_from_x, "viewport should idle while re-crossing the deadzone");

		Viewport->DeadzoneWidth = 0;
		Viewport->DeadzoneHeight = 0;
		Waitframe();

		// Lookahead: while the target moves, the viewport gradually aims ahead of (positive)
		// or behind (negative) its direction of travel; while it stands still, the offset
		// holds.
		Test::AssertEqual(Viewport->LookaheadX, 0);
		Test::AssertEqual(Viewport->LookaheadY, 0);
		Test::AssertEqual(Viewport->LookaheadSpeed, 1);

		// Stand in the middle of the region so the ±40px aims stay clear of the region
		// clamp on both axes.
		Hero->X = 600;
		Hero->Y = 350;
		Viewport->LookaheadX = 40;
		Viewport->LookaheadY = 40;
		Viewport->LookaheadSpeed = 2;
		Waitframe();

		// Walking right builds a lead in front; it settles after 40/2 = 20 frames of movement.
		walk(CB_RIGHT, 30);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 + 40, "viewport should lead ahead of movement");

		// Standing still (even turning in place) holds the offset.
		Hero->Dir = DIR_LEFT;
		Waitframes(20);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 + 40, "lookahead should hold while idle");

		// Walking the other way shifts the lead gradually, not instantly.
		walk(CB_LEFT, 10);
		int focus = Viewport->X + Viewport->Width / 2;
		Test::AssertEqual(focus > Floor(Hero->X) + 8 - 40 && focus < Floor(Hero->X) + 8 + 40, true, "lookahead should shift gradually");
		walk(CB_LEFT, 30);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 - 40, "viewport should lead ahead of movement");

		// Negative lookahead trails behind the direction of travel instead.
		Viewport->LookaheadX = -40;
		Viewport->LookaheadY = -40;
		walk(CB_LEFT, 40);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 + 40, "viewport should trail behind movement");

		// The vertical axis works the same way with its own distance, and the horizontal offset
		// eases out.
		Viewport->LookaheadX = 40;
		Viewport->LookaheadY = 20;
		walk(CB_UP, 40);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8, "horizontal offset should ease out when moving up");
		Test::AssertEqual(Viewport->Y + Viewport->Height / 2, Floor(Hero->Y) + 8 - 20, "viewport should lead vertically by its own distance");

		// Disabling lookahead eases the held offset away even while idle.
		Viewport->LookaheadX = 0;
		Viewport->LookaheadY = 0;
		Waitframes(30);
		Test::AssertEqual(Viewport->Y + Viewport->Height / 2, Floor(Hero->Y) + 8, "lookahead should ease out when disabled");

		// Fractional speeds are allowed: at 0.5 px/frame a 40px lead takes 80 frames of
		// movement to build, so it is still growing after 40.
		Viewport->LookaheadX = 40;
		Viewport->LookaheadY = 40;
		Viewport->LookaheadSpeed = 0.5;
		Test::AssertEqual(Viewport->LookaheadSpeed, 0.5);
		walk(CB_RIGHT, 40);
		focus = Viewport->X + Viewport->Width / 2;
		Test::AssertEqual(focus > Floor(Hero->X) + 8 && focus < Floor(Hero->X) + 8 + 40, true, "fractional speed should ease gradually");
		walk(CB_RIGHT, 50);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 + 40, "viewport should lead ahead of movement");

		// The configured distance may use the full signed-byte range; the offset the viewport
		// actually holds is capped per axis so the viewport can still reach the region edges.
		Viewport->LookaheadX = 127;
		Viewport->LookaheadY = 127;
		Test::AssertEqual(Viewport->LookaheadX, 127);
		Test::AssertEqual(Viewport->LookaheadY, 127);
		Viewport->LookaheadX = -128;
		Viewport->LookaheadY = -128;
		Test::AssertEqual(Viewport->LookaheadX, -128);
		Test::AssertEqual(Viewport->LookaheadY, -128);
		Viewport->LookaheadX = 0;
		Viewport->LookaheadY = 0;
		Viewport->LookaheadSpeed = 0.75;
		Viewport->ResetFollowSettings();
		Test::AssertEqual(Viewport->LookaheadSpeed, 1, "reset should restore the configured speed");
		Test::AssertEqual(Viewport->LookaheadX, 0, "reset should restore the configured lookahead");

		// Only movement in the direction the hero faces counts: a shove the other way
		// (knockback, conveyors, a script writing the position) neither flips nor builds
		// the lead. Freeze the offset while repositioning so the jump isn't read as movement.
		Viewport->LookaheadX = 40;
		Viewport->LookaheadY = 40;
		Viewport->LookaheadSpeed = 0;
		Hero->X = 600;
		Hero->Y = 350;
		Waitframe();
		Viewport->LookaheadSpeed = 1;
		walk(CB_RIGHT, 50);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 + 40, "viewport should lead ahead of movement");
		Hero->X -= 16;
		Waitframes(5);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8 + 40, "a backward shove should not move the lead");

		// Warps drop the offset: the viewport lands centered on the hero.
		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 18, 128, 88, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		Waitframe();
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8, "warp should drop the lookahead offset");
		Test::AssertEqual(Viewport->Y + Viewport->Height / 2, Floor(Hero->Y) + 8, "warp should drop the lookahead offset");

		// Scroll transitions carry the offset. Build a full leftward lead on the single screen
		// east of the region (it's exactly one viewport, so the lead isn't visible there), then
		// walk back into the region. Once past the region clamp, the viewport must already hold
		// the full lead rather than be re-easing it from zero, which the slow speed set just
		// before the scroll would make obvious.
		mapdata east = Game->LoadMapData(map, 4);
		fillScreen(east);
		east->Valid = 1;
		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 4, 224, 88, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_RESET_DM_SCRIPT);
		Waitframe();
		walk(CB_LEFT, 50);
		Viewport->LookaheadSpeed = 0.25;
		int frames = 0;
		while (Game->Scrolling[SCROLL_DIR] == -1 && frames++ < 300)
		{
			press(CB_LEFT);
			Waitframe();
		}
		Test::Assert(frames < 300, "never scrolled back into the region");
		while (Game->Scrolling[SCROLL_DIR] > -1)
			Waitframe();
		Test::AssertEqual(Game->CurScreen, 0, "should have scrolled into the region (origin screen 0)");
		Test::AssertEqual(Game->HeroScreen, 3, "should have arrived on the region's east column");
		frames = 0;
		while (Hero->X > 1024 - 16 - 100 && frames++ < 200)
		{
			press(CB_LEFT);
			Waitframe();
		}
		Test::Assert(frames * 0.25 < 40, "walk-in should be too short to rebuild the lead from zero");
		// The walk-in can end on a half pixel, which the viewport rounds.
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Round(Hero->X) + 8 - 40, "scroll should carry the lookahead offset");

		// A dmap can supply its own follow settings instead of Init Data's; script writes still
		// win over both, and ResetFollowSettings returns to whichever is configured.
		Viewport->ResetFollowSettings();
		dmapdata dm = Game->LoadDMapData(Test::TestingDmap);
		dm->Flagset[DMFS_CUSTOM_VIEWPORT_FOLLOW] = true;
		dm->ViewportDeadzoneWidth = 48;
		dm->ViewportLookaheadY = -16;
		Test::AssertEqual(Viewport->DeadzoneWidth, 48, "dmap follow settings should apply");
		Test::AssertEqual(Viewport->LookaheadY, -16, "dmap follow settings should apply");
		Viewport->DeadzoneWidth = 16;
		Test::AssertEqual(Viewport->DeadzoneWidth, 16, "script override should win over the dmap");
		Viewport->ResetFollowSettings();
		Test::AssertEqual(Viewport->DeadzoneWidth, 48, "reset should return to the dmap's settings");
		dm->Flagset[DMFS_CUSTOM_VIEWPORT_FOLLOW] = false;
		Test::AssertEqual(Viewport->DeadzoneWidth, 0, "without the flag the dmap defers to Init Data");
		Test::AssertEqual(Viewport->LookaheadY, 0, "without the flag the dmap defers to Init Data");
		Waitframe();

		// Idle recentering: after standing still for the delay, the viewport eases back until the
		// hero is centered in the deadzone box again. Drop any held lookahead offset first.
		Viewport->ResetFollowSettings();
		Viewport->LookaheadSpeed = 240;
		Hero->X = 600;
		Hero->Y = 350;
		Waitframes(2);
		Viewport->DeadzoneWidth = 64;
		Viewport->RecenterSpeed = 1;
		Viewport->RecenterDelay = 10;
		walk(CB_LEFT, 60);
		focus_x = Viewport->X + Viewport->Width / 2;
		Test::AssertEqual(focus_x - (Floor(Hero->X) + 8), 32, "hero should be pinned to the deadzone edge");
		Waitframes(8);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, focus_x, "viewport should wait out the recenter delay");
		Waitframes(50);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8, "viewport should recenter on the idle hero");

		// With a held lookahead offset, recentering eases toward the hero, not the shifted aim
		// point, so stopping never pushes the viewport further ahead. A lookahead smaller than
		// half the deadzone leaves the hero inside the box: walking left the viewport trails
		// 16px behind them, and recentering stops on them (the aim point is 16px past them).
		Viewport->ResetFollowSettings();
		Hero->X = 600;
		Waitframes(2);
		Viewport->DeadzoneWidth = 64;
		Viewport->LookaheadX = 16;
		Viewport->LookaheadSpeed = 240;
		Viewport->RecenterSpeed = 1;
		Viewport->RecenterDelay = 10;
		walk(CB_LEFT, 60);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2 - (Floor(Hero->X) + 8), 16, "viewport should trail the hero by half the deadzone minus the lead");
		Waitframes(50);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Floor(Hero->X) + 8, "recentering should stop on the hero, not the aim point ahead of them");

		// A lookahead larger than half the deadzone already holds the viewport ahead of the hero,
		// at the box's edge nearest them, so recentering has nowhere to go.
		Viewport->ResetFollowSettings();
		Hero->X = 600;
		Waitframes(2);
		Viewport->DeadzoneWidth = 32;
		Viewport->LookaheadX = 40;
		Viewport->LookaheadSpeed = 240;
		Viewport->RecenterSpeed = 1;
		Viewport->RecenterDelay = 10;
		walk(CB_LEFT, 60);
		focus_x = Viewport->X + Viewport->Width / 2;
		Test::AssertEqual(focus_x - (Floor(Hero->X) + 8), -24, "viewport should lead the hero by the lead minus half the deadzone");
		Waitframes(50);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, focus_x, "recentering should not push the viewport further ahead");
		Viewport->ResetFollowSettings();

		// Handing the viewport back from VIEW_MODE_SCRIPT drops the lookahead offset, as a warp
		// does: the follow logic didn't ease while the script owned the viewport.
		Hero->X = 600;
		Hero->Y = 350;
		Waitframes(2);
		Viewport->LookaheadX = 40;
		Viewport->LookaheadSpeed = 240;
		walk(CB_RIGHT, 50);
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Round(Hero->X) + 8 + 40, "viewport should lead ahead of movement");
		Viewport->Mode = VIEW_MODE_SCRIPT;
		Waitframe();
		Viewport->Mode = VIEW_MODE_CENTER_AND_BOUND;
		Waitframe();
		Test::AssertEqual(Viewport->X + Viewport->Width / 2, Round(Hero->X) + 8, "leaving script mode should drop the lookahead offset");
		Viewport->ResetFollowSettings();

		// ...but it keeps the focus: with the hero off-center inside the deadzone, a script that
		// only froze the viewport hands it back without it moving.
		Hero->X = 600;
		Waitframes(2);
		Viewport->DeadzoneWidth = 64;
		walk(CB_LEFT, 20);
		Waitframe();
		int frozen_x = Viewport->X;
		Test::Assert(Viewport->X + Viewport->Width / 2 - (Round(Hero->X) + 8) > 16, "hero should be well off-center inside the deadzone");
		Viewport->Mode = VIEW_MODE_SCRIPT;
		Waitframe();
		Viewport->Mode = VIEW_MODE_CENTER_AND_BOUND;
		Waitframe();
		Test::AssertEqual(Viewport->X, frozen_x, "leaving script mode should keep the deadzone focus");
		Viewport->ResetFollowSettings();

		// The held offset never exceeds what the deadzone leaves room for. After the deadzone
		// grows (shrinking the cap from 80 to 20), turning around shifts the lead straight away,
		// instead of first waiting out the 60px excess at 1px per frame.
		Hero->X = 600;
		Hero->Y = 350;
		Waitframes(2);
		Viewport->LookaheadY = 80;
		Viewport->LookaheadSpeed = 240;
		walk(CB_UP, 10);
		Test::AssertEqual(Viewport->Y + Viewport->Height / 2, Round(Hero->Y) + 8 - 80, "viewport should lead ahead of movement");
		Viewport->DeadzoneHeight = 120;
		Viewport->LookaheadSpeed = 1;
		walk(CB_DOWN, 30);
		int lead_y = Viewport->Y + Viewport->Height / 2 - (Round(Hero->Y) + 8);
		Test::Assert(lead_y > -70, "turning around should shift the lead straight away");
		Viewport->ResetFollowSettings();

		Test::End();
	}
}
