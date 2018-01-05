
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <ostream>
#include <utility>
#include <stdexcept>
#include "internal/variant.hpp"
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	//! @brief tag for dispatch in constructor of variant
	template<typename Type>
	constexpr internal::in_place_type_t<Type> in_place_type{};

	//! @brief exception thrown when trying to access a variant in an invalid way
	struct bad_variant_access : std::exception {
		auto what() const noexcept -> const char * override { return "bad_variant_access"; }
	};

	PTL_PACK_BEGIN
	//! @brief a type-safe union, storing one of multiple types
	//! @tparam DefaultType type that will be stored in the variant by default
	//! @tparam Types all additional types that may be stored in the variant
	template<typename DefaultType, typename... Types>
	class variant final {
		template<typename Type>
		using parameter_validation = std::integral_constant<bool, !std::is_same<std::decay_t<Type>, variant>::value && internal::find<std::decay_t<Type>, DefaultType, Types...>::value != internal::not_found>;

		std::uint8_t data[internal::max_sizeof<DefaultType, Types...>::value], type{internal::not_found};

		void reset() noexcept {
			if(type == internal::not_found) return;
			visit([](auto & value) {
				using Type = std::decay_t<decltype(value)>;
				value.~Type();
			});
			type = internal::not_found;
		}

		static_assert(internal::are_abi_compatible<DefaultType, Types...>::value, "Types do not fulfill ABI requirements");
		static_assert(1 + sizeof...(Types) < internal::not_found, "Too many types for variant specified");
		static_assert(internal::are_unique<DefaultType, Types...>::value, "variant does not support duplicated types");
	public:
		constexpr
		variant() : type{0} { new(data) DefaultType{}; }

		variant(const variant & other) { *this = other; }
		variant(variant && other) noexcept { *this = std::move(other); }

		template<typename Type, typename = std::enable_if_t<parameter_validation<Type>::value>>
		constexpr
		variant(Type && value) { *this = std::forward<Type>(value); }

		template<typename Type, typename... Args>
		explicit
		constexpr
		variant(internal::in_place_type_t<Type>, Args &&... args) { emplace<Type>(std::forward<Args>(args)...); }

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
			constexpr auto tmp{internal::find<DecayedType, DefaultType, Types...>::value};
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
			constexpr auto tmp{internal::find<DecayedType, DefaultType, Types...>::value};
			reset();
			new(data) DecayedType{std::forward<Args>(args)...};
			type = tmp;
			return *reinterpret_cast<Type *>(data);
		}

		constexpr
		auto valueless_by_exception() const noexcept -> bool { return type == internal::not_found; }

		template<typename Visitor>
		constexpr
		decltype(auto) visit(Visitor && visitor) const { return internal::visit<bad_variant_access, decltype(visitor(std::declval<DefaultType &>())), DefaultType, Types...>::dispatch(type, data, visitor); }
		template<typename Visitor>
		constexpr
		decltype(auto) visit(Visitor && visitor)       { return internal::visit<bad_variant_access, decltype(visitor(std::declval<DefaultType &>())), DefaultType, Types...>::dispatch(type, data, visitor); }

		friend
		void swap(variant & lhs, variant & rhs) noexcept {
			if(lhs.valueless_by_exception() && rhs.valueless_by_exception()) return;
			if(lhs.type == rhs.type)
				lhs.visit([&](auto & value) {
					using Type = std::decay_t<decltype(value)>;
					using std::swap;
					swap(value, *reinterpret_cast<Type *>(rhs.data));
				});
			else std::swap(lhs, rhs);
		}

		friend
		constexpr
		auto operator==(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type != rhs.type) return false;
			if(lhs.valueless_by_exception()) return true;
			return lhs.visit([&](const auto & value) {
				using Type = std::decay_t<decltype(value)>;
				return value == *reinterpret_cast<const Type *>(rhs.data);
			});
		}
		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type != rhs.type) return true;
			if(lhs.valueless_by_exception()) return false;
			return lhs.visit([&](const auto & value) {
				using Type = std::decay_t<decltype(value)>;
				return value != *reinterpret_cast<const Type *>(rhs.data);
			});
		}
		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) noexcept {
			if(rhs.valueless_by_exception()) return false;
			if(lhs.valueless_by_exception()) return true;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) {
				using Type = std::decay_t<decltype(value)>;
				return value <  *reinterpret_cast<const Type *>(rhs.data);
			});
		}
		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.valueless_by_exception()) return true;
			if(rhs.valueless_by_exception()) return false;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) {
				using Type = std::decay_t<decltype(value)>;
				return value <= *reinterpret_cast<const Type *>(rhs.data);
			});
		}
		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) noexcept {
			if(lhs.valueless_by_exception()) return false;
			if(rhs.valueless_by_exception()) return true;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) {
				using Type = std::decay_t<decltype(value)>;
				return value >  *reinterpret_cast<const Type *>(rhs.data);
			});
		}
		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) noexcept {
			if(rhs.valueless_by_exception()) return true;
			if(lhs.valueless_by_exception()) return false;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) {
				using Type = std::decay_t<decltype(value)>;
				return value >= *reinterpret_cast<const Type *>(rhs.data);
			});
		}

		friend
		decltype(auto) operator<<(std::ostream & os, const variant & self) {
			if(self.valueless_by_exception()) return os << "<valueless by exception>";
			self.visit([&](const auto & value) { os << value; });
			return os;
		}
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	constexpr
	auto holds_alternative(const variant<Types...> & self) noexcept -> bool { return self.valueless_by_exception() ? false : self.visit(internal::holds_alternative_visitor<Type>{}); }

	template<typename Type, typename... Types>
	constexpr
	decltype(auto) get(const variant<Types...> & self) {
		static_assert(internal::find<Type, Types...>::value != internal::not_found, "type not stored in variant");
		return self.visit(internal::get_visitor<bad_variant_access, const Type>());
	}

	template<typename Type, typename... Types>
	constexpr
	decltype(auto) get(      variant<Types...> & self) {
		static_assert(internal::find<Type, Types...>::value != internal::not_found, "type not stored in variant");
		return self.visit(internal::get_visitor<bad_variant_access,       Type>());
	}

	template<typename Type, typename... Types>
	constexpr
	decltype(auto) get(const variant<Types...> && self) { return std::move(get<Type>(self)); }

	template<typename Type, typename... Types>
	constexpr
	decltype(auto) get(      variant<Types...> && self) { return std::move(get<Type>(self)); }
}

namespace std {
	template<typename... Types>
	struct hash<ptl::variant<Types...>> final {
		auto operator()(const ptl::variant<Types...> & self) const noexcept -> std::size_t {
			return self.valueless_by_exception() ? 0 : self.visit([](const auto & value) { return std::hash<std::decay_t<decltype(value)>>{}(value); });
		}
	};
}
