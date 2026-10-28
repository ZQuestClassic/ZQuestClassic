// Tests the U_UTF8 string functions in third_party/allegro_legacy/src/unicode.c
// on text that isn't valid UTF-8. Quests made before 2.55-alpha-117 have such
// text (Latin-1 letters, or font glyphs in 128-255); each invalid byte reads as
// a character of its own and is written back unchanged.

#include "base/zc_alleg.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <string>
#include <vector>

static std::vector<int> decode(const std::string& s)
{
	std::vector<int> result;
	const char* p = s.c_str();
	while (int c = ugetxc(&p))
		result.push_back(c);
	return result;
}

static void test_decode()
{
	set_uformat(U_UTF8);

	assertEqual(decode("aZ"), (std::vector<int>{'a', 'Z'}));
	// Valid UTF-8.
	assertEqual(decode("\xC3\xA9"), (std::vector<int>{0xE9}));
	assertEqual(decode("\xE2\x82\xAC"), (std::vector<int>{0x20AC}));
	// A Latin-1 letter, then ASCII.
	assertEqual(decode("G\xE9n"), (std::vector<int>{'G', 0xDCE9, 'n'}));
	// A lone continuation byte (this used to read as 0x28, '(').
	assertEqual(decode("\xA8"), (std::vector<int>{0xDCA8}));
	// A sequence cut short by the end of the string.
	assertEqual(decode("\xC3"), (std::vector<int>{0xDCC3}));
	// Overlong encodings and encoded surrogates aren't valid.
	assertEqual(decode("\xC0\x80"), (std::vector<int>{0xDCC0, 0xDC80}));
	assertEqual(decode("\xED\xB3\xA9"), (std::vector<int>{0xDCED, 0xDCB3, 0xDCA9}));

	assertEqual(ugetc("\xE9"), 0xDCE9);
	assertEqual(uwidth("\xE9n"), 1);
	assertEqual(uwidth("\xC3\xA9"), 2);
	assertEqual(ustrlen("G\xE9n\xC3\xA9"), 4);
}

static void test_round_trip()
{
	set_uformat(U_UTF8);

	std::string s = "G\xE9n\xC3\xA9rique \x83\x84\xA8";
	assertEqual(ustrsizez(s.c_str()), (int)s.size() + 1);

	char buf[64];
	ustrzcpy(buf, sizeof(buf), s.c_str());
	assertEqual(std::string(buf), s);

	char c[8];
	int n = usetc(c, ugetc("\xE9"));
	assertEqual(n, 1);
	assertEqual((unsigned char)c[0], (unsigned char)0xE9);
	assertEqual(ucwidth(0xDCE9), 1);
}

static void test_sprintf()
{
	set_uformat(U_UTF8);

	char buf[64];
	// The DMap intro is drawn one byte at a time with "%c" and a char. For byte
	// 0x84 that passes -124 where char is signed, and 0x84 where it isn't (ARM
	// Linux), so check both rather than casting to char here.
	uszprintf(buf, sizeof(buf), "%c", -124);
	assertEqual(std::string(buf), std::string("\x84"));
	uszprintf(buf, sizeof(buf), "%c", 0x84);
	assertEqual(std::string(buf), std::string("\xC2\x84"));
	assertEqual(ugetc(buf), 0x84);
	uszprintf(buf, sizeof(buf), "%c", 0xE9);
	assertEqual(std::string(buf), std::string("\xC3\xA9"));
	// Invalid bytes in the format string and in a %s argument.
	uszprintf(buf, sizeof(buf), "\xE9%d%s\x84", 1, "\xA8");
	assertEqual(std::string(buf), std::string("\xE9" "1\xA8\x84"));
}

TestResults test_unicode(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "decode", test_decode },
		{ "round_trip", test_round_trip },
		{ "sprintf", test_sprintf },
	};

	for (auto& test : tests)
	{
		++tr.total;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return tr;
}
