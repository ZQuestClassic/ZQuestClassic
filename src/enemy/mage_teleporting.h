#ifndef ZC_ENEMY_MAGETELEPORTING_H
#define ZC_ENEMY_MAGETELEPORTING_H

#include "../guys.h"

class eWizzrobeTeleporting : public enemy
{
public:
	eWizzrobeTeleporting(enemy const & other, bool new_script_uid, bool clear_parent_script_UID);
	eWizzrobeTeleporting(zfix X,zfix Y,int Id,int Clk);
	virtual bool animate(int index) override;
    virtual void draw(BITMAP *dest) override;

private:
    enum class AnimState: char { normal, charging, firing };

    AnimState animState;

    /* Tries to teleport the wizzrobe according to its settings
     * and the version. Returns true if the teleport was successful.
     */
    bool tryTeleport();

    /* Sets position to a random location on the screen.
     * Returns true if it's okay to teleport there.
     */
    bool teleportRandomly();

    /* Teleport to a tile aligned with Link. This one never fails. */
    void teleportAligned(bool solid_ok);

    /* Turns the Wizzrobe to face Link. */
    void faceLink();

};

#endif
