
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/utility.hpp"
#include "internal/type_checks.hpp"
#include <cstddef>
#include <ostream>
#include <utility>
#include <iterator>
#include <stdexcept>
#include <boost/concept_check.hpp>

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a fixed-size array
	//! @tparam Type type of the stored array
	//! @tparam Size size of the stored array
	template<typename Type, std::size_t Size>
	class array final : public internal::random_access_container_base<array<Type, Size>, boost::totally_ordered1<array<Type, Size>>> {
		Type values[Size];
	public:
		static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");

		using value_type             = Type;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       value_type &;
		using const_reference        = const value_type &;
		using pointer                =       value_type *;
		using const_pointer          = const value_type *;
		using iterator               = internal::random_access_iterator<array<Type, Size>,       Type>;
		BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<iterator>));
		using const_iterator         = internal::random_access_iterator<array<Type, Size>, const Type>;
		BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<const_iterator>));
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
		auto operator[](size_type index) const noexcept -> const_reference {
			PTL_REQUIRES(index < size());
			return values[index];
		}
		constexpr
		auto operator[](size_type index)       noexcept ->       reference {
			PTL_REQUIRES(index < size());
			return values[index];
		}

		constexpr
		auto data() const noexcept -> const_pointer { return values; }
		constexpr
		auto data()       noexcept ->       pointer { return values; }

		constexpr
		auto begin() const noexcept { return const_iterator{values}; }
		constexpr
		auto begin()       noexcept { return iterator{values}; }
		constexpr
		auto end()   const noexcept { return const_iterator{values + Size}; }
		constexpr
		auto end()         noexcept { return iterator{values + Size}; }

		constexpr
		auto size() const noexcept { return Size; }
		constexpr
		auto max_size() const noexcept { return Size; }

		constexpr
		void fill(const_reference value) { for(auto & v : values) v = value; }

		constexpr
		void swap(array & other) noexcept {
			using std::swap;
			for(size_type i{0}; i < Size; ++i)
				swap(values[i], other.values[i]);
		}
		friend
		constexpr
		void swap(array & lhs, array & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const array & lhs, const array & rhs) noexcept {
			for(size_type i{0}; i < Size; ++i)
				if(lhs[i] != rhs[i])
					return false;
			return true;
		}
		friend
		constexpr
		auto operator< (const array & lhs, const array & rhs) noexcept {
			for(size_type i{0}; i < Size; ++i)
				if(!(lhs[i] < rhs[i]))
					return false;
			return true;
		}

		friend
		decltype(auto) operator<<(std::ostream & os, const array & self) {
			os << '[';
			if(!self.empty()) {
				auto it{self.begin()};
				os << *it;
				for(++it; it != self.end(); ++it) os << ", " << *it;
			}
			return os << ']';
		}
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
}
