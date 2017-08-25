
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "compiler_detection.hpp"

#define PTL_OS_WINDOWS 0
#define PTL_OS_LINUX   0

#ifdef _WIN32
	#undef  PTL_OS_WINDOWS
	#define PTL_OS_WINDOWS 1
#elif __linux__
	#undef  PTL_OS_LINUX
	#define PTL_OS_LINUX 1
#else
	#error unknown operating system
#endif

#if PTL_COMPILER_IS_MSVC || (PTL_COMPILER_IS_Intel && PTL_OS_WINDOWS)
	#define PTL_PACK_BEGIN __pragma(pack(push, 1))
	#define PTL_PACK_END   __pragma(pack(pop))
#elif PTL_COMPILER_IS_GNU || (PTL_COMPILER_IS_Intel && PTL_OS_LINUX)
	#define PTL_PACK_BEGIN _Pragma("pack(push, 1)")
	#define PTL_PACK_END   _Pragma("pack(pop)")
#endif

#if PTL_COMPILER_CXX_RELAXED_CONSTEXPR
	#define PTL_RELAXED_CONSTEXPR constexpr
#else
	#define PTL_RELAXED_CONSTEXPR
#endif
