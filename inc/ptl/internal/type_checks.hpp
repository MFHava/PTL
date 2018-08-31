
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl::internal {
	template<typename Type>
	class is_abi_compatible final {
		using type = typename std::remove_cv_t<Type>;
		enum {
			is_standard_layout            = std::is_standard_layout_v<type>,
			is_default_constructible      = std::is_default_constructible_v<type>,
			is_copy_constructible         = std::is_copy_constructible_v<type>,
			is_nothrow_move_constructible = std::is_nothrow_move_constructible_v<type>,
			is_copy_assignable            = std::is_copy_assignable_v<type>,
			is_nothrow_move_assignable    = std::is_nothrow_move_assignable_v<type>,
			is_nothrow_destructible       = std::is_nothrow_destructible_v<type>,
			is_nothrow_swappable          = std::is_nothrow_swappable_v<type>
		};

	public:
		enum {
			value = is_standard_layout            &&
			        is_default_constructible      &&
			        is_copy_constructible         &&
			        is_nothrow_move_constructible &&
			        is_copy_assignable            &&
			        is_nothrow_move_assignable    &&
			        is_nothrow_destructible       &&
			        is_nothrow_swappable
		};
	};

	template<typename Type>
	inline
	constexpr
	bool is_abi_compatible_v = is_abi_compatible<Type>::value;

	template<typename...>
	struct are_abi_compatible;

	template<typename... Types>
	inline
	constexpr
	bool are_abi_compatible_v = are_abi_compatible<Types...>::value;

	template<typename Type, typename... Types>
	struct are_abi_compatible<Type, Types...> final : std::bool_constant<is_abi_compatible_v<Type> && are_abi_compatible_v<Types...>> {};

	template<>
	struct are_abi_compatible<> final : std::bool_constant<true> {};
}
