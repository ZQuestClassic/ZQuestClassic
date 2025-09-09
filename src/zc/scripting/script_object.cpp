#include "zc/scripting/script_object.h"

#include "base/zc_array.h"
#include "zc/ffscript.h"
#include "zc/scripting/types/user_object.h"
#include "zscriptversion.h"

#include <ranges>
#include <utility>

// TODO: make these static.
std::map<uint32_t, std::unique_ptr<user_abstract_obj>> script_objects;
std::map<script_object_type, std::vector<uint32_t>> script_object_ids_by_type;
std::vector<uint32_t> script_object_autorelease_pool;
std::vector<uint32_t> next_script_object_id_freelist;

static int allocations_since_last_gc;
static int deallocations_since_last_gc;

// Returns true if an id is not usable for an object id.
static bool is_reserved_object_id(uint32_t id)
{
	// Used by internal bitmaps (see do_loadbitmapid).
	if (id >= 9 && id <= 16)
		return true;

	// RT_SCREEN, RT_BITMAP0, etc. can be given the draw commands. We must be able to
	// distinguish those from user bitmaps, so make this range unusable.
	if (id < 60000 || id == -10000 || id == -20000)
		return true;

	// These are often used to indicate "no value" (such as accessing an invalid array or out of
	// bounds).
	if (id == -10000 || id == -1)
		return true;

	return false;
}

static uint32_t get_next_script_object_id()
{
	const uint32_t ID_FREELIST_FILL_AMOUNT = 1000;

	if (next_script_object_id_freelist.empty())
	{
		// Don't start at 1 because is_reserved_object_id excludes all low values.
		uint32_t id = 60000;
		while (next_script_object_id_freelist.size() < ID_FREELIST_FILL_AMOUNT)
		{
			if (!script_objects.contains(id) && !is_reserved_object_id(id))
				next_script_object_id_freelist.push_back(id);
			id++;

			if (unlikely(id == (uint32_t)-1))
				break;
		}
	}

	// ~4 billion objects is a lot, so it's unlikely this condition will ever be true.
	// But just in case...
	if (next_script_object_id_freelist.empty())
		Z_error_fatal("Ran out of storage for script objects\n");

	auto id = next_script_object_id_freelist.back();
	next_script_object_id_freelist.pop_back();
	return id;
}

void init_script_objects()
{
	next_script_object_id_freelist.clear();
	// See `is_reserved_object_id` for why ids do not begin at 1.
	for (uint32_t id = 61000; id >= 60001; id--)
		next_script_object_id_freelist.push_back(id);
	script_objects.clear();
	script_object_ids_by_type.clear();
	script_object_autorelease_pool.clear();
	allocations_since_last_gc = 0;
	deallocations_since_last_gc = 0;

	game->load_objects(ZScriptVersion::gc_arrays());

	if (!ZScriptVersion::gc())
		return;

	// Set reference counts.

	for (size_t i = 0; i < MAX_SCRIPT_REGISTERS; i++)
	{
		auto type = game->global_d_types[i];
		if (can_restore_object_type(type) && script_objects.contains(game->global_d[i]))
			script_object_ref_inc(game->global_d[i]);
		else if (type != script_object_type::none)
			game->global_d[i] = 0;
	}

	for (auto& [id, object] : script_objects)
		object->restore_references();

	if (!ZScriptVersion::gc_arrays())
	{
		for (auto& aptr : game->globalRAM)
		{
			if (!aptr.HoldsObjects())
				continue;
	
			if (!can_restore_object_type(aptr.ObjectType()))
			{
				for (int i = 0; i < aptr.Size(); i++)
					aptr[i] = 0;
				continue;
			}
	
			for (int i = 0; i < aptr.Size(); i++)
			{
				if (script_objects.contains(aptr[i]))
					script_object_ref_inc(aptr[i]);
				else
					aptr[i] = 0;
			}
		}
		// This covers any arrays held by saved objects.
		for (auto& aptr : objectRAM | std::views::values)
		{
			if (!aptr.HoldsObjects())
				continue;
	
			if (!can_restore_object_type(aptr.ObjectType()))
			{
				for (int i = 0; i < aptr.Size(); i++)
					aptr[i] = 0;
				continue;
			}
	
			for (int i = 0; i < aptr.Size(); i++)
			{
				if (script_objects.contains(aptr[i]))
					script_object_ref_inc(aptr[i]);
				else
					aptr[i] = 0;
			}
		}
	}
}

void register_script_object(user_abstract_obj* object, script_object_type type, uint32_t id)
{
	if (id == -1)
		id = get_next_script_object_id();

	object->type = type;
	object->id = id;
	script_objects[id] = std::unique_ptr<user_abstract_obj>(object);
	script_object_ids_by_type[type].push_back(id);

	allocations_since_last_gc++;
	object->ref_count = 1;
	script_object_autorelease_pool.push_back(id);
}

static void script_object_ref_inc(user_abstract_obj* object)
{
	object->ref_count++;

	// Remove the autorelease pool when an object gets its first explicit retaining reference.
	if (object->ref_count == 2 && util::remove_if_exists(script_object_autorelease_pool, object->id))
		script_object_ref_dec(object->id);
}

void script_object_ref_inc(uint32_t id)
{
	if (!id)
		return;

	auto object = get_script_object_checked(id);
	if (object)
		script_object_ref_inc(object);
}

static void script_object_ref_dec(user_abstract_obj* object)
{
	assert(object->ref_count > 0);
	if (object->ref_count == 0) return;
	object->ref_count--;

	if (object->global)
		return;

	if (ZScriptVersion::gc() && object->ref_count == 0)
		delete_script_object(object->id);
}

void script_object_ref_dec(uint32_t id)
{
	if (!id)
		return;

	auto object = get_script_object_checked(id);
	if (object)
		script_object_ref_dec(object);
}

user_abstract_obj* get_script_object(uint32_t id)
{
	auto it = script_objects.find(id);
	if (it != script_objects.end())
		return it->second.get();
	return nullptr;
}

user_abstract_obj* get_script_object_checked(uint32_t id)
{
	if (is_reserved_object_id(id))
		return nullptr;

	auto object = get_script_object(id);
	if (!object)
		scripting_log_error_with_context("Invalid object pointer used in get_script_object_checked: {}", id);
	return object;
}

const std::map<uint32_t, std::unique_ptr<user_abstract_obj>>& get_script_objects()
{
	return script_objects;
}

void delete_script_object(uint32_t id, bool remove_refs)
{
	auto it = script_objects.find(id);
	if (it == script_objects.end())
		return;

	auto& object = it->second;

	// Bitmap objects can't be deleted right away, since drawing operations are deferred slightly.
	// We must wait for the script drawing to be done with it, which is signaled by script_bitmaps::update
	if (object->type == script_object_type::bitmap)
	{
		auto bitmap = static_cast<user_bitmap*>(object.get());
		if (!bitmap->can_del())
		{
			bitmap->free_obj();
			return;
		}
	}

	if (auto usr_object = dynamic_cast<user_object*>(object.get()))
		usr_object->destruct.execute();

	if (remove_refs)
	{
		std::vector<uint32_t> retained_ids;
		object->get_retained_ids(retained_ids);
		for (auto id : retained_ids)
			script_object_ref_dec(id);
	}

	if (!ZScriptVersion::gc_arrays())
	{
		if (auto usr_object = dynamic_cast<user_object*>(object.get()))
		{
			for (int ind = usr_object->owned_vars; ind < usr_object->data.size(); ++ind)
			{
				auto arrptr = usr_object->data.at(ind)/10000;
				auto it = objectRAM.find(-arrptr);
				if (it != objectRAM.end())
					objectRAM.erase(it);
			}
		}
	}

	util::remove_if_exists(script_object_ids_by_type[object->type], id);
	script_objects.erase(it);
	deallocations_since_last_gc++;

	if (next_script_object_id_freelist.size() < 10000)
		next_script_object_id_freelist.push_back(id);
}

void own_script_object(user_abstract_obj* object, ScriptType type, int i)
{
	if (!ZScriptVersion::gc())
	{
		object->set_owned_by_script(type, i);
		return;
	}

	bool was_owned = object->is_owned();
	object->set_owned_by_script(type, i);
	bool is_owned = type != ScriptType::None;
	if (was_owned && !is_owned)
		script_object_ref_dec(object);
	else if (!was_owned && is_owned)
		script_object_ref_inc(object);
}

void own_script_object(user_abstract_obj* object, sprite* sprite)
{
	if (!ZScriptVersion::gc())
	{
		object->set_owned_by_sprite(sprite);
		return;
	}

	bool was_owned = object->is_owned();
	object->set_owned_by_sprite(sprite);
	bool is_owned = sprite;
	if (was_owned && !is_owned)
		script_object_ref_dec(object);
	else if (!was_owned && is_owned)
		script_object_ref_inc(object);
}

void free_script_object(uint32_t id)
{
	if (!ZScriptVersion::gc())
		delete_script_object(id);
}

// Returns set of object ids that are reachable from root objects.
static auto run_mark_and_sweep(bool only_include_global_roots)
{
	std::set<uint32_t> live_object_ids;

	std::vector<user_abstract_obj*> all_objects;
	for (auto& [id, object] : script_objects)
		all_objects.push_back(object.get());

	for (auto& object : all_objects)
	{
		if (object->global)
			live_object_ids.insert(object->id);
	}
	for (auto& aptr : game->globalRAM)
	{
		if (aptr.HoldsObjects())
		{
			for (int i = 0; i < aptr.Size(); i++)
			{
				live_object_ids.insert(aptr[i]);
			}
		}
	}
	for (size_t i = 0; i < MAX_SCRIPT_REGISTERS; i++)
	{
		if (game->global_d_types[i] != script_object_type::none)
			live_object_ids.insert(game->global_d[i]);
	}
	if (!only_include_global_roots)
	{
		for (auto& aptr : localRAM)
		{
			if (aptr.HoldsObjects())
			{
				for (int i = 0; i < aptr.Size(); i++)
				{
					live_object_ids.insert(aptr[i]);
				}
			}
		}
		for (auto& data : scriptEngineDatas | std::views::values)
		{
			for (int i : data.ref.stack_pos_is_object)
				live_object_ids.insert(data.stack[i]);
		}
		for (auto id : script_object_autorelease_pool)
			live_object_ids.insert(id);
	}

	// Insert all root objects into worklist.
	std::set<user_abstract_obj*> worklist;
	for (auto& id : live_object_ids)
	{
		if (is_reserved_object_id(id))
			continue;

		auto object = get_script_object(id);
		assert(id == 0 || object);
		if (!object)
			continue;

		worklist.insert(object);
	}

	std::vector<uint32_t> retained_ids;

	// Find all the reachable objects.
	while (worklist.size())
	{
		auto base_object = *worklist.begin();
		worklist.erase(worklist.begin());

		retained_ids.clear();
		base_object->get_retained_ids(retained_ids);
		for (auto id : retained_ids)
		{
			if (id && !live_object_ids.contains(id))
			{
				live_object_ids.insert(id);
				worklist.insert(get_script_object(id));
			}
		}
	}

	return std::make_pair(live_object_ids, all_objects);
}

std::set<uint32_t> find_script_objects_reachable_from_global_roots()
{
	auto [live_object_ids, _] = run_mark_and_sweep(true);
	return live_object_ids;
}

// Find unreachable objects via mark-and-sweep, and destroy them.
// This handles cyclical objects.
// It should not be called very often as it can be expensive.
// Note: Most objects are cleared up via reference counting
// (when ref_count is zero in script_object_ref_dec).
void run_gc()
{
	auto [live_object_ids, all_objects] = run_mark_and_sweep(false);

	// Delete unreachable objects.
	for (auto& object : all_objects)
	{
		if (live_object_ids.contains(object->id))
			continue;

		bool remove_refs = false;
		delete_script_object(object->id, remove_refs);
	}

	allocations_since_last_gc = 0;
	deallocations_since_last_gc = 0;
}

void maybe_run_gc()
{
	if (allocations_since_last_gc - deallocations_since_last_gc > 700)
		run_gc();
}
