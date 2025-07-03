#ifndef ZC_SCRIPTING_ARRAYS_H_
#define ZC_SCRIPTING_ARRAYS_H_

// See arrays.cpp for how to define scripting arrays.

#include "base/combo.h"
#include "base/general.h"
#include "new_subscr.h"
#include "zc/ffscript.h"
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

int zasm_array_size(int zasm_var);
int zasm_array_get(int zasm_var, int ref, int index);
bool zasm_array_set(int zasm_var, int ref, int index, int value);
bool zasm_array_supports(int zasm_var);

template <typename T, size_t N>
static constexpr size_t comptime_array_size(T (&)[N])
{
	return N;
}

template <auto Fp>
struct field_type;

template<typename R, typename T, R (T::*FP)>
struct field_type<FP>
{
    using type = R;
};

template<typename R, typename T, typename...Args, R (T::*FP)(Args...)>
struct field_type<FP>
{
    using type = R;
}; 

template<auto FP>
using field_type_t = typename field_type<FP>::type;

namespace transforms
{
	template<int min_inclusive, int max_exclusive>
	static std::optional<int> validate(int value)
	{
		if (value < min_inclusive || value >= max_exclusive)
		{
			scripting_log_error_with_context("Attempted to set invalid value for array: {} - must be >= {} and < {}", value, min_inclusive, max_exclusive);
			return std::nullopt;
		}

		return value;
	}

	template<int max_exclusive>
	static std::optional<int> validate(int value)
	{
		if (value < 0 || value >= max_exclusive)
		{
			scripting_log_error_with_context("Attempted to set invalid value for array: {} - must be >= {} and < {}", value, 0, max_exclusive);
			return std::nullopt;
		}

		return value;
	}

	template<int low, int high>
	static std::optional<int> vbound(int value)
	{
		return ::vbound(value, low, high);
	}

	static int vboundByte(int value)
	{
		return ::vbound(value, 0, 255);
	}

	static int vboundWord(int value)
	{
		return ::vbound(value, 0, 65535);
	}

	static int vboundInt(int value)
	{
		return ::vbound(value, -214747, 214747);
	}
}

// Helper struct to make the static_assert dependent on the template parameter
template<typename>
struct always_false : std::false_type {};

template<typename T>
static T* resolveScriptingObject(int ref)
{
	if constexpr (std::is_same<T, user_genscript>::value)
		return checkGenericScr(ref);
	else if constexpr (std::is_same<T, sprite>::value)
		return ResolveBaseSprite(ref);
	else if constexpr (std::is_same<T, ffcdata>::value)
		return checkFFC(ref);
	else if constexpr (std::is_same<T, enemy>::value)
		return checkNPC(ref);
	else if constexpr (std::is_same<T, guydata>::value)
		return checkNPCData(ref);
	else if constexpr (std::is_same<T, item>::value)
		return checkItem(ref);
	else if constexpr (std::is_same<T, itemdata>::value)
		return checkItemData(ref);
	else if constexpr (std::is_same<T, newcombo>::value)
		return checkCombo(ref);
	else if constexpr (std::is_same<T, combo_trigger>::value)
		return checkComboTrigger(ref);
	else if constexpr (std::is_same<T, weapon>::value)
		return checkWpn(ref);
	else if constexpr (std::is_same<T, dmap>::value)
		return checkDmap(ref);
	else if constexpr (std::is_same<T, mapdata>::value)
		return checkMapData(ref);
	else if constexpr (std::is_same<T, mapscr>::value)
		return checkMapDataScr(ref);
	else if constexpr (std::is_same<T, user_paldata>::value)
		return checkPalData(ref);
	else if constexpr (std::is_same<T, screendata>::value)
		return checkScreen(ref);
	else if constexpr (std::is_same<T, ZCSubscreen>::value)
		return checkSubData(ref);
	else if constexpr (std::is_same<T, ZCSubscreenActive>::value)
		return (ZCSubscreenActive*)checkSubData(ref, sstACTIVE);
	else if constexpr (std::is_same<T, SubscrWidget>::value)
		return checkSubWidg(ref);
	else if constexpr (std::is_same<T, SubscrWidgetActive>::value)
		return (SubscrWidgetActive*)checkSubWidg(ref, sstACTIVE);
	else if constexpr (std::is_same<T, bottletype>::value)
		return checkBottleData(ref);
	else if constexpr (std::is_same<T, bottleshoptype>::value)
		return checkBottleShopData(ref);
	else if constexpr (std::is_same<T, item_drop_object>::value)
		return checkDropSetData(ref);
	else if constexpr (std::is_same<T, wpndata>::value)
		return checkSpriteData(ref);
	else if constexpr (std::is_same<T, MsgStr>::value)
		return checkMessageData(ref);
	else if constexpr (std::is_same<T, gamedata>::value)
		return game;
	else if constexpr (std::is_same<T, HeroClass>::value)
		return &Hero;
	else
		static_assert(always_false<T>::value, "Unhandled type in 'resolveScriptingObject' function");
}

class IScriptingArray
{
public:
	virtual ~IScriptingArray() = default;
	virtual size_t getSize(int ref) const = 0;
	virtual int getElement(int ref, int index) const = 0;
	virtual bool setElement(int ref, int index, int value) = 0;

	void setDefaultValue(int defaultValue) { m_defaultValue = defaultValue; }
	int getDefaultValue() const { return m_defaultValue; }

	void setMul10000(bool v) { m_mul10000 = v; }

	void setReadOnly() { m_readOnly = true; }
	bool readOnly() { return m_readOnly; }

	void boundIndex() { m_boundGetterIndex = m_boundSetterIndex = true; }
	void boundGetterIndex() { m_boundGetterIndex = true; }
	void boundSetterIndex() { m_boundSetterIndex = true; }

	void setValueTransform(std::function<std::optional<int>(int)> valueTransform) { m_valueTransform = std::move(valueTransform); }

protected:
	int m_defaultValue;
	bool m_mul10000 = true;
	bool m_readOnly;
	bool m_boundGetterIndex;
	bool m_boundSetterIndex;
	std::function<std::optional<int>(int)> m_valueTransform;

	template <typename T_Element>
	std::optional<int> transformValue(int value) const
	{
		if constexpr (!std::is_same<T_Element, bool>::value)
			value = m_mul10000 ? value / 10000 : value;
		return m_valueTransform ? m_valueTransform(value) : value;
	}
};

template <typename T>
static int convert_value(int value, bool mul10000)
{
	if constexpr (std::is_same<T, bool>::value)
		return value;
	else
		return mul10000 ? value / 10000 : value;
}

static int bound_index(int index, int low, int high)
{
	if (index >= low && index <= high)
		return index;

	scripting_log_error_with_context("Using invalid index {}, bounding to valid range between {} and {}", index, low, high);
	return index < low ? low : high;
}

template<typename T_Element>
class ScriptingArray_GlobalCArray : public IScriptingArray {
public:
	ScriptingArray_GlobalCArray(T_Element* data, size_t size) : m_data(data), m_size(size) {}

	size_t getSize(int) const override { return m_size; }

	int getElement(int, int index) const override
	{
		if (m_boundGetterIndex)
			index = bound_index(index, 0, m_size - 1);
		else if (BC::checkIndex2(index, m_size) != SH::_NoError)
			return m_defaultValue;

		return m_data[index] * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int, int index, int value) override
	{
		if (m_boundSetterIndex)
			index = bound_index(index, 0, m_size - 1);
		else if (BC::checkIndex2(index, m_size) != SH::_NoError)
			return false;

		if (auto val = transformValue<T_Element>(value))
		{
			m_data[index] = val.value();

			if (m_writeSideEffect)
				m_writeSideEffect(index);
			return true;
		}

		return false;
	}

	void setSideEffect(std::function<void(int)> writeSideEffect) { m_writeSideEffect = std::move(writeSideEffect); }

private:
	T_Element* m_data;
	const size_t m_size;
	std::function<void(int)> m_writeSideEffect;
};

template<typename T_Element>
class ScriptingArray_GlobalComputed : public IScriptingArray
{
public:
	using SizeFunc = std::function<size_t(int ref)>;
	using GetFunc = std::function<T_Element(int ref, int index)>;
	using SetFunc = std::function<bool(int ref, int index, T_Element value)>;

	template<typename T_GetFunc, typename T_SetFunc>
	ScriptingArray_GlobalComputed(SizeFunc sf, T_GetFunc gf, T_SetFunc setf)
		: m_sizeFunc(std::move(sf)), m_getFunc(std::move(gf)), m_setFunc(std::move(setf)), m_skipIndexCheck(false)
	{
		// Define the exact function pointer type we require.
		using RequiredGetFun = T_Element(*)(int, int);
		using RequiredSetFun = bool(*)(int, int, T_Element);

		// Perform a compile-time check to ensure the provided lambda's signature matches.
		// This check will fail if the lambda's parameters do not match exactly.
		static_assert(
			std::is_convertible_v<T_GetFunc, RequiredGetFun>,
			"The provided lambda's signature does not match the required 'T_Element(int, int)'. "
			"Ensure the value parameter's type is exactly T_Element."
		);
		static_assert(
			std::is_convertible_v<T_SetFunc, RequiredSetFun>,
			"The provided lambda's signature does not match the required 'bool(int, int, T_Element)'. "
			"Ensure the value parameter's type is exactly T_Element."
		);
	}

	size_t getSize(int ref) const override { return m_sizeFunc(ref); }

	int getElement(int ref, int index) const override
	{
		if (!m_skipIndexCheck)
		{
			size_t sz = getSize(ref);
			if (m_boundGetterIndex)
				index = bound_index(index, 0, sz - 1);
			else if (BC::checkIndex2(index, sz) != SH::_NoError)
				return m_defaultValue;
		}

		return m_getFunc(ref, index) * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		if (!m_skipIndexCheck)
		{
			size_t sz = getSize(ref);
			if (m_boundSetterIndex)
				index = bound_index(index, 0, sz - 1);
			else if (BC::checkIndex2(index, sz) != SH::_NoError)
				return false;
		}

		if (auto val = transformValue<T_Element>(value))
			return m_setFunc(ref, index, val.value());

		return false;
	}

	void skipIndexCheck() { m_skipIndexCheck = true; }

private:
	SizeFunc m_sizeFunc;
	GetFunc m_getFunc;
	SetFunc m_setFunc;
	bool m_skipIndexCheck;
};

// Never use this for new ZASM.
class ScriptingArray_GlobalComputedOneIndexed : public IScriptingArray
{
public:
	using SizeFunc = std::function<size_t(int ref)>;
	using GetFunc = std::function<int(int ref, int index)>;
	using SetFunc = std::function<bool(int ref, int index, int value)>;

	ScriptingArray_GlobalComputedOneIndexed(SizeFunc sf, GetFunc gf, SetFunc setf)
		: m_sizeFunc(std::move(sf)), m_getFunc(std::move(gf)), m_setFunc(std::move(setf)) {}

	size_t getSize(int ref) const override { return m_sizeFunc(ref); }

	int getElement(int ref, int index) const override
	{
		size_t sz = getSize(ref);
		if (m_boundGetterIndex)
			index = bound_index(index, 1, sz - 1);
		else if (BC::checkIndex2(index, sz) != SH::_NoError)
			return m_defaultValue;

		return m_getFunc(ref, index) * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		size_t sz = getSize(ref);
		if (m_boundSetterIndex)
			index = bound_index(index, 1, sz - 1);
		else if (BC::checkIndex2(index, sz) != SH::_NoError)
			return false;

		if (auto val = transformValue<int>(value))
			return m_setFunc(ref, index, val.value());

		return false;
	}

private:
	SizeFunc m_sizeFunc;
	GetFunc m_getFunc;
	SetFunc m_setFunc;
};

template<typename T_Object, auto T_MemberPtr>
class ScriptingArray_ObjectMemberCArray : public IScriptingArray
{
public:
	using MemberType = field_type<T_MemberPtr>::type;
	using ElementType = std::remove_extent<MemberType>::type;
	using N = std::extent<MemberType>;

	static_assert(std::is_array<MemberType>(), "Can only use ScriptingArray_ObjectMemberCArray for array types");

	size_t getSize(int) const override { return N::value; }

	int getElement(int ref, int index) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return 0;

		if (m_boundGetterIndex)
			index = bound_index(index, 0, N::value - 1);
		else if (BC::checkIndex2(index, N::value) != SH::_NoError)
			return m_defaultValue;

		return (obj->*T_MemberPtr)[index] * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return false;

		if (m_boundSetterIndex)
			index = bound_index(index, 0, N::value - 1);
		else if (BC::checkIndex2(index, N::value) != SH::_NoError)
			return false;

		if (auto val = transformValue<ElementType>(value))
		{
			(obj->*T_MemberPtr)[index] = val.value();

			if (m_writeSideEffect)
				m_writeSideEffect(obj, index, val.value());
			return true;
		}

		return false;
	}

	void setSideEffect(std::function<void(T_Object*, int, ElementType)> writeSideEffect) { m_writeSideEffect = std::move(writeSideEffect); }

private:
	std::function<void(T_Object*, int, ElementType)> m_writeSideEffect;
};

template<typename T_Object, typename T_Element>
class ScriptingArray_ObjectComputed : public IScriptingArray
{
public:
	using SizeFunc = std::function<size_t(T_Object* obj)>;
	using GetFunc = std::function<T_Element(T_Object* obj, int index)>;
	using SetFunc = std::function<void(T_Object* obj, int index, T_Element value)>;

	template<typename T_GetFunc, typename T_SetFunc>
	ScriptingArray_ObjectComputed(SizeFunc sf, T_GetFunc gf, T_SetFunc setf)
		: m_sizeFunc(std::move(sf)), m_getFunc(std::move(gf)), m_setFunc(std::move(setf)), m_skipIndexCheck(false)
	{
		// Define the exact function pointer type we require.
		using RequiredGetFun = T_Element(*)(T_Object*, int);
		using RequiredSetFun = void(*)(T_Object*, int, T_Element);

		// Perform a compile-time check to ensure the provided lambda's signature matches.
		// This check will fail if the lambda's parameters do not match exactly.
		static_assert(
			std::is_convertible_v<T_GetFunc, RequiredGetFun>,
			"The provided lambda's signature does not match the required 'T_Element(T_Object*, int)'. "
			"Ensure the value parameter's type is exactly T_Element."
		);
		static_assert(
			std::is_convertible_v<T_SetFunc, RequiredSetFun>,
			"The provided lambda's signature does not match the required 'void(T_Object*, int, T_Element)'. "
			"Ensure the value parameter's type is exactly T_Element."
		);
	}

	size_t getSize(int ref) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return 0;

		return m_sizeFunc(obj);
	}

	int getElement(int ref, int index) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return m_defaultValue;

		if (!m_skipIndexCheck)
		{
			size_t sz = m_sizeFunc(obj);
			if (m_boundGetterIndex)
				index = bound_index(index, 0, sz - 1);
			else if (BC::checkIndex2(index, sz) != SH::_NoError)
				return m_defaultValue;
		}

		T_Element result = m_getFunc(obj, index);
		
		if constexpr (std::is_same<T_Element, bool>::value)
			return result ? (m_mul10000 ? 10000 : 1) : 0;
		else
			return result * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return false;

		if (!m_skipIndexCheck)
		{
			size_t sz = m_sizeFunc(obj);
			if (m_boundSetterIndex)
				index = bound_index(index, 0, sz - 1);
			else if (BC::checkIndex2(index, sz) != SH::_NoError)
				return false;
		}

		if (auto val = transformValue<T_Element>(value))
		{
			m_setFunc(obj, index, val.value());
			return true;
		}

		return false;
	}

	void skipIndexCheck() { m_skipIndexCheck = true; }

private:
	SizeFunc m_sizeFunc;
	GetFunc m_getFunc;
	SetFunc m_setFunc;
	bool m_skipIndexCheck;
};

template<typename T_Object, auto T_MemberPtr, size_t N>
class ScriptingArray_ObjectMemberBitwiseFlags : public IScriptingArray
{
public:
	using MemberType = field_type<T_MemberPtr>::type;

	size_t getSize(int) const override { return N; }

	int getElement(int ref, int index) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return 0;

		if (m_boundGetterIndex)
			index = bound_index(index, 0, N - 1);
		else if (BC::checkIndex2(index, N) != SH::_NoError)
			return m_defaultValue;

		return ((obj->*T_MemberPtr >> index) & 1 ? 1 : 0) * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return false;

		if (m_boundSetterIndex)
			index = bound_index(index, 0, N - 1);
		else if (BC::checkIndex2(index, N) != SH::_NoError)
			return false;

		if (auto val = transformValue<bool>(value))
		{
			auto flag = (MemberType)(1<<index);
			SETFLAG(obj->*T_MemberPtr, flag, val.value());

			if (m_writeSideEffect)
				m_writeSideEffect(obj, index, val.value());
			return true;
		}

		return false;
	}

	void setSideEffect(std::function<void(T_Object*, int, int)> writeSideEffect) { m_writeSideEffect = std::move(writeSideEffect); }

private:
	std::function<void(T_Object*, int, int)> m_writeSideEffect;
};

template<typename T_Object, auto T_MemberPtr, size_t N>
class ScriptingArray_ObjectMemberBitstring : public IScriptingArray
{
public:
	using MemberType = field_type<T_MemberPtr>::type;

	size_t getSize(int) const override { return N; }

	int getElement(int ref, int index) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return 0;

		if (m_boundGetterIndex)
			index = bound_index(index, 0, N - 1);
		else if (BC::checkIndex2(index, N) != SH::_NoError)
			return m_defaultValue;

		return ((obj->*T_MemberPtr).get(index) ? 1 : 0) * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj)
			return false;

		if (m_boundSetterIndex)
			index = bound_index(index, 0, N - 1);
		else if (BC::checkIndex2(index, N) != SH::_NoError)
			return false;

		if (auto val = transformValue<bool>(value))
		{
			(obj->*T_MemberPtr).set(index, val.value());

			if (m_writeSideEffect)
				m_writeSideEffect(obj, index, val.value());
			return true;
		}

		return false;
	}

	void setSideEffect(std::function<void(T_Object*, int, int)> writeSideEffect) { m_writeSideEffect = std::move(writeSideEffect); }

private:
	std::function<void(T_Object*, int, int)> m_writeSideEffect;
};

template<typename T_Object, auto T_MemberPtr>
class ScriptingArray_ObjectMemberContainer : public IScriptingArray
{
public:
	using MemberType = field_type<T_MemberPtr>::type;
	using ElementType = MemberType::obj_type;

	size_t getSize(int ref) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj) return 0;

		return (obj->*T_MemberPtr).size();
	}

	int getElement(int ref, int index) const override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj) return m_defaultValue;

		const auto& container = obj->*T_MemberPtr;

		if (m_boundGetterIndex)
			index = bound_index(index, 0, container.size() - 1);
		else if (BC::checkIndex2(index, container.size()) != SH::_NoError)
			return m_defaultValue;

		return container[index] * (m_mul10000 ? 10000 : 1);
	}

	bool setElement(int ref, int index, int value) override
	{
		auto* obj = resolveScriptingObject<T_Object>(ref);
		if (!obj) return false;

		auto& container = (obj->*T_MemberPtr);

		if (m_boundSetterIndex)
			index = bound_index(index, 0, container.size() - 1);
		else if (BC::checkIndex2(index, container.size()) != SH::_NoError)
			return false;

		if (auto val = transformValue<ElementType>(value))
		{
			container[index] = val.value();
			return true;
		}
		return false;
	}
};

struct ArrayRegistrar
{
	ArrayRegistrar(int zasm_var, IScriptingArray* arrayImpl);
};

void zasm_array_register(int zasm_var, IScriptingArray* impl);

#endif
