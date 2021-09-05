#ifndef ZC_GUI_KEY_H
#define ZC_GUI_KEY_H

namespace gui::key
{

/* Just a class to put keys for shortcuts into a convenient form.
 * This is easy to screw up, but it's only expected to be used for
 * one specific purpose. Just make sure TopLevelWidget is
 * handling them correctly.
 */
class Key
{
public:
    /* This is used when converting 0-9 to a Key or converting an int
     * obtained by casting a Key to int back to a Key.
     * 27 is __allegro_KEY_0.
     */
    constexpr Key(int v):
        value((v&0x10000) ? v : ((v+27)<<8))
    {}

    constexpr Key(const Key& other): value(other.value)
    {}

    /* Used to combine keys, e.g. Ctrl+F. */
    inline constexpr Key operator+(Key rhs) const
    {
        // It's actually OR, but writing Ctrl+X is more natural.
        return Key((int)(value|rhs.value)|0x10000);
    }

    /* Used for number keys, e.g. Ctrl+9. */
    inline constexpr Key operator+(int rhs) const
    {
        return this->operator+(Key(rhs));
    }

    /* Returns the low 16 bits, which is what you want to compare against
     * Allegro's values. Don't confuse this with the cast to int!
     */
    inline constexpr unsigned short get() const
    {
        return value;
    }

    /* Returns the value with a bit set to indicate that the result
     * has been cast from Key to int. This should be converted back
     * to a Key before using.
     */
    inline constexpr operator int() const
    {
        return ((int)value)|0x10000;
    }

private:
    unsigned short value;
};

/* I don't know why you're writing 1+Ctrl instead of Ctrl+1, but it'll work. */
inline constexpr Key operator+(int lhs, Key rhs)
{
    return rhs+Key(lhs);
}

#define KEY(num) Key((num<<8)|0x10000)
#define MOD_KEY(num) Key(num|0x10000)

static constexpr Key
    // XXX These are made to compare with Allegro scancodes easily.
    // This is done in topLevel.cpp. Do they work on non-QWERTY keyboards?
    // These may be revised at some point, too.
    Shift=MOD_KEY(1),
    Ctrl=MOD_KEY(2),
    Alt=MOD_KEY(4),

    A=KEY(1),
    B=KEY(2),
    C=KEY(3),
    D=KEY(4),
    E=KEY(5),
    F=KEY(6),
    G=KEY(7),
    H=KEY(8),
    I=KEY(9),
    J=KEY(10),
    K=KEY(11),
    L=KEY(12),
    M=KEY(13),
    N=KEY(14),
    O=KEY(15),
    P=KEY(16),
    Q=KEY(17),
    R=KEY(18),
    S=KEY(19),
    T=KEY(20),
    U=KEY(21),
    V=KEY(22),
    W=KEY(23),
    X=KEY(24),
    Y=KEY(25),
    Z=KEY(26),

    // 0-9 omitted

    F1=KEY(47),
    F2=KEY(48),
    F3=KEY(49),
    F4=KEY(50),
    F5=KEY(51),
    F6=KEY(52),
    F7=KEY(53),
    F8=KEY(54),
    F9=KEY(55),
    F10=KEY(56),
    F11=KEY(57),
    F12=KEY(58),

    Esc=KEY(59),
    Tilde=KEY(60),
    Minus=KEY(61),
    Equals=KEY(62),
    Backspace=KEY(63),
    Tab=KEY(64),
    LeftBrace=KEY(65),
    RightBrace=KEY(66),
    Enter=KEY(67),
    Colon=KEY(68),
    Quote=KEY(69),
    Backslash=KEY(70),
    Comma=KEY(72),
    Period=KEY(73),
    Slash=KEY(74),
    Space=KEY(75),
    Ins=KEY(76),
    Del=KEY(77),
    Home=KEY(78),
    End=KEY(79),
    PgUp=KEY(80),
    PgDn=KEY(81),
    Left=KEY(82),
    Right=KEY(83),
    Up=KEY(84),
    Down=KEY(85),
    Asterisk=KEY(87);

#undef KEY
#undef MOD_KEY
}

#endif
