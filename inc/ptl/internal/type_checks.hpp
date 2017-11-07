
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl {
	namespace internal {
		template<typename Type>
		class is_abi_compatible final {
			using type = typename std::remove_cv_t<Type>;
			enum {
				is_standard_layout            = std::is_standard_layout<type>::value,
				is_default_constructible      = std::is_default_constructible<type>::value,
				is_copy_constructible         = std::is_copy_constructible<type>::value,
				is_nothrow_move_constructible = std::is_nothrow_move_constructible<type>::value,
				is_copy_assignable            = std::is_copy_assignable<type>::value,
				is_nothrow_move_assignable    = std::is_nothrow_move_assignable<type>::value,
				is_nothrow_destructible       = std::is_nothrow_destructible<type>::value,
			};

		public:
			enum {
				value = is_standard_layout            &&
				        is_default_constructible      &&
				        is_copy_constructible         &&
				        is_nothrow_move_constructible &&
				        is_copy_assignable            &&
				        is_nothrow_move_assignable    &&
				        is_nothrow_destructible
			};
		};

		template<typename...>
		struct are_abi_compatible;

		template<typename Type, typename... Types>
		struct are_abi_compatible<Type, Types...> final {
			enum { value = is_abi_compatible<Type>::value && are_abi_compatible<Types...>::value };
		};

		template<>
		struct are_abi_compatible<> final {
			enum { value = true };
		};
	}
}