#ifndef _MOVEINFO_H_
#define _MOVEINFO_H_

#ifdef IS_EDITOR

#include "base/ints.h"
#include "base/headers.h"
#include "base/containers.h"
#include "base/combo.h"
#include <sstream>
#include <functional>
#include <memory>
//Tiles

struct BaseTileRef
{
	string name;
	uint w, h; //should never be 0
	bool no_move;
	int xoff, yoff;
	vector<std::tuple<int,int,int>> extra_rects;
	int offset() const
	{
		return xoff+yoff*TILES_PER_ROW;
	}
	virtual int32_t getTile() const = 0;
	virtual void addTile(int32_t offs) = 0;
	virtual void setTile(int32_t val) = 0;
	virtual void forEach(std::function<void(int32_t)> proc) const = 0;
	BaseTileRef(string name = "", bool no_move = false, int xoff = 0, int yoff = 0,
		vector<std::tuple<int,int,int>> rects = {})
		: name(name), w(1), h(1), no_move(no_move), xoff(xoff), yoff(yoff), extra_rects(std::move(rects))
	{}
	template<class... Args>
	BaseTileRef(uint w, uint h, Args&&... args)
		: BaseTileRef(args...)
	{
		this->w = zc_max(1,w);
		this->h = zc_max(1,h);
	}
};
struct TileRefPtr : public BaseTileRef
{
	int32_t* tile;
	int32_t getTile() const override {return *tile;}
	void addTile(int32_t offs) override {*tile += offs;}
	void setTile(int32_t val) override {*tile = val;}
	void forEach(std::function<void(int32_t)> proc) const override;
	TileRefPtr()
		: BaseTileRef(), tile(nullptr)
	{}
	template<class... Args>
	TileRefPtr(int32_t* tile, Args&&... args)
		: BaseTileRef(args...), tile(tile)
	{}
	template<class... Args>
	TileRefPtr(int32_t* tile, uint w, uint h, Args&&... args)
		: BaseTileRef(w,h,args...), tile(tile)
	{}
};
struct TileRefPtr10k : public BaseTileRef
{
	int32_t* tile;
	int32_t getTile() const override {return *tile / 10000;}
	void addTile(int32_t offs) override {*tile += offs * 10000;}
	void setTile(int32_t val) override {*tile = val * 10000;}
	void forEach(std::function<void(int32_t)> proc) const override;
	TileRefPtr10k()
		: BaseTileRef(), tile(nullptr)
	{}
	template<class... Args>
	TileRefPtr10k(int32_t* tile, Args&&... args)
		: BaseTileRef(args...), tile(tile)
	{}
	template<class... Args>
	TileRefPtr10k(int32_t* tile, uint w, uint h, Args&&... args)
		: BaseTileRef(w,h,args...), tile(tile)
	{}
};
struct TileRefCombo : public BaseTileRef
{
	newcombo* combo;
	int32_t getTile() const override {return combo->o_tile;}
	void addTile(int32_t offs) override {combo->set_tile(combo->o_tile + offs);}
	void setTile(int32_t val) override {combo->set_tile(val);}
	void forEach(std::function<void(int32_t)> proc) const override;
	TileRefCombo()
		: BaseTileRef(), combo(nullptr)
	{}
	template<class... Args>
	TileRefCombo(newcombo* combo, Args&&... args)
		: BaseTileRef(args...), combo(combo)
	{}
	template<class... Args>
	TileRefCombo(newcombo* combo, uint w, uint h, Args&&... args)
		: BaseTileRef(w,h,args...), combo(combo)
	{}
};
/* TODO: SCCs?
struct TileRefSCC : public BaseTileRef
{
	int32_t getTile() const override {assert(false); return 0;}
	void addTile(int32_t offs) override {assert(false);}
	void setTile(int32_t val) override {assert(false);}
	void forEach(std::function<void(int32_t)> proc) const override {}
	TileRefSCC()
		: BaseTileRef()
	{}
	TileRefSCC(string name = "")
		: BaseTileRef(name)
	{}
	TileRefSCC(uint w, uint h, string name = "")
		: BaseTileRef(w,h,name)
	{}
};*/

struct TileMoveProcess
{
	bool rect;
	int _l, _t, _w, _h, _first, _last;
};
struct TileMoveList
{
	vector<std::unique_ptr<BaseTileRef>> move_refs;
	
	string msg; //message for the overwrite warning
	std::ostringstream warning_list; //list of overwrite warnings
	bool warning_flood; //if the overwrite warnings ran out of space
	
	//The processes to check tiles with
	optional<TileMoveProcess> source_process;
	TileMoveProcess dest_process;
	
	TileMoveList(TileMoveProcess dest_p, optional<TileMoveProcess> src_p = nullopt, string msg = "")
		: move_refs(), msg(msg), warning_list(), warning_flood(false),
		source_process(src_p), dest_process(dest_p)
	{}
	
	template<class... Args>
	void add_tile(int32_t* tile, Args&&... args)
	{
		if(!tile || !*tile)
			return;
		add_ref(std::move(std::make_unique<TileRefPtr>(tile, args...)));
	}
	template<class... Args>
	void add_tile_10k(int32_t* tile, Args&&... args)
	{
		if(!tile || !*tile)
			return;
		add_ref(std::move(std::make_unique<TileRefPtr10k>(tile, args...)));
	}
	template<class... Args>
	void add_combo(newcombo* combo, Args&&... args)
	{
		if(!combo || !combo->o_tile)
			return;
		add_ref(std::move(std::make_unique<TileRefCombo>(combo, args...)));
	}
	
	
	//Adds 'ref', either to 'move_refs' or 'warning_list' as appropriate based on the process rules.
	void add_ref(std::unique_ptr<BaseTileRef> ref)
	{
		if(source_process)
			if(process(false, ref, *source_process))
				return;
		process(true, ref, dest_process);
	}
	
	//Returns true if 'ref' was moved to the 'move_refs'
	bool process(bool is_dest, std::unique_ptr<BaseTileRef>& ref, TileMoveProcess const& proc);
	//Checks overwrite protection
	bool check_prot();
	//Adds 'diff' to every tile in 'move_refs'
	void add_diff(int diff);
};

//Combos
struct BaseComboRef
{
	string name;
	bool no_move;
	virtual int32_t getCombo() const = 0;
	virtual void addCombo(int32_t offs) = 0;
	virtual void setCombo(int32_t val) = 0;
	BaseComboRef(string name = "")
		: name(name), no_move(false)
	{}
};
struct ComboRefPtr : public BaseComboRef
{
	word* combo;
	int32_t getCombo() const override {return *combo;}
	void addCombo(int32_t offs) override {*combo += offs;}
	void setCombo(int32_t val) override {*combo = val;}
	ComboRefPtr()
		: BaseComboRef(), combo(nullptr)
	{}
	ComboRefPtr(word* combo, string name = "")
		: BaseComboRef(name), combo(combo)
	{}
};
struct ComboRefPtr32 : public BaseComboRef
{
	int32_t* combo;
	int32_t getCombo() const override {return *combo;}
	void addCombo(int32_t offs) override {*combo += offs;}
	void setCombo(int32_t val) override {*combo = val;}
	ComboRefPtr32()
		: BaseComboRef(), combo(nullptr)
	{}
	ComboRefPtr32(int32_t* combo, string name = "")
		: BaseComboRef(name), combo(combo)
	{}
};
struct ComboRefPtr10k : public BaseComboRef
{
	int32_t* combo;
	int32_t getCombo() const override {return *combo/10000;}
	void addCombo(int32_t offs) override {*combo += offs*10000;}
	void setCombo(int32_t val) override {*combo = val*10000;}
	ComboRefPtr10k()
		: BaseComboRef(), combo(nullptr)
	{}
	ComboRefPtr10k(int32_t* combo, string name = "")
		: BaseComboRef(name), combo(combo)
	{}
};
struct ComboMoveProcess
{
	int _first, _last;
};
struct ComboMoveList
{
	vector<std::unique_ptr<BaseComboRef>> move_refs;
	
	string msg; //message for the overwrite warning
	std::ostringstream warning_list; //list of overwrite warnings
	bool warning_flood; //if the overwrite warnings ran out of space
	
	map<int32_t,bool> processed_combos;
	SuperSet const& combo_links;
	
	//The processes to check combos with
	optional<ComboMoveProcess> source_process;
	ComboMoveProcess dest_process;
	
	ComboMoveList(SuperSet const& links, ComboMoveProcess dest_p, optional<ComboMoveProcess> src_p = nullopt, string msg = "")
		: move_refs(), msg(msg), warning_list(), warning_flood(false),
		processed_combos(), combo_links(links), source_process(src_p), dest_process(dest_p)
	{}
	
	template<class... Args>
	void add_combo(word* combo, Args&&... args)
	{
		if(!combo || !*combo)
			return;
		add_ref(std::move(std::make_unique<ComboRefPtr>(combo, args...)));
	}
	template<class... Args>
	void add_combo(int32_t* combo, Args&&... args)
	{
		if(!combo || !*combo)
			return;
		add_ref(std::move(std::make_unique<ComboRefPtr32>(combo, args...)));
	}
	template<class... Args>
	void add_combo_10k(int32_t* combo, Args&&... args)
	{
		if(!combo || !*combo)
			return;
		add_ref(std::move(std::make_unique<ComboRefPtr10k>(combo, args...)));
	}
	
	//Adds 'ref', either to 'move_refs' or 'warning_list' as appropriate based on the process rules.
	void add_ref(std::unique_ptr<BaseComboRef> ref)
	{
		if(source_process)
			if(process(false, ref, *source_process))
				return;
		process(true, ref, dest_process);
	}
	
	//Returns true if 'ref' was moved to the 'move_refs'
	bool process(bool is_dest, std::unique_ptr<BaseComboRef>& ref, ComboMoveProcess const& proc);
	//Checks overwrite protection
	bool check_prot();
	//Adds 'diff' to every combo in 'move_refs'
	void add_diff(int diff);
};

//Undo handlers
struct ComboMoveUndo
{
	SuperSet combo_links;
	vector<std::unique_ptr<ComboMoveList>> vec;
	int diff;
	bool state;
	void undo()
	{
		if(!state) return;
		state = false;
		for(auto& list : vec)
			list->add_diff(-diff);
	}
	void redo()
	{
		if(state) return;
		state = true;
		for(auto& list : vec)
			list->add_diff(diff);
	}
};
struct TileMoveUndo
{
	vector<std::unique_ptr<TileMoveList>> vec;
	int diff;
	bool state;
	void undo()
	{
		if(!state) return;
		state = false;
		for(auto& list : vec)
			list->add_diff(-diff);
	}
	void redo()
	{
		if(state) return;
		state = true;
		for(auto& list : vec)
			list->add_diff(diff);
	}
};

#endif

#endif