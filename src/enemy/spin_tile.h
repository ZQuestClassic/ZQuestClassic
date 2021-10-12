#ifndef ZC_ENEMY_SPINTILE_H
#define ZC_ENEMY_SPINTILE_H

#include "../guys.h"

class SpinTile: public enemy
{
public:
	SpinTile(const enemy& other, bool newScriptUID, bool clearParentUID);
	SpinTile(zfix x, zfix y, int id, int comboTile);

private:
	bool animate(int index) override;
	void draw(BITMAP* dest) override;
	void drawshadow(BITMAP* dest, bool translucent) override;
};

#endif
