#ifndef ZC_GUI_DIALOGMESSAGE_H_
#define ZC_GUI_DIALOGMESSAGE_H_

#include <functional>
#include <memory>
#include <string_view>
#include <utility>
#include <variant>

namespace GUI
{

class Widget;

class MessageArg
{
public:
	/* Use this when a message has no argument. */
	static constexpr auto none = std::monostate();

	inline constexpr MessageArg() noexcept: value(std::monostate())
	{}

	inline constexpr MessageArg(const MessageArg& other) noexcept=default;

	inline constexpr MessageArg(MessageArg&& other) noexcept=default;
	
	MessageArg& operator=(const MessageArg& other) = default;

	// You would think a template constructor would work, but apparently not.
	inline constexpr MessageArg(std::monostate) noexcept: value(none)
	{}

	inline constexpr MessageArg(bool value) noexcept: value(value)
	{}

	inline constexpr MessageArg(int32_t value) noexcept: value(value)
	{}

	inline constexpr MessageArg(std::string_view value) noexcept: value(value)
	{}

	/* Returns true if the argument is the specified type. */
	template<typename T>
	inline constexpr bool is() const
	{
		return std::holds_alternative<T>(value);
	}

	inline constexpr operator bool() const
	{
		return std::get<bool>(value);
	}

	inline constexpr operator int32_t() const
	{
		return std::get<int32_t>(value);
	}

	inline constexpr operator std::string_view() const
	{
		return std::get<std::string_view>(value);
	}

private:
	std::variant<
		std::monostate,
		bool,
		int32_t,
		std::string_view
	> value;
};

template<typename T>
struct DialogMessage
{
	T message;
	MessageArg argument;
	std::shared_ptr<Widget> sender;
};

class MessageDispatcher
{
public:
	MessageDispatcher(
		std::shared_ptr<Widget> sender,
		std::function<void(int32_t, MessageArg, std::shared_ptr<Widget>)> send):
			sender(sender),
			send(send)
	{}

	inline void operator()(int32_t msg, MessageArg arg)
	{
		send(msg, arg, sender);
	}

private:
	std::shared_ptr<Widget> sender;
	std::function<void(int32_t, MessageArg, std::shared_ptr<Widget>)> send;
};

}

#endif
