
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "compiler_detection.hpp"
#include <utility>

#define PTL_REQUIRES(args) ((void)0)

namespace ptl {
	namespace internal {
		template<typename Type>
		constexpr
		void swap(Type & lhs, Type & rhs) noexcept {
			auto tmp{std::move(lhs)};
			lhs = std::move(rhs);
			rhs = std::move(tmp);
		}

#if PTL_TARGET_BITNESS_IS_32
		constexpr std::size_t FNV_offset_basis{2166136261UL},            FNV_prime{16777619UL};
#elif PTL_TARGET_BITNESS_IS_64
		constexpr std::size_t FNV_offset_basis{14695981039346656037ULL}, FNV_prime{1099511628211ULL};
#endif

		inline
		constexpr
		auto fnv_1a(const char * ptr, std::size_t count) {
			std::size_t result{FNV_offset_basis};
			for(std::size_t i{0}; i < count; ++i) {
				result ^= static_cast<std::size_t>(ptr[i]);
				result *= FNV_prime;
			}
			return result;
		}
	}
}