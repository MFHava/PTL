
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <utility>
#include <variant>
#include <type_traits>

namespace ptl {
	//! @brief a non-owning reference to one of multiple types
	//! @tparam Types all types that may be referenced by the variant_ref
	//! @attention Types must be non-empty and unique!
	template<typename... Types>
	class variant_ref final { //TODO: static_assert(sizeof(variant_ref<Types...>) == 2 * sizeof(void *));
		static_assert(sizeof...(Types) > 0);
		static_assert(sizeof...(Types) <= static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()));

		template<typename Type, typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto determine_index(std::ptrdiff_t index = 0) noexcept -> std::ptrdiff_t {
			if constexpr(std::is_same_v<Type, T>) return index;
			else if constexpr(sizeof...(Ts) != 0) return determine_index<Type, Ts...>(index + 1);
			else return -1;
		}

		template<typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto validate_unique() noexcept -> bool {
			if constexpr(sizeof...(Ts) == 0) return true;
			else if constexpr(determine_index<T, Ts...>() != -1) return false;
			else return validate_unique<Ts...>();
		}

		static_assert(validate_unique<Types...>());

		void * ptr;
		std::size_t type;

		template<typename T, typename...>
		struct first_type { using type = T; };

		template<typename T>
		static
		constexpr
		bool can_store{determine_index<T, Types...>() != -1}; //TODO: [C++20] replace with concepts/requires-clause
	public:
		template<typename T, typename = std::enable_if_t<(std::is_reference_v<T> /*prevent binding mutable reference to prvalues*/ && can_store<std::remove_reference_t<T>>) || can_store<const std::remove_reference_t<T>>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		variant_ref(T && val) noexcept : ptr{const_cast<void *>(reinterpret_cast<const void *>(std::addressof(val)))} {
			constexpr auto id{determine_index<std::remove_reference_t<T>, Types...>()};
			if constexpr(std::is_reference_v<T> && id != -1) type = id;
			else {
				constexpr auto id{determine_index<const std::remove_reference_t<T>, Types...>()};
				static_assert(id != -1);
				type = id;
			}
		}

		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors) const -> decltype(auto) {
			if constexpr(sizeof...(Visitors) == 1) {
				using Type = typename first_type<Types...>::type;
				using Visitor = typename first_type<Visitors...>::type;
				using Result = decltype(std::declval<Visitor>()(std::declval<Type &>()));
				using Dispatch = Result(*)(void *, Visitor &);
				constexpr Dispatch dispatch[]{+[](void * ptr, Visitor & visitor) -> Result { return visitor(*reinterpret_cast<Types *>(ptr)); }...};
				return dispatch[type](ptr, visitors...);
			} else {
				struct combined_visitor : Visitors... { using Visitors::operator()...; };
				combined_visitor visitor{std::forward<Visitors>(visitors)...};
				return visit(visitor);
			}
		}

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto holds() const noexcept -> bool { return type == static_cast<std::size_t>(determine_index<T, Types...>()); }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get_if() const noexcept -> T * { return holds<T>() ? reinterpret_cast<T *>(ptr) : nullptr; }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get() const -> T & {
			if(const auto ptr{get_if<T>()}) return *ptr;
			throw std::bad_variant_access{};
		}

		constexpr
		void swap(variant_ref & other) noexcept {
			std::swap(ptr,  other.ptr);
			std::swap(type, other.type);
		}
		friend
		constexpr
		void swap(variant_ref & lhs, variant_ref & rhs) noexcept { lhs.swap(rhs); }
	};
}
