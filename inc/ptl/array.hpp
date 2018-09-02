
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/array.hpp"
#include "internal/adl_swap.hpp"
#include "internal/compiler_detection.hpp"
#include "internal/contiguous_container_base.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a fixed-size array
	//! @tparam Type type of the stored array
	//! @tparam Size size of the stored array
	template<typename Type, std::size_t Size>
	class array final : public internal::contiguous_container_base<array<Type, Size>, Type> {
		using base_type = internal::contiguous_container_base<array<Type, Size>, Type>;

		internal::array_storage_t<Type, Size> values;
	public:
		template<typename... Args, typename = std::enable_if_t<internal::are_convertible_v<Type, Args...>>>
		constexpr
		array(Args &&... args) : values{std::forward<Args>(args)...} {}

		constexpr
		array(const array &) =default;

		constexpr
		auto operator=(const array &) -> array & =default;

		~array() noexcept =default;

		constexpr
		auto data() const noexcept -> const Type * { return values; }
		constexpr
		auto data()       noexcept ->       Type * { return values; }
		constexpr
		auto size() const noexcept { return Size; }
		constexpr
		auto max_size() const noexcept { return Size; }

		constexpr
		void fill(const Type & value) { for(auto it{base_type::begin()}, end{base_type::end()}; it != end; ++it) *it = value; }

		constexpr
		void swap(array & other) noexcept { for(auto it1{base_type::begin()}, it2{other.begin()}, end{base_type::end()}; it1 != end;) internal::adl_swap(*it1++, *it2++); }

		friend
		constexpr
		auto operator==(const array & lhs, const array & rhs) noexcept { return base_type::equal(lhs, rhs); }
		friend
		constexpr
		auto operator< (const array & lhs, const array & rhs) noexcept { return base_type::less(lhs, rhs); }
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	array(Type, Types...) -> array<Type, 1 + sizeof...(Types)>;

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	decltype(auto) get(const array<Type, Size> & self) noexcept {
		static_assert(Index < Size);
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	decltype(auto) get(      array<Type, Size> & self) noexcept {
		static_assert(Index < Size);
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	decltype(auto) get(const array<Type, Size> && self) noexcept { return std::move(get<Index>(self)); }

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	decltype(auto) get(      array<Type, Size> && self) noexcept { return std::move(get<Index>(self)); }
}

namespace std {
	template<typename Type, std::size_t Size>
	struct tuple_size<ptl::array<Type, Size>> : std::integral_constant<std::size_t, Size> {};

	template<std::size_t Index, typename Type, std::size_t Size>
	struct tuple_element<Index, ptl::array<Type, Size>> {
		using type = Type;
	};
}
