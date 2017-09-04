
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
		constexpr
		array(Args &&... args) : values{std::forward<Args>(args)...} {}

		array(const array &) =default;
		array(array && other) noexcept { swap(*this, other); }
		auto operator=(const array &) -> array & =default;
		auto operator=(array && other) noexcept -> array & { swap(*this, other); return *this; }
		~array() noexcept =default;

		constexpr
		auto at(size_type index) const -> const_reference { return validate_index(index), (*this)[index]; }
		constexpr
		auto at(size_type index)       ->       reference { return validate_index(index), (*this)[index]; }

		constexpr
		auto operator[](size_type index) const noexcept -> const_reference { assert(index < Size); return values[index]; }
		constexpr
		auto operator[](size_type index)       noexcept ->       reference { assert(index < Size); return values[index]; }

		constexpr
		auto front() const -> const_reference { return (*this)[0]; }
		constexpr
		auto front()       ->       reference { return (*this)[0]; }

		constexpr
		auto back() const -> const_reference { return (*this)[Size - 1]; }
		constexpr
		auto back()       ->       reference { return (*this)[Size - 1]; }

		constexpr
		auto data() const noexcept -> const_pointer { return values; }
		constexpr
		auto data()       noexcept ->       pointer { return values; }

		constexpr
		auto begin()  const noexcept -> const_iterator { return values; }
		constexpr
		auto begin()        noexcept ->       iterator { return values; }
		constexpr
		auto cbegin() const noexcept -> const_iterator { return values; }

		constexpr
		auto end()  const noexcept -> const_iterator { return values + Size; }
		constexpr
		auto end()        noexcept ->       iterator { return values + Size; }
		constexpr
		auto cend() const noexcept -> const_iterator { return values + Size; }

		constexpr
		auto rbegin()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }
		constexpr
		auto rbegin()        noexcept ->       reverse_iterator { return reverse_iterator{end()}; }
		constexpr
		auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cend()}; }

		constexpr
		auto rend()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }
		constexpr
		auto rend()        noexcept ->       reverse_iterator { return reverse_iterator{begin()}; }
		constexpr
		auto crend() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cbegin()}; }

		constexpr
		auto empty() const noexcept -> bool { return size() == 0; }
		constexpr
		auto size() const noexcept -> size_type { return Size; }
		constexpr
		auto max_size() const noexcept -> size_type { return Size; }

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
		constexpr
		void validate_index(size_type index) const { if(index >= size()) throw std::out_of_range{"index out of range"}; }

		value_type values[Size];
	};
	PTL_PACK_END

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	auto get(const array<Type, Size> & self) noexcept -> typename array<Type, Size>::const_reference {
		static_assert(Index < Size, "index out of range");
		return self[Index];
	}

	template<std::size_t Index, typename Type, std::size_t Size>
	constexpr
	auto get(      array<Type, Size> & self) noexcept -> typename array<Type, Size>::      reference {
		static_assert(Index < Size, "index out of range");
		return self[Index];
	}
}