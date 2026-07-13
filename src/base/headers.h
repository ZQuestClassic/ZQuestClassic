#ifndef HEADERS_H_
#define HEADERS_H_

#include <optional>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include "base/ints.h"
#include "base/general.h"

using std::optional;
using std::nullopt;
using std::string;
using std::vector;
using std::shared_ptr;
using std::map;
using std::to_string;

template<typename T>
concept uint_type = std::unsigned_integral<T>;

template<typename T>
concept sint_type = std::signed_integral<T>;

template<typename T>
concept int_type = std::integral<T>;

#endif
