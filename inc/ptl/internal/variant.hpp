
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <variant>
#include <type_traits>

namespace ptl::internal {
	template<typename... Types>
	struct max_sizeof final : std::integral_constant<std::size_t, 0> {};

	template<typename... Types>
	inline
	constexpr
	auto max_sizeof_v{max_sizeof<Types...>::value};

	template<typename Type, typename... Types>
	struct max_sizeof<Type, Types...> final : std::integral_constant<std::size_t,
		(sizeof(Type) > max_sizeof_v<Types...> ? sizeof(Type) : max_sizeof_v<Types...>)
	>{};

	template<typename TypeToFind, typename... Types>
	struct find final : std::integral_constant<std::uint8_t, std::numeric_limits<std::uint8_t>::max()> {};

	template<typename TypeToFind, typename... Types>
	inline
	constexpr
	std::uint8_t find_v{find<TypeToFind, Types...>::value};//TODO: why can't GCC deduce the correct type here?!

	inline
	constexpr
	auto not_found{find_v<void>};

	template<typename TypeToFind, typename Type, typename... Types>
	struct find<TypeToFind, Type, Types...> final : std::integral_constant<std::uint8_t,
		std::is_same_v<TypeToFind, Type> 
			? 0
			: find_v<TypeToFind, Types...> == not_found
				? not_found
				: find_v<TypeToFind, Types...> + 1
	>{};

	template<typename... Types>
	struct are_unique final : std::true_type {};

	template<typename... Types>
	inline
	constexpr
	auto are_unique_v{are_unique<Types...>::value};

	template<typename Type, typename... Types>
	struct are_unique<Type, Types...> final : std::bool_constant<
		find_v<Type, Types...> == not_found &&
		are_unique_v<Types...>
	> {};

	template<typename ResultType, typename... Types>
	struct visit final {
		template<typename Visitor>
		static
		constexpr
		auto dispatch(std::uint8_t, const void *, Visitor &) -> ResultType { throw std::bad_variant_access{}; }
	};

	template<typename ResultType, typename Type, typename... Types>
	struct visit<ResultType, Type, Types...> final {
		template<typename Visitor>
		static
		constexpr
		auto dispatch(std::uint8_t index, const void * ptr, Visitor & visitor) -> ResultType {
			return index ? visit<ResultType, Types...>::dispatch(static_cast<std::uint8_t>(index - 1), ptr, visitor)
			             : visitor(*reinterpret_cast<const Type *>(ptr));
		}
		template<typename Visitor>
		static
		constexpr
		auto dispatch(std::uint8_t index,       void * ptr, Visitor & visitor) -> ResultType {
			return index ? visit<ResultType, Types...>::dispatch(static_cast<std::uint8_t>(index - 1), ptr, visitor)
			             : visitor(*reinterpret_cast<      Type *>(ptr));
		}
	};

	template<typename... Functors>
	struct combined_visitor;

	template<typename Functor>
	struct combined_visitor<Functor> : Functor {
		constexpr
		combined_visitor(Functor functor) : Functor{functor} {}
	};

	template<typename Functor, typename... Functors>
	struct combined_visitor<Functor, Functors...> : Functor, combined_visitor<Functors...> {
		using Functor::operator();
		using combined_visitor<Functors...>::operator();

		constexpr
		combined_visitor(Functor functor, Functors... functors) : Functor{functor}, combined_visitor<Functors...>{functors...} {}
	};

	template<typename... Functors>
	constexpr
	auto combine(Functors &&... functors) -> combined_visitor<Functors...> { return {std::forward<Functors>(functors)...}; }
}
