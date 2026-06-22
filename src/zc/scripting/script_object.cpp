#include "zc/scripting/script_object.h"

#include "base/general.h"
#include "base/zc_array.h"
#include "user_object.h"
#include "zc/ffscript.h"
#include "zc/scripting/types/bitmap.h"
#include "zc/scripting/types/user_object.h"
#include "zc/zscriptversion.h"

#include <ranges>
#include <unordered_set>
#include <utility>

// TODO: make these static.
std::map<uint32_t, std::unique_ptr<script_object_base>> script_objects;
std::map<script_object_type, std::vector<uint32_t>> script_object_ids_by_type;
std::vector<uint32_t> script_object_autorelease_pool;
std::vector<uint32_t> next_script_object_id_freelist;
std::vector<uint32_t> untyped_internal_arrays_retaining_references;

// O(1) membership index for script_object_autorelease_pool. Only the functions
// below touch either container, keeping them in sync.
static std::unordered_set<uint32_t> script_object_autorelease_pool_index;

void script_object_autorelease_pool_add(uint32_t id)
{
	if (script_object_autorelease_pool_index.insert(id).second)
		script_object_autorelease_pool.push_back(id);
}

bool script_object_autorelease_pool_remove(uint32_t id)
{
	if (!script_object_autorelease_pool_index.erase(id))
		return false;
	util::remove_if_exists(script_object_autorelease_pool, id);
	return true;
}

bool script_object_autorelease_pool_contains(uint32_t id)
{
	return script_object_autorelease_pool_index.contains(id);
}

std::vector<uint32_t> script_object_autorelease_pool_take()
{
	script_object_autorelease_pool_index.clear();
	return std::move(script_object_autorelease_pool);
}

static int allocations_since_last_gc;
static int deallocations_since_last_gc;

// Objects whose ZScript destructor is currently executing. They (and their
// destructors' local stacks, see active_object_dtor_script_datas) are GC roots,
// and must never be deleted re-entrantly.
static std::vector<uint32_t> objects_being_destructed;

// While run_gc iterates its unreachable-id lists, ids freed along the way (by
// destructors releasing references) must not be handed out to new allocations:
// a reused id would make those lists point at fresh, live objects. Freed ids
// are parked here until the GC finishes.
static bool gc_in_progress;
static std::vector<uint32_t> ids_freed_during_gc;

static bool is_reserved_bitmap_id(uint32_t id)
{
	// Used by internal bitmaps (see do_loadbitmapid).
	if (id >= 9 && id <= 16)
		return true;

	return false;
}

// Returns true if an id is not usable for an object id.
static bool is_reserved_object_id(uint32_t id)
{
	if (is_reserved_bitmap_id(id))
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
	// Don't start at 1 because is_reserved_object_id excludes all low values.
	const uint32_t MIN_ID = 60000;

	if (next_script_object_id_freelist.empty())
	{
		// Remember where we left off.
		static uint32_t id = MIN_ID;
		uint32_t items_checked = 0;

		while (next_script_object_id_freelist.size() < ID_FREELIST_FILL_AMOUNT)
		{
			if (!script_objects.contains(id) && !is_reserved_object_id(id) &&
			    !(gc_in_progress && util::contains(ids_freed_during_gc, id)))
				next_script_object_id_freelist.push_back(id);

			id++;
			items_checked++;

			if (unlikely(id == (uint32_t)-1))
				id = MIN_ID;

			// Prevent an infinite loop if memory is truly full.
			if (unlikely(items_checked > (uint32_t)-1))
				break;
		}
	}

	// ~4 billion objects is a lot, so it's unlikely this condition will ever be true.
	// But just in case...
	if (next_script_object_id_freelist.empty())
		Z_error_fatal("Ran out of storage for script objects\n");

	auto ret_id = next_script_object_id_freelist.back();
	next_script_object_id_freelist.pop_back();
	return ret_id;
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
	script_object_autorelease_pool_index.clear();
	objects_being_destructed.clear();
	gc_in_progress = false;
	ids_freed_during_gc.clear();
	allocations_since_last_gc = 0;
	deallocations_since_last_gc = 0;

	game->load_objects(ZScriptVersion::gc_arrays());

	if (!ZScriptVersion::gc())
		return;

	// Set reference counts.

	for (size_t i = 0; i < MAX_GLOBAL_VARIABLES; i++)
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

void register_script_object(script_object_base* object, script_object_type type, uint32_t id)
{
	if (id == -1)
		id = get_next_script_object_id();

	object->type = type;
	object->id = id;
	script_objects[id] = std::unique_ptr<script_object_base>(object);
	script_object_ids_by_type[type].push_back(id);

	allocations_since_last_gc++;
	object->ref_count = 1;
	script_object_autorelease_pool_add(id);
}

static void script_object_ref_inc(script_object_base* object)
{
	object->ref_count++;

	// Remove the autorelease pool when an object gets its first explicit retaining reference.
	if (object->ref_count == 2 && script_object_autorelease_pool_remove(object->id))
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

static void script_object_ref_dec(script_object_base* object)
{
	DCHECK(ZScriptVersion::gc());

	if (object->ref_count <= 0)
	{
		scripting_log_error_with_context("Invalid object reference decrement in script_object_ref_dec: {}", object->id);
		DCHECK(object->ref_count > 0);
		return;
	}

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

// Transfers a counted reference into the autorelease pool: the object stays alive
// (with no net reference count change) until the pool next drains. If the object is
// already in the pool, the reference is released outright. Use this when removing
// the last reference to an object that is about to be returned to a script.
void script_object_transfer_ref_to_autorelease_pool(uint32_t id)
{
	if (!id)
		return;

	if (!script_object_autorelease_pool_contains(id))
		script_object_autorelease_pool_add(id);
	else
		script_object_ref_dec(id);
}

script_object_base* get_script_object(uint32_t id)
{
	auto it = script_objects.find(id);
	if (it != script_objects.end())
		return it->second.get();
	return nullptr;
}

script_object_base* get_script_object_checked(uint32_t id)
{
	if (is_reserved_bitmap_id(id))
		return nullptr;

	auto object = get_script_object(id);
	if (!object)
		scripting_log_error_with_context("Invalid object pointer used in get_script_object_checked: {}", id);
	return object;
}

const std::map<uint32_t, std::unique_ptr<script_object_base>>& get_script_objects()
{
	return script_objects;
}

// Runs the object's ZScript destructor, at most once ever. The object is kept
// alive (and guarded against re-entrant deletion) while it runs.
static bool run_destructor(script_object_base* object)
{
	auto usr_object = dynamic_cast<user_object*>(object);
	if (!usr_object || !usr_object->destruct.pc)
		return false;

	// Bump the reference count so temporary references created while the
	// destructor runs can't hit 0 and delete the object re-entrantly.
	object->ref_count++;
	// Clear the destructor before running it, so that no re-entrant path can
	// ever run it a second time.
	auto exec = usr_object->destruct;
	usr_object->destruct.pc = 0;
	objects_being_destructed.push_back(object->id);
	exec.execute();
	objects_being_destructed.pop_back();
	object->ref_count--;
	return true;
}

void delete_script_object(uint32_t id, bool remove_refs)
{
	auto it = script_objects.find(id);
	if (it == script_objects.end())
		return;

	script_object_base* object = it->second.get();

	// Bitmap objects can't be deleted right away, since drawing operations are deferred slightly.
	// We must wait for the script drawing to be done with it, which is signaled by script_bitmaps::update
	if (object->type == script_object_type::bitmap)
	{
		auto bitmap = static_cast<user_bitmap*>(object);
		if (!bitmap->can_del())
		{
			bitmap->free_obj();
			return;
		}
	}

	// An object whose destructor is currently running must not be deleted out
	// from under it (for example, by a GC run triggered within the destructor).
	if (util::contains(objects_being_destructed, id))
		return;

	// Artificially bump the reference count to prevent re-entrant deletion.
	// When the ZScript destructor runs, temporary engine stack references
	// will increment/decrement this safely without hitting 0 again.
	object->ref_count++;

	run_destructor(object);

	if (remove_refs)
	{
		std::vector<uint32_t> retained_ids;
		object->get_retained_ids(retained_ids);
		for (auto id : retained_ids)
			script_object_ref_dec(id);
	}

	if (!ZScriptVersion::gc_arrays())
	{
		if (auto usr_object = dynamic_cast<user_object*>(object))
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

	// Restore the reference count (doesn't really matter, about to delete it).
	object->ref_count--;

	util::remove_if_exists(script_object_ids_by_type[object->type], id);
	script_objects.erase(it);
	deallocations_since_last_gc++;

	if (gc_in_progress)
		ids_freed_during_gc.push_back(id);
	else if (next_script_object_id_freelist.size() < 10000)
		next_script_object_id_freelist.push_back(id);
}

void own_script_object(script_object_base* object, ScriptType type, int i)
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

void own_script_object(script_object_base* object, sprite* sprite)
{
	if (!sprite)
		return;

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

	std::vector<script_object_base*> all_objects;
	std::vector<script_array*> script_arrays;
	for (auto& [id, object] : script_objects)
	{
		all_objects.push_back(object.get());

		if (object->type == script_object_type::array)
		{
			auto array = static_cast<script_array*>(object.get());
			script_arrays.push_back(array);
		}
	}

	for (auto& object : all_objects)
	{
		if (object->global)
			live_object_ids.insert(object->id);
	}
	if (!ZScriptVersion::gc_arrays())
	{
		for (auto& aptr : game->globalRAM)
		{
			if (aptr.HoldsObjects())
			{
				for (int i = 0; i < aptr.Size(); i++)
				{
					live_object_ids.insert(aptr[i]);
				}
			}
			// Not checking "MaybeHoldsObjects" because globalRAM is only used in older quests.
		}
	}
	for (size_t i = 0; i < MAX_GLOBAL_VARIABLES; i++)
	{
		if (game->global_d_types[i] != script_object_type::none)
			live_object_ids.insert(game->global_d[i]);
	}
	if (!only_include_global_roots)
	{
		if (!ZScriptVersion::gc_arrays())
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
				// Not checking "MaybeHoldsObjects" because localRAM is only used in older quests.
			}
		}
		for (auto array : script_arrays)
		{
			if (array->internal_id && !array->internal_expired)
				live_object_ids.insert(array->id);
		}
		for (auto& data : scriptEngineDatas | std::views::values)
		{
			for (int i : data.ref.stack_pos_is_object)
				live_object_ids.insert(data.stack[i]);
			
			for (size_t q = 0; q < data.ref.zs_vargs_stack.size(); ++q)
				for (int i : data.ref.zs_vargs_pos_is_object[q])
					live_object_ids.insert(data.ref.zs_vargs_stack.at(q).at(i));
			
			for (int i : data.ref.script_d_is_object)
				live_object_ids.insert(data.ref.script_d[i]);
		}
		for (auto id : script_object_autorelease_pool)
			live_object_ids.insert(id);
		// Objects currently running their destructor, and the local stacks of those
		// destructors, are roots too - otherwise a GC triggered from within a
		// destructor could delete objects the destructor is still using.
		for (auto id : objects_being_destructed)
			live_object_ids.insert(id);
		for (auto* named_data : active_object_dtor_script_datas)
		{
			auto& data = *named_data->data;
			for (int i : data.ref.stack_pos_is_object)
				live_object_ids.insert(data.stack[i]);
			for (size_t q = 0; q < data.ref.zs_vargs_stack.size(); ++q)
				for (int i : data.ref.zs_vargs_pos_is_object[q])
					live_object_ids.insert(data.ref.zs_vargs_stack.at(q).at(i));
		}
	}

	// Insert all root objects into worklist.
	std::set<script_object_base*> worklist;
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
				if (auto object = get_script_object(id))
				{
					live_object_ids.insert(id);
					worklist.insert(object);
				}
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
	// Park ids freed during the collection (a nested run_gc, triggered by an
	// allocating destructor, keeps the outer collection's parking in effect).
	bool was_gc_in_progress = gc_in_progress;
	gc_in_progress = true;

	auto [live_object_ids, all_objects] = run_mark_and_sweep(false);

	// Collect ids rather than using the object pointers directly: the destructors
	// below run script code that can delete other unreachable objects.
	std::vector<uint32_t> unreachable_ids;
	for (auto& object : all_objects)
	{
		if (!live_object_ids.contains(object->id))
			unreachable_ids.push_back(object->id);
	}

	// Run destructors first. They execute script code that can write to object
	// members, and those writes modify reference counts. That must settle before
	// the references retained by unreachable objects are released below, or a
	// destructor clearing a member would release the same reference twice -
	// possibly deleting a still-reachable object.
	bool any_destructor_ran = false;
	for (auto id : unreachable_ids)
	{
		auto object = get_script_object(id);
		if (!object)
			continue;

		if (run_destructor(object))
			any_destructor_ran = true;
	}

	if (any_destructor_ran)
	{
		// The destructors ran script code, which may have changed what is reachable
		// (or deleted some of these objects outright, allowing their ids to be
		// reused by new allocations). Recompute, and only delete what is still
		// unreachable.
		auto [live_object_ids2, _] = run_mark_and_sweep(false);
		std::erase_if(unreachable_ids, [&](uint32_t id) {
			return live_object_ids2.contains(id) || !script_objects.contains(id);
		});
	}

	// We're about to delete unreachable objects, but it's possible that they
	// retain objects that are reachable. It's important to release those
	// references now.
	for (auto id : unreachable_ids)
	{
		auto object = get_script_object(id);
		if (!object)
			continue;

		std::vector<uint32_t> retained_ids;
		object->get_retained_ids(retained_ids);
		for (auto retained_id : retained_ids)
		{
			// The child may be unreachable too, and thus will be deleted when the
			// loop below reaches it. So just update ref_count.
			auto child = get_script_object(retained_id);
			if (child) child->ref_count--;
		}
	}

	for (auto id : unreachable_ids)
	{
		bool remove_refs = false;
		delete_script_object(id, remove_refs);
	}

	gc_in_progress = was_gc_in_progress;
	if (!gc_in_progress)
	{
		for (auto id : ids_freed_during_gc)
		{
			if (next_script_object_id_freelist.size() < 10000)
				next_script_object_id_freelist.push_back(id);
		}
		ids_freed_during_gc.clear();
	}

	allocations_since_last_gc = 0;
	deallocations_since_last_gc = 0;
}

void maybe_run_gc()
{
	if (allocations_since_last_gc - deallocations_since_last_gc > 700)
		run_gc();
}
