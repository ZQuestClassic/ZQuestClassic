#ifndef ZC_SCRIPTING_ARRAY_MANAGER_H_
#define ZC_SCRIPTING_ARRAY_MANAGER_H_

#include "base/zc_array.h"
#include "user_object.h"
#include <cstdint>
#include <functional>
#include <string>

class ArrayManager
{
public:
	ArrayManager(int32_t ptr, bool neg);
	ArrayManager(int32_t ptr);
	
	int32_t get(int32_t indx) const;
	void set(int32_t indx, int32_t val);
	int32_t size() const;
	
	bool resize(size_t newsize);
	bool resize_min(size_t minsz);
	bool can_resize();
	bool push(int32_t val, int indx = -1);
	int32_t pop(int indx = -1);
	
	bool invalid() const {return _invalid;}
	bool internal() const {return !_invalid && !aptr;}

	std::string asString(std::function<char const*(int32_t)> formatter, const size_t& limit) const;
	void log_invalid_operation() const;
	
	bool negAccess;
private:
	ZScriptArray* aptr;
	int32_t legacy_internal_id;
	script_array::internal_array_id internal_array_id;
	bool _invalid;
};

#endif
