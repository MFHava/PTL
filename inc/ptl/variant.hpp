
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"
#include <limits>
#include <cstdint>
#include <ostream>
#include <utility>
#include <stdexcept>

namespace ptl {
	//! @brief exception thrown when trying to access a variant in an invalid way
	struct bad_variant_access : std::exception {
		auto what() const noexcept -> const char * override { return "bad_variant_access"; }
	};

	namespace internal {
		template<typename... Types>
		struct max_sizeof final : std::integral_constant<std::size_t, 0> {};

		template<typename Type, typename... Types>
		struct max_sizeof<Type, Types...> final : std::integral_constant<std::size_t, (sizeof(Type) > max_sizeof<Types...>::value ? sizeof(Type) : max_sizeof<Types...>::value)>{};

		template<typename TypeToFind, typename... Types>
		struct find final : std::integral_constant<std::uint8_t, std::numeric_limits<std::uint8_t>::max()> {};

		constexpr auto not_found{find<void>::value};

		template<typename TypeToFind, typename Type, typename... Types>
		struct find<TypeToFind, Type, Types...> final : std::integral_constant<
			std::uint8_t,
			std::is_same<TypeToFind, Type>::value 
				? 0 : find<TypeToFind, Types...>::value == not_found
					? not_found : find<TypeToFind, Types...>::value + 1
		>{};

		template<typename... Types>
		struct are_unique final : std::true_type {};

		template<typename Type, typename... Types>
		struct are_unique<Type, Types...> final : std::integral_constant<bool, (find<Type, Types...>::value == not_found && are_unique<Types...>::value)> {};

		template<typename ResultType, typename... Types>
		struct visit final {
			template<typename Visitor>
			constexpr
			static
			auto dispatch(std::uint8_t, const void *, Visitor &) -> ResultType { throw bad_variant_access{}; }
		};

		template<typename ResultType, typename Type, typename... Types>
		struct visit<ResultType, Type, Types...> final {
			template<typename Visitor>
			constexpr
			static
			auto dispatch(std::uint8_t index, const void * ptr, Visitor & visitor) -> ResultType {
				return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, visitor)
				             : visitor(*reinterpret_cast<const Type *>(ptr));
			}
			template<typename Visitor>
			constexpr
			static
			auto dispatch(std::uint8_t index,       void * ptr, Visitor & visitor) -> ResultType {
				return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, visitor)
				             : visitor(*reinterpret_cast<      Type *>(ptr));
			}
		};
	}

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
		variant(Type && value) { *this = std::forward<Type>(value); }

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

		constexpr
		auto valueless_by_exception() const noexcept -> bool { return type == internal::not_found; }

		template<typename Visitor>
		constexpr
		auto visit(Visitor && visitor) const -> decltype(auto) { return internal::visit<decltype(visitor(std::declval<DefaultType &>())), DefaultType, Types...>::dispatch(type, data, visitor); }
		template<typename Visitor>
		constexpr
		auto visit(Visitor && visitor)       -> decltype(auto) { return internal::visit<decltype(visitor(std::declval<DefaultType &>())), DefaultType, Types...>::dispatch(type, data, visitor); }

		template<typename Type>
		constexpr
		auto holds_alternative() const noexcept -> bool { return !valueless_by_exception() && internal::find<Type, DefaultType, Types...>::value == type; }

		template<typename Type>
		constexpr
		auto get() const -> const Type & {
			if(!holds_alternative<Type>()) throw bad_variant_access{};
			return *reinterpret_cast<const Type *>(data);
		}
		template<typename Type>
		constexpr
		auto get()       ->       Type & { return const_cast<Type &>(static_cast<const variant *>(this)->get<Type>()); }

		friend
		void swap(variant & lhs, variant & rhs) noexcept {
			if(lhs.valueless_by_exception() && rhs.valueless_by_exception()) return;
			if(lhs.type == rhs.type)
				lhs.visit([&](auto & value) {
					using Type = std::decay_t<decltype(value)>;
					using std::swap;
					swap(value, rhs.template get<Type>());
				});
			else std::swap(lhs, rhs);
		}

		friend
		constexpr
		auto operator==(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return false;
			if(lhs.valueless_by_exception()) return true;
			return lhs.visit([&](const auto & value) { return value == rhs.template get<std::decay_t<decltype(value)>>(); });
		}
		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return true;
			if(lhs.valueless_by_exception()) return false;
			return lhs.visit([&](const auto & value) { return value != rhs.template get<std::decay_t<decltype(value)>>(); });
		}
		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) -> bool {
			if(rhs.valueless_by_exception()) return false;
			if(lhs.valueless_by_exception()) return true;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <  rhs.template get<std::decay_t<decltype(value)>>(); });
		}
		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return true;
			if(rhs.valueless_by_exception()) return false;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <= rhs.template get<std::decay_t<decltype(value)>>(); });
		}
		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return false;
			if(rhs.valueless_by_exception()) return true;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >  rhs.template get<std::decay_t<decltype(value)>>(); });
		}
		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) -> bool {
			if(rhs.valueless_by_exception()) return true;
			if(lhs.valueless_by_exception()) return false;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >= rhs.template get<std::decay_t<decltype(value)>>(); });
		}

		friend
		auto operator<<(std::ostream & os, const variant & self) -> std::ostream & {
			if(self.valueless_by_exception()) return os << "<valueless by exception>";
			self.visit([&](const auto & value) { os << value; });
			return os;
		}
	};
	PTL_PACK_END
}

namespace std {
	template<typename... Types>
	struct hash<ptl::variant<Types...>> final {
		auto operator()(const ptl::variant<Types...> & self) const -> std::size_t {
			if(self.valueless_by_exception()) return 0;
			return self.visit([](const auto & value) { return std::hash<std::decay_t<decltype(value)>>{}(value); });
		}
	};
}