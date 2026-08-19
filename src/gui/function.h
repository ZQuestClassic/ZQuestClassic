#ifndef ZC_GUI_FUNCTION_H_
#define ZC_GUI_FUNCTION_H_

#include <cstddef>
#include <type_traits>
#include <utility>

namespace GUI
{

/*
 * A minimal move-only, owning, type-erased callable, used to store widget
 * callbacks.
 *
 * This exists purely to cut compile time. The GUI builder DSL constructs a
 * type-erased function from a unique lambda at nearly every widget call site,
 * and the dialog files contain hundreds of them. libc++'s std::function is
 * very expensive to instantiate per callable type (it pulls in __value_func,
 * __func, __alloc_func, allocator rebinding and __compressed_pair). This type
 * erases a callable with just two captureless thunks and a heap allocation,
 * which the compiler instantiates far more cheaply.
 *
 * Widget callbacks are set once and only ever invoked, so move-only ownership
 * with no small-buffer optimization is all that's needed.
 */
template<typename Signature>
class function;

template<typename R, typename... Args>
class function<R(Args...)>
{
public:
	function() noexcept = default;
	function(std::nullptr_t) noexcept {}

	template<typename F,
		typename = std::enable_if_t<
			!std::is_same_v<std::decay_t<F>, function> &&
			std::is_invocable_r_v<R, std::decay_t<F>&, Args...>>>
	function(F&& f)
		: obj(new std::decay_t<F>(std::forward<F>(f))),
		  invoke([](void* p, Args... args) -> R
		  {
			  return (*static_cast<std::decay_t<F>*>(p))(static_cast<Args&&>(args)...);
		  }),
		  destroy([](void* p) { delete static_cast<std::decay_t<F>*>(p); })
	{}

	function(function&& other) noexcept
		: obj(other.obj), invoke(other.invoke), destroy(other.destroy)
	{
		other.obj = nullptr;
		other.invoke = nullptr;
		other.destroy = nullptr;
	}

	function& operator=(function&& other) noexcept
	{
		if (this != &other)
		{
			if (destroy)
				destroy(obj);
			obj = other.obj;
			invoke = other.invoke;
			destroy = other.destroy;
			other.obj = nullptr;
			other.invoke = nullptr;
			other.destroy = nullptr;
		}
		return *this;
	}

	function(const function&) = delete;
	function& operator=(const function&) = delete;

	~function()
	{
		if (destroy)
			destroy(obj);
	}

	explicit operator bool() const noexcept { return invoke != nullptr; }

	R operator()(Args... args) const
	{
		return invoke(obj, static_cast<Args&&>(args)...);
	}

private:
	void* obj = nullptr;
	R (*invoke)(void*, Args...) = nullptr;
	void (*destroy)(void*) = nullptr;
};

} // namespace GUI

#endif
