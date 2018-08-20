
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <type_traits>

namespace ptl::internal {
	template<typename Type, std::size_t Size>
	struct array_storage final { using type = Type[Size]; };

	template<typename Type>
	struct array_storage<Type, 0> final { using type = Type *; };

	template<typename Type, std::size_t Size>
	using array_storage_t = typename array_storage<Type, Size>::type;

	template<typename Type, typename... Args>
	struct are_convertible;

	template<typename Type, typename... Args>
	inline
	constexpr
	auto are_convertible_v{are_convertible<Type, Args...>::value};

	template<typename Type>
	struct are_convertible<Type> : std::true_type {};

	template<typename Type, typename Arg, typename... Args>
	struct are_convertible<Type, Arg, Args...> : std::bool_constant<
		std::is_convertible_v<Arg, Type> &&
		are_convertible_v<Type, Args...>
	> {};
}
