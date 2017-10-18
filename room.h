#ifndef _ZC_ROOM_H_
#define _ZC_ROOM_H_

#define NOMINMAX //stupid windows headers..

struct BITMAP;
class item;
struct mapscr;

class RoomItems
{
public:
    RoomItems();
    void initialize();
    void addItem(item* it, int price=0);
    void drawPrices(BITMAP* target, int offset);
    void clear();
    inline void setPrice(int index, int newPrice)
    {
        prices[index]=newPrice;
    }
    
    inline void setSignedPrices(bool val)
    {
        showSignedPrices=val;
    }
    
    void setPricesVisible(bool val, bool showRupee);
    
    inline void setPricesVisible(bool val)
    {
        setPricesVisible(val, val);
    }
    
    inline item* getItem(int index)
    {
        return (index<numItems) ? items[index] : 0;
    }
    
private:
    item* items[3];
    int prices[3];
    int numItems;
    bool pricesVisible;
    bool showSignedPrices;
    item* rupeeMarker; // The one on the left
};

extern RoomItems roomItems;

void setUpRoom(const mapscr& screen);

#endif
