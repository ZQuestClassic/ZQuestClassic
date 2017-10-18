#ifndef _ZC_ITEM_ITEMACTION_H_
#define _ZC_ITEM_ITEMACTION_H_

/** This is the base class for things that happen when an item is used,
 *  such as the hookshot being fired or a spell being cast.
 *  This does not include any effects that occur when Link is able
 *  to move freely again. For instance, this would cover the casting
 *  sequence of Nayru's Love, but not the spell's effect afterward.
 */
class ItemAction
{
public:
    virtual ~ItemAction() {}
    virtual void update()=0;
    virtual bool isFinished() const=0;
    virtual void abort() {}
};

#endif
