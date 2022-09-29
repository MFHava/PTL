
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
	namespace internal_variant_ref {
		template<typename...>
		class storage_t final {};

		template<typename Head, typename... Tail>
		class storage_t<Head, Tail...> final {
			union {
				Head * ptr;
				storage_t<Tail...> next;
			};
		public:
			using value_type = Head;

			template<std::size_t I>
			constexpr
			auto get() const noexcept {
				if constexpr(I != 0) return next.template get<I - 1>();
				else return ptr;
			}

			template<std::size_t I, typename T>
			constexpr
			void set(T & val) noexcept {
				if constexpr(I != 0) next.template set<I - 1>(val);
				else ptr = std::addressof(val);
			}
		};

		template<typename T, typename Head, typename... Tail>
		constexpr //TODO: [C++20] replace with consteval
		auto determine_index(std::ptrdiff_t index = 0) noexcept -> std::ptrdiff_t {
			if constexpr(std::is_same_v<T, Head>) return index;
			else if constexpr(sizeof...(Tail) != 0) return determine_index<T, Tail...>(index + 1);
			else return -1;
		}

		template<typename Head, typename... Tail>
		constexpr //TODO: [C++20] replace with consteval
		auto validate_unique() noexcept -> bool {
			if constexpr(sizeof...(Tail) == 0) return true;
			else if constexpr(determine_index<Head, Tail...>() != -1) return false;
			else return validate_unique<Tail...>();
		}

		template<std::size_t... Indices, typename Storage>
		constexpr
		void assign(std::index_sequence<Indices...>, std::size_t type, Storage & lhs, const Storage & rhs) noexcept {
			using Dispatch = void(*)(Storage &, const Storage &) noexcept;
			constexpr Dispatch dispatch[]{+[](Storage & storage, const Storage & other) noexcept { storage.template set<Indices>(*other.template get<Indices>()); }...};
			dispatch[type](lhs, rhs);
		}

		template<typename Result, typename Storage, std::size_t... Indices>
		constexpr
		auto get(std::index_sequence<Indices...>, std::size_t type, const Storage & storage) noexcept -> Result * {
			using Dispatch = Result*(*)(const Storage &) noexcept;
			constexpr Dispatch dispatch[]{+[](const Storage & storage) noexcept -> Result * {
				if constexpr(auto ptr{storage.template get<Indices>()}; std::is_same_v<Result *, decltype(ptr)>) return ptr;
				else std::terminate(); //TODO: [C++23] use std::unreachable
			}...};
			return dispatch[type](storage);
		}

		template<typename Storage, typename Visitor, std::size_t... Indices>
		constexpr
		auto visit(std::index_sequence<Indices...>, std::size_t type, const Storage & storage, Visitor & visitor) -> decltype(auto) {
			using Head = typename Storage::value_type;
			using Result = decltype(std::declval<Visitor>()(std::declval<Head &>()));
			using Dispatch = Result(*)(const Storage &, Visitor &);
			constexpr Dispatch dispatch[]{+[](const Storage & storage, Visitor & visitor) -> Result { return visitor(*storage.template get<Indices>()); }...};
			return dispatch[type](storage, visitor);
		}
	}

	//! @brief a non-owning reference to one of multiple types
	//! @tparam Types all types that may be referenced by the variant_ref
	//! @attention Types must be non-empty and unique!
	template<typename... Types>
	class variant_ref;
	
	template<typename Head, typename... Tail>
	class variant_ref<Head, Tail...> final {
		static_assert(sizeof...(Tail) < static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()));
		static_assert(internal_variant_ref::validate_unique<Head, Tail...>());

		using storage_t = internal_variant_ref::storage_t<Head, Tail...>;
		static_assert(sizeof(storage_t) == sizeof(void *));

		using indices_t = std::index_sequence_for<Head, Tail...>;

		storage_t storage;
		std::size_t type;

		template<typename T>
		static
		constexpr
		bool can_store{internal_variant_ref::determine_index<T, Head, Tail...>() != -1}; //TODO: [C++20] replace with concepts/requires-clause
	public:
		template<typename T, typename = std::enable_if_t<(std::is_reference_v<T> /*prevent binding mutable reference to prvalues*/ && can_store<std::remove_reference_t<T>>) || can_store<const std::remove_reference_t<T>>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		variant_ref(T && val) noexcept {
			constexpr auto id{internal_variant_ref::determine_index<std::remove_reference_t<T>, Head, Tail...>()};
			if constexpr(std::is_reference_v<T> && id != -1) {
				type = id;
				storage.template set<id>(val);
			} else {
				constexpr auto id{internal_variant_ref::determine_index<const std::remove_reference_t<T>, Head, Tail...>()};
				static_assert(id != -1);
				type = id;
				storage.template set<id>(val);
			}
		}

		constexpr
		variant_ref(const variant_ref & other) { internal_variant_ref::assign(indices_t{}, type = other.type, storage, other.storage); }
		constexpr
		auto operator=(const variant_ref & other) -> variant_ref & {
			internal_variant_ref::assign(indices_t{}, type = other.type, storage, other.storage);
			return *this;
		}
		~variant_ref() noexcept =default;//TODO: [C++20] mark dtor as constexpr

		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors) const -> decltype(auto) {
			if constexpr(sizeof...(Visitors) == 1) return internal_variant_ref::visit(indices_t{}, type, storage, visitors...);
			else {
				struct combined_visitor : Visitors... { using Visitors::operator()...; };
				combined_visitor visitor{std::forward<Visitors>(visitors)...};
				return visit(visitor);
			}
		}

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto holds() const noexcept -> bool { return type == static_cast<std::size_t>(internal_variant_ref::determine_index<T, Head, Tail...>()); }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get_if() const noexcept -> T * {
			if(!holds<T>()) return nullptr;
			return internal_variant_ref::get<T>(indices_t{}, type, storage);
		}

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get() const -> T & {
			if(const auto ptr{get_if<T>()}) return *ptr;
			throw std::bad_variant_access{};
		}
	};

	//TODO: static_assert(sizeof(variant_ref<Types...>) == 2 * sizeof(void *));
}
