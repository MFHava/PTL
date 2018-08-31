
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>
#include <cstdint>
#include <variant>
#include <type_traits>

namespace ptl::internal {
	template<typename... Types>
	struct max_sizeof final : std::integral_constant<std::size_t, 0> {};

	template<typename Type, typename... Types>
	struct max_sizeof<Type, Types...> final : std::integral_constant<std::size_t, (sizeof(Type) > max_sizeof<Types...>::value ? sizeof(Type) : max_sizeof<Types...>::value)>{};

	template<typename TypeToFind, typename... Types>
	struct find final : std::integral_constant<std::uint8_t, std::numeric_limits<std::uint8_t>::max()> {};

	constexpr
	auto not_found{find<void>::value};

	template<typename TypeToFind, typename Type, typename... Types>
	struct find<TypeToFind, Type, Types...> final : std::integral_constant<
		std::uint8_t,
		std::is_same_v<TypeToFind, Type> 
			? 0 : find<TypeToFind, Types...>::value == not_found
				? not_found : find<TypeToFind, Types...>::value + 1
	>{};

	template<typename... Types>
	struct are_unique final : std::true_type {};

	template<typename Type, typename... Types>
	struct are_unique<Type, Types...> final : std::bool_constant<(find<Type, Types...>::value == not_found && are_unique<Types...>::value)> {};

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
			return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, visitor)
			             : visitor(*reinterpret_cast<const Type *>(ptr));
		}
		template<typename Visitor>
		static
		constexpr
		auto dispatch(std::uint8_t index,       void * ptr, Visitor & visitor) -> ResultType {
			return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, visitor)
			             : visitor(*reinterpret_cast<      Type *>(ptr));
		}
	};

	template<typename Type>
	struct get_visitor final {
		auto operator()(Type & self) const -> Type & { return self; }

		template<typename OtherType>
		auto operator()(const OtherType & self) const -> Type & { throw std::bad_variant_access{}; }
	};

	template<typename Type>
	struct holds_alternative_visitor final {
		auto operator()(const Type &) const noexcept { return true; }

		template<typename OtherType>
		auto operator()(const OtherType &) const noexcept { return false; }
	};
}
