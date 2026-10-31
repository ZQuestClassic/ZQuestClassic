#ifndef CORE_TITLE_THEME_H_
#define CORE_TITLE_THEME_H_

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string_view>

// The choices for the player's "Title Screen" setting ([zeldadx] title_screen), which the
// launcher sets and the title screen reads.
enum class TitleTheme
{
	// A holiday's scene on that holiday, and the standard backdrop the rest of the year.
	Seasonal,
	Standard,
	Halloween,
	Count,
};

// What each theme is saved as in the config, in TitleTheme's order.
inline constexpr const char* title_theme_names[] = {"seasonal", "standard", "halloween"};
static_assert(std::size(title_theme_names) == (size_t)TitleTheme::Count);

// The theme saved as `name` (ignoring case), or Seasonal if it isn't one.
inline TitleTheme title_theme_from_name(std::string_view name)
{
	for (size_t i = 0; i < std::size(title_theme_names); i++)
	{
		std::string_view theme_name = title_theme_names[i];
		if (name.size() == theme_name.size() && std::equal(name.begin(), name.end(), theme_name.begin(),
			[](char a, char b) { return std::tolower((unsigned char)a) == b; }))
			return (TitleTheme)i;
	}
	return TitleTheme::Seasonal;
}

#endif
