// Debug instrumentation (-state-hash-log): each frame, append a hash of candidate global state
// regions to a file. Diffing a replay's frame-0 line between a solo run and a run preceded by a
// "poisoner" replay reveals which region is leaking across in-process games.

#include "zc/zelda.h"

#include "base/random.h"
#include "gamedata.h"
#include "iter.h"
#include "zc/ffscript.h"
#include "zc/hero.h"
#include "zc/maps.h"
#include "zc/replay.h"

#include <cstdio>
#include <string>

static std::string g_state_hash_log;

void set_state_hash_log(const std::string& path)
{
	g_state_hash_log = path;
}

static uint64_t state_fnv1a(const void* data, size_t len, uint64_t h)
{
	const uint8_t* p = static_cast<const uint8_t*>(data);
	for (size_t i = 0; i < len; i++)
	{
		h ^= p[i];
		h *= 1099511628211ULL;
	}
	return h;
}

static uint64_t state_hash_zarray(const ZScriptArray& arr, uint64_t h)
{
	auto& data = const_cast<ZScriptArray&>(arr).getData();
	return state_fnv1a(data.data(), data.size() * sizeof(int32_t), h);
}

static uint64_t state_hash_sprite_list(sprite_list& list, uint64_t h)
{
	int32_t count = list.Count();
	h = state_fnv1a(&count, sizeof(count), h);
	for (int32_t i = 0; i < count; i++)
	{
		sprite* s = list.spr(i);
		if (!s)
			continue;
		int32_t sd[] = {
			s->x.getZLong(), s->y.getZLong(), s->z.getZLong(),
			(int32_t)s->dir, s->id, s->uid,
		};
		h = state_fnv1a(sd, sizeof(sd), h);
	}
	return h;
}

void dump_state_hashes()
{
	if (g_state_hash_log.empty() || !replay_is_active())
		return;

	const uint64_t SEED = 1469598103934665603ULL;

	int32_t hero_state[] = {
		Hero.getX().getZLong(), Hero.getY().getZLong(), Hero.getZ().getZLong(),
		Hero.fall.getZLong(), Hero.fakefall.getZLong(),
		Hero.xofs.getZLong(), Hero.yofs.getZLong(), (int32_t)Hero.dir,
	};
	uint64_t hero_h = state_fnv1a(hero_state, sizeof(hero_state), SEED);

	extern zc_randgen script_rnggens[MAX_USER_RNGS];
	uint64_t rng_h = state_fnv1a(script_rnggens, sizeof(script_rnggens), SEED);

	uint64_t globalram_h = SEED;
	if (game)
		for (auto& arr : game->globalRAM)
			globalram_h = state_hash_zarray(arr, globalram_h);

	uint64_t objectram_h = SEED;
	for (auto& [id, arr] : objectRAM)
	{
		objectram_h = state_fnv1a(&id, sizeof(id), objectram_h);
		objectram_h = state_hash_zarray(arr, objectram_h);
	}

	uint64_t sprites_h = SEED;
	for (sprite_list* list : {&guys, &items, &Ewpns, &Lwpns, &chainlinks, &decorations, &portals})
		sprites_h = state_hash_sprite_list(*list, sprites_h);

	int32_t vp[] = {viewport.x, viewport.y, viewport.w, viewport.h};
	uint64_t viewport_h = state_fnv1a(vp, sizeof(vp), SEED);

	uint64_t ffc_h = SEED;
	for_every_ffc([&](const ffc_handle_t& ffc_handle) {
		ffcdata* fc = ffc_handle.ffc;
		int32_t fd[] = {
			fc->x.getZLong(), fc->y.getZLong(), fc->vx.getZLong(), fc->vy.getZLong(),
		};
		ffc_h = state_fnv1a(fd, sizeof(fd), ffc_h);
	});

	int32_t scr_idx[] = {cur_dmap, (int32_t)Hero.current_screen};
	uint64_t screen_h = state_fnv1a(scr_idx, sizeof(scr_idx), SEED);
	if (hero_scr)
	{
		screen_h = state_fnv1a(hero_scr->data, sizeof(hero_scr->data), screen_h);
		screen_h = state_fnv1a(hero_scr->sflag, sizeof(hero_scr->sflag), screen_h);
		screen_h = state_fnv1a(hero_scr->cset, sizeof(hero_scr->cset), screen_h);
	}

	int32_t scroll_state[] = {
		(int32_t)screenscrolling, scrolling_hero_screen, scrolling_map,
		scrolling_dmap, scrolling_destdmap, (int32_t)scrolling_using_new_region_coords,
		newscr_clk,
	};
	uint64_t scroll_h = state_fnv1a(scroll_state, sizeof(scroll_state), SEED);
	scroll_h = state_fnv1a(&cur_region, sizeof(cur_region), scroll_h);
	scroll_h = state_fnv1a(&scrolling_region, sizeof(scrolling_region), scroll_h);

	FILE* f = fopen(g_state_hash_log.c_str(), "a");
	if (!f)
		return;
	fprintf(f,
		"%s frame=%d hero=%016llx heroint=%016llx rng=%016llx globalRAM=%016llx objectRAM=%016llx sprites=%016llx viewport=%016llx ffc=%016llx screen=%016llx scroll=%016llx\n",
		replay_get_replay_path().filename().string().c_str(), replay_get_frame(),
		(unsigned long long)hero_h, (unsigned long long)Hero.debug_state_hash(),
		(unsigned long long)rng_h, (unsigned long long)globalram_h,
		(unsigned long long)objectram_h, (unsigned long long)sprites_h,
		(unsigned long long)viewport_h, (unsigned long long)ffc_h,
		(unsigned long long)screen_h, (unsigned long long)scroll_h);
	if (replay_get_frame() == 0)
		fprintf(f, "HEROFIELDS %s %s\n",
			replay_get_replay_path().filename().string().c_str(),
			Hero.debug_state_string().c_str());
	fclose(f);
}
