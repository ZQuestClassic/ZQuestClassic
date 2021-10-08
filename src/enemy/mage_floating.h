#ifndef ZC_ENEMY_MAGEFLOATING_H
#define ZC_ENEMY_MAGEFLOATING_H

#include "../guys.h"

class MageFloating : public enemy
{
public:
	MageFloating(const enemy& other, bool newScriptUID, bool clearParentUID);
	MageFloating(zfix x, zfix y, int id, int clk);

private:
	enum class action: int
	{
		// XXX This is set up according to preserve the old behavior.
		// It doesn't necessarily make any sense otherwise. currAction is misc,
		// so the numbers can't change as long as that's used elsewhere.
		INIT=-3, INIT_2, INIT_3,
		WALKING=0, PHASING, PAUSING, JUMPING
	};

	// These are bound to the old clk3 and misc because
	// who knows what else uses them
	action& currAction;
	int& actionTimer;
	int shotTimer;

	/* Returns true if the mage is in position to fire. */
	bool readyToFire() const;

	/* Turns the mage to face a new direction. */
	void turn(int homing);

	/* May start a diagonal "jump" phase. Decides at random whether to try
	* and fails if the destination would be out of range. Returns true
	* if a jump was started.
	*/
	bool maybeJump();

	bool animate(int index) override;
	void draw(BITMAP *dest) override;
};

#endif
