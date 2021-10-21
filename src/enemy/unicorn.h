#ifndef ZC_ENEMY_UNICORN_H
#define ZC_ENEMY_UNICORN_H

#include "../guys.h"

class Unicorn : public enemy
{
public:
	Unicorn(enemy const & other, bool newScriptUID, bool clearParentUID);
	Unicorn(zfix x, zfix y, int32_t id, int32_t clk);

private:
	enum class facing: bool { LEFT, RIGHT };

	facing facingDir;
	int32_t walkTimer;
	int32_t& shotTimer;

	inline bool tooFarLeft() const
	{
		return x <= ((facingDir == facing::RIGHT) ? 40_x : 136_x);
	}

	inline bool tooFarRight() const
	{
		return x >= ((facingDir == facing::RIGHT) ? 104_x : 200_x);
	}

	bool animate(int32_t index) override;
	void draw(BITMAP *dest) override;
	bool hit(weapon *w) override;
};

#endif
