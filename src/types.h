#ifndef _ZC_TYPES_H_
#define _ZC_TYPES_H_

#include <allegro.h>

// Common data types

typedef unsigned char        byte;  //0-                       255  ( 8 bits)
typedef unsigned short       word;  //0-                    65,535  (16 bits)
typedef unsigned long        dword; //0-             4,294,967,295  (32 bits)
typedef unsigned long long   qword; //0-18,446,744,073,709,551,616  (64 bits)

enum dir { dir_INVALID=-1, dir_UP, dir_DOWN, dir_LEFT, dir_RIGHT };

// This should probably be moved into tiles.h or something
class Orientation
{
public:
    // Yes, it's horrible. I don't care. :p
    // It's not as bad as it looks. The enum values just map the value
    // before rotation to the value after rotation.
    enum rotation
    {
        rot_NONE   = 0x76543210, // 0->0, 1->1, 2->2, etc.
        rot_90CW   = 0x02137564, // 0->4, 1->6, 2->5, etc.
        rot_270CCW = 0x02137564,
        rot_180    = 0x45670123,
        rot_270CW  = 0x31204657,
        rot_90CCW  = 0x31204657
    };

    Orientation(): val() {}
    Orientation(int v): val(v) {}
    inline void flipHorizontal() { val^=1; }
    inline void flipVertical() { val^=2; }
    inline void rotate(int rot) { val=(rot>>(val<<2)); }
    inline operator int() const { return val; }
    
private:
    unsigned int val:3;
};

#endif
