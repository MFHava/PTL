
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
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

		template<typename T>
		struct storage_type final { using type = T; };

		static
		constexpr //TODO(C++20): consteval
		auto determine_storage() noexcept {
			if constexpr(Size == 0) return storage_type<Type *>{};
			else                    return storage_type<Type[Size]>{};
		}

		typename decltype(determine_storage())::type values;
	public:
		template<typename... Args, typename = std::enable_if_t<(std::is_convertible_v<Type, Args> &&...)>>
		constexpr
		array(Args &&... args) : values{std::forward<Args>(args)...} {}

		constexpr
		auto data() const noexcept -> const Type * { return values; }
		constexpr
		auto data()       noexcept ->       Type * { return values; }
		static
		constexpr
		auto size() noexcept { return Size; }
		static
		constexpr
		auto empty() noexcept { return size() == 0; }
		static
		constexpr
		auto max_size() noexcept { return Size; }

		constexpr
		void fill(const Type & value) { std::fill(base_type::begin(), base_type::end(), value); }

		constexpr
		void swap(array & other) noexcept { std::swap_ranges(base_type::begin(), base_type::end(), other.begin()); }
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	array(Type, Types...) -> array<Type, 1 + sizeof...(Types)>;

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	auto get(const array<Type, Size> & self) noexcept -> const Type & {
		static_assert(Index < Size);
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	auto get(      array<Type, Size> & self) noexcept ->       Type & {
		static_assert(Index < Size);
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	auto get(const array<Type, Size> && self) noexcept -> const Type && { return std::move(get<Index>(self)); }

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	auto get(      array<Type, Size> && self) noexcept ->       Type && { return std::move(get<Index>(self)); }
}

namespace std {
	template<typename Type, std::size_t Size>
	struct tuple_size<ptl::array<Type, Size>> : std::integral_constant<std::size_t, Size> {};

	template<std::size_t Index, typename Type, std::size_t Size>
	struct tuple_element<Index, ptl::array<Type, Size>> { using type = Type; };
}
