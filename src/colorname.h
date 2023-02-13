#ifndef COLORNAME_H
#define COLORNAME_H
#include <vector>
#include <map>

std::string const& get_color_name(int r, int g, int b);
std::string const& get_color_name(int c);
std::string get_tile_colornames(int tile, int cs);
std::string get_combo_colornames(int combo, int cs);
#endif