
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <ostream>
#include <utility>
#include "internal/variant.hpp"
#include "internal/adl_swap.hpp"
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a type-safe union, storing one of multiple types
	//! @tparam DefaultType type that will be stored in the variant by default
	//! @tparam Types all additional types that may be stored in the variant
	template<typename DefaultType, typename... Types>
	class variant final {
		template<typename Type>
		using parameter_validation = std::bool_constant<!std::is_same_v<std::decay_t<Type>, variant> && internal::find_v<std::decay_t<Type>, DefaultType, Types...> != internal::not_found>;

		std::uint8_t data[internal::max_sizeof_v<DefaultType, Types...>], type{internal::not_found};

		void reset() noexcept {
			if(type == internal::not_found) return;
			visit([](auto & value) {
				using Type = std::decay_t<decltype(value)>;
				value.~Type();
			});
			type = internal::not_found;
		}

		static_assert(internal::are_abi_compatible_v<DefaultType, Types...>);
		static_assert(internal::are_unique_v<DefaultType, Types...>);
		static_assert(1 + sizeof...(Types) < internal::not_found, "Too many types for variant specified");
	public:
		constexpr
		variant() : type{0} { new(data) DefaultType{}; }

		variant(const variant & other) { *this = other; }
		variant(variant && other) noexcept { *this = std::move(other); }

		template<typename Type, typename = std::enable_if_t<parameter_validation<Type>::value>>
		constexpr
		variant(Type && value) { *this = std::forward<Type>(value); }

		template<typename Type, typename... Args>
		constexpr
		explicit
		variant(std::in_place_type_t<Type>, Args &&... args) { emplace<Type>(std::forward<Args>(args)...); }

		auto operator=(const variant & other) -> variant & {
			if(other.valueless_by_exception()) reset();
			else other.visit([&](auto & value) { *this = value; });
			return *this;
		}
		auto operator=(variant && other) noexcept -> variant & {
			if(other.valueless_by_exception()) reset();
			else other.visit([&](auto & value) { *this = std::move(value); });
			return *this;
		}

		template<typename Type>
		auto operator=(Type && value) -> std::enable_if_t<parameter_validation<Type>::value, variant &> {
			using DecayedType = std::decay_t<Type>;
			constexpr auto tmp{internal::find_v<DecayedType, DefaultType, Types...>};
			if(type == tmp) *reinterpret_cast<DecayedType *>(data) = std::forward<Type>(value);
			else {
				reset();
				new(data) DecayedType{std::forward<Type>(value)};
				type = tmp;
			}
			return *this;
		}

		~variant() noexcept { reset(); }

		template<typename Type, typename... Args>
		auto emplace(Args &&... args) -> std::enable_if_t<parameter_validation<Type>::value, Type &> {
			using DecayedType = std::decay_t<Type>;
			constexpr auto tmp{internal::find_v<DecayedType, DefaultType, Types...>};
			reset();
			new(data) DecayedType{std::forward<Args>(args)...};
			type = tmp;
			return *reinterpret_cast<Type *>(data);
		}

		constexpr
		auto valueless_by_exception() const noexcept { return type == internal::not_found; }

		template<typename Visitor, typename... Visitors>
		constexpr
		auto visit(Visitor && visitor, Visitors &&... visitors) const -> decltype(auto) {
			if constexpr(sizeof...(visitors) == 0) return internal::visit<decltype(visitor(std::declval<DefaultType &>())), DefaultType, Types...>::dispatch(type, data, visitor);
			else return visit(internal::combined_visitor{std::forward<Visitor>(visitor), std::forward<Visitors>(visitors)...});
		}
		template<typename Visitor, typename... Visitors>
		constexpr
		auto visit(Visitor && visitor, Visitors &&... visitors)       -> decltype(auto) {
			if constexpr(sizeof...(visitors) == 0) return internal::visit<decltype(visitor(std::declval<DefaultType &>())), DefaultType, Types...>::dispatch(type, data, visitor);
			else return visit(internal::combined_visitor{std::forward<Visitor>(visitor), std::forward<Visitors>(visitors)...});
		}

		void swap(variant & other) noexcept {
			if(valueless_by_exception() && other.valueless_by_exception()) return;
			if(type == other.type) visit([&](auto & value) { internal::adl_swap(value, *reinterpret_cast<std::decay_t<decltype(value)> *>(other.data)); });
			else std::swap(*this, other);
		}
		friend
		void swap(variant & lhs, variant & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type != rhs.type) return false;
			if(lhs.valueless_by_exception()) return true;
			return lhs.visit([&](const auto & value) { return value == *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type != rhs.type) return true;
			if(lhs.valueless_by_exception()) return false;
			return lhs.visit([&](const auto & value) { return value != *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) noexcept {
			if(rhs.valueless_by_exception()) return false;
			if(lhs.valueless_by_exception()) return true;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <  *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.valueless_by_exception()) return true;
			if(rhs.valueless_by_exception()) return false;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <= *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) noexcept {
			if(lhs.valueless_by_exception()) return false;
			if(rhs.valueless_by_exception()) return true;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >  *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) noexcept {
			if(rhs.valueless_by_exception()) return true;
			if(lhs.valueless_by_exception()) return false;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >= *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}

		friend
		auto operator<<(std::ostream & os, const variant & self) -> std::ostream & {
			if(self.valueless_by_exception()) return os << "<valueless by exception>";
			self.visit([&](const auto & value) { os << value; });
			return os;
		}
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	constexpr
	auto holds_alternative(const variant<Types...> & self) noexcept -> bool {
		if(self.valueless_by_exception()) return false;
		return self.visit(
			[](const Type &) { return true; },
			[](const auto &) { return false; }
		);
	}

	template<typename Type, typename... Types>
	constexpr
	auto get(const variant<Types...> & self) -> const Type & {
		static_assert(internal::find_v<Type, Types...> != internal::not_found);
		return self.visit(
			[](const Type & self) -> const Type & { return self; },
			[](const auto &) -> const Type & { throw std::bad_variant_access{}; }
		);
	}

	template<typename Type, typename... Types>
	constexpr
	auto get(      variant<Types...> & self) ->       Type & {
		static_assert(internal::find_v<Type, Types...> != internal::not_found);
		return self.visit(
			[](Type & self) -> Type & { return self; },
			[](auto &) -> Type & { throw std::bad_variant_access{}; }
		);
	}

	template<typename Type, typename... Types>
	constexpr
	auto get(const variant<Types...> && self) -> const Type && { return std::move(get<Type>(self)); }

	template<typename Type, typename... Types>
	constexpr
	auto get(      variant<Types...> && self) ->       Type && { return std::move(get<Type>(self)); }
}

namespace std {
	template<typename... Types>
	struct hash<ptl::variant<Types...>> final {
		auto operator()(const ptl::variant<Types...> & self) const noexcept -> std::size_t {
			return self.valueless_by_exception() ? 0 : self.visit([](const auto & value) { return std::hash<std::decay_t<decltype(value)>>{}(value); });
		}
	};
}
