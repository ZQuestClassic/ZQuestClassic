#ifndef _CPOOL_H_
#define _CPOOL_H_

#include "base/headers.h"
#include "base/containers.h"
#include "base/ints.h"
#include "base/general.h"

struct combo_alias
{
	combo_alias() = default;
	
	byte width;      // Max 15
	byte height;     // Max 10
	byte layermask;  // Specifies layers to be drawn
	word combo;      // Optional thumbnail combo for the alias list
	word cset;
	bounded_vec<word,word> combos = {16*11*7, 0};
	bounded_vec<word,byte> csets = {16*11*7, 0};
	
	bool valid() const;
	void clear();
	combo_alias& operator=(combo_alias const& other) = default;
};

struct cpool_entry
{
	int32_t cid = -1;
	int8_t cset = -1;
	word quant;
	void clear()
	{
		*this = cpool_entry();
	}
	bool valid() const
	{
		return quant > 0 && unsigned(cid) < MAXCOMBOS;
	}
	cpool_entry() = default;
	cpool_entry(int32_t data,int8_t cs,word q) :
		cid(data), cset(cs), quant(q)
	{}
};

struct combo_pool
{
	std::vector<cpool_entry> combos;
	
	combo_pool() : totalweight(0)
	{}
	combo_pool& operator=(combo_pool const& other);
	void push(int32_t cid, int8_t cs, word q=1); //add a quantity of a combo entry
	void add(int32_t cid, int8_t cs, word q=1); //add a new combo entry
	void swap(size_t ind1, size_t ind2);
	void erase(size_t ind); //Remove a combo
	void trim(); //Trim any invalid entries
	cpool_entry const* get_ind(size_t index) const;
	cpool_entry const* get_w(size_t weight_index) const;
	cpool_entry const* get_w_wrap(size_t weight_index) const;
	cpool_entry const* pick() const;
	bool get_ind(int32_t& cid, int8_t& cs, size_t index) const;
	bool get_w(int32_t& cid, int8_t& cs, size_t weight_index) const;
	bool get_w_wrap(int32_t& cid, int8_t& cs, size_t weight_index) const;
	bool pick(int32_t& cid, int8_t& cs) const;
	void clear()
	{
		combos.clear();
		combos.shrink_to_fit();
		totalweight = 0;
	}
	void recalc();
	size_t getTotalWeight() const
	{
		return totalweight;
	}
	bool valid() const
	{
		return totalweight > 0;
	}
private:
	//Does not need saving
	size_t totalweight;
	
	cpool_entry* get(int32_t cid, int8_t cs); //get a combo existing in the list
};

extern combo_alias combo_aliases[MAXCOMBOALIASES];
extern combo_pool combo_pools[MAXCOMBOPOOLS];

#endif

