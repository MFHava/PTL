
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <utility>
#include <variant>
#include <type_traits>
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a type-safe union, storing one of multiple types
	//! @tparam Types all types that may be stored in the variant
	template<typename... Types>
	class variant final {
		static_assert(sizeof...(Types) > 0);
		static_assert(sizeof...(Types) < 255);
		static_assert((internal::is_abi_compatible_v<Types> && ...));

		static
		constexpr
		unsigned char not_found{255};

		template<typename Type, typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto determine_index(unsigned char index = 0) noexcept -> unsigned char {
			if constexpr(std::is_same_v<Type, T>) return index;
			else if constexpr(sizeof...(Ts) != 0) return determine_index<Type, Ts...>(index + 1);
			else return not_found;
		}

		template<typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto validate_unique() noexcept -> bool {
			if constexpr(sizeof...(Ts) == 0) return true;
			else if constexpr(determine_index<T, Ts...>() != not_found) return false;
			else return validate_unique<Ts...>();
		}

		static_assert(validate_unique<Types...>());

		template<typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto max_sizeof(std::size_t size = 0) noexcept -> std::size_t {
			if(sizeof(T) > size) size = sizeof(T);
			if constexpr(sizeof...(Ts) != 0) return max_sizeof<Ts...>(size);
			else return size;
		}

		unsigned char data[max_sizeof<Types...>()], type;

		template<typename Type>
		void construct(Type && value) {
			using DecayedType = std::decay_t<Type>;
			new(data) DecayedType{std::forward<Type>(value)};
			constexpr auto id{determine_index<DecayedType, Types...>()};
			static_assert(id != not_found);
			type = id;
		}

		template<typename T, typename...>
		struct first_type { using type = T; };

		template<typename T>
		static
		constexpr
		bool can_store{determine_index<std::remove_const_t<std::remove_reference_t<T>>, Types...>() != not_found}; //TODO: [C++20] replace with concepts/requires-clause

		template<typename T>
		struct type_identity final { using type = T; }; //TODO: [C++20] replace with std::type_identity

		template<unsigned char Index, typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto determine_type() noexcept {
			if constexpr(Index == 0) return type_identity<T>{};
			else {
				static_assert(sizeof...(Ts));
				return determine_type<Index - 1, Ts...>();
			}
		}

		template<bool Move, typename Ptr, typename... Visitors>
		static
		auto visit_impl(std::size_t type, Ptr * data, Visitors &&... visitors) -> decltype(auto) {
			if constexpr(sizeof...(Visitors) == 1) {
				using Type = typename first_type<Types...>::type;
				using Visitor = typename first_type<Visitors...>::type;
				using Tmp1 = std::conditional_t<std::is_const_v<Ptr>, const Type, Type>;
				using Tmp2 = std::conditional_t<Move, Tmp1, Tmp1 &>;
				using Result = decltype(std::declval<Visitor>()(std::declval<Tmp2>()));
				using Dispatch = Result(*)(Ptr *, Visitor &);
				constexpr Dispatch dispatch[]{+[](Ptr * ptr, Visitor & visitor) -> Result {
					using Ref = std::conditional_t<std::is_const_v<Ptr>, const Types, Types>;
					if constexpr(Move) return visitor(std::move(*reinterpret_cast<Ref *>(ptr)));
					else return visitor(*reinterpret_cast<Ref *>(ptr));
				}...};
				return dispatch[type](data, visitors...);
			} else {
				struct combined_visitor : Visitors... { using Visitors::operator()...; };
				combined_visitor visitor{std::forward<Visitors>(visitors)...};
				return visit_impl<Move>(type, data, visitor);
			}
		}
	public:
		constexpr
		variant() : type{0} { new(data) typename first_type<Types...>::type{}; }

		variant(const variant & other) { other.visit([&](const auto & value) { construct(value); }); }
		variant(variant && other) noexcept { other.visit([&](auto & value) { construct(std::move(value)); }); }

		template<typename Type, typename = std::enable_if_t<can_store<Type>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		variant(Type && value) { construct(std::forward<Type>(value)); }

		template<std::size_t Index, typename... Args, typename = std::enable_if_t<(Index > 0 && Index < sizeof...(Types))>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		explicit
		variant(std::in_place_index_t<Index>, Args &&... args) {
			using Type = typename decltype(determine_type<Index, Types...>())::type;
			new(data) Type{std::forward<Args>(args)...};
			type = Index;
		}
		template<typename Type, typename... Args, typename = std::enable_if_t<can_store<Type>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		explicit
		variant(std::in_place_type_t<Type>, Args &&... args) : variant{std::in_place_index<determine_index<Type, Types...>()>, std::forward<Args>(args)...} {}

		auto operator=(const variant & other) -> variant & {
			other.visit([&](const auto & value) { *this = value; });
			return *this;
		}
		auto operator=(variant && other) noexcept -> variant & {
			other.visit([&](auto & value) { *this = std::move(value); });
			return *this;
		}

		template<typename Type, typename = std::enable_if_t<can_store<Type>>> //TODO: [C++20] replace with concepts/requires-clause
		auto operator=(Type && value) -> variant & {
			using DecayedType = std::decay_t<Type>;
			constexpr auto id{determine_index<DecayedType, Types...>()};
			static_assert(id != not_found); //TODO: [C++20] replace with requires-clause
			if(type == id) *reinterpret_cast<DecayedType *>(data) = std::forward<Type>(value);
			else emplace<DecayedType>(std::forward<Type>(value));
			return *this;
		}

		~variant() noexcept {
			visit([](auto & value) {
				using Type = std::decay_t<decltype(value)>;
				value.~Type();
			});
		}

		template<std::size_t Index, typename... Args, typename = std::enable_if_t<(Index > 0 && Index < sizeof...(Types))>> //TODO: [C++20] replace with concepts/requires-clause
		auto emplace(Args &&... args) -> decltype(auto) { return emplace<typename decltype(determine_type<Index, Types...>())::type>(std::forward<Args>(args)...); }
		template<typename Type, typename... Args, typename = std::enable_if_t<can_store<Type>>> //TODO: [C++20] replace with concepts/requires-clause
		auto emplace(Args &&... args) -> Type & {
			Type storage{std::forward<Args>(args)...};
			this->~variant();
			new(this) variant{std::move(storage)};
			return *reinterpret_cast<Type *>(data);
		}

		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors) const & -> decltype(auto) { return visit_impl<false>(type, data, std::forward<Visitors>(visitors)...); }
		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors)       & -> decltype(auto) { return visit_impl<false>(type, data, std::forward<Visitors>(visitors)...); }
		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors) const && -> decltype(auto) { return visit_impl<true>(type, data, std::forward<Visitors>(visitors)...); }
		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors)       && -> decltype(auto) { return visit_impl<true>(type, data, std::forward<Visitors>(visitors)...); }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto holds() const noexcept -> bool { return type == determine_index<T, Types...>(); }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get_if() const noexcept -> const T * { return holds<T>() ? reinterpret_cast<const T *>(data) : nullptr; }
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get_if()       noexcept ->       T * { return holds<T>() ? reinterpret_cast<      T *>(data) : nullptr; }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get() const &  -> const T & {
			const auto ptr{get_if<T>()};
			return ptr ? *ptr : throw std::bad_variant_access{};
		}
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get()      &  ->       T & {
			const auto ptr{get_if<T>()};
			return ptr ? *ptr : throw std::bad_variant_access{};
		}
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get() const && -> const T && { return std::move(get<T>()); }
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get()       && ->       T && { return std::move(get<T>()); }

		void swap(variant & other) noexcept {
			if(type == other.type) visit([&](auto & value) {
				using std::swap;
				swap(value, *reinterpret_cast<std::decay_t<decltype(value)> *>(other.data));
			});
			else std::swap(*this, other);
		}
		friend
		void swap(variant & lhs, variant & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type != rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value == *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) noexcept -> bool { //TODO: [C++20] remove as implicitly generated
			if(lhs.type != rhs.type) return true;
			return lhs.visit([&](const auto & value) { return value != *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <  *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <= *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >  *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >= *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
	};
	PTL_PACK_END
}
