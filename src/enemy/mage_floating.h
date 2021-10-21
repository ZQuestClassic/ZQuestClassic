#ifndef ZC_ENEMY_MAGEFLOATING_H
#define ZC_ENEMY_MAGEFLOATING_H

#include "../guys.h"

class MageFloating : public enemy
{
public:
	MageFloating(const enemy& other, bool newScriptUID, bool clearParentUID);
	MageFloating(zfix x, zfix y, int32_t id, int32_t clk);

private:
	enum class action: int32_t
	{
		// These numbers are chosen to match the original behavior. They don't
		// necessarily make any sense otherwise. currAction is misc, so the
		// numbers probably shouldn't change as long as that's used elsewhere.
		INIT=-3, INIT_2, INIT_3,
		WALKING=0, PHASING, PAUSING, JUMPING
	};

	// These are bound to the old clk3 and misc because
	// who knows what else uses them
	action& currAction;
	int32_t& actionTimer;
	int32_t shotTimer;

	/* Returns true if the mage is in position to fire. */
	bool readyToFire() const;

	/* Turns the mage to face a new direction. */
	void turn(int32_t homing);

	/* May start a diagonal "jump" phase. Decides at random whether to try
	* and fails if the destination would be out of range. Returns true
	* if a jump was started.
	*/
	bool maybeJump();

	bool animate(int32_t index) override;
	void draw(BITMAP *dest) override;
};

#endif
