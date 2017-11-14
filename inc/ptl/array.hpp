
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/array.hpp"
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
		BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<typename base_type::iterator>));

		typename internal::array_storage<Type, Size>::type values;
	public:
		template<typename... Args>
		constexpr
		array(Args &&... args) : values{std::forward<Args>(args)...} {}

		constexpr
		array(const array &) =default;
		constexpr
		array(array && other) noexcept { swap(*this, other); }
		constexpr
		auto operator=(const array &) -> array & =default;
		constexpr
		auto operator=(array && other) noexcept -> array & { swap(*this, other); return *this; }

		constexpr
		auto data() const noexcept -> const Type * { return values; }
		constexpr
		auto data()       noexcept ->       Type * { return values; }
		constexpr
		auto size() const noexcept { return Size; }
		constexpr
		auto max_size() const noexcept { return Size; }

		constexpr
		void fill(const Type & value) { for(auto & v : values) v = value; }

		constexpr
		void swap(array & other) noexcept { for(auto it1{this->begin()}, it2{other.begin()}; it1 != this->end(); ++it1, ++it2) std::iter_swap(it1, it2); }

		friend
		constexpr
		auto operator==(const array & lhs, const array & rhs) noexcept { return base_type::equal(lhs, rhs); }
		friend
		constexpr
		auto operator< (const array & lhs, const array & rhs) noexcept { return base_type::less(lhs, rhs); }
	};
	PTL_PACK_END

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	decltype(auto) get(const array<Type, Size> & self) noexcept {
		static_assert(Index < Size, "index out of range");
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	decltype(auto) get(      array<Type, Size> & self) noexcept {
		static_assert(Index < Size, "index out of range");
		return self[Index];
	}

	//TODO: arrays of size 0 are explicitly allowed (http://en.cppreference.com/w/cpp/container/array)
	//TODO: r-value versions of get (http://en.cppreference.com/w/cpp/container/array/get)
	//TODO: tuple_size (http://en.cppreference.com/w/cpp/container/array/tuple_size)
	//TODO: tuple_element (http://en.cppreference.com/w/cpp/container/array/tuple_element)
	//TODO: make_array (http://en.cppreference.com/w/cpp/experimental/make_array)
	//TODO: to_array (http://en.cppreference.com/w/cpp/experimental/to_array)
	//TODO: deduction guides (http://en.cppreference.com/w/cpp/container/array/deduction_guides)
}