#ifndef _ZC_LINKZSCRIPTINTERFACE_H_
#define _ZC_LINKZSCRIPTINTERFACE_H_

#include "types.h"
class LinkClass;

/** All ZScript access to Link should go through this class. It'll perform
 *  validation and any extra steps needed to keep things working correctly.
 */
class LinkZScriptInterface
{
public:
    LinkZScriptInterface();
    int getX() const;
    int getY() const;
    int getZ() const;
    void setX(int x);
    void setY(int y);
    void setZ(int z);
    int getDir() const;
    void setDir(int newDir);
    void setJump(fix newJump);
    void setAction(int action);
    int getLadderX() const;
    int getLadderY() const;
    void warp(int dmap, int screen, bool pit);
    inline bool warpPending() const { return warpDMap!=-1; }
    void activateWarp();
    
private:
    LinkClass& link;
    int warpDMap, warpScreen;
    bool warpIsPit;
};

#endif
