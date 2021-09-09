#ifndef ZC_GUI_DIALOGMESSAGE_H
#define ZC_GUI_DIALOGMESSAGE_H

#include <functional>
#include <string_view>
#include <utility>
#include <variant>

namespace gui
{

class MessageArg
{
public:
	/* Use this when a message has no argument. */
	static constexpr auto none=std::monostate();

	constexpr MessageArg(): value(std::monostate())
	{}

	constexpr MessageArg(const MessageArg& other)=default;

	constexpr MessageArg(MessageArg&& other)=default;

	// You would think a template constructor would work, but apparently not.
	constexpr MessageArg(std::monostate): value(none)
	{}

	constexpr MessageArg(bool value): value(value)
	{}

	constexpr MessageArg(int value): value(value)
	{}

	constexpr MessageArg(std::string_view value): value(value)
	{}

	/* Returns true if the argument is the specified type. */
	template<typename T>
	inline bool is() const
	{
		return std::holds_alternative<T>(value);
	}

	inline operator bool() const
	{
		return std::get<bool>(value);
	}

	inline operator int() const
	{
		return std::get<int>(value);
	}

	inline operator std::string_view() const
	{
		return std::get<std::string_view>(value);
	}

private:
	std::variant<
		std::monostate,
		bool,
		int,
		std::string_view
	> value;
};

using MessageDispatcher=std::function<void(int, MessageArg)>;

}

#endif
