#ifndef ZC_SCRIPTING_SCRIPT_OBJECT_H_
#define ZC_SCRIPTING_SCRIPT_OBJECT_H_

#include "base/general.h"
#include "base/ints.h"
#include "zc/ffscript.h"
#include "zc/zelda.h"

#include <map>
#include <memory>

extern std::map<uint32_t, std::unique_ptr<user_abstract_obj>> script_objects;
extern std::map<script_object_type, std::vector<uint32_t>> script_object_ids_by_type;
extern std::vector<uint32_t> script_object_autorelease_pool;
extern std::vector<uint32_t> next_script_object_id_freelist;

void init_script_objects();
void register_script_object(user_abstract_obj* object, script_object_type type, uint32_t id = -1);
void script_object_ref_inc(uint32_t id);
void script_object_ref_dec(uint32_t id);
user_abstract_obj* get_script_object(uint32_t id);
user_abstract_obj* get_script_object_checked(uint32_t id);
void own_script_object(user_abstract_obj* object, ScriptType type, int i);
void free_script_object(uint32_t id);
void delete_script_object(uint32_t id);
void run_gc();
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
		auto ids = script_object_ids_by_type[type];
		for (auto id : ids)
			delete_script_object(id);
	}

	T* create(bool skipError = false)
	{
		if (script_object_ids_by_type[type].size() >= Max)
			run_gc();

		if (script_object_ids_by_type[type].size() >= Max)
		{
			if (!skipError) Z_scripterrlog("could not find a valid free %s pointer!\n", name);
			return nullptr;
		}

		auto object = new T{};
		register_script_object(object, type);
		return object;
	}

	uint32_t get_free(bool skipError = false)
	{
		T* object = create(skipError);
		if (!object) return 0;
		return object->id;
	}

	T* check(uint32_t id, bool skipError = false)
	{
		if (util::contains(script_object_ids_by_type[type], id))
		{
			auto& t = script_objects.at(id);
			return static_cast<T*>(t.get());
		}

		if (skipError) return NULL;

		scripting_log_error_with_context("Invalid {} using UID = {}", name, id);
		return NULL;
	}
};

#endif
