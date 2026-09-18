#include "std.zh"
#include "auto/test_runner.zs"

// Covers the Cutscene Effect combo type: the Camera effect (each subtype,
// easing curve and causes mode, freezing the game, returning to the Hero,
// chaining into a second camera combo, being cancelled by a script or a
// screen load) and the Player Walk effect.
generic script camera_effect
{
	const int CUTEFF_PLAYER_WALK = 0;
	const int CUTEFF_CAMERA = 1;

	const int SUB_RELATIVE_TO_COMBO = 0;
	const int SUB_RELATIVE_TO_HERO = 1;
	const int SUB_RELATIVE_TO_VIEWPORT = 2;
	const int SUB_ABSOLUTE = 3;
	const int SUB_RETURN_TO_HERO = 4;

	const int CAUSES_ON_FINISH = 0;
	const int CAUSES_BEFORE_RETURN = 1;
	const int CAUSES_BEFORE_IDLE = 2;

	const int NUM_INTERPOLATION_MODES = 8;

	// Each effect combo changes into the next combo id when its
	// 'ComboType Causes->' fire, which is how the test observes them.
	const int CMB_CAMERA = 3000;
	const int CMB_CHAINED = 3010;
	const int CMB_WALK = 3020;

	const int COPYCAT_CHAIN = 7;

	// The camera combo sits near the bottom-right corner of a 2x2 region, so
	// panning to it puts the viewport at its maximum.
	const int CAMERA_X = 448;
	const int CAMERA_Y = 288;
	// Where the Player Walk combo goes.
	const int WALK_X = 128;
	const int WALK_Y = 32;

	int cameraPos;
	int maxViewX;
	int maxViewY;

	// Outlines a combo's position in world space and names it, marking it as
	// done once its causes have changed it into the next combo.
	void drawMarker(int x, int y, int cid, char32[] name)
	{
		int d = Screen->ComboD[ComboAt(x, y)];
		if (d != cid && d != cid + 1)
			return;
		char32 text[32];
		sprintf(text, d == cid + 1 ? "%s done" : "%s", name);
		Screen->DrawOrigin = DRAW_ORIGIN_REGION;
		Screen->Rectangle(7, x, y, x + 15, y + 15, 0x01, 1, 0, 0, 0, false, OP_OPAQUE);
		Screen->DrawString(7, x, y - 8, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, text, OP_OPAQUE);
	}

	// Draws the markers and the viewport position (under the test's label,
	// which the test runner draws) and waits a frame. Only the engine's own
	// output is asserted; this is so the replay can be followed by eye.
	void frame()
	{
		drawMarker(CAMERA_X, CAMERA_Y, CMB_CAMERA, "camera");
		drawMarker(CAMERA_X - 64, CAMERA_Y, CMB_CHAINED, "chained");
		drawMarker(WALK_X, WALK_Y, CMB_WALK, "walk");

		char32 status[64];
		sprintf(status, "viewport %d, %d", Viewport->X, Viewport->Y);
		Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD;
		Screen->DrawString(7, 2, 10, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, status, OP_OPAQUE);
		Waitframe();
	}

	void frames(int n)
	{
		for (int i = 0; i < n; i++)
			frame();
	}

	combodata setupEffect(int cid, int effect)
	{
		combodata cd = Test::setupCombo(cid, CT_CUTSCENE_EFFECT);
		cd->Attributes[8] = effect;

		cd->NumTriggers = 2;
		combotrigger effects = cd->Triggers[0];
		effects->Flags[TRIGFLAG_CTYPE_EFFECTS] = true;
		combotrigger causes = cd->Triggers[1];
		causes->Flags[TRIGFLAG_CTYPE_CAUSES] = true;
		causes->ComboChange = 1;
		// The combo it changes into does nothing.
		Test::setupCombo(cid + 1, CT_NONE);
		return cd;
	}

	combodata setupCamera(int subtype, int dx, int dy, int speed)
	{
		combodata cd = setupEffect(CMB_CAMERA, CUTEFF_CAMERA);
		cd->Attributes[0] = speed;
		cd->Attributes[3] = subtype;
		cd->Attributes[4] = dx;
		cd->Attributes[5] = dy;
		Screen->ComboD[cameraPos] = CMB_CAMERA;
		return cd;
	}

	void trigger(int pos)
	{
		Test::Assert(Screen->TriggerCombo(0, pos, 0), "TriggerCombo succeeds");
	}

	bool causesFired(int pos, int cid)
	{
		return Screen->ComboD[pos] == cid + 1;
	}

	bool viewportAt(int x, int y)
	{
		return Viewport->X == x && Viewport->Y == y;
	}

	// Waits until the viewport reaches (x, y), and returns how many frames it took.
	int waitForViewport(int x, int y, char32[] what)
	{
		for (int i = 0; i < 600; i++)
		{
			if (viewportAt(x, y))
				return i;
			frame();
		}
		printf("viewport stuck at %d, %d\n", Viewport->X, Viewport->Y);
		Test::Fail(what);
		return -1;
	}

	void placeHero(int x, int y)
	{
		Hero->X = x;
		Hero->Y = y;
		frame();
	}

	void testRelativeToComboParksThenReturns()
	{
		placeHero(32, 32);
		Test::Assert(viewportAt(0, 0), "viewport starts at the top-left");

		Test::Label("relative to combo: pan to the camera combo");
		setupCamera(SUB_RELATIVE_TO_COMBO, 0, 0, 4);
		trigger(cameraPos);
		frame();
		Test::Assert(Viewport->X > 0 && Viewport->X < maxViewX, "viewport is panning");
		Test::Assert(!causesFired(cameraPos, CMB_CAMERA), "causes don't fire mid-pan");

		int frames = waitForViewport(maxViewX, maxViewY, "pans to the combo");
		printf("relative to combo: arrived after %d frames\n", frames);
		frame();
		Test::Assert(causesFired(cameraPos, CMB_CAMERA), "causes fire when the pan finishes");

		// With no return, the camera holds there.
		Test::Label("relative to combo: parked, no return");
		frames(30);
		Test::Assert(viewportAt(maxViewX, maxViewY), "camera stays parked at the target");

		// A 'Return To Hero' combo pans back.
		Test::Label("return to hero combo");
		setupCamera(SUB_RETURN_TO_HERO, 0, 0, 8);
		trigger(cameraPos);
		frames = waitForViewport(0, 0, "returns to the hero");
		printf("return to hero: arrived after %d frames\n", frames);
		frame();
		Test::Assert(causesFired(cameraPos, CMB_CAMERA), "causes fire after returning");

		// The camera follows the hero again.
		Test::Label("the viewport follows the hero again");
		placeHero(256, 160);
		Test::AssertEqual(Viewport->X, 256 + 8 - Viewport->Width / 2, "viewport follows the hero's x");
	}

	void testRelativeToHero()
	{
		placeHero(32, 32);
		Test::Label("relative to hero: 200px right, then back");
		// 200px right of the hero's center.
		setupCamera(SUB_RELATIVE_TO_HERO, 200, 0, 8);
		trigger(cameraPos);
		waitForViewport(32 + 8 + 200 - Viewport->Width / 2, 0, "pans relative to the hero");
		setupCamera(SUB_RETURN_TO_HERO, 0, 0, 8);
		trigger(cameraPos);
		waitForViewport(0, 0, "returns to the hero");
	}

	void testAbsoluteIsClamped()
	{
		placeHero(32, 32);
		Test::Label("absolute 5000,5000: clamped to the region");
		// Way past the bottom-right of the region: clamped to its edge.
		setupCamera(SUB_ABSOLUTE, 5000, 5000, 16);
		trigger(cameraPos);
		waitForViewport(maxViewX, maxViewY, "absolute target is clamped to the region");
		setupCamera(SUB_RETURN_TO_HERO, 0, 0, 16);
		trigger(cameraPos);
		waitForViewport(0, 0, "returns to the hero");
	}

	void testInterpolationModes()
	{
		placeHero(32, 32);
		for (int mode = 0; mode < NUM_INTERPOLATION_MODES; mode++)
		{
			char32 what[64];
			sprintf(what, "interpolation mode %d: 200px right and back", mode);
			Test::Label(what);
			combodata cd = setupCamera(SUB_RELATIVE_TO_VIEWPORT, 200, 0, 10);
			cd->Attributes[2] = mode;
			cd->Flags[1] = true; // return to hero
			trigger(cameraPos);

			// 20 frames there, then 20 frames back.
			int samples[8];
			for (int i = 0; i < 8; i++)
			{
				frames(5);
				samples[i] = Viewport->X;
			}
			printf("interpolation %d: %d %d %d %d | %d %d %d %d\n", mode,
				samples[0], samples[1], samples[2], samples[3],
				samples[4], samples[5], samples[6], samples[7]);
			Test::AssertEqual(samples[3], 200, "reaches the target");
			waitForViewport(0, 0, "returns to the hero");
			frame();
			Test::Assert(causesFired(cameraPos, CMB_CAMERA), "causes fire on finish");
		}
	}

	// Runs an absolute pan with idle time and a return, and checks when the causes fire.
	void runCausesMode(int causesMode, int idle, bool returnToHero, char32[] expected)
	{
		placeHero(32, 32);
		char32 what[64];
		sprintf(what, "causes mode %d, idle %d, return %d: %s", causesMode, idle, returnToHero, expected);
		Test::Label(what);
		combodata cd = setupCamera(SUB_ABSOLUTE, 384, 264, 16);
		cd->Attributes[1] = idle;
		cd->Attributes[6] = causesMode;
		cd->Flags[1] = returnToHero;
		trigger(cameraPos);

		int arrived = -1;
		int left = -1;
		int fired = -1;
		for (int i = 0; i < 200; i++)
		{
			if (fired == -1 && causesFired(cameraPos, CMB_CAMERA))
				fired = i;
			if (arrived == -1 && viewportAt(maxViewX, maxViewY))
				arrived = i;
			if (arrived != -1 && left == -1 && !viewportAt(maxViewX, maxViewY))
				left = i;
			if (fired != -1 && (!returnToHero || viewportAt(0, 0)))
				break;
			frame();
		}
		printf("causes mode %d idle %d return %d: arrived %d, left %d, causes %d (%s)\n",
			causesMode, idle, returnToHero, arrived, left, fired, expected);
		Test::Assert(fired != -1, "causes fired");
		Test::Assert(arrived != -1, "camera arrived");

		if (!returnToHero)
		{
			Test::AssertEqual(left, -1, "camera stays without a return");
			// Send the camera back for the next test.
			Test::Label("return to hero combo");
			setupCamera(SUB_RETURN_TO_HERO, 0, 0, 16);
			trigger(cameraPos);
		}
		waitForViewport(0, 0, "returns to the hero");
	}

	void testCausesModes()
	{
		// Before Idle: as soon as the camera arrives.
		runCausesMode(CAUSES_BEFORE_IDLE, 20, true, "on arrival");
		// Before Return: once idling is done, as the camera starts back.
		runCausesMode(CAUSES_BEFORE_RETURN, 20, true, "when leaving");
		// On Finish: once the camera is back.
		runCausesMode(CAUSES_ON_FINISH, 20, true, "back at the hero");
		// Before Idle with no idle time acts as Before Return.
		runCausesMode(CAUSES_BEFORE_IDLE, 0, true, "when leaving");
		// Before Return with no return acts as On Finish.
		runCausesMode(CAUSES_BEFORE_RETURN, 20, false, "after idling");
	}

	void testFreeze()
	{
		placeHero(32, 32);
		Test::Label("freeze: the arrow must not move until the camera is back");
		eweapon e = Screen->CreateEWeapon(EW_ARROW);
		e->X = 32;
		e->Y = 120;
		e->Dir = DIR_RIGHT;
		e->Step = 100;
		e->Damage = 0;
		frame();

		combodata cd = setupCamera(SUB_RELATIVE_TO_VIEWPORT, 160, 0, 8);
		cd->Flags[0] = true; // freeze
		cd->Flags[1] = true; // return to hero
		trigger(cameraPos);
		frame();
		int x = e->X;
		int frames = 0;
		while (!viewportAt(0, 0) || frames < 2)
		{
			Test::AssertEqual(e->X, x, "weapons don't move while the camera freezes the game");
			frame();
			frames++;
		}
		printf("freeze: lasted %d frames\n", frames);
		frames(2);
		Test::Assert(e->X > x, "weapons move again once the camera is done");
	}

	void testChained()
	{
		placeHero(32, 32);
		Test::Label("chained: right to the first, down to the second, then back");
		// The first camera pans right; when it finishes, its causes copycat
		// into a second camera combo, which pans down.
		combodata first = setupCamera(SUB_ABSOLUTE, 384, 88, 16);
		first->Triggers[1]->Copycat = COPYCAT_CHAIN;
		first->Triggers[1]->ComboChange = 0;

		combodata second = setupEffect(CMB_CHAINED, CUTEFF_CAMERA);
		second->Attributes[0] = 16;
		second->Attributes[3] = SUB_ABSOLUTE;
		second->Attributes[4] = 384;
		second->Attributes[5] = 264;
		second->Flags[1] = true; // return to hero
		second->Triggers[0]->Copycat = COPYCAT_CHAIN;
		int secondPos = ComboAt(CAMERA_X - 64, CAMERA_Y);
		Screen->ComboD[secondPos] = CMB_CHAINED;

		trigger(cameraPos);
		waitForViewport(maxViewX, 0, "first camera pans right");
		waitForViewport(maxViewX, maxViewY, "second camera pans down");
		waitForViewport(0, 0, "second camera returns to the hero");
		frame();
		Test::Assert(causesFired(secondPos, CMB_CHAINED), "second camera's causes fire");
	}

	void testScriptCancels()
	{
		placeHero(32, 32);
		Test::Label("a script takes the viewport mid-pan: effect cancelled");
		setupCamera(SUB_RELATIVE_TO_COMBO, 0, 0, 2);
		trigger(cameraPos);
		frames(10);
		Test::Assert(Viewport->X > 0, "viewport is panning");

		// Taking script control of the viewport cancels the effect.
		Viewport->Mode = VIEW_MODE_SCRIPT;
		frame();
		Viewport->Mode = VIEW_MODE_CENTER_AND_BOUND;
		frame();
		Test::Assert(viewportAt(0, 0), "viewport is back on the hero");
		frames(10);
		Test::Assert(viewportAt(0, 0), "effect does not resume");
		Test::Assert(Screen->ComboD[cameraPos] == CMB_CAMERA, "a cancelled effect's causes don't fire");
	}

	void testInvalidConfig()
	{
		placeHero(32, 32);
		// A camera with no speed is ignored.
		Test::Label("invalid: zero speed, nothing should happen");
		setupCamera(SUB_RELATIVE_TO_COMBO, 0, 0, 0);
		trigger(cameraPos);
		frames(10);
		Test::Assert(viewportAt(0, 0), "zero speed is ignored");

		Test::Label("invalid: bad interpolation mode, nothing should happen");
		combodata cd = setupCamera(SUB_RELATIVE_TO_COMBO, 0, 0, 4);
		cd->Attributes[2] = NUM_INTERPOLATION_MODES;
		trigger(cameraPos);
		frames(10);
		Test::Assert(viewportAt(0, 0), "an invalid interpolation mode is ignored");

		Test::Label("invalid: bad subtype, nothing should happen");
		cd = setupCamera(99, 0, 0, 4);
		trigger(cameraPos);
		frames(10);
		Test::Assert(viewportAt(0, 0), "an invalid subtype is ignored");
	}

	void testWalk(int destX, int destY, int speed, bool relCombo, bool relHero, int expectX, int expectY, char32[] what)
	{
		placeHero(32, 32);
		char32 text[64];
		sprintf(text, "player walk (%s) to %d, %d", what, expectX, expectY);
		Test::Label(text);
		int walkPos = ComboAt(WALK_X, WALK_Y);
		combodata cd = setupEffect(CMB_WALK, CUTEFF_PLAYER_WALK);
		cd->Attributes[0] = destX;
		cd->Attributes[1] = destY;
		cd->Attributes[2] = speed;
		cd->Flags[0] = relCombo;
		cd->Flags[1] = relHero;
		Screen->ComboD[walkPos] = CMB_WALK;
		trigger(walkPos);

		int frames = 0;
		while (!causesFired(walkPos, CMB_WALK) && frames < 600)
		{
			frame();
			frames++;
		}
		printf("walk (%s): %d frames, ended at %d, %d facing %d\n", what, frames, Hero->X, Hero->Y, Hero->Dir);
		Test::Assert(causesFired(walkPos, CMB_WALK), "causes fire when the walk ends");
		Test::AssertEqual(Hero->X, expectX, what);
		Test::AssertEqual(Hero->Y, expectY, what);
	}

	void testPlayerWalk()
	{
		// Player Walk requires 'Newer Hero Movement'.
		Game->FFRules[qr_NEW_HERO_MOVEMENT2] = true;
		// Absolute destination, at the walk speed set on the combo.
		testWalk(96, 80, 2, false, false, 96, 80, "absolute");
		// Relative to the combo (its center, minus half a tile).
		testWalk(16, 32, 3, true, false, WALK_X + 16, WALK_Y + 32, "relative to combo");
		// Relative to the hero, at the hero's normal walking speed.
		testWalk(-16, 48, 0, false, true, 32 - 16, 32 + 48, "relative to hero");
	}

	void testScreenLoadClearsEffect()
	{
		placeHero(32, 32);
		Test::Label("warp away mid-pan: the new screen ignores the effect");
		setupCamera(SUB_RELATIVE_TO_COMBO, 0, 0, 1);
		trigger(cameraPos);
		frames(10);
		Test::Assert(Viewport->X > 0, "viewport is panning");

		// Leaving the region mid-effect ends it.
		Player->Warp(Test::TestingDmap, 0x22);
		frames(5);
		Test::Assert(viewportAt(0, 0), "a new screen's viewport is not driven by the old effect");
	}

	// With a deadzone, the viewport's focus can sit away from the hero's center. A return trip
	// must end where the viewport will follow from afterwards, not snap there once it hands back.
	void testReturnWithDeadzone()
	{
		placeHero(200, 96);
		Test::Label("return with a deadzone: ends on the deadzone's focus");
		Viewport->DeadzoneWidth = 32;
		// Walk right until the hero is pinned to the box's right edge.
		for (int i = 0; i < 40; i++)
		{
			WaitTo(SCR_TIMING_POST_PLAYER_ACTIVE);
			Input->Button[CB_RIGHT] = true;
			frame();
		}
		int pinned_x = Viewport->X;
		Test::AssertEqual(Hero->X + 8 - (pinned_x + Viewport->Width / 2), 16, "hero is pinned to the deadzone edge");

		combodata cd = setupCamera(SUB_RELATIVE_TO_VIEWPORT, 0, 64, 8);
		cd->Flags[1] = true; // return to hero
		trigger(cameraPos);
		for (int i = 0; i < 600 && !causesFired(cameraPos, CMB_CAMERA); i++)
			frame();
		Test::Assert(causesFired(cameraPos, CMB_CAMERA), "causes fire after returning");
		Test::AssertEqual(Viewport->X, pinned_x, "the return ends on the focus");
		frame();
		Test::AssertEqual(Viewport->X, pinned_x, "the viewport doesn't jump once the effect hands back");

		Viewport->DeadzoneWidth = 0;
	}

	void run()
	{
		Test::Init();

		Test::loadRegion(0, 2);
		cameraPos = ComboAt(CAMERA_X, CAMERA_Y);
		maxViewX = 512 - Viewport->Width;
		maxViewY = 352 - Viewport->Height;
		printf("viewport %d x %d\n", Viewport->Width, Viewport->Height);

		testRelativeToComboParksThenReturns();
		testRelativeToHero();
		testAbsoluteIsClamped();
		testInterpolationModes();
		testCausesModes();
		testFreeze();
		testChained();
		testScriptCancels();
		testInvalidConfig();
		testPlayerWalk();
		testReturnWithDeadzone();
		testScreenLoadClearsEffect();

		Test::End();
	}
}
