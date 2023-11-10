#include "combo.h"
#include "cpool.h"
#include "random.h"
#include <assert.h>

combo_alias combo_aliases[MAXCOMBOALIASES];
combo_pool combo_pools[MAXCOMBOPOOLS];
extern vector<newcombo> combobuf;

int comboa_lmasktotal(byte layermask)
{
    int result = 0;
	for(int q = 0; q < 6; ++q)
		if(layermask & (1<<q))
			++result;
    return result;
}
combo_alias::combo_alias()
{
	memset(this, 0, sizeof(combo_alias));
	combos = new word[1];
	csets = new byte[1];
	combos[0] = 0;
	csets[0] = 0;
}
bool combo_alias::valid() const
{
	if(combo)
		return combobuf[combo].tile;
	return width || height || (combos && combos[0]);
}
combo_alias& combo_alias::operator=(combo_alias const& other)
{
	width = other.width;
	height = other.height;
	layermask = other.layermask;
	combo = other.combo;
	cset = other.cset;
	int count = (comboa_lmasktotal(layermask)+1)*(width+1)*(height+1);
	if(combos) delete[] combos;
	if(csets) delete[] csets;
	combos = new word[count];
	csets = new byte[count];
	memcpy(combos, other.combos, sizeof(word)*count);
	memcpy(csets, other.csets, sizeof(byte)*count);
	return *this;
}
void combo_alias::clear()
{
	*this = combo_alias();
}

combo_pool& combo_pool::operator=(combo_pool const& other)
{
	clear();
	for(cpool_entry const& cp : other.combos)
	{
		add(cp.cid, cp.cset, cp.quant);
	}
	recalc();
	return *this;
}
void combo_pool::push(int32_t cid, int8_t cs, word q) //add a combo with quantity
{
	if(!q) return;
	totalweight += q;
	for(cpool_entry& cp : combos)
	{
		if(cp.cid == cid && cp.cset == cs)
		{
			cp.quant += q;
			return;
		}
	}
	combos.emplace_back(cid,cs,q);
}
void combo_pool::add(int32_t cid, int8_t cs, word q) //add a new combo entry
{
	totalweight += q;
	combos.emplace_back(cid,cs,q);
}
void combo_pool::swap(size_t ind1, size_t ind2) //Swap 2 combos
{
	if(ind1 >= combos.size()) return;
	if(ind2 >= combos.size()) return;
	
	auto it1 = combos.begin();
	for(size_t q = 0; q < ind1 && it1 != combos.end(); ++q)
		++it1;
	if(it1 == combos.end()) return;
	
	auto it2 = combos.begin();
	for(size_t q = 0; q < ind2 && it2 != combos.end(); ++q)
		++it2;
	if(it2 == combos.end()) return;
	
	cpool_entry cp1 = *it1;
	cpool_entry cp2 = *it2;
	it1 = combos.insert(it1, cp2);
	it1 = combos.erase(++it1);
	//it2 is invalidated, find it again?
	it2 = combos.begin();
	for(size_t q = 0; q < ind2 && it2 != combos.end(); ++q)
		++it2;
	if(it2 == combos.end()) assert(false); //should never happen
	it2 = combos.insert(it2, cp1);
	it2 = combos.erase(++it2);
}
void combo_pool::erase(size_t ind) //Remove a combo
{
	if(ind >= combos.size()) return;
	auto it = combos.begin();
	for(size_t q = 0; q < ind && it != combos.end(); ++q)
		++it;
	if(it == combos.end()) return;
	totalweight -= it->quant;
	combos.erase(it);
}
void combo_pool::trim() //Trim any invalid entries or unneeded space
{
	for(auto it = combos.begin(); it != combos.end();)
	{
		if (!it->valid()) //0 quant or bad cid, remove
		{
			it = combos.erase(it);
			continue;
		}
		
		cpool_entry* entry = &(*it);
		cpool_entry* canon_entry = get(entry->cid, entry->cset);
		if(canon_entry != entry) //Duplicate entry, merge them
		{
			canon_entry->quant += entry->quant;
			it = combos.erase(it);
			continue;
		}
		++it;
	}
	recalc();
}
cpool_entry const* combo_pool::get_ind(size_t index) const
{
	if(index >= combos.size()) return nullptr;
	return &(combos.at(index));
}
cpool_entry const* combo_pool::get_w(size_t weight_index) const
{
	if(!combos.size() || totalweight < 1
		|| weight_index >= size_t(totalweight))
		return nullptr;
	size_t curweight = 0;
	for(cpool_entry const& cp : combos)
	{
		curweight += cp.quant;
		if(weight_index < curweight)
			return &cp;
	}
	return nullptr; //Error?
}
cpool_entry const* combo_pool::get_w_wrap(size_t weight_index) const
{
	if(!combos.size() || totalweight < 1)
		return nullptr;
	weight_index %= size_t(totalweight);
	size_t curweight = 0;
	for(cpool_entry const& cp : combos)
	{
		curweight += cp.quant;
		if(weight_index < curweight)
			return &cp;
	}
	return nullptr; //Error?
}
cpool_entry const* combo_pool::pick() const
{
	if(totalweight < 1)
		return nullptr;
	return get_w(zc_rand(totalweight-1));
}
static bool load_entry(cpool_entry const* entry, int32_t& cid, int8_t& cs)
{
	if(entry)
	{
		cid = entry->cid;
		if(entry->cset > -1)
		{
			cs = entry->cset;
		}
		return true;
	}
	return false;
}
bool combo_pool::get_ind(int32_t& cid, int8_t& cs, size_t index) const
{
	return load_entry(get_ind(index), cid, cs);
}
bool combo_pool::get_w(int32_t& cid, int8_t& cs, size_t weight_index) const
{
	return load_entry(get_w(weight_index), cid, cs);
}
bool combo_pool::get_w_wrap(int32_t& cid, int8_t& cs, size_t weight_index) const
{
	return load_entry(get_w_wrap(weight_index), cid, cs);
}
bool combo_pool::pick(int32_t& cid, int8_t& cs) const
{
	return load_entry(pick(), cid, cs);
}
void combo_pool::recalc()
{
	totalweight = 0;
	for(cpool_entry const& cp : combos)
	{
		totalweight += cp.quant;
	}
}
cpool_entry* combo_pool::get(int32_t cid, int8_t cs)
{
	for(cpool_entry& cp : combos)
	{
		if(cp.cid == cid && cp.cset == cs)
		{
			return &cp;
		}
	}
	return nullptr;
}
