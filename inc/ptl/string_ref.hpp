
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ostream>
#include <functional>
#include <string_view>
#include "internal/adl_swap.hpp"
#include "internal/compiler_detection.hpp"
#include "internal/contiguous_container_base.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a read-only, non-owning reference to a string
	//! @attention the referenced string is not necessarily null-terminated!
	class string_ref final : public internal::contiguous_container_base<string_ref, const char> {
		const_pointer first{nullptr}, last{nullptr};
	public:
		constexpr
		string_ref() noexcept =default;

		//! @brief construct string_ref from c-string
		//! @param[in] str null-terminated string
		constexpr
		string_ref(const_pointer str) noexcept : first{str}, last{first} { if(last) while(*last) ++last; }

		//! @brief construct string_ref from c-string + length
		//! @param[in] str string to reference
		//! @param[in] size size of string to reference
		//! @attention [str, size) must be a valid range!
		constexpr
		string_ref(const_pointer str, size_type size) noexcept : first{str}, last{str + size} { PTL_REQUIRES(str || (!str && !size)); }

		//! @brief construct string_ref from std::string_view
		//! @param[in] str string to reference
		string_ref(const std::string_view & str) noexcept : string_ref{str.data(), str.size()} {}

		constexpr
		string_ref(const string_ref &) noexcept =default;

		constexpr
		auto operator=(const string_ref &) noexcept -> string_ref & =default;

		~string_ref() noexcept =default;

		constexpr
		auto data() const noexcept -> const_pointer { return first; }
		constexpr
		auto size() const noexcept -> size_type { return last - first; }
		constexpr
		auto max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

		constexpr
		void swap(string_ref & other) noexcept {
			internal::adl_swap(first, other.first);
			internal::adl_swap(last,  other.last);
		}

		friend
		auto operator<<(std::ostream & os, const string_ref & self) -> std::ostream & { return os << static_cast<std::string_view>(self); }

		operator std::string_view() const { return {this->data(), this->size()}; }
	};
	PTL_PACK_END

	BOOST_CONCEPT_ASSERT((boost::RandomAccessContainer<string_ref>));
	static_assert(sizeof(string_ref) == 2 * sizeof(const char *));

	namespace literals {
		inline
		constexpr
		auto operator""_sr(const char * ptr, std::size_t) noexcept { return string_ref{ptr}; }
	}
}

namespace std {
	template<>
	struct hash<ptl::string_ref> final {
		auto operator()(const ptl::string_ref & self) const noexcept -> std::size_t { return std::hash<std::string_view>{}(self); }
	};
}
