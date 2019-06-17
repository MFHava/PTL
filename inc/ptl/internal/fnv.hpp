
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ptl::internal {
	template<std::size_t>
	struct fnv_offset_basis;

	template<std::size_t Size>
	inline
	constexpr
	auto fnv_offset_basis_v{fnv_offset_basis<Size>::value};


	template<>
	struct fnv_offset_basis<32> final : std::integral_constant<std::uint32_t, 2166136261U> {};

	template<>
	struct fnv_offset_basis<64> final : std::integral_constant<std::uint64_t, 14695981039346656037ULL> {};


	template<std::size_t>
	struct fnv_prime;

	template<std::size_t Size>
	inline
	constexpr
	auto fnv_prime_v{fnv_prime<Size>::value};

	template<>
	struct fnv_prime<32> final : std::integral_constant<std::uint32_t, 16777619U> {};

	template<>
	struct fnv_prime<64> final : std::integral_constant<std::uint64_t, 1099511628211ULL> {};


	template<std::size_t Size>
	constexpr
	auto fnv1a(const std::uint8_t (& values)[Size]) noexcept -> std::size_t {
		constexpr auto bitness{8 * sizeof(std::size_t)};
		std::size_t hash{fnv_offset_basis_v<bitness>};
		for(const auto & value : values) {
			hash ^= value;
			hash *= fnv_prime_v<bitness>;
		}
		return hash;
	}
}
