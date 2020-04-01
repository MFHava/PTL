
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

//emulation of C++20 features
namespace ptl::internal {
	template<typename T>
	struct identity_type final {
		using type = T;
	};

	template<typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
}
