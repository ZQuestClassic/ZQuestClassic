#include "zc/replay_compat.h"
#include "base/zc_math.h"
#include "zc/replay.h"

// Returns true if the active replay was created in a build less recent than 2.55.patch.
//
// For bugs also fixed in 2.55, we must also check the ZC version rather than just the replay
// version, since in 2.55 the replay version is frozen at 39.
static bool check_2_55(int patch)
{
	if (!replay_is_active())
		return false;

	auto zc_version_created = replay_get_zc_version_created();
	if (zc_version_created.well_formed)
	{
		if (zc_version_created.major < 2)
			return true; // Replays didn't exist at this point ... but whatever.
		if (zc_version_created.major == 2 && zc_version_created.minor < 55)
			return true; // Replays didn't exist at this point ... but whatever.
		if (zc_version_created.major == 2 && zc_version_created.minor == 55 && zc_version_created.patch < patch)
			return true;
	}

	return false;
}

bool replay_compat_pitslide_bug()
{
	if (check_2_55(11))
		return true;

	return !replay_version_check(45);
}

bool replay_compat_held_items_only_held_always_bug()
{
	if (check_2_55(11))
		return true;

	return !replay_version_check(44);
}

bool replay_compat_hookshot_snap_player_bug()
{
	if (check_2_55(12))
		return true;

	return !replay_version_check(46);
}

bool replay_compat_old_movement_off_by_one_bug()
{
	if (check_2_55(12))
		return true;

	return !replay_version_check(48);
}

bool replay_compat_charging_during_scroll_bug()
{
	if (check_2_55(13))
		return true;

	return !replay_version_check(50);
}

// https://discord.com/channels/876899628556091432/1479336319674220575
bool replay_compat_whistle_stuck_bug()
{
	if (check_2_55(14))
		return true;

	return !replay_version_check(52);
}

// Trig, inverse trig, and log/pow switched from libm (and the replay-only Q15
// trig) to the deterministic tables in zc_math.cpp in replay version 59 /
// ZC 2.55.15. This owns the entire mode policy so zc_math.cpp needs no
// knowledge of the replay system.
// https://discord.com/channels/876899628556091432/1509803675605008384
void replay_compat_setup_zc_maths()
{
	using zc::math::MathsMode;

	// legacy_maths can only be true when a replay is active: check_2_55 returns
	// false with no replay, and replay_version_check(59) returns true with no
	// replay, so an inactive session always resolves to New.
	bool legacy_maths = check_2_55(15) || !replay_version_check(59);
	if (!legacy_maths)
		zc::math::set_maths_mode(MathsMode::New);
	else if (replay_version_check(21))
		zc::math::set_maths_mode(MathsMode::LegacyReplayV21);
	else if (replay_version_check(4))
		zc::math::set_maths_mode(MathsMode::LegacyReplayV4);
	else
		zc::math::set_maths_mode(MathsMode::LegacyReplayV0);
}
