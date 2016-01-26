#ifndef _ZC_ITEM_ITEMEFFECT_H_
#define _ZC_ITEM_ITEMEFFECT_H_

class ItemEffectList;

enum itemEffectType { ie_nayrusLove, ie_clock };

/** This is the base class for effects that persist independently after
 *  an item or weapon has been used.
 */
class ItemEffect
{
public:
    ItemEffect(itemEffectType type);
    virtual ~ItemEffect();
    virtual void activate()=0;
    virtual void update()=0;
    inline void setList(ItemEffectList* l) { list=l; }
    inline itemEffectType getType() const { return type; }
    
private:
    itemEffectType type;
    ItemEffectList* list;
};

/// A set of item effects.
class ItemEffectList
{
public:
    ItemEffectList();
    void add(ItemEffect* newEffect);
    void remove(ItemEffect* effect, bool del);
    void remove(itemEffectType type, bool del);
    void clear();
    void update();
    
private:
    ItemEffect* effects[256];
    int numEffects;
};

/// Return the ItemEffect the given item should create.
// This will be replaced sooner or later.
ItemEffect* getItemEffect(int itemID);

#endif
