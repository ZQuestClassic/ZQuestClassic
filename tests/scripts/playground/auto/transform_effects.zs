#include "std.zh"
#include "auto/test_runner.zs"

// Covers the effect an enemy shows when its 'Transform' defense changes it into
// another enemy (the 'Transform Effect' attribute): a sprite, an explosion, a
// bomb blast that is only for show, or a bomb blast that does damage.
generic script transform_effects
{
	const int ENEMY_X = 128;
	const int ENEMY_Y = 64;
	// Close enough for a bomb blast on the enemy to reach.
	const int HERO_X = 128;
	const int HERO_Y = 88;

	const int NEW_ENEMY = NPC_STALFOS1;
	const int NEW_ENEMY_HP = 100;
	// The sprite used for the sprite effect. Any sprite with a few frames will do.
	const int EFFECT_SPRITE = 22;
	// How long to watch the effect for.
	const int WATCH_FRAMES = 150;

	// What was seen after a transform.
	int firstId;
	int firstWidth;
	int firstHeight;
	int life;
	int maxCount;
	int heroLost;
	int enemyLost;

	// The index of the enemy with the given id, or 0 if there is none.
	int findEnemy(int id)
	{
		for (int i = 1; i <= Screen->NumNPCs(); i++)
		{
			if (Screen->LoadNPC(i)->ID == id)
				return i;
		}
		return 0;
	}

	void clearSprites()
	{
		for (int i = Screen->NumNPCs(); i > 0; i--)
			Screen->LoadNPC(i)->Remove();
		for (int i = Screen->NumLWeapons(); i > 0; i--)
			Screen->LoadLWeapon(i)->Remove();
		for (int i = Screen->NumEWeapons(); i > 0; i--)
			Screen->LoadEWeapon(i)->Remove();
	}

	// Transforms a goriya with the given effect, and records what the effect did.
	// The goriya is `width` by `height` tiles.
	void transform(int effect, int width, int height, char32[] what)
	{
		clearSprites();
		Hero->X = HERO_X;
		Hero->Y = HERO_Y;
		Hero->HP = Hero->MaxHP;
		Waitframe();

		npc n = Screen->CreateNPC(NPC_GORIYA1);
		n->X = ENEMY_X;
		n->Y = ENEMY_Y;
		n->HP = 100;
		n->Step = 0;
		n->Haltrate = 0;
		n->Defense[NPCD_ARROW] = NPCDT_TRANSFORM;
		n->Attributes[14] = effect;
		n->Attributes[15] = NEW_ENEMY;
		// Keep the new enemy stunned in place for longer than the effect is watched.
		n->Attributes[16] = WATCH_FRAMES + 60;
		if (width != 1 || height != 1)
		{
			n->Extend = EXT_NORMAL;
			n->TileWidth = width;
			n->TileHeight = height;
		}
		// Wait out the spawn animation, during which enemies can't be hit at all.
		Waitframes(60);

		lweapon arrow = Screen->CreateLWeapon(LW_ARROW);
		arrow->X = ENEMY_X - 40;
		arrow->Y = ENEMY_Y;
		arrow->Dir = DIR_RIGHT;
		arrow->Step = 300;
		arrow->Damage = 4;

		int index = 0;
		for (int i = 0; i < 60 && !index; i++)
		{
			Waitframe();
			index = findEnemy(NEW_ENEMY);
		}
		Test::Assert(index, "transforms into the new enemy");
		npc changed = Screen->LoadNPC(index);
		if (arrow->isValid())
			arrow->Remove();
		changed->HP = NEW_ENEMY_HP;
		int heroHP = Hero->HP;

		firstId = -1;
		firstWidth = 0;
		firstHeight = 0;
		life = -1;
		maxCount = 0;
		for (int frame = 0; frame < WATCH_FRAMES; frame++)
		{
			int count = Screen->NumLWeapons();
			if (count > maxCount)
				maxCount = count;
			if (count > 0 && firstId == -1)
			{
				lweapon w = Screen->LoadLWeapon(1);
				firstId = w->ID;
				firstWidth = w->TileWidth;
				firstHeight = w->TileHeight;
			}
			if (count == 0 && life == -1)
				life = frame;
			Waitframe();
		}

		heroLost = heroHP - Hero->HP;
		enemyLost = changed->isValid() ? NEW_ENEMY_HP - changed->HP : 1000;
		printf("transform effect %d (%dx%d): weapon %d sized %dx%d, most at once %d, gone after %d frames, hero lost %d, new enemy lost %d\n",
			effect, width, height, firstId, firstWidth, firstHeight, maxCount, life, heroLost, enemyLost);
	}

	// A blast is one weapon, which goes away once it has played out.
	void assertBlast(int id)
	{
		Test::AssertEqual(firstId, id, "the effect is a blast");
		Test::AssertEqual(maxCount, 1, "the effect is one blast");
		Test::Assert(life > 1, "the blast lasts longer than a frame");
	}

	void run()
	{
		Test::Init();
		Waitframes(10);

		transform(0, 1, 1, "none");
		Test::AssertEqual(maxCount, 0, "no effect makes no weapon");
		Test::AssertEqual(heroLost, 0, "no effect leaves the hero alone");
		Test::AssertEqual(enemyLost, 0, "no effect leaves the new enemy alone");

		transform(EFFECT_SPRITE, 1, 1, "a sprite");
		Test::AssertEqual(firstId, LW_SPARKLE, "a sprite effect is a sparkle");
		Test::Assert(life > 1, "a sprite effect lasts longer than a frame");
		Test::AssertEqual(heroLost, 0, "a sprite effect leaves the hero alone");
		Test::AssertEqual(enemyLost, 0, "a sprite effect leaves the new enemy alone");

		// The effect is as big as the enemy.
		transform(EFFECT_SPRITE, 2, 1, "a sprite, on a wide enemy");
		Test::AssertEqual(firstWidth, 2, "a sprite effect is as wide as the enemy");
		Test::AssertEqual(firstHeight, 1, "a sprite effect is as tall as the enemy");

		transform(EFFECT_SPRITE, 1, 2, "a sprite, on a tall enemy");
		Test::AssertEqual(firstWidth, 1, "a sprite effect is as wide as the enemy");
		Test::AssertEqual(firstHeight, 2, "a sprite effect is as tall as the enemy");

		transform(-1, 1, 1, "explosion 0");
		transform(-2, 1, 1, "explosion 2");
		transform(-3, 1, 1, "explosion 1");

		transform(-5, 1, 1, "a bomb blast for show");
		assertBlast(LW_BOMBBLAST);
		Test::AssertEqual(heroLost, 0, "a blast for show leaves the hero alone");
		Test::AssertEqual(enemyLost, 0, "a blast for show leaves the new enemy alone");
		transform(-4, 1, 1, "a super bomb blast for show");
		assertBlast(LW_SBOMBBLAST);
		Test::AssertEqual(heroLost, 0, "a blast for show leaves the hero alone");
		Test::AssertEqual(enemyLost, 0, "a blast for show leaves the new enemy alone");

		transform(-7, 1, 1, "a bomb blast");
		assertBlast(LW_BOMBBLAST);
		transform(-6, 1, 1, "a super bomb blast");
		assertBlast(LW_SBOMBBLAST);

		Test::End();
	}
}
