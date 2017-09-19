
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include "internal/operators.hpp"
#include <string>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <ostream>
#include <utility>
#include <iterator>
#include <stdexcept>

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a readonly, non-owning reference to a NUL-terminated string
	class string_ref final :
		internal::comparison1<string_ref,
			internal::comparison2_symmetric<string_ref, char *>
		>
	{
		const char * first{nullptr}, * last{nullptr};
	public:
		string_ref() =default;

		//! @brief construct string_ref from c-string
		//! @param[in] ptr string to reference
		string_ref(const char * ptr) noexcept : first{ptr}, last{first + std::strlen(ptr)} {}

		//! @brief construct string_ref from string
		//! @param[in] str string to reference
		string_ref(const std::string & str) noexcept : first{str.data()}, last{first + str.size()} {}

		auto operator[](std::size_t index) const noexcept { return first[index]; }
		auto at(std::size_t index) const {
			if(index >= size()) throw std::out_of_range{"index out of range"}; 
			return (*this)[index];
		}

		auto size() const noexcept -> std::size_t { return last - first; }
		auto empty() const noexcept { return size() == 0; }

		auto data() const noexcept { return first; }
		auto c_str() const noexcept { return first; }

		auto begin() const noexcept { return first; }
		auto end() const noexcept { return last; }

		auto rbegin() const noexcept { return std::make_reverse_iterator(end()); }
		auto rend() const noexcept { return std::make_reverse_iterator(begin()); }

		friend
		void swap(string_ref & lhs, string_ref & rhs) noexcept {
			using std::swap;
			swap(lhs.first, rhs.first);
			swap(lhs.last, rhs.last);
		}

		friend
		auto operator==(const string_ref & lhs, const string_ref & rhs) noexcept { return std::strcmp(lhs.c_str(), rhs.c_str()) == 0; }
		friend
		auto operator< (const string_ref & lhs, const string_ref & rhs) noexcept { return std::strcmp(lhs.c_str(), rhs.c_str()) <  0; }

		friend
		auto operator==(const char * lhs, const string_ref & rhs) noexcept { return std::strcmp(lhs, rhs.c_str()) == 0; }
		friend
		auto operator< (const char * lhs, const string_ref & rhs) noexcept { return std::strcmp(lhs, rhs.c_str()) <  0; }

		friend
		auto operator==(const string_ref & lhs, const char * rhs) noexcept { return std::strcmp(lhs.c_str(), rhs) == 0; }
		friend
		auto operator< (const string_ref & lhs, const char * rhs) noexcept { return std::strcmp(lhs.c_str(), rhs) <  0; }

		friend
		decltype(auto) operator<<(std::ostream & os, const string_ref & self) {
			for(auto & tmp : self) os << tmp;
			return os;
		}
	};
	PTL_PACK_END
	static_assert(sizeof(string_ref) == 2 * sizeof(const char *), "invalid size of string_ref detected");

	namespace literals {
		inline
		auto operator""_sr(const char * ptr, std::size_t) noexcept -> string_ref { return {ptr}; }
	}
}