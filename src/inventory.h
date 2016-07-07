#ifndef ZC_INVENTORY_H
#define ZC_INVENTORY_H

// Not quite sure yet where this stuff will end up...

class LinkClass;

// Used in LinkClass::startwpn(). canUse returns true if the item can be used.
// activate's return value becomes startwpn's return value, which does whatever
// it is that does.
struct ItemUseFuncs
{
    typedef bool (*func)(int, LinkClass&);
    func canUse;
    func activate;
};

extern const ItemUseFuncs itemFuncs[66]; // 66 is itype_misc

#endif
