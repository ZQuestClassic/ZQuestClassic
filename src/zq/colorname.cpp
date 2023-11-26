#include "base/zdefs.h"
#include "colorname.h"
#include "base/util.h"
#include <sstream>
#include "zq/zquest.h"
#include "tiles.h"

using std::map, std::string, std::vector, std::pair;

static int makehex(int r, int g, int b)
{
	return (r<<16)|(g<<8)|b;
}
static int compresshex(int hexval)
{
	int r = (hexval&0xFF0000) >> 16;
	int g = (hexval&0x00FF00) >> 8;
	int b = (hexval&0x0000FF);
	return ((r/4)<<16)+((g/4)<<8)+(b/4);
}
static std::map<int,std::string> colornames;
static std::map<int,std::string const*> colorcache;
void addname(int hex, char const* name, bool hex63 = false)
{
	int ind = hex63 ? hex : compresshex(hex);
	if(colornames.find(ind) != colornames.end())
		zprint2("Duplicate values '%s' and '%s' at %d!\n", colornames[ind].c_str(), name, ind);
	colornames[ind] = std::string(name);
}

std::string const* closest_color_name(int r, int g, int b, zfix* wei = nullptr)
{
	double best = 99999;
	std::string const* match = nullptr;
	for(auto& pair : colorcache)
	{
		int nr = (pair.first&0xFF0000) >> 16;
		int ng = (pair.first&0x00FF00) >> 8;
		int nb = (pair.first&0x0000FF);
		
		double weight = sqrt(pow(r-nr,2)+pow(g-ng,2)+pow(b-nb,2));
		if(weight < best)
		{
			best = weight;
			match = pair.second;
		}
	}
	if(wei)
		*wei = best;
	return match;
}

void init_color_names()
{
	static bool initialized = false;
	if(initialized) return;
	//Color Names
	{ //https://www.rapidtables.com/web/color/RGB_Color.html
		addname(0x000000, "Black");
		addname(0xFFFFFF, "White");
		addname(0xFF0000, "Red");
		addname(0x00FF00, "Lime");
		addname(0x0000FF, "Blue");
		addname(0xFFFF00, "Yellow");
		addname(0x00FFFF, "Aqua/Cyan");
		addname(0xFF00FF, "Magenta");
		addname(0xC0C0C0, "Silver");
		addname(0x808080, "Gray");
		addname(0x800000, "Maroon");
		addname(0x808000, "Olive");
		addname(0x008000, "Green");
		addname(0x800080, "Purple");
		addname(0x008080, "Teal");
		addname(0x000080, "Navy");
		
		addname(0x8B0000, "Dark Red");
		addname(0xA52A2A, "Brown");
		addname(0xB22222, "Firebrick");
		addname(0xDC143C, "Crimson");
		addname(0xFF6347, "Tomato");
		addname(0xFF7F50, "Coral");
		addname(0xCD5C5C, "Indian Red");
		addname(0xF08080, "Light Coral");
		addname(0xE9967A, "Dark Salmon");
		addname(0xFA8072, "Salmon");
		addname(0xFFA07A, "Light Salmon");
		addname(0xFF4500, "Orange Red");
		addname(0xFF8C00, "Dark Orange");
		addname(0xFFA500, "Orange");
		addname(0xFFD700, "Gold");
		addname(0xB8860B, "Dark Golden Rod");
		addname(0xDAA520, "Golden Rod");
		addname(0xEEE8AA, "Pale Golden Rod");
		addname(0xBDB76B, "Dark Khaki");
		addname(0xF0E68C, "Khaki");;
		addname(0x9ACD32, "Yellow Green");
		addname(0x556B2F, "Dark Olive Green");
		addname(0x6B8E23, "Olive Drab");
		addname(0x7BFB00, "Lawn Green");
		addname(0x7FFF00, "Chartreuse");
		addname(0xADFF2F, "Green Yellow");
		addname(0x006400, "Dark Green");
		addname(0x228B22, "Forest Green");
		addname(0x32CD32, "Lime Green");
		addname(0x90EE90, "Light Green");
		addname(0x98FB98, "Pale Green");
		addname(0x8FBC8F, "Dark Sea Green");
		addname(0x00FA9A, "Medium Spring Green");
		addname(0x00FF7F, "Spring Green");
		addname(0x2E8B57, "Sea Green");
		addname(0x66CDAA, "Medium Aquamarine");
		addname(0x3CB371, "Medium Sea Green");
		addname(0x20B2AA, "Light Sea Green");
		addname(0x2F4F4F, "Dark Slate Gray");
		addname(0x008B8B, "Dark Cyan");
		addname(0xE0FFFF, "Light Cyan");
		addname(0x00CED1, "Dark Turquoise");
		addname(0x40E0D0, "Turquoise");
		addname(0x48D1CC, "Medium Turquoise");
		addname(0xAFEEEE, "Pale Turquoise");
		addname(0x7FFFD4, "Aquamarine");
		addname(0xB0E0E6, "Powder Blue");
		addname(0x5F9EA0, "Cadet Blue");
		addname(0x4682B4, "Steel Blue");
		addname(0x6495ED, "Corn Flower Blue");
		addname(0x00BFFF, "Deep Sky Blue");
		addname(0x1E90FF, "Dodger Blue");
		addname(0xADD8E6, "Light Blue");
		addname(0x87CEEB, "Sky Blue");
		addname(0x87CEFA, "Light Sky Blue");
		addname(0x191970, "Midnight Blue");
		addname(0x00008B, "Dark Blue");
		addname(0x0000CD, "Medium Blue");
		addname(0x4169E1, "Royal Blue");
		addname(0x8A2BE2, "Blue Violet");
		addname(0x4B0082, "Indigo");
		addname(0x483D8B, "Dark Slate Blue");
		addname(0x6A5ACD, "Slate Blue");
		addname(0x7B68EE, "Medium Slate Blue");
		addname(0x9370DB, "Medium Purple");
		addname(0x8B008B, "Dark Magenta");
		addname(0x9400D3, "Dark Violet");
		addname(0x9932CC, "Dark Orchid");
		addname(0xBA55D3, "Medium Orchid");
		addname(0xD8BFD8, "Thistle");
		addname(0xDDA0DD, "Plum");
		addname(0xEE82EE, "Violet");
		addname(0xDA70D6, "Orchid");
		addname(0xC71585, "Medium Violet Red");
		addname(0xDB7093, "Pale Violet Red");
		addname(0xFF1493, "Deep Pink");
		addname(0xFF69B4, "Hot Pink");
		addname(0xFFB6C1, "Light Pink");
		addname(0xFFC0CB, "Pink");
		addname(0xFAEBD7, "Antique White");
		addname(0xF5F5DC, "Beige");
		addname(0xFFE4C4, "Bisque");
		addname(0xFFEBCD, "Blanched Almond");
		addname(0xF5DEB3, "Wheat");
		addname(0xFFF8DC, "Corn Silk");
		addname(0xFFFACD, "Lemon Chiffon");
		addname(0xFAFAD2, "Light Golden Rod Yellow");
		addname(0xFFFFE0, "Light Yellow");
		addname(0x8B4513, "Saddle Brown");
		addname(0xA0522D, "Sienna");
		addname(0xCD853F, "Peru");
		addname(0xF4A460, "Sandy Brown");
		addname(0xDEB887, "Burly Wood");
		addname(0xD2B48C, "Tan");
		addname(0xBC8F8F, "Rosy Brown");
		addname(0xFFE4B5, "Moccasin");
		addname(0xFFDEAD, "Navajo White");
		addname(0xFFDAB9, "Peach Puff");
		addname(0xFFE4E1, "Misty Rose");
		addname(0xFFF0F5, "Lavender Blush");
		addname(0xFAF0E6, "Linen");
		addname(0xFDF5E6, "Old Lace");
		addname(0xFFEFD5, "Papaya Whip");
		addname(0xFFF5EE, "Sea Shell");
		addname(0xF5FFFA, "Mint Cream");
		addname(0x708090, "Slate Gray");
		addname(0x778899, "Light Slate Gray");
		addname(0xB0C4DE, "Light Steel Blue");
		addname(0xE6E6FA, "Lavender");
		addname(0xFFFAF0, "Floral White");
		addname(0xF0F8FF, "Alice Blue");
		addname(0xF8F8FF, "Ghost White");
		addname(0xF0FFF0, "Honeydew");
		addname(0xFFFFF0, "Ivory");
		addname(0xF0FFFF, "Azure");
		addname(0xFFFAFA, "Snow");
		addname(0x696969, "Dim Gray");
		addname(0xD3D3D3, "Light Gray");
		addname(0xDCDCDC, "Gainsboro");
		addname(0xF5F5F5, "White Smoke");
	}
	
	{ //https://cloford.com/resources/colours/500col.htm
		addname(0xB0171F, "Indian Red");
		addname(0xFFAEB9, "Light Pink");
		addname(0xEEA2AD, "Light Pink");
		addname(0xCD8C95, "Light Pink");
		addname(0x8B5F65, "Light Pink");
		addname(0xFFB5C5, "Pink");
		addname(0xEEA9B8, "Pink");
		addname(0xCD919E, "Pink");
		addname(0x8B636C, "Pink");
		addname(0xFF82AB, "Pale Violet Red");
		addname(0xEE799F, "Pale Violet Red");
		addname(0xCD6889, "Pale Violet Red");
		addname(0x8B475D, "Pale Violet Red");
		addname(0xEEE0E5, "Lavender Blush");
		addname(0xCDC1C5, "Lavender Blush");
		addname(0x8B8386, "Lavender Blush");
		addname(0xFF3E96, "Violet Red");
		addname(0xEE3A8C, "Violet Red");
		addname(0xCD3278, "Violet Red");
		addname(0x8B2252, "Violet Red");
		addname(0xFF6EB4, "Hot Pink");
		addname(0xEE6AA7, "Hot Pink");
		addname(0xCD6090, "Hot Pink");
		addname(0x8B3A62, "Hot Pink");
		addname(0x872657, "Raspberry");
		addname(0xEE1289, "Deep Pink");
		addname(0xCD1076, "Deep Pink");
		addname(0x8B0A50, "Deep Pink");
		addname(0xFF34B3, "Maroon");
		addname(0xEE30A7, "Maroon");
		addname(0xCD2990, "Maroon");
		addname(0x8B1C62, "Maroon");
		addname(0xD02090, "Violet Red");
		addname(0xFF83FA, "Orchid");
		addname(0xEE7AE9, "Orchid");
		addname(0xCD69C9, "Orchid");
		addname(0x8B4789, "Orchid");
		addname(0xFFE1FF, "Thistle");
		addname(0xEED2EE, "Thistle");
		addname(0xCDB5CD, "Thistle");
		addname(0x8B7B8B, "Thistle");
		addname(0xFFBBFF, "Plum");
		addname(0xEEAEEE, "Plum");
		addname(0xCD96CD, "Plum");
		addname(0x8B668B, "Plum");
		addname(0xEE00EE, "Magenta");
		addname(0xCD00CD, "Magenta");
		addname(0xE066FF, "Medium Orchid");
		addname(0xD15FEE, "Medium Orchid");
		addname(0xB452CD, "Medium Orchid");
		addname(0x7A378B, "Medium Orchid");
		addname(0xBF3EFF, "Dark Orchid");
		addname(0xB23AEE, "Dark Orchid");
		addname(0x68228B, "Dark Orchid");
		addname(0x9B30FF, "Purple");
		addname(0x912CEE, "Purple");
		addname(0x7D26CD, "Purple");
		addname(0x551A8B, "Purple");
		addname(0xAB82FF, "Medium Purple");
		addname(0x9F79EE, "Medium Purple");
		addname(0x8968CD, "Medium Purple");
		addname(0x5D478B, "Medium Purple");
		addname(0x8470FF, "Light Slate Blue");
		addname(0x836FFF, "Slate Blue");
		addname(0x7A67EE, "Slate Blue");
		addname(0x473C8B, "Slate Blue");
		addname(0x0000EE, "Blue");
		addname(0x3D59AB, "Cobalt");
		addname(0x4876FF, "Royal Blue");
		addname(0x436EEE, "Royal Blue");
		addname(0x3A5FCD, "Royal Blue");
		addname(0x27408B, "Royal Blue");
		addname(0xCAE1FF, "Light Steel Blue");
		addname(0xBCD2EE, "Light Steel Blue");
		addname(0xA2B5CD, "Light Steel Blue");
		addname(0x6E7B8B, "Light Steel Blue");
		addname(0xC6E2FF, "Slate Gray");
		addname(0xB9D3EE, "Slate Gray");
		addname(0x9FB6CD, "Slate Gray");
		addname(0x1C86EE, "Dodger Blue");
		addname(0x1874CD, "Dodger Blue");
		addname(0x104E8B, "Dodger Blue");
		addname(0x63B8FF, "Steel Blue");
		addname(0x5CACEE, "Steel Blue");
		addname(0x4F94CD, "Steel Blue");
		addname(0x36648B, "Steel Blue");
		addname(0xB0E2FF, "Light Sky Blue");
		addname(0xA4D3EE, "Light Sky Blue");
		addname(0x8DB6CD, "Light Sky Blue");
		addname(0x607B8B, "Light Sky Blue");
		addname(0x87CEFF, "Sky Blue");
		addname(0x7EC0EE, "Sky Blue");
		addname(0x6CA6CD, "Sky Blue");
		addname(0x4A708B, "Sky Blue");
		addname(0x00B2EE, "Deep Sky Blue");
		addname(0x009ACD, "Deep Sky Blue");
		addname(0x00688B, "Deep Sky Blue");
		addname(0x33A1C9, "Peacock");
		addname(0xBFEFFF, "Light Blue");
		addname(0xB2DFEE, "Light Blue");
		addname(0x9AC0CD, "Light Blue");
		addname(0x68838B, "Light Blue");
		addname(0x98F5FF, "Cadet Blue");
		addname(0x8EE5EE, "Cadet Blue");
		addname(0x7AC5CD, "Cadet Blue");
		addname(0x53868B, "Cadet Blue");
		addname(0x00F5FF, "Turquoise");
		addname(0x00E5EE, "Turquoise");
		addname(0x00C5CD, "Turquoise");
		addname(0x00868B, "Turquoise");
		addname(0xE0EEEE, "Azure");
		addname(0xC1CDCD, "Azure");
		addname(0x838B8B, "Azure");
		addname(0xD1EEEE, "Light Cyan");
		addname(0xB4CDCD, "Light Cyan");
		addname(0x7A8B8B, "Light Cyan");
		addname(0xBBFFFF, "Pale Turquoise");
		addname(0x96CDCD, "Pale Turquoise");
		addname(0x668B8B, "Pale Turquoise");
		addname(0x97FFFF, "Dark Slate Gray");
		addname(0x8DEEEE, "Dark Slate Gray");
		addname(0x79CDCD, "Dark Slate Gray");
		addname(0x528B8B, "Dark Slate Gray");
		addname(0x00EEEE, "Cyan");
		addname(0x00CDCD, "Cyan");
		addname(0x03A89E, "Manganese Blue");
		addname(0x808A87, "Cold Grey");
		addname(0x00C78C, "Turquoise Blue");
		addname(0x76EEC6, "Aquamarine");
		addname(0x458B74, "Aquamarine");
		addname(0x00EE76, "Spring Green");
		addname(0x00CD66, "Spring Green");
		addname(0x008B45, "Spring Green");
		addname(0x54FF9F, "Sea Green");
		addname(0x4EEE94, "Sea Green");
		addname(0x43CD80, "Sea Green");
		addname(0x00C957, "Emerald Green");
		addname(0xBDFCC9, "Mint");
		addname(0x3D9140, "Cobalt Green");
		addname(0xE0EEE0, "Honeydew");
		addname(0xC1CDC1, "Honeydew");
		addname(0x838B83, "Honeydew");
		addname(0xC1FFC1, "Dark Sea Green");
		addname(0xB4EEB4, "Dark Sea Green");
		addname(0x9BCD9B, "Dark Sea Green");
		addname(0x698B69, "Dark Sea Green");
		addname(0x9AFF9A, "Pale Green");
		addname(0x7CCD7C, "Pale Green");
		addname(0x548B54, "Pale Green");
		addname(0x00EE00, "Green");
		addname(0x00CD00, "Green");
		addname(0x008B00, "Green");
		addname(0x308014, "Sap Green");
		addname(0x76EE00, "Chartreuse");
		addname(0x66CD00, "Chartreuse");
		addname(0x458B00, "Chartreuse");
		addname(0xCAFF70, "Dark Olive Green");
		addname(0xBCEE68, "Dark Olive Green");
		addname(0xA2CD5A, "Dark Olive Green");
		addname(0x6E8B3D, "Dark Olive Green");
		addname(0xC0FF3E, "Olive Drab");
		addname(0xB3EE3A, "Olive Drab");
		addname(0x698B22, "Olive Drab");
		addname(0xEEEEE0, "Ivory");
		addname(0xCDCDC1, "Ivory");
		addname(0x8B8B83, "Ivory");
		addname(0xEEEED1, "Light Yellow");
		addname(0xCDCDB4, "Light Yellow");
		addname(0x8B8B7A, "Light Yellow");
		addname(0xEEEE00, "Yellow");
		addname(0xCDCD00, "Yellow");
		addname(0x8B8B00, "Yellow");
		addname(0x808069, "Warm Gray");
		addname(0xFFF68F, "Khaki");
		addname(0xEEE685, "Khaki");
		addname(0xCDC673, "Khaki");
		addname(0x8B864E, "Khaki");
		addname(0xEEE9BF, "Lemon Chiffon");
		addname(0xCDC9A5, "Lemon Chiffon");
		addname(0x8B8970, "Lemon Chiffon");
		addname(0xFFEC8B, "Light Goldenrod");
		addname(0xEEDC82, "Light Goldenrod");
		addname(0xCDBE70, "Light Goldenrod");
		addname(0x8B814C, "Light Goldenrod");
		addname(0xE3CF57, "Banana");
		addname(0xEEC900, "Gold");
		addname(0xCDAD00, "Gold");
		addname(0x8B7500, "Gold");
		addname(0xEEE8CD, "Corn Silk");
		addname(0xCDC8B1, "Corn Silk");
		addname(0xFFC125, "Goldenrod");
		addname(0xEEB422, "Goldenrod");
		addname(0xCD9B1D, "Goldenrod");
		addname(0x8B6914, "Goldenrod");
		addname(0xFFB90F, "Dark Goldenrod");
		addname(0xEEAD0E, "Dark Goldenrod");
		addname(0xCD950C, "Dark Goldenrod");
		addname(0x8B6508, "Dark Goldenrod");
		addname(0xEE9A00, "Orange");
		addname(0xCD8500, "Orange");
		addname(0x8B5A00, "Orange");
		addname(0xFFE7BA, "Wheat");
		addname(0xEED8AE, "Wheat");
		addname(0xCDBA96, "Wheat");
		addname(0x8B7E66, "Wheat");
		addname(0xEECFA1, "Navajo White");
		addname(0xCDB38B, "Navajo White");
		addname(0x8B795E, "Navajo White");
		addname(0xFCE6C9, "Eggshell");
		addname(0x9C661F, "Brick");
		addname(0xFF9912, "Cadmium Yellow");
		addname(0xFFEFDB, "Antique White");
		addname(0xEEDFCC, "Antique White");
		addname(0xCDC0B0, "Antique White");
		addname(0x8B8378, "Antique White");
		addname(0xFFD39B, "Burly Wood");
		addname(0xEEC591, "Burly Wood");
		addname(0xCDAA7D, "Burly Wood");
		addname(0x8B7355, "Burly Wood");
		addname(0xEED5B7, "Bisque");
		addname(0xCDB79E, "Bisque");
		addname(0x8B7D6B, "Bisque");
		addname(0xE3A869, "Melon");
		addname(0xED9121, "Carrot");
		addname(0xFF7F00, "Dark Orange");
		addname(0xEE7600, "Dark Orange");
		addname(0xCD6600, "Dark Orange");
		addname(0x8B4500, "Dark Orange");
		addname(0xFF8000, "Orange");
		addname(0xFFA54F, "Tan");
		addname(0xEE9A49, "Tan");
		addname(0x8B5A2B, "Tan");
		addname(0xEECBAD, "Peach Puff");
		addname(0xCDAF95, "Peach Puff");
		addname(0x8B7765, "Peach Puff");
		addname(0xEEE5DE, "Seashell");
		addname(0xCDC5BF, "Seashell");
		addname(0x8B8682, "Seashell");
		addname(0xC76114, "Raw Sienna");
		addname(0x292421, "Ivory Black");
		addname(0xFF7D40, "Flesh");
		addname(0xFF6103, "Cadmium Orange");
		addname(0x8A360F, "Burnt Sienna");
		addname(0xFF8247, "Sienna");
		addname(0xEE7942, "Sienna");
		addname(0xCD6839, "Sienna");
		addname(0x8B4726, "Sienna");
		addname(0xEE9572, "Light Salmon");
		addname(0xCD8162, "Light Salmon");
		addname(0x8B5742, "Light Salmon");
		addname(0xEE4000, "Orange Red");
		addname(0xCD3700, "Orange Red");
		addname(0x8B2500, "Orange Red");
		addname(0x5E2612, "Sepia");
		addname(0xFF8C69, "Salmon");
		addname(0xEE8262, "Salmon");
		addname(0xCD7054, "Salmon");
		addname(0xFF7256, "Coral");
		addname(0xEE6A50, "Coral");
		addname(0xCD5B45, "Coral");
		addname(0x8B3E2F, "Coral");
		addname(0x8A3324, "Burnt Umber");
		addname(0xEE5C42, "Tomato");
		addname(0xCD4F39, "Tomato");
		addname(0x8B3626, "Tomato");
		addname(0xEED5D2, "Misty Rose");
		addname(0xCDB7B5, "Misty Rose");
		addname(0x8B7D7B, "Misty Rose");
		addname(0xEEE9E9, "Snow");
		addname(0xCDC9C9, "Snow");
		addname(0x8B8989, "Snow");
		addname(0xFFC1C1, "Rosy Brown");
		addname(0xEEB4B4, "Rosy Brown");
		addname(0xCD9B9B, "Rosy Brown");
		addname(0x8B6969, "Rosy Brown");
		addname(0xFF6A6A, "Indian Red");
		addname(0xEE6363, "Indian Red");
		addname(0x8B3A3A, "Indian Red");
		addname(0xCD5555, "Indian Red");
		addname(0xFF4040, "Brown");
		addname(0xEE3B3B, "Brown");
		addname(0xCD3333, "Brown");
		addname(0x8B2323, "Brown");
		addname(0xFF3030, "Fire Brick");
		addname(0xEE2C2C, "Fire Brick");
		addname(0xCD2626, "Fire Brick");
		addname(0x8B1A1A, "Fire Brick");
		addname(0xEE0000, "Red");
		addname(0xCD0000, "Red");
		addname(0x8E388E, "Beet");
		addname(0x7171C6, "Slate Blue");
		addname(0x7D9EC0, "Light Blue");
		addname(0x388E8E, "Teal");
		addname(0x71C671, "Chartreuse");
		addname(0x8E8E38, "Olive Drab");
		addname(0xC5C1AA, "Bright Gray");
		addname(0xC67171, "Salmon");
		addname(0x555555, "Dark Gray");
		addname(0x1E1E1E, "Dark Gray");
		addname(0x282828, "Dark Gray");
		addname(0x515151, "Dark Gray");
		addname(0x5B5B5B, "Dark Gray");
		addname(0x848484, "Gray");
		addname(0x8E8E8E, "Gray");
		addname(0xB7B7B7, "Light Gray");
		addname(0xEAEAEA, "Light Gray");
		addname(0xFAFAFA, "Light Gray");
		addname(0xF2F2F2, "Light Gray");
		addname(0xEDEDED, "Light Gray");
		addname(0xE5E5E5, "Light Gray");
		addname(0xE3E3E3, "Light Gray");
		addname(0xDBDBDB, "Light Gray");
		addname(0xD6D6D6, "Light Gray");
		addname(0xCFCFCF, "Light Gray");
		addname(0xC9C9C9, "Light Gray");
		addname(0xC7C7C7, "Light Gray");
		addname(0xBFBFBF, "Light Gray");
		addname(0xBABABA, "Light Gray");
		addname(0xB3B3B3, "Light Gray");
		addname(0xADADAD, "Light Gray");
		addname(0xA6A6A6, "Light Gray");
		addname(0xA3A3A3, "Light Gray");
		addname(0x9E9E9E, "Gray");
		addname(0x999999, "Gray");
		addname(0x969696, "Gray");
		addname(0x919191, "Gray");
		addname(0x7F7F7F, "Gray");
		addname(0x7A7A7A, "Gray");
		addname(0x757575, "Gray");
		addname(0x737373, "Gray");
		addname(0x6E6E6E, "Gray");
		addname(0x666666, "Gray");
		addname(0x636363, "Gray");
		addname(0x5E5E5E, "Gray");
		addname(0x4F4F4F, "Dark Gray");
		addname(0x4A4A4A, "Dark Gray");
		addname(0x474747, "Dark Gray");
		addname(0x424242, "Dark Gray");
		addname(0x3D3D3D, "Dark Gray");
		addname(0x3B3B3B, "Dark Gray");
		addname(0x363636, "Dark Gray");
		addname(0x333333, "Dark Gray");
		addname(0x2E2E2E, "Dark Gray");
		addname(0x262626, "Dark Gray");
		addname(0x212121, "Dark Gray");
		addname(0x1A1A1A, "Dark Gray");
		addname(0x171717, "Dark Gray");
		addname(0x121212, "Dark Gray");
		addname(0x0F0F0F, "Dark Gray");
		addname(0x0A0A0A, "Dark Gray");
		addname(0x050505, "Dark Gray");
	}
	
	{ //Manual
		addname(0x9490FF, "Light Blue");
		addname(0xFCD8A8, "Light Tan");
		addname(0x440C80, "Dark Purple");
		addname(0x8C30DC, "Purple");
		addname(0x9C4C00, "Brown");
		addname(0xAC3C00, "Red Brown");
		addname(0xA9A9A9, "Medium Gray");
		addname(0x44104C, "Dark Purple");
		addname(0xB0A8A0, "Light Gray");
		addname(0x8B4C39, "Paarl");
		addname(0xFF7F24, "Pumpkin");
		addname(0xEE7621, "Pumpkin");
		addname(0xCD661D, "Orange");
		addname(0xD2691E, "Orange");
		addname(0x6B360F, "Chocolate");
	}
	//Cache colors
	for(auto& pair : colornames)
	{
		colorcache[pair.first] = &pair.second;
	}
	initialized = true;
}

std::string const& get_color_name(int r, int g, int b)
{
	init_color_names();
	int ind = makehex(r,g,b);
	auto it = colornames.find(ind);
	if(it != colornames.end())
		return it->second;
	char buf[256];
	zfix weight;
	std::string const* closename = closest_color_name(r,g,b,&weight);
	if(closename)
	{
		char qbuf[10] = {0};
		int qind = 1;
		for(zfix q = weight; q > 10; q -= 5)
		{
			qbuf[qind++] = '?';
			if(qind == 9) break;
		}
		if(qind > 1)
			qbuf[0] = ' ';
		qbuf[qind] = 0;
		sprintf(buf, "%s%s", closename->c_str(), qbuf);
		addname(ind, buf, true);
		return colornames[ind];
	}
	else
	{
		sprintf(buf, "Unnamed 'R%02d G%02d B%02d'", r,g,b);
		addname(ind, buf, true);
		return colornames[ind];
	}
}
std::string const& get_color_name(int c, bool is8b)
{
	static std::string tmp;
	tmp = get_color_name(RAMpal[c].r, RAMpal[c].g, RAMpal[c].b);
	if(!(is8b ? c : c%16))
		tmp += " (Trans)";
	return tmp;
}
std::string const& get_color_name(int c)
{
	return get_color_name(RAMpal[c].r, RAMpal[c].g, RAMpal[c].b);
}

void get_tile_colornames(int tile, int cs, map<string,int>& res)
{
	bool is8b = newtilebuf[tile].format>tf4Bit;
	unpack_tile(newtilebuf, tile, 0, true);
	byte *si = unpackbuf;
	
	int offs = is8b ? 0 : CSET(cs);
	for(int32_t q=0; q<256; ++q)
	{
		std::string const& name = get_color_name(offs+si[q], is8b);
		if(res.find(name) == res.end())
			res[name] = 1;
		else
			res[name] = res[name]+1;
	}
}
void get_combo_colornames(int combo, int cs, map<string,int>& res)
{
	newcombo const& cmb = combobuf[combo];
	cmb.each_tile([&](int tile){
		get_tile_colornames(tile,cs,res);
		return false;
	});
}

bool sortByValAsc(const pair<string,int>& a,const pair<string,int>& b) 
{ 
	return (a.second < b.second); 
}
bool sortByValDesc(const pair<string,int>& a,const pair<string,int>& b) 
{ 
	return (a.second > b.second); 
}
vector<pair<string,int>> sortRes(map<string,int>& res)
{
	vector<pair<string,int>> ret;
	for(auto& p : res)
		ret.push_back(p);
	std::sort(ret.begin(),ret.end(),sortByValDesc);
	return ret;
}

string get_tile_colornames(int tile, int cs)
{
	map<string,int> res;
	get_tile_colornames(tile,cs,res);
	
	auto vec = sortRes(res);
	std::ostringstream oss;
	
	for(auto& p : vec)
		oss << p.first << ": " << p.second << "\n";
	return oss.str();
}
string get_combo_colornames(int combo, int cs)
{
	map<string,int> res;
	get_combo_colornames(combo,cs,res);
	
	auto vec = sortRes(res);
	std::ostringstream oss;
	
	for(auto& p : vec)
		oss << p.first << ": " << p.second << "\n";
	return oss.str();
}
