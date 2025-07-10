#ifndef ZC_ARRAY_H_
#define ZC_ARRAY_H_

#include "base/general.h"
#include <cstdint>
#include <vector>

class ZScriptArray
{
public:
    bool operator == (const ZScriptArray &_Array) const = default;

    int32_t&		 operator [](const size_t& i)
    {
        return _data[i];
    }
    const int32_t& operator [](const size_t& i) const
    {
        return _data[i];
    }

	std::vector<int32_t>& getData()
	{
		return _data;
	}

    size_t Size() const
    {
        return _data.size();
    }
    bool Empty() const
    {
        return _data.empty();
    }
	bool Valid() const
	{
		return _valid;
	}
	void setValid(bool v)
	{
		_valid = v;
	}
	void setObjectType(script_object_type v)
	{
		_object_type = v;
	}
	script_object_type ObjectType() const
	{
		return _object_type;
	}
	bool HoldsObjects() const
	{
		return _object_type != script_object_type::none;
	}

    void Resize(const size_t size)
    {
		_data.resize(size);
    }

	void Push(int32_t val, int indx = -1)
	{
		if (indx < 0 || indx > _data.size())
			indx = _data.size();
		auto it = _data.begin() + indx;
    	_data.insert(it, val);
	}

	int32_t Pop(int indx = -1)
	{
		if (indx < 0 || indx >= _data.size())
			indx = _data.size() - 1;
		auto it = _data.begin();
		std::advance(it, indx);
		int32_t ret = *it;
		_data.erase(it);
		return ret;
	}

    void Copy(const ZScriptArray &_Array)
    {
		*this = _Array;
    }

    void Clear()
    {
		*this = {};
    }

private:
	std::vector<int32_t> _data;
	bool _valid;
	script_object_type _object_type;
};

#endif
