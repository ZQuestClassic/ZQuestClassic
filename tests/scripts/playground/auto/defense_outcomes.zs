#include "std.zh"
#include "auto/test_runner.zs"

// Covers defense outcomes: the Hero's defenses against enemy weapons, and an
// enemy's defense outcomes against the Hero's weapons - including the ones
// that replace the enemy (split, summon, transform, explode), trigger
// secrets, or switch places with the Hero.
generic script defense_outcomes
{
	const int ENEMY_X = 128;
	const int ENEMY_Y = 64;

	// A combo flagged to change into its screen's first secret combo.
	const int SECRET_POS = 0;
	const int SECRET_COMBO = 12;

	bool lastAlive;
	int lastStun;

	// Waits out the Hero's knockback and hit invincibility.
	void waitForHero()
	{
		Waitframes(90);
	}

	// Fires an enemy arrow into the Hero from behind (so the shield isn't
	// involved), and returns the damage taken, in the weapon's damage units
	// (each a quarter heart).
	int hitHero(int defense, int damage, char32[] what)
	{
		char32 text[64];
		sprintf(text, "hero defense: %s (damage %d)", what, damage);
		Test::Label(text);
		// Each hit knocks the Hero back, so put them back first.
		Hero->X = 64;
		Hero->Y = 112;
		Hero->Dir = DIR_UP;
		Hero->Stun = 0;
		Hero->Defense[EW_ARROW] = defense;
		Hero->HP = Hero->MaxHP / 2;
		Waitframe();
		int hp = Hero->HP;

		eweapon e = Screen->CreateEWeapon(EW_ARROW);
		e->X = Hero->X;
		e->Y = Hero->Y + 32;
		e->Dir = DIR_UP;
		e->Step = 200;
		e->Damage = damage;
		Waitframes(20);
		int lost = (hp - Hero->HP) / 4;
		lastAlive = e->isValid() && e->DeadState == WDS_ALIVE;
		lastStun = Hero->Stun;
		printf("hero defense %d (damage %d): lost %d, weapon alive %d, stun %d\n", defense, damage, lost, lastAlive, lastStun);
		if (e->isValid())
			Remove(e);
		waitForHero();
		return lost;
	}

	void testHeroDefenses()
	{
		Hero->MaxHP = 16 * 16;

		Test::AssertEqual(hitHero(NPCDT_NONE, 8, "normal damage"), 8, "normal damage");
		Test::AssertEqual(hitHero(NPCDT_HALFDAMAGE, 8, "half damage"), 4, "half damage");
		Test::AssertEqual(hitHero(NPCDT_QUARTERDAMAGE, 8, "quarter damage"), 2, "quarter damage");
		// Unlike for enemies, the Hero's stun defenses stun on top of the damage.
		Test::AssertEqual(hitHero(NPCDT_STUN, 8, "stun"), 8, "stun");
		Test::Assert(lastStun > 0, "stunned");
		Test::AssertEqual(hitHero(NPCDT_STUNORBLOCK, 8, "stun or block, with damage"), 8, "stun or block, with damage");
		Test::Assert(lastStun > 0, "stunned");
		Test::AssertEqual(hitHero(NPCDT_STUNORBLOCK, 0, "stun or block, without damage"), 0, "stun or block, without damage");
		Test::Assert(!lastAlive && lastStun == 0, "blocked");
		Test::AssertEqual(hitHero(NPCDT_STUNORIGNORE, 8, "stun or ignore, with damage"), 8, "stun or ignore, with damage");
		Test::Assert(lastStun > 0, "stunned");
		Test::AssertEqual(hitHero(NPCDT_STUNORIGNORE, 0, "stun or ignore, without damage"), 0, "stun or ignore, without damage");
		Test::Assert(lastAlive && lastStun == 0, "ignored");
		Test::AssertEqual(hitHero(NPCDT_BLOCK1, 0, "block below 1"), 0, "block below 1");
		Test::Assert(!lastAlive, "blocked");
		Test::AssertEqual(hitHero(NPCDT_BLOCK1, 8, "no block at 1 or more"), 8, "no block at 1 or more");
		Test::AssertEqual(hitHero(NPCDT_BLOCK2, 1, "block below 2"), 0, "block below 2");
		Test::AssertEqual(hitHero(NPCDT_BLOCK2, 8, "no block at 2 or more"), 8, "no block at 2 or more");
		Test::AssertEqual(hitHero(NPCDT_BLOCK4, 2, "block below 4"), 0, "block below 4");
		Test::AssertEqual(hitHero(NPCDT_BLOCK6, 4, "block below 6"), 0, "block below 6");
		Test::AssertEqual(hitHero(NPCDT_BLOCK8, 6, "block below 8"), 0, "block below 8");
		Test::AssertEqual(hitHero(NPCDT_BLOCK10, 8, "block below 10"), 0, "block below 10");
		Test::AssertEqual(hitHero(NPCDT_BLOCK10, 12, "no block at 10 or more"), 12, "no block at 10 or more");
		Test::AssertEqual(hitHero(NPCDT_BLOCK, 8, "always block"), 0, "always block");
		Test::Assert(!lastAlive, "blocked");
		Test::AssertEqual(hitHero(NPCDT_IGNORE1, 0, "ignore below 1"), 0, "ignore below 1");
		Test::Assert(lastAlive, "ignored");
		Test::AssertEqual(hitHero(NPCDT_IGNORE1, 8, "no ignore at 1 or more"), 8, "no ignore at 1 or more");
		Test::AssertEqual(hitHero(NPCDT_IGNORE, 8, "always ignore"), 0, "always ignore");
		Test::Assert(lastAlive, "ignored");
		Test::AssertEqual(hitHero(NPCDT_2XDAMAGE, 4, "double damage"), 8, "double damage");
		Test::AssertEqual(hitHero(NPCDT_3XDAMAGE, 4, "triple damage"), 12, "triple damage");
		Test::AssertEqual(hitHero(NPCDT_4XDAMAGE, 4, "quadruple damage"), 16, "quadruple damage");
		// Heals by the weapon's damage before the enemy damage multiplier, so
		// by a quarter of what the hit would otherwise have taken.
		Test::AssertEqual(hitHero(NPCDT_HEAL, 4, "heal by the damage"), -1, "heal by the damage");
	}

	npc spawnEnemy(int id, int defense)
	{
		// Each enemy test starts with an empty screen.
		Test::clearNPCs();
		Test::clearWeapons();
		Waitframe();

		npc n = Screen->CreateNPC(id);
		n->X = ENEMY_X;
		n->Y = ENEMY_Y;
		n->HP = 100;
		n->Step = 0;
		n->Haltrate = 0;
		n->Defense[NPCD_ARROW] = defense;
		// Wait out the spawn animation, during which enemies can't be hit at all.
		Waitframes(60);
		return n;
	}

	// Shoots the enemy with an arrow.
	void shootEnemy(npc n, int damage)
	{
		lweapon w = Screen->CreateLWeapon(LW_ARROW);
		w->X = n->X - 40;
		w->Y = n->Y;
		w->Dir = DIR_RIGHT;
		w->Step = 300;
		w->Damage = damage;
		Waitframes(20);
		if (w->isValid())
			Remove(w);
	}

	// Returns the HP the enemy lost, or 1000 if it died.
	int hitEnemy(int defense, int damage, char32[] what)
	{
		char32 text[64];
		sprintf(text, "enemy defense: %s (damage %d)", what, damage);
		Test::Label(text);
		npc n = spawnEnemy(NPC_GORIYA1, defense);
		int hp = n->HP;
		shootEnemy(n, damage);
		int lost = n->isValid() && n->HP > 0 ? hp - n->HP : 1000;
		int stunned = n->isValid() ? n->Stun : -1;
		printf("enemy defense %d (damage %d): lost %d hp, stun %d\n", defense, damage, lost, stunned);
		return lost;
	}

	void testDamageDefenses()
	{
		Test::AssertEqual(hitEnemy(NPCDT_NONE, 4, "normal damage"), 4, "normal damage");
		Test::AssertEqual(hitEnemy(NPCDT_HALFDAMAGE, 4, "half damage"), 2, "half damage");
		Test::AssertEqual(hitEnemy(NPCDT_QUARTERDAMAGE, 4, "quarter damage"), 1, "quarter damage");
		Test::AssertEqual(hitEnemy(NPCDT_STUN, 4, "stun only"), 0, "stun only");
		Test::AssertEqual(hitEnemy(NPCDT_BLOCK1, 0, "block below 1"), 0, "block below 1");
		Test::AssertEqual(hitEnemy(NPCDT_BLOCK1, 4, "no block at 1 or more"), 4, "no block at 1 or more");
		// For enemies, this threshold is scaled by the Hero's damage multiplier.
		Test::AssertEqual(hitEnemy(NPCDT_BLOCK10, 4, "block below 10"), 0, "block below 10");
		Test::AssertEqual(hitEnemy(NPCDT_BLOCK10, 20, "no block at 10 or more"), 20, "no block at 10 or more");
		Test::AssertEqual(hitEnemy(NPCDT_IGNORE1, 0, "ignore below 1"), 0, "ignore below 1");
		Test::AssertEqual(hitEnemy(NPCDT_IGNORE1, 4, "no ignore at 1 or more"), 4, "no ignore at 1 or more");
		Test::AssertEqual(hitEnemy(NPCDT_IGNORE, 4, "always ignore"), 0, "always ignore");
		Test::AssertEqual(hitEnemy(NPCDT_BLOCK, 4, "always block"), 0, "always block");
		Test::AssertEqual(hitEnemy(NPCDT_STUNORBLOCK, 4, "stun or block"), 0, "stun or block");
		Test::AssertEqual(hitEnemy(NPCDT_STUNORIGNORE, 4, "stun or ignore"), 0, "stun or ignore");
		Test::AssertEqual(hitEnemy(NPCDT_2XDAMAGE, 4, "double damage"), 8, "double damage");
		Test::AssertEqual(hitEnemy(NPCDT_3XDAMAGE, 4, "triple damage"), 12, "triple damage");
		Test::AssertEqual(hitEnemy(NPCDT_4XDAMAGE, 4, "quadruple damage"), 16, "quadruple damage");
		Test::AssertEqual(hitEnemy(NPCDT_HEAL, 4, "heal by the damage"), -4, "heal by the damage");
		Test::AssertEqual(hitEnemy(NPCDT_ONEHITKILL, 1, "one hit kill"), 1000, "one hit kill");
	}

	void testSplit()
	{
		Test::Label("split: the goriya splits into 3 gels");
		npc n = spawnEnemy(NPC_GORIYA1, NPCDT_SPLIT);
		n->Attributes[2] = NPC_GEL;
		n->Attributes[3] = 3;
		shootEnemy(n, 4);
		Test::Assert(!n->isValid() || n->HP <= 0, "the split enemy is gone");
		Test::AssertEqual(Test::countNPCs(NPC_GEL), 3, "splits into its split count");
	}

	void testSummon()
	{
		Test::Label("summon: the goriya summons keese and survives");
		npc n = spawnEnemy(NPC_GORIYA1, NPCDT_SUMMON);
		n->Attributes[2] = NPC_KEESE1;
		n->Attributes[3] = 4;
		shootEnemy(n, 4);
		int summoned = Test::countNPCs(NPC_KEESE1);
		printf("summoned %d\n", summoned);
		// Summoning doesn't stop the hit from doing damage.
		Test::Assert(n->isValid() && n->HP == 96, "the summoner takes the hit");
		// How many are summoned is random, up to the summon count.
		Test::Assert(summoned >= 1 && summoned <= 4, "summons up to its summon count");
	}

	void testTransform(int into)
	{
		char32 text[64];
		sprintf(text, "transform: the goriya becomes enemy %d", into);
		Test::Label(text);
		npc n = spawnEnemy(NPC_GORIYA1, NPCDT_TRANSFORM);
		n->Attributes[15] = into;
		// The new enemy is stunned for this long.
		n->Attributes[16] = 30;
		shootEnemy(n, 4);
		int count = Test::countNPCs(into);
		int oldHP = n->isValid() ? n->HP : 0;
		printf("transform into %d: %d enemies of that id, %d total\n", into, count, Screen->NumNPCs);
		// A Zora out of water dies as soon as it appears.
		if (into != NPC_ZORA)
			Test::Assert(count >= 1, "transforms into the new enemy");
		Test::Assert(oldHP <= 0, "the old enemy dies");
	}

	void testTransforms()
	{
		// One of each enemy type, since each is created differently.
		int ids[] = {
			NPC_STALFOS1, NPC_LEEVER1, NPC_TEKTITE1, NPC_PEAHAT, NPC_ZORA,
			NPC_GHINI1, NPC_KEESE1, NPC_WIZZROBE1, NPC_OCTOROCK1S, NPC_WALLMASTER,
			NPC_AQUAMENTUSR, NPC_MOLDORM, NPC_MANHANDLA, NPC_GLEEOK2, NPC_GOHMA1,
			NPC_LANMOLA1, NPC_ENEMYFIRE, NPC_SPINTILE, NPC_ROCK, NPC_TRAP,
			NPC_DODONGO, NPC_DIGDOGGER1, NPC_PATRA1, NPC_PATRABS,
		};
		for (int i = 0; i < SizeOfArray(ids); i++)
			testTransform(ids[i]);
	}

	void testExplosion(int defense, int weaponId, char32[] name)
	{
		Test::Label(name);
		npc n = spawnEnemy(NPC_GORIYA1, defense);
		shootEnemy(n, 4);
		int blasts = Test::countEWeapons(weaponId);
		bool gone = !n->isValid() || n->HP <= 0;
		printf("%s: %d blasts, enemy gone %d\n", name, blasts, gone);
		Test::Assert(gone, "the enemy explodes");
		Test::Assert(blasts > 0, "makes an explosion");
	}

	void testTriggerSecrets()
	{
		Test::Label("trigger secrets: the hit changes the top-left combo");
		Screen->ComboD[SECRET_POS] = 0;
		Screen->ComboF[SECRET_POS] = CF_SECRETS01;
		Screen->SecretCombo[4] = SECRET_COMBO;

		npc n = spawnEnemy(NPC_GORIYA1, NPCDT_TRIGGERSECRETS);
		shootEnemy(n, 4);
		Test::AssertEqual(Screen->ComboD[SECRET_POS], SECRET_COMBO, "triggers the screen's secrets");
		Test::Assert(n->isValid() && n->HP == 96, "the enemy takes the hit");
	}

	void testSwitch()
	{
		Test::Label("switch: the hero and the goriya swap places");
		Hero->X = 32;
		Hero->Y = 64;
		Waitframe();
		npc n = spawnEnemy(NPC_GORIYA1, NPCDT_SWITCH);
		int hx = Hero->X;
		int hy = Hero->Y;
		shootEnemy(n, 4);
		Waitframes(60);
		printf("switch: hero now at %d, %d; enemy at %d, %d\n", Hero->X, Hero->Y, n->X, n->Y);
		Test::AssertEqual(Hero->X, ENEMY_X, "the hero moves to the enemy");
		Test::AssertEqual(n->X, hx, "the enemy moves to the hero");
	}

	void run()
	{
		Test::Init();
		for (int i = 0; i < 176; i++)
		{
			Screen->ComboD[i] = 0;
			Screen->ComboF[i] = 0;
		}

		testHeroDefenses();

		// Out of the way, out of reach of the explosions, and at full health
		// for the enemy tests.
		Hero->X = 16;
		Hero->Y = 144;
		Hero->Dir = DIR_UP;
		Hero->HP = Hero->MaxHP;
		Waitframe();

		testDamageDefenses();
		testSplit();
		testSummon();
		testTransforms();
		testExplosion(NPCDT_BOMBEXPLOSION, EW_BOMBBLAST, "bomb explosion");
		testExplosion(NPCDT_SUPERBOMBEXPLOSION, EW_SBOMBBLAST, "superbomb explosion");
		testExplosion(NPCDT_HARMLESSEXPLOSION, EW_SBOMBBLAST, "harmless explosion");
		testTriggerSecrets();
		testSwitch();

		Test::End();
	}
}
