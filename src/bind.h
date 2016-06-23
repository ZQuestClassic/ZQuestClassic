
#include "config.h"

// For std::bind, std::function.

#if ZC_HAS_CPP11
	#include <functional>
	#define zc_function		std::function
	#define zc_bind			std::bind
#else
	//For C++03 we need to use the boost library for some c++11/14 features.
	#define BOOST_FUNCTION_MAX_ARGS 4
	#include <boost/function.hpp>
	#include <boost/bind.hpp>

	#define zc_function		boost::function
	#define zc_bind			boost::bind
#endif


