#include "zc/replay_compat.h"
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
