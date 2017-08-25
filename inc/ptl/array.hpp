
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include <cassert>
#include <cstddef>
#include <ostream>
#include <utility>
#include <iterator>
#include <stdexcept>

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a fixed-size array
	//! @tparam Type type of the stored array
	//! @tparam Size size of the stored array
	template<typename Type, std::size_t Size>
	struct array final {//TODO: evaluate differences to the standard!  
		static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");

		using value_type             = Type;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       value_type &;
		using const_reference        = const value_type &;
		using pointer                =       value_type *;
		using const_pointer          = const value_type *;
		using iterator               =       value_type *;
		using const_iterator         = const value_type *;
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		template<typename... Args>
		PTL_CXX_RELAXED_CONSTEXPR
		array(Args &&... args) : values{std::forward<Args>(args)...} {}

		array(const array &) =default;
		array(array && other) noexcept { swap(*this, other); }
		auto operator=(const array &) -> array & =default;
		auto operator=(array && other) noexcept -> array & { swap(*this, other); return *this; }
		~array() noexcept =default;

		PTL_CXX_RELAXED_CONSTEXPR
		auto operator[](size_type index)       noexcept ->       reference { assert(Size); return values[index]; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto operator[](size_type index) const noexcept -> const_reference { assert(Size); return values[index]; }

		PTL_CXX_RELAXED_CONSTEXPR
		auto at(size_type index)       ->       reference { return validate_index(index), (*this)[index]; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto at(size_type index) const -> const_reference { return validate_index(index), (*this)[index]; }

		PTL_CXX_RELAXED_CONSTEXPR
		auto size()  const noexcept -> size_type { return Size; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto empty() const noexcept -> bool { return size() == 0; }

		PTL_CXX_RELAXED_CONSTEXPR
		auto data()       noexcept ->       pointer { return values; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto data() const noexcept -> const_pointer { return values; }

		PTL_CXX_RELAXED_CONSTEXPR
		auto begin()        noexcept ->       iterator { return values; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto begin()  const noexcept -> const_iterator { return values; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto cbegin() const noexcept -> const_iterator { return values; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto end()          noexcept ->       iterator { return values + Size; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto end()    const noexcept -> const_iterator { return values + Size; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto cend()   const noexcept -> const_iterator { return values + Size; }

		PTL_CXX_RELAXED_CONSTEXPR
		auto rbegin()        noexcept ->       reverse_iterator { return reverse_iterator{end()}; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto rbegin()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cend()}; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto rend()          noexcept ->       reverse_iterator { return reverse_iterator{begin()}; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto rend()    const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto crend()   const noexcept -> const_reverse_iterator { return const_reverse_iterator{cbegin()}; }

		void fill(const_reference value) { for(auto & v : values) v = value; }

		friend
		void swap(array & lhs, array & rhs) noexcept {
			using std::swap;
			for(size_type i{0}; i < Size; ++i)
				swap(lhs.values[i], rhs.values[i]);
		}

		friend
		auto operator==(const array & lhs, const array & rhs) noexcept -> bool {
			for(size_type i{0}; i < Size; ++i)
				if(lhs[i] != rhs[i])
					return false;
			return true;
		}

		friend
		auto operator!=(const array & lhs, const array & rhs) noexcept -> bool { return !(lhs == rhs); }

		friend
		auto operator< (const array & lhs, const array & rhs) noexcept -> bool {
			for(size_type i{0}; i < Size; ++i)
				if(!(lhs[i] < rhs[i]))
					return false;
			return true;
		}

		friend
		auto operator<=(const array & lhs, const array & rhs) noexcept -> bool { return !(rhs < lhs); }

		friend
		auto operator> (const array & lhs, const array & rhs) noexcept -> bool { return rhs < lhs; }

		friend
		auto operator>=(const array & lhs, const array & rhs) noexcept -> bool { return !(lhs < rhs); }

		friend
		auto operator<<(std::ostream & os, const array & self) -> std::ostream & {
			os << '[';
			if(!self.empty()) {
				auto it{self.begin()};
				os << *it;
				for(++it; it != self.end(); ++it) os << ", " << *it;
			}
			return os << ']';
		}
	private:
		PTL_CXX_RELAXED_CONSTEXPR
		void validate_index(size_type index) const { if(index >= size()) throw std::out_of_range{"index out of range"}; }

		value_type values[Size];
	};
	PTL_PACK_END

	template<std::size_t Index, typename Type, std::size_t Size>
	PTL_CXX_RELAXED_CONSTEXPR
	auto get(      array<Type, Size> & self) noexcept -> typename array<Type, Size>::      reference {
		static_assert(Index < Size, "index out of range");
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	PTL_CXX_RELAXED_CONSTEXPR
	auto get(const array<Type, Size> & self) noexcept -> typename array<Type, Size>::const_reference {
		static_assert(Index < Size, "index out of range");
		return self[Index];
	}
}
