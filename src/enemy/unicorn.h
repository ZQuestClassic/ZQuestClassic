#ifndef ZC_ENEMY_UNICORN_H
#define ZC_ENEMY_UNICORN_H

#include "../guys.h"

class eAquamentus : public enemy
{
public:
	eAquamentus(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	eAquamentus(zfix X,zfix Y,int Id,int Clk);
	virtual bool animate(int index) override;
	virtual void draw(BITMAP *dest) override;
	virtual bool hit(weapon *w) override;

private:
    enum class Facing: bool { left, right };

    Facing facingDir;
    int walkTimer;
    int& shotTimer;

    inline bool tooFarLeft()
	{
		return x<=(facingDir==Facing::right ? 40 : 136);
	}

    inline bool tooFarRight()
	{
		return x>=(facingDir==Facing::right ? 104 : 200);
	}
};

#endif
