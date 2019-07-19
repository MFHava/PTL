
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#ifdef PTL_ENABLE_DEBUGGING
	#include <cstdio>
	#include <exception>

	#define PTL_REQUIRES_STRINGIFY(args) #args

	#define PTL_REQUIRES(args)\
		do {\
			if(!(args)) {\
				std::fprintf(stderr, "PRECONDITION VIOLATION DETECTED (%s:%d) \"" PTL_REQUIRES_STRINGIFY(args) "\"\n", __FILE__, __LINE__);\
				std::terminate();\
			}\
		} while(0)
#else
	#define PTL_REQUIRES(...) ((void)0)
#endif
