
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ptl::internal {
	template<std::size_t Size>
	constexpr
	auto fnv1a(const std::uint8_t (& values)[Size]) noexcept -> std::size_t {
		static_assert(CHAR_BIT == 8);
		constexpr auto bitness{8 * sizeof(std::size_t)};
		static_assert(bitness == 32 || bitness == 64);

		constexpr auto offset_basis{[&] {
			constexpr auto bitness{8 * sizeof(std::size_t)};
			if constexpr(bitness == 32) return std::uint32_t{2166136261U};
			else return std::uint64_t{14695981039346656037ULL};
		}()};

		constexpr auto prime{[&] {
			constexpr auto bitness{8 * sizeof(std::size_t)};
			if constexpr(bitness == 32) return std::uint32_t{16777619U};
			else return std::uint64_t{1099511628211ULL};
		}()};

		std::size_t hash{offset_basis};
		for(const auto & value : values) {
			hash ^= value;
			hash *= prime;
		}
		return hash;
	}
}
