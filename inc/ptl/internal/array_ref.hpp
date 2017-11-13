
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <initializer_list>

namespace ptl {
	namespace internal {//C++17 feature emulation
		template<typename Container>
		constexpr
		auto size(const Container & c) -> std::size_t { return c.size(); }

		template<typename Type, std::size_t Size>
		constexpr
		auto size(const Type(&array)[Size]) noexcept { return Size; }

		template<typename Container>
		constexpr
		auto data(      Container & c) { return c.data(); }

		template<typename Container>
		constexpr
		auto data(const Container & c) { return c.data(); }

		template<typename Type, std::size_t Size>
		constexpr
		auto data(Type(&array)[Size]) noexcept { return array; }

		template<typename Type>
		constexpr
		auto data(std::initializer_list<Type> ilist) noexcept { return ilist.begin(); }
	}
}