
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace ptl {
	namespace internal {
		struct nullopt_t final {
			explicit
			constexpr
			nullopt_t(int) noexcept {}
		};

		struct in_place_t final {
			explicit
			in_place_t() =default;
		};
	}
}
