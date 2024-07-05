// Defines operator overloads for bitwise operators that work on enum classes.

#ifndef BASE_FLAGS_H_
#define BASE_FLAGS_H_

#include <type_traits>

// This is within a namespace because otherwise these operator overloads would apply to all
// enum types, which could be problematic for non-flag enums.
namespace flags {

constexpr unsigned int F(unsigned int n)
{
	return 1 << n;
}

// These are all purposefully unscoped enums (`guy_bhit` instead of `flags::guy_flags::guy_bhit`)
// for two reasons:
// 1) less typing throughout the code base
// 2) they were original defines, so lots of code would otherwise need to be updated
// Since they are all unscoped, follow a naming convention where each value is prepended by
// its enum name to avoid collisions.

// TODO: combine guy_flags and guy_flags_2 (be sure to handle npcdata scripting).

enum guy_flags : uint32_t
{
	guy_bhit                = F(0), // Boomerangs will stun.
	guy_invisible           = F(1), // Skip drawing.
	guy_never_return        = F(2), // Once dead, never returns even after leaving the screen.
	guy_doesnt_count        = F(3), // Doesn't count as beatable enemy.
	                                // qr_UNBEATABLES_DONT_KEEP_DEAD impacts this behavior.
	guy_fade_flicker        = F(4),
	guy_fade_instant        = F(5),
	guy_shield_front        = F(24), // Invulnerable in the front (relative to direction of movement).
	guy_shield_left         = F(25), // Invulnerable on the left (relative to direction of movement).
	guy_shield_right        = F(26), // Invulnerable on the right (relative to direction of movement).
	guy_shield_back         = F(27), // Invulnerable in the back (relative to direction of movement).
	guy_bkshield            = F(28), // Shield can be broken by hammer.
	guy_lens_only           = F(31), // Only draw when lens item is active.

	// Old flags, some reused.
	guy_superman            = F(3),
	guy_sbombonly           = F(4),
	guy_weak_arrow          = F(29),
};

enum guy_flags2 : uint32_t
{
	guy_flashing            = F(0),
	guy_zora                = F(1),
	guy_rock                = F(2),
	guy_trap                = F(3),
	guy_trph                = F(4),
	guy_trpv                = F(5),
	guy_trp4                = F(6),
	guy_trplr               = F(7),
	guy_trpud               = F(8),
	guy_trp2                = F(9),
	guy_fire                = F(10),
	guy_armos               = F(11),
	guy_ghini               = F(12),
	guy_ganon               = F(13),
	guy_blinking            = F(14),
	guy_transparent         = F(15),
	guy_ignoretmpnr         = F(16),
	guy_ignore_kill_all     = F(17),
};

enum move_flags : uint32_t
{
	move_none               = 0,
	move_obeys_grav         = F(0),
	move_can_pitfall        = F(1),
	move_can_pitwalk        = F(2),
	move_can_waterdrown     = F(3),
	move_can_waterwalk      = F(4),
	move_only_waterwalk     = F(5),
	move_only_shallow_waterwalk = F(6),
	move_only_pitwalk       = F(7),
	move_no_fake_z          = F(8),
	move_no_real_z          = F(9),
	move_use_fake_z         = F(10),
	move_ignore_solidity    = F(11),
	move_ignore_blockflags  = F(12),
	move_ignore_screenedge  = F(13),
	move_new_movement       = F(14),
	move_not_pushable       = F(15),
};

// Shield projectile blocking.
// TODO: use this type in itemdata and guydata (right now they are using an `int misc` field)
enum shield_flags : uint32_t
{
	sh_rock                 = F(0),
	sh_arrow                = F(1),
	sh_brang                = F(2),
	sh_fireball             = F(3),
	sh_sword                = F(4),
	sh_magic                = F(5),
	sh_flame                = F(6),
	sh_script               = F(7), // Blocks all of Script1 through Script10
	sh_fireball2            = F(8), // Boss fireball, not ewFireball2
	sh_lightbeam            = F(9), // Light puzzle beams
	sh_script1              = F(10),
	sh_script2              = F(11),
	sh_script3              = F(12),
	sh_script4              = F(13),
	sh_script5              = F(14),
	sh_script6              = F(15),
	sh_script7              = F(16),
	sh_script8              = F(17),
	sh_script9              = F(18),
	sh_script10             = F(19),
	sh_flame2               = F(20),

	sh_ALL_SCR              = 0x000FFC00,
};

// The rest is template soup, starting point is courtesy of https://stackoverflow.com/a/35672584/2788187

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
inline constexpr T operator&(T lhs, T rhs)
{
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) &
        static_cast<typename std::underlying_type<T>::type>(rhs));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
inline constexpr T operator|(T lhs, T rhs)
{
    return static_cast<T>(
        static_cast<typename std::underlying_type<T>::type>(lhs) |
        static_cast<typename std::underlying_type<T>::type>(rhs));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
inline constexpr T operator~(const T unary)
{
    return static_cast<T>(
        ~static_cast<typename std::underlying_type<T>::type>(unary));
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
inline T& operator|=(T& X, T Y)
{
    X = X | Y;
	return X;
}

template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
inline T& operator&=(T& X, T Y)
{
    X = X & Y;
	return X;
}

} // ends namespace

using namespace flags;

#endif
