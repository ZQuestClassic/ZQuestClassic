#ifndef ZC_ENEMY_MAGEFLOATING_H
#define ZC_ENEMY_MAGEFLOATING_H

#include "../guys.h"

class eWizzrobeFloating : public enemy
{
public:
	eWizzrobeFloating(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	eWizzrobeFloating(zfix X, zfix Y, int Id, int Clk);
	virtual bool animate(int index) override;
	virtual void draw(BITMAP *dest) override;

private:
    enum class Action: int
    {
        init=-3, init2, init3, // TODO Not this
        walking=0, phasing, pausing, jumping, initialize
    };

    // These are bound to the old clk3 and misc because
    // who knows what else uses them
    Action& action;
    int& actionTimer;
    int shotTimer;

    /* Returns true if the wizzrobe is in position to fire. */
    bool readyToFire() const;

    /* Turns the wizzrobe to face a new direction. */
    void turn(int homing);

    /* May start a diagonal "jump" phase. Decides at random whether to try
     * and fails if the destination would be out of range. Returns true
     * if a jump was started.
     */
    bool maybeJump();

    /* Returns true if the wizzrobe can jump to this position. */
    inline bool canJumpTo(zfix x, zfix y) const
    {
        return x>=32_x && x<=208_x && y>=32_x && y<=128_x;
    }
};

#endif
