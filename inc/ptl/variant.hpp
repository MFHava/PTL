
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <ostream>
#include <utility>
#include <variant>
#include "type_list.hpp"
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a type-safe union, storing one of multiple types
	//! @tparam Types all types that may be stored in the variant
	template<typename... Types>
	class variant final {
		using TL = type_list<Types...>;

		static_assert(sizeof...(Types) > 0);
		static_assert(sizeof...(Types) < 256);
		static_assert((internal::is_abi_compatible_v<Types> && ...));
		static_assert(std::is_same_v<TL, typename TL::unique>);

		template<typename T1, typename T2>
		using greater_sizeof = std::bool_constant<(sizeof(T1) < sizeof(T2))>;

		unsigned char data[sizeof(typename TL::template at<TL::template max_element<greater_sizeof>>)], type;

		template<typename Type>
		void construct(Type && value) {
			using DecayedType = std::decay_t<Type>;
			new(data) DecayedType{std::forward<Type>(value)};
			constexpr auto id{TL::template find<DecayedType>};
			static_assert(id != not_found);
			type = id;
		}

		template<typename... Functors>
		struct combined_visitor : Functors... {
			using Functors::operator()...;
		};

		template<typename TypeList, typename Self, typename Visitor, typename Result = decltype(std::declval<Visitor &>()(std::declval<typename TL::template at<0> &>()))>
		static
		constexpr
		auto dispatch(Self & self, std::size_t index, Visitor && visitor) -> Result {
			if constexpr(TypeList::empty) {
				(void)self;
				(void)index;
				(void)visitor;
				throw std::bad_variant_access{};
			} else {
				if(index) return dispatch<typename TypeList::pop_front>(self, index - 1, std::forward<Visitor>(visitor));
				else {
					using Type = typename TypeList::template at<0>;
					using TargetType = std::conditional_t<std::is_const_v<Self>, const Type, Type>;
					return visitor(*reinterpret_cast<TargetType *>(self.data));
				}
			}
		}

		template<typename Type>
		using compatible_check = std::enable_if_t<!std::is_same_v<std::decay_t<Type>, variant> && TL::template find<std::decay_t<Type>> != not_found>;
		template<std::size_t Index>
		using compatible_index_check = compatible_check<typename TL::template at<Index>>;
	public:
		constexpr
		variant() : type{0} { new(data) typename TL::template at<0>{}; }

		variant(const variant & other) { other.visit([&](const auto & value) { construct(value); }); }
		variant(variant && other) noexcept { other.visit([&](auto & value) { construct(std::move(value)); }); }

		template<typename Type, typename = compatible_check<Type>>
		constexpr
		variant(Type && value) { construct(std::forward<Type>(value)); }

		template<std::size_t Index, typename... Args>
		constexpr
		explicit
		variant(std::in_place_index_t<Index>, Args &&... args) {
			static_assert(Index != not_found);
			using Type = typename TL::template at<Index>;
			new(data) Type{std::forward<Args>(args)...};
			type = Index;
		}
		template<typename Type, typename... Args>
		constexpr
		explicit
		variant(std::in_place_type_t<Type>, Args &&... args) : variant{std::in_place_index<TL::template find<Type>>, std::forward<Args>(args)...} {}

		auto operator=(const variant & other) -> variant & {
			other.visit([&](const auto & value) { *this = value; });
			return *this;
		}
		auto operator=(variant && other) noexcept -> variant & {
			other.visit([&](auto & value) { *this = std::move(value); });
			return *this;
		}

		template<typename Type, typename = compatible_check<Type>>
		auto operator=(Type && value) -> variant & {
			using DecayedType = std::decay_t<Type>;
			constexpr auto id{TL::template find<DecayedType>};
			static_assert(id != not_found);
			if(type == id) *reinterpret_cast<DecayedType *>(data) = std::forward<Type>(value);
			else emplace<Type>(std::forward<Type>(value));
			return *this;
		}

		~variant() noexcept {
			visit([](auto & value) {
				using Type = std::decay_t<decltype(value)>;
				value.~Type();
			});
		}

		template<std::size_t Index, typename... Args, typename = compatible_index_check<Index>>
		auto emplace(Args &&... args) -> decltype(auto) {
			static_assert(Index != not_found);
			using Type = typename TL::template at<Index>;
			using DecayedType = std::decay_t<Type>;
			DecayedType storage{std::forward<Args>(args)...};
			this->~variant();
			new(this) variant{std::move(storage)};
			return *reinterpret_cast<DecayedType *>(data);
		}
		template<typename Type, typename... Args, typename = compatible_check<Type>>
		auto emplace(Args &&... args) -> Type & { return emplace<TL::template find<std::decay_t<Type>>>(std::forward<Args>(args)...); }

		template<typename Visitor, typename... Visitors>
		constexpr
		auto visit(Visitor && visitor, Visitors &&... visitors) const -> decltype(auto) {
			if constexpr(sizeof...(Visitors)) {
				combined_visitor<Visitor, Visitors...> tmp{std::forward<Visitor>(visitor), std::forward<Visitors>(visitors)...};
				return visit(tmp);
			} else return dispatch<TL>(*this, type, std::forward<Visitor>(visitor));
		}
		template<typename Visitor, typename... Visitors>
		constexpr
		auto visit(Visitor && visitor, Visitors &&... visitors)       -> decltype(auto) {
			if constexpr(sizeof...(Visitors)) {
				combined_visitor<Visitor, Visitors...> tmp{std::forward<Visitor>(visitor), std::forward<Visitors>(visitors)...};
				return visit(tmp);
			} else return dispatch<TL>(*this, type, std::forward<Visitor>(visitor));
		}

		constexpr
		auto index() const noexcept -> std::size_t { return type; }

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
		auto operator==(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type != rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value == *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type != rhs.type) return true;
			return lhs.visit([&](const auto & value) { return value != *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <  *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value <= *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >  *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}
		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) noexcept {
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit([&](const auto & value) { return value >= *reinterpret_cast<const std::decay_t<decltype(value)> *>(rhs.data); });
		}

		friend
		auto operator<<(std::ostream & os, const variant & self) -> std::ostream & {
			self.visit([&](const auto & value) { os << value; });
			return os;
		}
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	constexpr
	auto holds_alternative(const variant<Types...> & self) noexcept -> bool {
		constexpr auto id{type_list<Types...>::template find<Type>};
		static_assert(id != not_found);
		return self.index() == static_cast<std::size_t>(id);
	}

	template<typename Type, typename... Types>
	constexpr
	auto get(const variant<Types...> & self) -> const Type & {
		static_assert(type_list<Types...>::template find<Type> != not_found);
		return self.visit(
			[](const Type & self) -> const Type & { return self; },
			[](const auto &) -> const Type & { throw std::bad_variant_access{}; }
		);
	}

	template<typename Type, typename... Types>
	constexpr
	auto get(      variant<Types...> & self) ->       Type & {
		static_assert(type_list<Types...>::template find<Type> != not_found);
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
	struct hash<ptl::variant<Types...>> {
		auto operator()(const ptl::variant<Types...> & self) const noexcept -> std::size_t {
			return self.visit([](const auto & value) { return std::hash<std::decay_t<decltype(value)>>{}(value); });
		}
	};

	template<typename... Types>
	struct variant_size<ptl::variant<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

	template<std::size_t Index, typename... Types>
	struct variant_alternative<Index, ptl::variant<Types...>> {
		using type = typename ptl::type_list<Types...>::template at<Index>;
	};
}
