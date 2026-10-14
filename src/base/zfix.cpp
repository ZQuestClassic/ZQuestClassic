#include "base/zfix.h"
#include <fmt/format.h>

bool(*zfix_round_force_nearest)();

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
	if(zfix_round_force_nearest && zfix_round_force_nearest())
		return round(ROUND_NEAREST);
	return round(rt);
}
zfix& zfix_round::doRound()
{
	if(zfix_round_force_nearest && zfix_round_force_nearest())
		return do_round(ROUND_NEAREST);
	return do_round(rt);
}

void to_json(json& j, const zfix& z)
{
	j = z.str_trim();
}
void from_json(const json& j, zfix& z)
{
	std::string s;
	j.get_to(s);
	z = atozfix(s.c_str());
}

