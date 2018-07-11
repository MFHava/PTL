
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>

namespace ptl {
	namespace internal {
		template<typename Type>
		constexpr
		void adl_swap(Type & lhs, Type & rhs) noexcept {
			using std::swap;
			swap(lhs, rhs);
		}
	}
}

