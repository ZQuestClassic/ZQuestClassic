#ifndef _AUTOPATTERN_DORMTN_H_
#define _AUTOPATTERN_DORMTN_H_

#include <functional>
#include "base/autocombo.h"
#include "zq/autocombo/autopattern_base.h"

namespace AutoPattern
{
	enum
	{
		TOP = 0x100,
		RIM = 0x200,
		BASE = 0x400,
		HALF = 0x800,
		OUTERCORNER = 0x1000,
		SOUTHUPPER = 0x2000,
		SIDE = 0x4000
	};

	class dor_face;

	class autopattern_dormtn : public autopattern_container
	{
	public:
		explicit autopattern_dormtn(int32_t ntype, int32_t nlayer, int32_t nbasescreen, int32_t nbasepos, combo_auto* nsource, bool nnocrossedge, byte nheight = 1) :
			autopattern_container(ntype, nlayer, nbasescreen, nbasepos, nsource, nnocrossedge), height(nheight), tops() 
		{
			screenboundary_x = 16;
			screenboundary_y = 11;
		}
		virtual bool execute(int32_t exscreen, int32_t expos) override;
		virtual bool erase(int32_t exscreen, int32_t expos) override;
		void calculate_connections(apcombo* p);
		virtual uint32_t slot_to_flags(int32_t slot) override;
		virtual int32_t flags_to_slot(uint32_t flags) override;
		bool is_top(int32_t slot);
		void load_all_tops(apcombo* p);
		void update_top_combos();
		bool is_vertex(apcombo* p);
		void find_sides();
		void update_sides();
		void write_side(dor_face* p);
		void mask_out_context(apcombo* center);
		apcombo* add_side_combo(apcombo* p, int32_t x, int32_t y);
	private:
		std::map<int32_t, apcombo*> tops;
		std::map<int32_t, apcombo*> sides;
		std::map<int32_t, dor_face*> side_faces;
		byte height;
	};

	class dor_face
	{
	public:
		bool valid;
		apcombo* vert[2] = { 0 };
		byte vert_dir[2] = { 0 };
		bool outer[2] = { false, false };
		byte facing_dir;
		byte scan_dir;
		byte height;
		byte side_length;
		bool special_corner;
		bool steep;

		dor_face(apcombo* firstvertex, autopattern_dormtn* npattern, byte nheight);
		dor_face(apcombo* firstvertex, autopattern_dormtn* npattern, byte ndir, byte nheight);
		void get_vertex_dir(apcombo* v, byte& facing, byte& vertexfacing, byte& scan, bool first);
	private:
		autopattern_dormtn* pattern;
	};

}

#endif