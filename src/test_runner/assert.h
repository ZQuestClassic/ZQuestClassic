#ifndef TEST_RUNNER_ASSERT_H_
#define TEST_RUNNER_ASSERT_H_

#include <cstddef>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "fmt/core.h"
#include "fmt/ranges.h"

// -----------------------------------------------------------------------------
// Assert True
// -----------------------------------------------------------------------------

inline void _assertTrueImpl(bool value, const char* expressionText, const char* file, int line)
{
	if (!value)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expression: {}", file, line, expressionText));
}

#define assertTrue(expr) _assertTrueImpl((expr), #expr, __FILE__, __LINE__)

// -----------------------------------------------------------------------------
// Assert Throws
// -----------------------------------------------------------------------------

inline void _failAssertThrows(const char* expressionText, const char* file, int line)
{
	throw std::runtime_error(
		fmt::format("Expected exception at {}:{}\n  But code did NOT throw: {}", file, line, expressionText));
}

#define assertThrows(expr) \
	do \
	{ \
		bool _caught = false; \
		try \
		{ \
			expr; \
		} \
		catch (...) \
		{ \
			_caught = true; \
		} \
		if (!_caught) \
			_failAssertThrows(#expr, __FILE__, __LINE__); \
	} while (false)

// -----------------------------------------------------------------------------
// Assert Size
// -----------------------------------------------------------------------------

template <typename T>
inline void _assertSizeImpl(const T& v, size_t expected, const char* exprText, const char* file, int line)
{
	if (v.size() != expected)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expression: {}.size()\n  Expected: {}\n  Got: {}", file, line, exprText, expected, v.size()));
}

#define assertSize(v, expected) _assertSizeImpl((v), (expected), #v, __FILE__, __LINE__)

// -----------------------------------------------------------------------------
// Assert Equal
// -----------------------------------------------------------------------------

template<typename T>
concept IsEnum = std::is_enum_v<T>;

template <typename T> requires (!IsEnum<T>)
inline void _assertEqualImpl(const T& v, const T& expected, const char* vText, const char* expText, const char* file, int line)
{
	if (v != expected)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expected: {} ({})\n  But got: {} ({})", file, line, expected, expText, v, vText));
}

template<IsEnum T>
inline void _assertEqualImpl(T v, T expected, const char* vText, const char* expText, const char* file, int line)
{
	if (v != expected)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expected: {} ({})\n  But got: {} ({})", file, line, (int)expected, expText, (int)v, vText));
}

inline void _assertEqualImpl(const std::vector<std::string>& v, const std::vector<std::string>& expected, const char* vText, const char* expText, const char* file, int line)
{
	if (v != expected)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expected: {} ({})\n  But got: {} ({})", file, line, fmt::join(expected, ", "), expText, fmt::join(v, ", "), vText));
}

#define assertEqual(v, expected) _assertEqualImpl((v), (expected), #v, #expected, __FILE__, __LINE__)

// -----------------------------------------------------------------------------
// Assert Not Equal
// -----------------------------------------------------------------------------

template <typename T>
inline void _assertNotEqualImpl(const T& v, const T& expected, const char* vText, const char* expText, const char* file, int line)
{
	if (v == expected)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expected: {} ({})\n  To NOT equal: {} ({})", file, line, v, vText, expected, expText));
}

#define assertNotEqual(v, expected) _assertNotEqualImpl((v), (expected), #v, #expected, __FILE__, __LINE__)

// -----------------------------------------------------------------------------
// Assert Greater Than
// -----------------------------------------------------------------------------

template <typename T>
inline void _assertGreaterThanImpl(const T& v, const T& expected, const char* vText, const char* expText, const char* file, int line)
{
	if (v <= expected)
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  Expected: {} ({})\n  To be greater than: {} ({})", file, line, v, vText, expected, expText));
}

#define assertGreaterThan(v, expected) _assertGreaterThanImpl((v), (expected), #v, #expected, __FILE__, __LINE__)

// -----------------------------------------------------------------------------
// Assert Some
// -----------------------------------------------------------------------------

template<typename Container, typename Predicate>
inline void _assertSomeImpl(const Container& v, Predicate predicate, const char* vText, const char* predText, const char* file, int line)
{
	if (!std::any_of(v.begin(), v.end(), predicate))
		throw std::runtime_error(
			fmt::format("Assertion failed at {}:{}\n  No elements in {} matched predicate {}", file, line, vText, predText));
}

#define assertSome(v, pred) _assertSomeImpl((v), (pred), #v, #pred, __FILE__, __LINE__)

#endif
