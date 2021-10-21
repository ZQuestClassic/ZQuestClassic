#ifndef ZC_ENEMY_MAGETELEPORTING_H
#define ZC_ENEMY_MAGETELEPORTING_H

#include "../guys.h"

class MageTeleporting : public enemy
{
public:
	MageTeleporting(const enemy& other, bool newScriptUID, bool clearParentUID);
	MageTeleporting(zfix x, zfix y, int32_t id, int32_t clk);

private:
	enum class animState: char { NORMAL, CHARGING, FIRING };

	animState currAnimState;

	/* Tries to teleport the mage according to its settings
	 * and the version. Returns true if the teleport was successful.
	 */
	bool tryTeleport();

	/* Sets position to a random location on the screen.
	 * Returns true if it's okay to teleport there.
	 */
	bool teleportRandomly();

	/* Teleport to a tile aligned with the hero. This one never fails. */
	void teleportAligned(bool solid_ok);

	/* Turns the Mage to face the hero. */
	void faceHero();

	bool animate(int32_t index) override;
	void draw(BITMAP *dest) override;
};

#endif
