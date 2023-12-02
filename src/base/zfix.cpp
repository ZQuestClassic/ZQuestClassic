#include "base/zfix.h"
#include <fmt/format.h>
#include "zc/replay.h"

zfix_round zfix::rnd(roundType rt) const
{
	return zfix_round(*this,rt);
}
zfix& zfix::do_round(roundType rt)
{
	switch(rt)
	{
		default:
		case ROUND_NEAREST:
			return zfix::doRound();
		case ROUND_DOWN:
			return doFloor();
		case ROUND_UP:
			return doCeil();
		case ROUND_TO_0:
			return doTrunc();
		case ROUND_AWAY_0:
			return doRoundAway();
	}
}

int32_t zfix_round::getRound() const
{
	if(replay_version_check(0,23))
		return round(ROUND_NEAREST);
	return round(rt);
}
zfix& zfix_round::doRound()
{
	if(replay_version_check(0,23))
		return do_round(ROUND_NEAREST);
	return do_round(rt);
}

