#include "std.zh"
#include "auto/test_runner.zs"

generic script sprites
{
	char32 str_itemsprite[] = "item";
	char32 str_ffc[] = "ffc";
	char32 str_player[] = "player";
	char32 str_lweapon[] = "lwpn";
	char32 str_eweapon[] = "ewpn";
	char32 str_npc[] = "npc";

	void drawText(int type, sprite s)
	{
		// TODO: come up with nice way to get "type" of variable at runtime.
		if (type == 0)
			Screen->DrawString(0, s->X, s->Y - 10, FONT_S, 0, 1, TF_NORMAL, str_itemsprite);
		else if (type == 1)
			Screen->DrawString(0, s->X, s->Y - 10, FONT_S, 0, 1, TF_NORMAL, str_ffc);
		else if (type == 2)
			Screen->DrawString(0, s->X, s->Y - 10, FONT_S, 0, 1, TF_NORMAL, str_player);
		else if (type == 3)
			Screen->DrawString(0, s->X, s->Y - 10, FONT_S, 0, 1, TF_NORMAL, str_lweapon);
		else if (type == 4)
			Screen->DrawString(0, s->X, s->Y - 10, FONT_S, 0, 1, TF_NORMAL, str_eweapon);
		else if (type == 5)
			Screen->DrawString(0, s->X, s->Y - 10, FONT_S, 0, 1, TF_NORMAL, str_npc);
	}

	void run()
	{
		Test::Init();

		Waitframe();

		itemsprite item = Screen->CreateItem(I_HEARTCONTAINER);
		Test::Assert(item, "item not ok");

		sprite a_sprite = item;
		Test::Assert(a_sprite, "a_sprite not ok");
		Test::AssertEqual(<sprite>item, a_sprite); // TODO: template types/generics should be able to match on a base type w/o an explicit cast

		print_it(item);
		print_it(a_sprite);
		print_it(Hero);

		ffc f = Screen->LoadFFC(1);
		f->Data = 161;

		lweapon lwpn = Screen->CreateLWeapon(LW_SCRIPT1);
		lwpn->ScriptTile = 65; // fire

		eweapon ewpn = Screen->CreateEWeapon(EW_SCRIPT1);
		ewpn->ScriptTile = 65; // fire

		npc enemy = Screen->CreateNPC(NPC_ARMOS);
		enemy->Step = 0;

		sprite sprites[] = {item, f, Hero, lwpn, ewpn, enemy};
		Test::AssertEqual(<sprite>item, sprites[0]);
		Test::AssertEqual(<sprite>f, sprites[1]);
		Test::AssertEqual(<sprite>Hero, sprites[2]);
		Test::AssertEqual(<sprite>lwpn, sprites[3]);
		Test::AssertEqual(<sprite>ewpn, sprites[4]);
		Test::AssertEqual(<sprite>enemy, sprites[5]);

		int num_sprites = SizeOfArray(sprites);
		for (int i = 0; i < num_sprites; i++)
		{
			sprite s = sprites[i];
			s->X = i * 230 / num_sprites;
			s->Y = 32 + i * 16;
		}

		int[] ox = {item->X, f->X, Hero->X, lwpn->X, ewpn->X, enemy->X};
		int[] oy = {item->Y, f->Y, Hero->Y, lwpn->Y, ewpn->Y, enemy->Y};
		int[] ocset = {item->CSet, f->CSet, Hero->CSet, lwpn->CSet, ewpn->CSet, enemy->CSet};

		int time = 0;
		while (time <= 60 * 5)
		{
			Screen->DrawString(0, 0, 10, FONT_S, 0, 1, TF_NORMAL, "x y rotation cset scale");
			for (int i = 0; i < num_sprites; i++)
			{
				sprite s = sprites[i];
				s->X = ox[i];
				s->Y = oy[i];
				modify_sprite(s, 16, time);
				drawText(i, s);
			}
			time += 1;
			Waitframe();
		}
		for (int i = 0; i < num_sprites; i++)
		{
			sprite s = sprites[i];
			s->X = ox[i];
			s->Y = oy[i];
			s->Rotation = 0;
			s->CSet = ocset[i];
			s->Scale = 1;
		}

		time = 0;
		while (time <= 60 * 5)
		{
			Screen->DrawString(0, 0, 10, FONT_S, 0, 1, TF_NORMAL, "DrawXOffset DrawYOffset");
			for (int i = 0; i < num_sprites; i++)
			{
				sprite s = sprites[i];
				s->Scale = 1;
				s->DrawXOffset = time / 20;
				s->DrawYOffset = time / 20;
				drawText(i, s);
			}
			time += 1;
			Waitframe();
		}
		for (int i = 0; i < num_sprites; i++)
		{
			sprite s = sprites[i];
			s->DrawXOffset = 0;
			s->DrawYOffset = 0;
		}

		time = 0;
		while (time <= 60 * 5)
		{
			Screen->DrawString(0, 0, 10, FONT_S, 0, 1, TF_NORMAL, "ScriptTile ScriptFlip");
			for (int i = 0; i < num_sprites; i++)
			{
				sprite s = sprites[i];
				s->Scale = 1;
				s->ScriptTile = 100 + time / 30;
				s->ScriptFlip = time / 30;
				drawText(i, s);
			}
			time += 1;
			Waitframe();
		}
		for (int i = 0; i < num_sprites; i++)
		{
			sprite s = sprites[i];
			s->ScriptTile = -1;
			s->ScriptFlip = -1;
		}

		time = 0;
		while (time <= 60 * 5)
		{
			Screen->DrawString(0, 0, 10, FONT_S, 0, 1, TF_NORMAL, "TileWidth TileHeight");
			for (int i = 0; i < num_sprites; i++)
			{
				sprite s = sprites[i];
				s->ScriptTile = 26780;
				s->Extend = 3;
				s->TileWidth = 1 + time / 30;
				s->TileHeight = 1 + time / 30;
				drawText(i, s);
			}
			time += 1;
			Waitframe();
		}
		for (int i = 0; i < num_sprites; i++)
		{
			sprite s = sprites[i];
			s->Extend = 0;
			s->ScriptTile = -1;
			s->TileWidth = 1;
			s->TileHeight = 1;
		}

		Test::End();
	}

	void print_it(itemsprite i)
	{
		printf("itemsprite: %d %d %d\n", i->ID, i->X, i->Y);
	}

	void print_it(sprite i)
	{
		printf("sprite: %d %d\n", i->X, i->Y);
	}

	void modify_sprite(sprite sprite, int radius, int time)
	{
		int degrees = time * 3;
		sprite->X += radius * Cos(degrees);
		sprite->Y += radius * Sin(degrees);
		sprite->Rotation = degrees;
		sprite->CSet = (time / 10) % 16;
		sprite->Scale = time / 60;
	}
}
