#include "auto/test_runner.zs"

// Repro for text with bytes 128-255 that aren't valid UTF-8 drawing '^' (or the
// wrong letter) for each such byte. ZC drew text as plain bytes before
// 2.55-alpha-117, so quests made before then have such bytes in DMap titles,
// DMap intros and item names, meaning Latin-1 letters (é is 0xE9) or font
// glyphs (the Zelda NES font has symbols in 128-255). A byte like that should
// draw the font's glyph for its value, the same as the UTF-8 for that value.
generic script bug_text_latin1_bytes
{
	bitmap b;

	// Draws `str` and returns a checksum of the pixels it drew.
	int Ink(char32[] str, int font)
	{
		b->Clear(0);
		b->DrawString(0, 0, 0, font, 1, -1, TF_NORMAL, str, OP_OPAQUE);
		Waitframe();
		int sum = 0;
		for (int y = 0; y < 16; ++y)
			for (int x = 0; x < 32; ++x)
				if (b->GetPixel(x, y) > 0)
					sum += y * 32 + x + 1;
		return sum;
	}

	void run()
	{
		Test::Init();
		b = new bitmap(32, 16);

		char32 caret[] = "^";

		// A Latin-1 letter, in a font that has Latin-1 letters.
		char32 latin1_e[] = "x";
		latin1_e[0] = 0xE9;
		char32 utf8_e[] = "é";
		int e_ink = Ink(utf8_e, FONT_L2);
		Test::Assert(e_ink != Ink(caret, FONT_L2));
		Test::AssertEqual(Ink(latin1_e, FONT_L2), e_ink);
		Test::AssertEqual(Text->CharWidth(0xE9, FONT_L2), Text->StringWidth(utf8_e, FONT_L2));

		// Latin-1 letters mixed with ASCII.
		char32 latin1_word[] = "Gxnxrique";
		latin1_word[1] = 0xE9;
		latin1_word[3] = 0xE9;
		char32 utf8_word[] = "Générique";
		Test::AssertEqual(Text->StringWidth(latin1_word, FONT_L2), Text->StringWidth(utf8_word, FONT_L2));

		// A Zelda NES font symbol. 0x83 can't start a UTF-8 sequence.
		char32 symbol[] = "x";
		symbol[0] = 0x83;
		char32 utf8_symbol[] = "xx";
		utf8_symbol[0] = 0xC2;
		utf8_symbol[1] = 0x83;
		int symbol_ink = Ink(utf8_symbol, FONT_Z1);
		Test::Assert(symbol_ink != Ink(caret, FONT_Z1));
		Test::AssertEqual(Ink(symbol, FONT_Z1), symbol_ink);

		// 0xA8 used to draw as '(' (0x28).
		char32 symbol2[] = "x";
		symbol2[0] = 0xA8;
		char32 utf8_symbol2[] = "xx";
		utf8_symbol2[0] = 0xC2;
		utf8_symbol2[1] = 0xA8;
		char32 paren[] = "(";
		Test::Assert(Ink(symbol2, FONT_Z1) != Ink(paren, FONT_Z1));
		Test::AssertEqual(Ink(symbol2, FONT_Z1), Ink(utf8_symbol2, FONT_Z1));

		Test::End();
	}
}
