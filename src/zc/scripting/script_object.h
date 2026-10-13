#ifndef ZC_SCRIPTING_SCRIPT_OBJECT_H_
#define ZC_SCRIPTING_SCRIPT_OBJECT_H_

#include "base/general.h"
#include "zc/ffscript.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

extern std::map<uint32_t, std::unique_ptr<script_object_base>> script_objects;
// Mutate only via register_script_object/delete_script_object/
// clear_script_object_ids_by_type, which keep the O(1) removal locator in
// sync. The per-type vectors are not kept in insertion order.
extern std::map<script_object_type, std::vector<uint32_t>> script_object_ids_by_type;
void clear_script_object_ids_by_type();
extern std::vector<uint32_t> next_script_object_id_freelist;

// The autorelease pool. Mutate only via the functions below, which keep an
// O(1) membership index in sync (opcode handlers query membership per object
// push/pop, so a linear scan of the pool would make N pops cost O(N^2)).
// Removal is lazy: the vector can contain stale entries, so don't read it
// directly - the index is the source of truth.
extern std::vector<uint32_t> script_object_autorelease_pool;
void script_object_autorelease_pool_add(uint32_t id);
bool script_object_autorelease_pool_remove(uint32_t id);
bool script_object_autorelease_pool_contains(uint32_t id);
// Empties the pool (and its membership index), returning the ids in insertion
// order for draining.
std::vector<uint32_t> script_object_autorelease_pool_take();

// Direct-mapped cache in front of the script_objects map, keyed by the low bits of the id.
// Scripts resolve the same few object ids millions of times per frame (member reads/writes,
// refcount updates), and each resolution is otherwise a tree walk. Entries are removed on
// object deletion and container reset (delete_script_object / script_object_lookup_cache_clear,
// called from the same places that invalidate the script array cache).
constexpr uint32_t SCRIPT_OBJECT_CACHE_SIZE = 512; // power of two
extern script_object_base* script_object_lookup_cache[SCRIPT_OBJECT_CACHE_SIZE];
void script_object_lookup_cache_clear();

void init_script_objects();
void register_script_object(script_object_base* object, script_object_type type, uint32_t id = -1);
void script_object_ref_inc(uint32_t id);
void script_object_ref_dec(uint32_t id);
void script_object_transfer_ref_to_autorelease_pool(uint32_t id);
script_object_base* get_script_object(uint32_t id);
script_object_base* get_script_object_checked(uint32_t id);

// Cache-first lookup; falls back to (and fills from) the script_objects map.
inline script_object_base* get_script_object_fast(uint32_t id)
{
	script_object_base* obj = script_object_lookup_cache[id & (SCRIPT_OBJECT_CACHE_SIZE - 1)];
	if (obj && obj->id == id)
		return obj;
	return get_script_object(id);
}
const std::map<uint32_t, std::unique_ptr<script_object_base>>& get_script_objects();
void own_script_object(script_object_base* object, ScriptType type, int i);
void own_script_object(script_object_base* object, sprite* sprite);
void free_script_object(uint32_t id);
void delete_script_object(uint32_t id, bool remove_refs = true);
std::set<uint32_t> find_script_objects_reachable_from_global_roots();
void run_gc();

// Engine storage slots that hold objects (see engine_object_slot). Scripts write these
// through untyped internal arrays, so - unlike for the arrays of sprites, which die - the
// objects must be tracked per slot of the actual storage, rather than per array: for
// example, Screen->D[] is a different slot on each screen.
//
// Writes `value` to the slot via `write` (which returns false if it couldn't), retaining
// `value` if `type` says it is an object and releasing any object the slot held.
bool engine_object_slot_write(engine_object_slot slot, int32_t value, script_object_type type, const std::function<bool()>& write);
// Engine code writing a non-object value directly to a slot must do it via `write` here,
// to release any object the slot held.
void engine_object_slot_overwrite(engine_object_slot slot, const std::function<void()>& write);
// Resizes a key's storage (Screen->DataSize, genericdata->DataSize), releasing the objects
// held by the slots it drops.
void engine_object_storage_resize(engine_object_storage storage, uint32_t key, int32_t size);
// The type of object the slot holds, or none.
script_object_type engine_object_slot_type(engine_object_slot slot);
// The slots that hold objects, for saving.
std::vector<std::pair<engine_object_slot, script_object_type>> get_engine_object_slots();
// Releases (and forgets) every slot of the storage that holds an object, for when the
// storage itself is about to be cleared.
void engine_object_slots_cleared(engine_object_storage storage);
// Forgets every slot, without releasing anything - for when all objects are deleted.
void engine_object_slots_reset();
void maybe_run_gc();

template <typename T, uint32_t Max>
struct UserDataContainer
{
	script_object_type type;
	const char* name;

	T& operator[](uint32_t id)
	{
		auto& t = script_objects.at(id);
		assert(t->type == type);
		T* ptr = static_cast<T*>(t.get());
		return *ptr;
	}

	void clear()
	{
		// intentionally copying ids as script_object_ids_by_type may be mutated
		auto ids = script_object_ids_by_type[type];
		for (auto const& id : ids)
			delete_script_object(id);
	}

	T* create(bool skipError = false)
	{
		if (script_object_ids_by_type[type].size() >= Max)
			run_gc();

		if (script_object_ids_by_type[type].size() >= Max)
		{
			if (!skipError) scripting_log_error_with_context("Failed to create {}: {} arrays already in use, no more can be allocated", name, Max);
			return nullptr;
		}

		auto object = new T{};
		register_script_object(object, type);
		return object;
	}

	void register_existing(T* object)
	{
		auto& vec = next_script_object_id_freelist;
		vec.erase(std::remove(vec.begin(), vec.end(), object->id), vec.end());
		register_script_object(object, type, object->id);
	}

	uint32_t get_free(bool skipError = false)
	{
		T* object = create(skipError);
		if (!object) return 0;
		return object->id;
	}

	T* check(uint32_t id, bool skipError = false)
	{
		script_object_base* obj = get_script_object_fast(id);
		if (obj && obj->type == type)
			return static_cast<T*>(obj);

		if (skipError) return NULL;

		scripting_log_error_with_context("Invalid {} using UID = {}", name, id);
		return NULL;
	}
	
	// Return true to stop iteration
	bool foreach(std::function<bool(uint32_t,T*)> const& proc)
	{
		auto const& ids = script_object_ids_by_type[type];
		for (auto const& id : ids)
		{
			auto& t = script_objects.at(id);
			assert(t->type == type);
			T* ptr = static_cast<T*>(t.get());
			if (proc(id, ptr))
				return true;
		}
		return false;
	}
};

#endif
