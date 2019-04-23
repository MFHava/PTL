
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "adl_swap.hpp"
#include "type_checks.hpp"
#include "compiler_detection.hpp"

namespace ptl::internal {
	PTL_PACK_BEGIN
	template<typename... Types>
	struct tuple_storage;

	template<>
	struct tuple_storage<> {
		constexpr
		tuple_storage() noexcept =default;

		constexpr
		tuple_storage(const tuple_storage &) noexcept =default;
		constexpr
		tuple_storage(tuple_storage &&) noexcept =default;

		constexpr
		auto operator=(const tuple_storage &) noexcept -> tuple_storage & =default;
		constexpr
		auto operator=(tuple_storage &&) noexcept -> tuple_storage & =default;

		~tuple_storage() noexcept =default;

		constexpr
		void swap(tuple_storage &) noexcept {}

		friend
		constexpr
		auto operator==(const tuple_storage &, const tuple_storage &) noexcept { return true; }
		friend
		constexpr
		auto operator< (const tuple_storage &, const tuple_storage &) noexcept { return false; }
	};

	template<typename Head, typename... Tail>
	struct tuple_storage<Head, Tail...> : tuple_storage<Tail...> {
		static_assert(internal::is_abi_compatible_v<Head>);
		using base_type = tuple_storage<Tail...>;

		Head value;

		constexpr
		tuple_storage() =default;

		constexpr
		tuple_storage(const tuple_storage &) =default;
		constexpr
		tuple_storage(tuple_storage &&) noexcept =default;

		template<typename Arg, typename... Args>
		constexpr
		tuple_storage(Arg && arg, Args &&... args) : base_type{std::forward<Args>(args)...}, value{std::forward<Arg>(arg)} {}

		constexpr
		auto operator=(const tuple_storage &) -> tuple_storage & =default;
		constexpr
		auto operator=(tuple_storage &&) noexcept -> tuple_storage & =default;

		~tuple_storage() noexcept =default;

		template<std::size_t Index>
		constexpr
		auto get() const noexcept -> decltype(auto) {
			if constexpr(Index == 0) return (value);
			else return static_cast<const base_type &>(*this).template get<Index - 1>();
		}

		template<std::size_t Index>
		constexpr
		auto get()       noexcept -> decltype(auto) {
			if constexpr(Index == 0) return (value);
			else return static_cast<      base_type &>(*this).template get<Index - 1>();
		}

		constexpr
		void swap(tuple_storage & other) noexcept {
			internal::adl_swap(value, other.value);
			base_type::swap(other);
		}

		friend
		constexpr
		auto operator==(const tuple_storage & lhs, const tuple_storage & rhs) noexcept { return (lhs.value != rhs.value) ? false : static_cast<const base_type &>(lhs) == static_cast<const base_type &>(rhs); }
		friend
		constexpr
		auto operator< (const tuple_storage & lhs, const tuple_storage & rhs) noexcept { return (lhs.value < rhs.value) || (!(rhs.value < lhs.value) && static_cast<const base_type &>(lhs) < static_cast<const base_type &>(rhs)); }
	};
	PTL_PACK_END
}
