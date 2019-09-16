
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/tuple.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a fixed-size collection of heterogeneous value
	//! @tparam Types types to store inside the tuple
	template<typename... Types>
	class tuple final {
		internal::tuple_storage<Types...> storage;

		template<std::size_t Index, typename... Ts>
		friend
		constexpr
		auto get(const tuple<Ts...> &) noexcept -> decltype(auto);

		template<std::size_t Index, typename... Ts>
		friend
		constexpr
		auto get(      tuple<Ts...> &) noexcept -> decltype(auto);
	public:
		template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == sizeof...(Types) || sizeof...(Args) == 0>>
		constexpr
		tuple(Args &&... args) : storage{std::forward<Args>(args)...} {}

		constexpr
		tuple(const tuple &) =default;
		constexpr
		tuple(tuple &&) noexcept =default;

		constexpr
		auto operator=(const tuple &) -> tuple & =default;
		constexpr
		auto operator=(tuple &&) noexcept -> tuple & =default;

		~tuple() noexcept =default;

		constexpr
		void swap(tuple & other) noexcept { storage.swap(other.storage); }
		friend
		constexpr
		void swap(tuple & lhs, tuple & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const tuple & lhs, const tuple & rhs) noexcept { return lhs.storage == rhs.storage; }
		friend
		constexpr
		auto operator!=(const tuple & lhs, const tuple & rhs) noexcept { return !(lhs == rhs); }
		friend
		constexpr
		auto operator< (const tuple & lhs, const tuple & rhs) noexcept { return lhs.storage <  rhs.storage; }
		friend
		constexpr
		auto operator> (const tuple & lhs, const tuple & rhs) noexcept { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const tuple & lhs, const tuple & rhs) noexcept { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const tuple & lhs, const tuple & rhs) noexcept { return !(lhs < rhs); }
	};
	PTL_PACK_END

	template<typename... Types>
	tuple(Types...) -> tuple<Types...>;

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(const tuple<Types...> & self) noexcept -> decltype(auto) { return self.storage.template get<Index>(); }

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(      tuple<Types...> & self) noexcept -> decltype(auto) { return self.storage.template get<Index>(); }

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(const tuple<Types...> && self) noexcept -> decltype(auto) { return std::move(get<Index>(self)); }

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(      tuple<Types...> && self) noexcept -> decltype(auto) { return std::move(get<Index>(self)); }
}

namespace std {
	template<typename... Types>
	struct tuple_size<ptl::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

	template<std::size_t Index, typename Head, typename... Tail>
	struct tuple_element<Index, ptl::tuple<Head, Tail...>> : tuple_element<Index - 1, ptl::tuple<Tail...>> {};

	template<typename Head, typename... Tail>
	struct tuple_element<0, ptl::tuple<Head, Tail...>> { using type = Head; };
}
