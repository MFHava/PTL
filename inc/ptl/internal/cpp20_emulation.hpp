
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace ptl::internal {
	//emulating C++20 feature
	template<typename T>
	struct identity_type final {
		using type = T;
	};
}
