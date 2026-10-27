#include "auto/test_runner.zs"

// Repro for characters outside ASCII having platform-dependent values in script
// strings. The engine passed string bytes through a plain `char`, which is signed
// on x86 and macOS but unsigned on ARM Linux, so the UTF-8 bytes of "é" (0xC3 0xA9)
// were stored as negative numbers on most platforms. File->ReadChars also stopped
// at the first such byte, taking it for the end of the file.
generic script bug_string_non_ascii_chars
{
	void run()
	{
		Test::Init();

		char32 s[] = "aé";
		Test::AssertEqual(s[0], 'a');
		Test::AssertEqual(s[1], 0xC3);
		Test::AssertEqual(s[2], 0xA9);

		// Case conversion leaves bytes outside ASCII alone.
		ltou(s);
		Test::AssertEqual(s[0], 'A');
		Test::AssertEqual(s[1], 0xC3);
		Test::AssertEqual(s[2], 0xA9);
		utol(s);
		Test::AssertEqual(s[0], 'a');
		Test::AssertEqual(s[1], 0xC3);
		Test::AssertEqual(s[2], 0xA9);

		file f = new file("bug_string_non_ascii_chars.txt", "w+b");
		f->WriteString("aé\n");

		f->Rewind();
		char32 chars[8];
		Test::AssertEqual(f->ReadChars(chars), 4);
		Test::AssertEqual(chars[0], 'a');
		Test::AssertEqual(chars[1], 0xC3);
		Test::AssertEqual(chars[2], 0xA9);
		Test::AssertEqual(chars[3], '\n');

		f->Rewind();
		char32 line[8];
		Test::AssertEqual(f->ReadString(line), 4);
		Test::AssertEqual(line[0], 'a');
		Test::AssertEqual(line[1], 0xC3);
		Test::AssertEqual(line[2], 0xA9);
		Test::AssertEqual(line[3], '\n');

		f->Remove();

		Test::End();
	}
}
