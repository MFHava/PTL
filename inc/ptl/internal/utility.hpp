
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
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
	}
}