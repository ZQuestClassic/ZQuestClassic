// Logs to allegro.log a simple summary of how expensive each frame was to process.

#include "zc/frame_timings.h"
#include "allegro/debug.h"
#include "zc/replay.h"
#include <chrono>
#include <cstdint>
#include <limits>
#include <vector>

static std::vector<uint32_t> timings;
static std::chrono::time_point<std::chrono::steady_clock> last_time;
static bool has_registered_exit_handler;
static bool is_enabled;

void frame_timings_init(bool enable)
{
	if (!timings.empty())
		frame_timings_end();

	is_enabled = enable;
	if (!is_enabled)
		return;

	if (replay_is_replaying())
		timings.reserve(replay_get_meta_int("frames", 0));
	else
		timings.reserve(5 * 60 * 60); // 5 minutes

	last_time = std::chrono::steady_clock::now();

	if (!has_registered_exit_handler)
	{
		has_registered_exit_handler = true;
		std::atexit(frame_timings_end);
	}

	al_trace("DEBUG: -frame-timings is on\n");
}

void frame_timings_poll()
{
	if (!is_enabled)
		return;

	auto now = std::chrono::steady_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time).count();
	if (delta >= 0 && delta <= std::numeric_limits<uint32_t>::max())
		timings.push_back(delta);
	else
		timings.push_back(std::numeric_limits<uint32_t>::max());
	last_time = now;
}

void frame_timings_end()
{
	if (timings.empty())
		return;

	#define MICRO(ms) ((int)(ms*1000))
	int thresholds[5] = {MICRO(1), MICRO(5), MICRO(10), MICRO(16.666), MICRO(30)};
	int counts[5] = {};
	double total = 0;
	for (int i = 0; i < timings.size(); i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (timings[i] >= thresholds[j])
				counts[j]++;
		}

		total += timings[i];
	}

	auto sorted = timings;
	std::sort(sorted.begin(), sorted.end());
	double min_time_ms = sorted.front() / 1000.0;
	double max_time_ms = sorted.back() / 1000.0;
	double median_time_ms = sorted[sorted.size() / 2] / 1000.0;
	double p95_time_ms = sorted[sorted.size() * 0.95] / 1000.0;
	double p99_time_ms = sorted[sorted.size() * 0.99] / 1000.0;
	double p99_9_time_ms = sorted[sorted.size() * 0.999] / 1000.0;

	al_trace("\n=== frame timings ===\n\n");
	al_trace("frames: %zu\n", timings.size());
	al_trace("avg: %.1f ms\n", (double)total / timings.size() / 1000);
	al_trace("min: %.1f ms\n", min_time_ms);
	al_trace("max: %.1f ms\n", max_time_ms);
	al_trace("p50 (median): %.1f ms\n", median_time_ms);
	al_trace("p95: %.1f ms\n", p95_time_ms);
	al_trace("p99: %.1f ms\n", p99_time_ms);
	al_trace("p99.9: %.1f ms\n", p99_9_time_ms);
	for (int j = 0; j < 5; j++)
		al_trace("frames >= %d ms: %d (%.1f%%)\n", thresholds[j] / 1000, counts[j], 100.0 * counts[j] / timings.size());

	al_trace("all frames >= 10 ms:\n");
	for (int i = 0; i < timings.size(); i++)
	{
		// The +1 aligns with the replay frame count.
		int frame = i + 1;
		if (timings[i] >= MICRO(10))
			al_trace("%d: %.1f ms\n", frame, timings[i] / 1000.0);
	}
	al_trace("\n");

	timings.clear();
}
