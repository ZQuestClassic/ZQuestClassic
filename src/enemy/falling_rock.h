#ifndef ZC_ENEMY_FALLINGROCK_H
#define ZC_ENEMY_FALLINGROCK_H

#include "guys.h"

class FallingRock: public enemy
{
public:
	FallingRock(zfix x, zfix y, int id);
	FallingRock(const enemy& other, bool newScriptUID, bool clearParentUID);

protected:
	short fallTimer;
	char bounceTimer;
	direction drawDir;

	inline bool isBig() const { return dmisc10; }
	bool animate(int index) override;
	int takehit(weapon* w) override;
	void draw(BITMAP* dest) override;
	void drawshadow(BITMAP* dest, bool translucent) override;
};

#endif
