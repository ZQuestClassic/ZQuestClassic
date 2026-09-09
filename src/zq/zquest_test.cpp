// Editor tests. Run via `zeditor -test-zc <tests dir>` (see
// zeditor_handle_commands); tests/test_zeditor.py drives that.

#include "base/combo.h"
#include "base/misctypes.h"
#include "base/zdefs.h"
#include "qst.h"
#include "items.h"
#include "test_runner/assert.h"
#include "test_runner/test_runner.h"
#include "tiles.h"
#include "base/colors.h"
#include "zq/zq_class.h"
#include "zq/zquest.h"

#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static fs::path test_dir;

void set_zquest_test_dir(std::string const& dir)
{
	test_dir = dir;
}

namespace
{

// What Quest > Defaults resets, captured from the loaded quest so two
// quest loads can be compared.
struct ComboKey
{
	int32_t tile;
	byte flip, walk, type, csets, frames, speed;
	word nextcombo;
	byte nextcset, flag;
	bool operator==(ComboKey const&) const = default;
};

struct Tile
{
	byte format;
	std::vector<byte> data;
	bool operator==(Tile const&) const = default;
};

struct DefaultsSnapshot
{
	std::vector<ComboKey> combos;
	std::vector<Tile> tiles;
	std::vector<byte> colors;
	std::vector<itemdata> items;
	std::vector<wpndata> sprites;
};

// itemdata and wpndata have no operator==; both are plain data that the
// loader memsets before filling, so a byte compare is sound.
template<typename T>
bool bytes_equal(std::vector<T> const& a, std::vector<T> const& b)
{
	return a.size() == b.size() && memcmp(a.data(), b.data(), a.size() * sizeof(T)) == 0;
}

DefaultsSnapshot snapshot_defaults()
{
	DefaultsSnapshot s;
	for (auto const& c : combobuf)
		s.combos.push_back({c.tile, c.flip, c.walk, c.type, c.csets, c.frames, c.speed,
			c.nextcombo, c.nextcset, c.flag});
	for (int32_t t = 0; t < NEWMAXTILES; ++t)
	{
		Tile tile{newtilebuf[t].format, {}};
		if (newtilebuf[t].data)
			tile.data.assign(newtilebuf[t].data, newtilebuf[t].data + tilesize(tile.format));
		s.tiles.push_back(std::move(tile));
	}
	s.colors.assign(colordata, colordata + psTOTAL255);
	s.items.assign(itemsbuf, itemsbuf + MAXITEMS);
	s.sprites.assign(wpnsbuf, wpnsbuf + MAXWPNS);
	return s;
}

void load(fs::path const& path)
{
	int32_t ret = load_quest(path.string().c_str(), false);
	if (ret)
		throw std::runtime_error(fmt::format("failed to load {}: ret == {}", path.string(), ret));
}

// Quest > Defaults reads sections out of the template quest with another
// quest open. Each must succeed and leave that part of the open quest
// matching the template.
void test_defaults_apply()
{
	load(test_dir / "replays/classic_1st.qst");
	auto before = snapshot_defaults();

	assertTrue(init_combos(true, &header));
	assertTrue(init_tiles(true, &header));
	assertTrue(init_colordata(true, &header, &QMisc));
	assertTrue(reset_items(true, &header));
	assertTrue(reset_wpns(true, &header));
	auto after = snapshot_defaults();

	// The two quests differ, so a reset that did nothing shows up here.
	assertTrue(after.combos != before.combos);
	assertTrue(after.tiles != before.tiles);
	assertTrue(after.colors != before.colors);
	assertTrue(!bytes_equal(after.items, before.items));
	assertTrue(!bytes_equal(after.sprites, before.sprites));

	// Same path init_section opens the template by, relative to the cwd.
	load("modules/classic/default.qst");
	auto expected = snapshot_defaults();
	assertTrue(after.combos == expected.combos);
	assertTrue(after.tiles == expected.tiles);
	assertTrue(after.colors == expected.colors);
	assertTrue(bytes_equal(after.items, expected.items));
	assertTrue(bytes_equal(after.sprites, expected.sprites));
}

} // namespace

TestResults test_zquest(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "defaults_apply", test_defaults_apply },
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
