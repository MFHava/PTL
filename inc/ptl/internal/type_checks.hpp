
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl::internal {
	template<typename Type>
	inline
	constexpr
	auto is_abi_compatible_v{
		std::is_standard_layout_v<std::remove_cv_t<Type>> &&
		std::is_default_constructible_v<std::remove_cv_t<Type>> &&
		std::is_copy_constructible_v<std::remove_cv_t<Type>> &&
		std::is_nothrow_move_constructible_v<std::remove_cv_t<Type>> &&
		std::is_copy_assignable_v<std::remove_cv_t<Type>> &&
		std::is_nothrow_move_assignable_v<std::remove_cv_t<Type>> &&
		std::is_nothrow_destructible_v<std::remove_cv_t<Type>> &&
		std::is_nothrow_swappable_v<std::remove_cv_t<Type>>
	};
}
