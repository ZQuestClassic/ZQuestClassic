#ifndef ZC_GUI_KEY_H
#define ZC_GUI_KEY_H

#include <utility>

namespace GUI
{

struct KeyboardShortcut
{
	int key;
	int message;
};

/* Just a class to put keys for shortcuts into a convenient form. */
class ShortcutKey
{
public:
	struct DummyType {};
	static constexpr DummyType dummy=DummyType {};

	/* This is used when converting 0-9 to a ShortcutKey.
	 * 27 is __allegro_KEY_0.
	 */
	constexpr ShortcutKey(int value): value((value+27)<<8)
	{}

	/* This one is used for constants. It just has a second argument
	* to distinguish it from the one above.
	*/
	constexpr ShortcutKey(unsigned short value, DummyType): value(value)
	{}

	constexpr ShortcutKey(const ShortcutKey& other): value(other.value)
	{}

	/* Used to combine keys, e.g. Ctrl+F. */
	inline constexpr ShortcutKey operator+(ShortcutKey rhs) const
	{
		// It's actually OR, but writing Ctrl+X is more natural.
		// Not that it should make any difference with these.
		return ShortcutKey(value|rhs.value, dummy);
	}

	/* Used for number keys, e.g. Ctrl+9. */
	inline constexpr ShortcutKey operator+(int rhs) const
	{
		return this->operator+(ShortcutKey(rhs));
	}

	template<typename T>
	inline constexpr KeyboardShortcut operator=(T t) const
	{
		return KeyboardShortcut { value, static_cast<int>(t) };
	}

	inline constexpr unsigned short get() const
	{
		return value;
	}

private:
	unsigned short value;
};

/* I don't know why you're writing 1+Ctrl instead of Ctrl+1, but it'll work. */
inline constexpr ShortcutKey operator+(int lhs, ShortcutKey rhs)
{
	return rhs+ShortcutKey(lhs);
}


namespace Key
{
#define KEY(num) ShortcutKey(num<<8, ShortcutKey::dummy)
#define MOD_KEY(num) ShortcutKey(num, ShortcutKey::dummy)

static constexpr ShortcutKey
	// XXX These are made to compare with Allegro scancodes easily.
	// This is done in topLevel.cpp. Do they work on non-QWERTY keyboards?
	// They may be revised at some point either way.
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

	// 0-9 omitted - just use literals or Key(num)

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

}} // namespace GUI::Key

#endif
