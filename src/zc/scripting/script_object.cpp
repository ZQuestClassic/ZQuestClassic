#include "zc/scripting/script_object.h"

#include "zc/scripting/types/user_object.h"
#include "zscriptversion.h"

#include <ranges>

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

	return false;
}

static uint32_t get_next_script_object_id()
{
	const uint32_t ID_FREELIST_FILL_AMOUNT = 1000;

	if (next_script_object_id_freelist.empty())
	{
		// Don't start at 1 because the first objects are expected to remain allocated.
		uint32_t id = 1000;
		while (next_script_object_id_freelist.size() < ID_FREELIST_FILL_AMOUNT)
		{
			if (!script_objects.contains(id))
				next_script_object_id_freelist.push_back(id);
			id++;
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
	for (uint32_t id = 1000; id >= 100; id--)
		next_script_object_id_freelist.push_back(id);
	script_objects.clear();
	script_object_ids_by_type.clear();
	script_object_autorelease_pool.clear();
	allocations_since_last_gc = 0;
	deallocations_since_last_gc = 0;

	// Load globals and set their reference counts.
	// Only custom user objects can be restored right now, so
	// any other object type will be set to NULL.
	game->load_user_objects();
	if (!ZScriptVersion::gc())
		return;
	for (size_t i = 0; i < MAX_SCRIPT_REGISTERS; i++)
	{
		auto type = game->global_d_types[i];
		if (type == script_object_type::object && script_objects.contains(game->global_d[i]))
			script_object_ref_inc(game->global_d[i]);
		else if (type != script_object_type::none)
			game->global_d[i] = 0;
	}
	for (auto& aptr : game->globalRAM)
	{
		if (!aptr.HoldsObjects())
			continue;

		if (aptr.ObjectType() != script_object_type::object)
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

		if (aptr.ObjectType() != script_object_type::object)
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
	for (auto object : get_user_objects())
	{
		assert(object->isGlobal());
		for (int i = 0; i < object->owned_vars; i++)
		{
			if (!object->isMemberObjectType(i))
				continue;

			if (object->var_types[i] == script_object_type::object && script_objects.contains(object->data[i]))
				script_object_ref_inc(object->data[i]);
			else
				object->data[i] = 0;
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
	object->ref_count--;

	if (auto usr_object = dynamic_cast<user_object*>(object))
		if (usr_object->isGlobal())
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
		Z_error_fatal("Invalid object pointer used in get_script_object_checked\n");
	return object;
}

void delete_script_object(uint32_t id)
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

	if (object->type == script_object_type::object)
	{
		auto usr_object = static_cast<user_object*>(object.get());
		for (int i = 0; i < usr_object->owned_vars; i++)
		{
			if (usr_object->isMemberObjectType(i))
				script_object_ref_dec(usr_object->data[i]);
		}

		usr_object->destruct.execute();
		usr_object->clear_nodestruct();
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
		object->own(type, i);
		return;
	}

	bool was_owned = object->owned_type != ScriptType::None;
	object->own(type, i);
	bool is_owned = type != ScriptType::None;
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

// Find unreachable objects via mark-and-sweep, and destroy them.
// This handles cyclical objects.
// It should not be called very often as it can be expensive.
// Note: Most objects are cleared up via reference counting
// (when ref_count is zero in script_object_ref_dec).
void run_gc()
{
	std::vector<user_abstract_obj*> all_objects;
	for (auto& [id, object] : script_objects)
		all_objects.push_back(object.get());

	std::set<uint32_t> live_object_ids;

	for (auto& object : all_objects)
	{
		if (object->type != script_object_type::object)
			continue;

		auto usr_object = static_cast<user_object*>(object);
		if (usr_object->isGlobal())
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
	for (size_t i = 0; i < MAX_SCRIPT_REGISTERS; i++)
	{
		if (game->global_d_types[i] != script_object_type::none)
			live_object_ids.insert(game->global_d[i]);
	}
	for (auto& data : scriptEngineDatas | std::views::values)
	{
		for (int i : data.ref.stack_pos_is_object)
			live_object_ids.insert(data.stack[i]);
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

		bool can_hold_objects = false;
		switch (object->type)
		{
			case script_object_type::object:
			// TODO: handle stacks?
			// case script_object_type::stack:
				can_hold_objects = true;
				break;
		}
		if (!can_hold_objects)
			continue;

		auto usr_object = dynamic_cast<user_object*>(object);
		assert(usr_object);
		if (!usr_object)
			continue;

		worklist.insert(usr_object);
	}

	// Find all the reachable objects.
	while (worklist.size())
	{
		auto base_object = *worklist.begin();
		worklist.erase(worklist.begin());

		// Currently only user objects retain references.
		if (base_object->type != script_object_type::object)
			continue;

		auto object = static_cast<user_object*>(base_object);
		for (int i = 0; i < object->owned_vars; i++)
		{
			if (!object->isMemberObjectType(i))
				continue;

			auto id = object->data[i];
			if (id && !live_object_ids.contains(id))
			{
				live_object_ids.insert(id);
				worklist.insert(get_script_object(id));
			}
		}

		for (int i = object->owned_vars; i < object->data.size(); i++)
		{
			auto ptr = object->data[i]/10000;
			if (ptr == 0)
				continue;

			auto& aptr = objectRAM.at(-ptr);
			if (!aptr.HoldsObjects())
				continue;

			for (int i = 0; i < aptr.Size(); i++)
			{
				auto id = aptr[i];
				if (id && !live_object_ids.contains(id))
				{
					live_object_ids.insert(id);
					worklist.insert(get_script_object(id));
				}
			}
		}
	}

	// Delete unreachable objects.
	for (auto& object : all_objects)
	{
		if (live_object_ids.contains(object->id))
			continue;

		// This object is not reachable.

		if (auto usr_object = dynamic_cast<user_object*>(object))
		{
			// To avoid problems when `delete_script_object` deletes the object, clear the
			// members here.
			// Any references held in members are also not reachable, so this is fine to do
			// because they will also be deleted by this gc call.
			for (int i = usr_object->owned_vars; i < usr_object->data.size(); i++)
			{
				auto ptr = usr_object->data[i]/10000;
				void destroy_object_arr(int32_t ptr, bool dec_refs);
				destroy_object_arr(ptr, false);
			}
			usr_object->owned_vars = 0;
			usr_object->data.clear();
		}

		util::remove_if_exists(script_object_autorelease_pool, object->id);
		delete_script_object(object->id);
	}

	allocations_since_last_gc = 0;
	deallocations_since_last_gc = 0;
}

void maybe_run_gc()
{
	if (allocations_since_last_gc - deallocations_since_last_gc > 700)
		run_gc();
}
