// Halloween easter egg: on October 31st the title screen's sky turns burnt orange over a
// moonlit graveyard, keese flit about, and lightning strikes now and then. Setting the
// launcher's "Title Screen" option to Halloween shows it on any other day.

#include "base/general.h"
#include "base/zc_math.h"
#include "zalleg/pal_tables.h"
#include "zc/title_scene.h"
#include "zc/zc_sys.h"
#include "zc/zelda.h"
#include "zcsfx.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <random>
#include <vector>

namespace {

enum
{
	hcSky0 = vc(1), hcSky1, hcSky2, hcSky3, hcMoon, hcCrater, hcShadow, hcGlow, hcBolt, hcBoltHalo,
	hcCount = hcBoltHalo - hcSky0 + 1,
};

const RGB halloween_colors[] = {
	{0x3a, 0x12, 0x08}, // sky, top
	{0x7a, 0x2a, 0x06},
	{0xb8, 0x48, 0x08}, // burnt orange
	{0xd8, 0x66, 0x10}, // sky, horizon
	{0xf6, 0xe6, 0xb0}, // moon
	{0xd6, 0xbe, 0x80}, // moon craters
	{0x16, 0x0a, 0x14}, // silhouettes
	{0xff, 0xcc, 0x30}, // eyes, jack-o'-lantern glow
	{0xff, 0xff, 0xff}, // lightning bolt
	{0xc8, 0xb4, 0xff}, // lightning bolt's glow
};

static_assert(std::size(halloween_colors) == hcCount);

// What the sky lights up to when lightning strikes.
const RGB halloween_flash_color = {0x9c, 0x7c, 0xbc};

// '#' is silhouette, 'o' is glow.
const char* const keese_frames[3][8] = {
	{
		"#..............#",
		"##............##",
		"###...#..#...###",
		".####.####.####.",
		"..####o##o####..",
		"...##########...",
		".....######.....",
		"......#..#......",
	},
	{
		"................",
		"................",
		"......#..#......",
		"......####......",
		"######o##o######",
		".##############.",
		"..###.####.###..",
		"......#..#......",
	},
	{
		"................",
		"......#..#......",
		".....######.....",
		"....##o##o##....",
		"..############..",
		".#####.##.#####.",
		"###....##....###",
		"##............##",
	},
};

const char* const pumpkin_sprite[] = {
	"....##.....",
	".#########.",
	"###########",
	"##o#####o##",
	"#ooo###ooo#",
	"#####o#####",
	"#o#######o#",
	"##ooooooo##",
	"###o#o#o###",
	".#########.",
};

void draw_sprite_map(BITMAP* dest, const char* const* rows, int num_rows, int x, int y)
{
	for (int r = 0; r < num_rows; r++)
	{
		for (int c = 0; rows[r][c]; c++)
		{
			char ch = rows[r][c];
			if (ch == '#')
				putpixel(dest, x + c, y + r, hcShadow);
			else if (ch == 'o')
				putpixel(dest, x + c, y + r, hcGlow);
		}
	}
}

double angle_diff(double a, double b)
{
	return std::atan2(std::sin(a - b), std::cos(a - b));
}

class HalloweenScene : public TitleScene
{
public:
	HalloweenScene(int w, int h) : w(w), h(h), rng(std::random_device{}())
	{
		for (int i = 0; i < 10; i++)
		{
			// Half swoop in from each side.
			bool from_right = i % 2;
			Keese k;
			k.x = from_right ? w + 16 + uniform(0, 100) : -32 - uniform(0, 100);
			k.y = uniform(8, h * 0.6);
			k.heading = (from_right ? PI : 0) + uniform(-0.4, 0.4);
			k.speed = k.target_speed = uniform(1.0, 1.6);
			k.turn_timer = (int)uniform(20, 60);
			k.flap = uniform(0, 4);
			keese.push_back(k);
		}

		// Tree.
		grow_branch(20, ground_y(20) + 1, PI / 2 + 0.05, 30, 5);

		// Counted from the end of the logo's intro, so the first strike follows soon after it.
		strike_timer = (int)uniform(20, 60);
	}

	void update() override
	{
		for (auto& k : keese)
		{
			// Flutter about like keese do: every so often, veer off and change speed.
			if (--k.turn_timer <= 0)
			{
				k.heading += uniform(-1.6, 1.6);
				k.target_speed = uniform(0.4, 1.9);
				k.turn_timer = (int)uniform(15, 60);
			}
			// Wander too far and swing back toward the middle of the sky.
			if (k.x < 8 || k.x > w - 24 || k.y < 4 || k.y > h - 64)
			{
				double want = std::atan2(h * 0.4 - k.y, w / 2.0 - k.x);
				k.heading += angle_diff(want, k.heading) * 0.08;
			}
			k.speed += (k.target_speed - k.speed) * 0.05;
			k.x += std::cos(k.heading) * k.speed;
			k.y += std::sin(k.heading) * k.speed;
			k.flap += 0.12 + k.speed * 0.1;
		}

		if (strike_frame >= 0 && ++strike_frame >= (int)std::size(strike_intensity))
			strike_frame = -1;
		// No lightning until the logo's intro is over.
		if (!intro_playing && --strike_timer <= 0)
		{
			if (!flash_reduction_enabled(false))
			{
				strike();
				strike_frame = 0;
			}
			// Sometimes a second strike follows right on the first's heels.
			strike_timer = uniform(0, 1) < 0.3 ? (int)uniform(20, 40) : (int)uniform(150, 420);
		}

		if (--glow_timer <= 0)
		{
			// A few set levels rather than any brightness at all, since every new palette
			// costs a lookup table (see refresh_rgb_tables).
			glow = 0.75 + 0.0625 * (int)uniform(0, 5);
			glow_timer = 4;
		}

		double flash = strike_frame >= 0 ? strike_intensity[strike_frame] : 0;
		for (int i = 0; i < hcCount; i++)
		{
			RGB c = halloween_colors[i];
			int index = hcSky0 + i;
			if (index <= hcSky3)
			{
				c.r += (halloween_flash_color.r - c.r) * flash;
				c.g += (halloween_flash_color.g - c.g) * flash;
				c.b += (halloween_flash_color.b - c.b) * flash;
			}
			else if (index == hcGlow)
			{
				c.r *= glow;
				c.g *= glow;
				c.b *= glow;
			}
			if (c.r != RAMpal[index].r || c.g != RAMpal[index].g || c.b != RAMpal[index].b)
			{
				RAMpal[index] = c;
				refreshpal = true;
			}
		}
	}

	void draw(BITMAP* dest) override
	{
		// The sky: solid bands, with ordered dithering where they meet.
		static const int bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
		// Bounded by `dest` itself, as this writes pixels directly.
		for (int y = 0; y < dest->h; y++)
		{
			double t = (double)y / h * 3;
			int band = std::min(2, (int)t);
			double frac = std::clamp((t - band - 0.25) * 2, 0.0, 1.0);
			int threshold = frac * 16;
			for (int x = 0; x < dest->w; x++)
				dest->line[y][x] = hcSky0 + band + (bayer[y & 3][x & 3] < threshold);
		}

		int moon_x = w - 48, moon_y = 44;
		circlefill(dest, moon_x, moon_y, 20, hcMoon);
		circlefill(dest, moon_x - 6, moon_y - 4, 4, hcCrater);
		circlefill(dest, moon_x + 7, moon_y + 5, 3, hcCrater);
		circlefill(dest, moon_x - 2, moon_y + 10, 2, hcCrater);

		if (strike_frame >= 0 && strike_intensity[strike_frame] >= 0.5)
		{
			for (auto& s : bolt)
			{
				for (auto [dx, dy] : {std::pair{-1, 0}, {2, 0}, {0, -1}, {1, 1}})
					line(dest, s.x1 + dx, s.y1 + dy, s.x2 + dx, s.y2 + dy, hcBoltHalo);
			}
			for (auto& s : bolt)
			{
				line(dest, s.x1, s.y1, s.x2, s.y2, hcBolt);
				if (s.thick)
					line(dest, s.x1 + 1, s.y1, s.x2 + 1, s.y2, hcBolt);
			}
		}

		for (auto& s : tree)
		{
			for (int t = 0; t < s.thick; t++)
				line(dest, s.x1 + t, s.y1, s.x2 + t, s.y2, hcShadow);
		}

		for (int x = 0; x < w; x++)
			vline(dest, x, ground_y(x), h - 1, hcShadow);

		// Tombstones, and a cross.
		for (int x : {46, 74, 198})
		{
			int gy = ground_y(x + 4);
			rectfill(dest, x, gy - 9, x + 8, gy + 1, hcShadow);
			circlefill(dest, x + 4, gy - 9, 4, hcShadow);
		}
		int cross_x = 228, cross_gy = ground_y(cross_x);
		rectfill(dest, cross_x, cross_gy - 15, cross_x + 1, cross_gy, hcShadow);
		rectfill(dest, cross_x - 3, cross_gy - 12, cross_x + 4, cross_gy - 11, hcShadow);

		for (int x : {96, 170})
			draw_sprite_map(dest, pumpkin_sprite, std::size(pumpkin_sprite), x, ground_y(x + 5) - 8);

		for (auto& k : keese)
		{
			// Up, level, down, level.
			static const int flap_cycle[4] = {0, 1, 2, 1};
			int frame = flap_cycle[(int)k.flap % 4];
			draw_sprite_map(dest, keese_frames[frame], 8, (int)k.x, (int)k.y);
		}
	}

private:
	struct Keese
	{
		double x, y, heading, speed, target_speed, flap;
		int turn_timer;
	};

	struct Segment
	{
		int x1, y1, x2, y2, thick;
	};

	// Brightness of the flash on each frame of a strike: a flicker, then a fade.
	static constexpr double strike_intensity[] = {1.0, 0.6, 0.15, 0.9, 0.75, 0.55, 0.4, 0.28, 0.18, 0.1, 0.04};

	int w, h;
	std::mt19937 rng;
	std::vector<Keese> keese;
	std::vector<Segment> tree;
	std::vector<Segment> bolt;
	int strike_timer;
	int strike_frame = -1;
	int glow_timer = 0;
	double glow = 1;

	double uniform(double lo, double hi)
	{
		return std::uniform_real_distribution<double>(lo, hi)(rng);
	}

	int ground_y(int x) const
	{
		// High enough that "PRESS ANYTHING TO START" sits on the ground, below the graves.
		return h - 26 + 3 * std::sin(x * 0.035) + 2 * std::sin(x * 0.09 + 1.3);
	}

	void grow_branch(double x, double y, double angle, double len, int depth)
	{
		double x2 = x + std::cos(angle) * len;
		double y2 = y - std::sin(angle) * len;
		tree.push_back({(int)x, (int)y, (int)x2, (int)y2, std::max(1, depth - 1)});
		if (depth == 1)
			return;

		grow_branch(x2, y2, angle + uniform(0.3, 0.7), len * uniform(0.6, 0.75), depth - 1);
		grow_branch(x2, y2, angle - uniform(0.3, 0.7), len * uniform(0.6, 0.75), depth - 1);
	}

	// A jagged bolt from the top of the sky down to the ground, with a fork or two.
	void strike()
	{
		bolt.clear();
		// Clear of the moon.
		double x = uniform(30, w - 90), y = 0;
		// Bomb sfx = thunder.
		sfx_ex(WAV_BOMB, x * 255 / w, false, true, 40_zf);
		while (y < ground_y(x))
		{
			double nx = x + uniform(-10, 10);
			double ny = std::min<double>(y + uniform(6, 16), ground_y(nx));
			bolt.push_back({(int)x, (int)y, (int)nx, (int)ny, 2});
			if (y > 20 && uniform(0, 1) < 0.25)
			{
				double bx = nx, by = ny;
				double dir = nx < x ? -1 : 1;
				for (int i = 0, n = (int)uniform(2, 5); i < n; i++)
				{
					double bnx = bx + dir * uniform(3, 10);
					double bny = by + uniform(4, 10);
					bolt.push_back({(int)bx, (int)by, (int)bnx, (int)bny, 0});
					bx = bnx;
					by = bny;
				}
			}
			x = nx;
			y = ny;
		}
	}
};

}

bool is_halloween_today()
{
	time_t now = time(nullptr);
	tm* t = localtime(&now);
	return t && t->tm_mon == 9 && t->tm_mday == 31;
}

std::unique_ptr<TitleScene> create_halloween_title_scene(int w, int h)
{
	return std::make_unique<HalloweenScene>(w, h);
}
