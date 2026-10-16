// Round-trips data through a packed PACKFILE, exercising the LZSS packer in
// third_party/allegro_legacy/src/lzss.c against its decoder.

#include "zalleg/zalleg.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"
#include "allegro/system.h"
#include <fmt/format.h>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static void ensure_allegro()
{
	static bool done = false;
	if (done) return;
	done = true;
	install_allegro(SYSTEM_NONE, &errno, atexit);
}

static fs::path temp_path()
{
	return fs::temp_directory_path() / "zc_lzss_test.bin";
}

// Writes `data` packed, reads it back, checks it matches, and returns the
// packed file size.
static size_t round_trip(const std::vector<uint8_t>& data, bool byte_at_a_time = false)
{
	ensure_allegro();
	auto path = temp_path();
	std::string path_str = path.string();

	PACKFILE* f = pack_fopen(path_str.c_str(), F_WRITE_PACKED);
	assertTrue(f != nullptr);
	if (byte_at_a_time)
	{
		for (uint8_t b : data)
			pack_putc(b, f);
	}
	else if (!data.empty())
	{
		assertEqual(pack_fwrite(data.data(), data.size(), f), (long)data.size());
	}
	assertEqual(pack_fclose(f), 0);

	size_t packed_size = fs::file_size(path);

	f = pack_fopen(path_str.c_str(), F_READ_PACKED);
	assertTrue(f != nullptr);
	std::vector<uint8_t> result(data.size() + 1);
	long got = pack_fread(result.data(), result.size(), f);
	pack_fclose(f);
	fs::remove(path);

	assertEqual(got, (long)data.size());
	result.resize(data.size());
	assertTrue(result == data);
	return packed_size;
}

static std::vector<uint8_t> pseudo_random(size_t n, uint32_t seed)
{
	std::vector<uint8_t> v(n);
	uint32_t x = seed;
	for (auto& b : v)
	{
		x = x * 1664525u + 1013904223u;
		b = x >> 24;
	}
	return v;
}

static void test_empty_and_tiny()
{
	round_trip({});
	round_trip({42});
	round_trip({1, 2});
	round_trip({7, 7, 7});
	round_trip({0, 0, 0, 0});
}

static void test_zeros()
{
	// Runs of zeros can reference the decoder's zero-filled initial window
	// and then overlap themselves, so they pack extremely well.
	std::vector<uint8_t> zeros(1 << 20, 0);
	size_t packed = round_trip(zeros);
	assertTrue(packed < zeros.size() / 6);
}

static void test_runs_and_periods()
{
	std::vector<uint8_t> data;
	for (int period = 1; period <= 40; period++)
		for (int i = 0; i < 5000; i++)
			data.push_back(uint8_t((i % period) * 7 + period));
	size_t packed = round_trip(data);
	assertTrue(packed < data.size() / 4);
}

static void test_random()
{
	// Incompressible data must still round-trip, and may only grow by the
	// flag bytes (one per eight items).
	auto data = pseudo_random(200000, 1);
	size_t packed = round_trip(data);
	assertTrue(packed <= data.size() + data.size() / 8 + 16);
}

static void test_mixed()
{
	// Structured content with far and near repeats, like a quest file.
	std::vector<uint8_t> data;
	auto noise = pseudo_random(3000, 9);
	std::string text = "The quick brown fox jumps over the lazy dog. ";
	for (int rep = 0; rep < 300; rep++)
	{
		data.insert(data.end(), text.begin(), text.end());
		data.insert(data.end(), noise.begin(), noise.begin() + (rep % 50) * 60);
		data.insert(data.end(), 300, uint8_t(rep));
		for (int i = 0; i < 100; i++)
			data.push_back(uint8_t(i * (rep + 1)));
	}
	size_t packed = round_trip(data);
	assertTrue(packed < data.size() / 2);
}

static void test_chunk_boundaries()
{
	// The packer sees the stream in buffer-sized pieces; make sure matches
	// and literals that straddle those boundaries survive.
	for (size_t n : {4093, 4094, 4095, 4096, 4097, 8189, 8190, 8191, 8192, 12285, 12286})
	{
		std::vector<uint8_t> data(n);
		for (size_t i = 0; i < n; i++)
			data[i] = uint8_t((i / 13) ^ (i % 251));
		round_trip(data);
		round_trip(data, true);
		auto rnd = pseudo_random(n, uint32_t(n));
		round_trip(rnd);
	}
}

static void test_repeat_far_back()
{
	// Repeats exactly at and just beyond the 4K window edge.
	auto block = pseudo_random(4096, 5);
	for (size_t gap : {0u, 1u, 4077u, 4078u, 4079u, 4095u, 4096u, 4097u, 5000u})
	{
		std::vector<uint8_t> data(block.begin(), block.begin() + 64);
		data.insert(data.end(), gap, 0xAB);
		data.insert(data.end(), block.begin(), block.begin() + 64);
		round_trip(data);
	}
}

TestResults test_lzss(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "empty_and_tiny", test_empty_and_tiny },
		{ "zeros", test_zeros },
		{ "runs_and_periods", test_runs_and_periods },
		{ "random", test_random },
		{ "mixed", test_mixed },
		{ "chunk_boundaries", test_chunk_boundaries },
		{ "repeat_far_back", test_repeat_far_back },
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
