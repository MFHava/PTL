
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include <cassert>
#include <cstddef>
#include <cstring>
#include <ostream>
#include <utility>
#include <iterator>
#include <stdexcept>

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a readonly, non-owning reference to a null-terminated string
	struct string_ref final {//TODO: evaluate differences to the standard!  
		using value_type             = const char;
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

		string_ref() =default;
		string_ref(const string_ref &) =default;
		string_ref(string_ref &&) noexcept =default;
		auto operator=(const string_ref &) -> string_ref & =default;
		auto operator=(string_ref &&) noexcept -> string_ref & =default;
		~string_ref() noexcept =default;

		//! @brief construct string_ref from c-string
		//! @param[in] ptr string to reference
		string_ref(const_pointer ptr) noexcept : first{ptr}, last{ptr + std::strlen(ptr)} {}

		auto operator[](size_type index)       noexcept ->       reference { assert(!empty()); return first[index]; }
		auto operator[](size_type index) const noexcept -> const_reference { assert(!empty()); return first[index]; }

		auto at(size_type index)       ->       reference { return validate_index(index), (*this)[index]; }
		auto at(size_type index) const -> const_reference { return validate_index(index), (*this)[index]; }

		auto size()  const noexcept -> size_type { return last - first; }
		auto empty() const noexcept -> bool { return size() == 0; }

		auto data()       noexcept ->       pointer { return first; }
		auto data() const noexcept -> const_pointer { return first; }

		auto c_str()       noexcept ->       pointer { return first; }
		auto c_str() const noexcept -> const_pointer { return first; }

		auto begin()        noexcept ->       iterator { return first; }
		auto begin()  const noexcept -> const_iterator { return first; }
		auto cbegin() const noexcept -> const_iterator { return first; }
		auto end()          noexcept ->       iterator { return last; }
		auto end()    const noexcept -> const_iterator { return last; }
		auto cend()   const noexcept -> const_iterator { return last; }

		auto rbegin()        noexcept ->       reverse_iterator { return       reverse_iterator{end()}; }
		auto rbegin()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }
		auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cend()}; }
		auto rend()          noexcept ->       reverse_iterator { return       reverse_iterator{begin()}; }
		auto rend()    const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }
		auto crend()   const noexcept -> const_reverse_iterator { return const_reverse_iterator{cbegin()}; }

		friend
		void swap(string_ref & lhs, string_ref & rhs) noexcept {
			using std::swap;
			swap(lhs.first, rhs.first);
			swap(lhs.last,  rhs.last);
		}

		friend
		auto operator==(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs.c_str()) == 0; }

		friend
		auto operator!=(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs.c_str()) != 0; }

		friend
		auto operator< (const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs.c_str()) <  0; }

		friend
		auto operator<=(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs.c_str()) <= 0; }

		friend
		auto operator> (const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs.c_str()) >  0; }

		friend
		auto operator>=(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs.c_str()) >= 0; }

		friend
		auto operator==(const_pointer lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs, rhs.c_str()) == 0; }

		friend
		auto operator!=(const_pointer lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs, rhs.c_str()) != 0; }

		friend
		auto operator< (const_pointer lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs, rhs.c_str()) <  0; }

		friend
		auto operator<=(const_pointer lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs, rhs.c_str()) <= 0; }

		friend
		auto operator> (const_pointer lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs, rhs.c_str()) >  0; }

		friend
		auto operator>=(const_pointer lhs, const string_ref & rhs) noexcept -> bool { return std::strcmp(lhs, rhs.c_str()) >= 0; }

		friend
		auto operator==(const string_ref & lhs, const_pointer rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs) == 0; }

		friend
		auto operator!=(const string_ref & lhs, const_pointer rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs) != 0; }

		friend
		auto operator< (const string_ref & lhs, const_pointer rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs) <  0; }

		friend
		auto operator<=(const string_ref & lhs, const_pointer rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs) <= 0; }

		friend
		auto operator> (const string_ref & lhs, const_pointer rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs) >  0; }

		friend
		auto operator>=(const string_ref & lhs, const_pointer rhs) noexcept -> bool { return std::strcmp(lhs.c_str(), rhs) >= 0; }

		friend
		auto operator<<(std::ostream & os, const string_ref & self) -> std::ostream & { return os << self.c_str(); }
	private:
		void validate_index(size_type index) const { if(index >= size()) throw std::out_of_range{"index out of range"}; }

		pointer first{nullptr}, last{nullptr};
	};
	PTL_PACK_END

	namespace literals {
		inline
		auto operator""_sr(const char * ptr, std::size_t) noexcept -> string_ref { return {ptr}; }
	}
}
