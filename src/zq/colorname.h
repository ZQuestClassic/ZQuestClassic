#ifndef COLORNAME_H
#define COLORNAME_H

#include <string>

std::string const& get_color_name(int r, int g, int b);
std::string const& get_color_name(int c, bool is8b); //Includes '(Trans)' for transparents
std::string const& get_color_name(int c);
std::string get_tile_colornames(int tile, int cs);
std::string get_combo_colornames(int combo, int cs);

#endif
