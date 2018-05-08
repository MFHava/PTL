
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <string>
#include <ostream>
#include <functional>
#include <boost/functional/hash.hpp>
#include "internal/compiler_detection.hpp"
#include "internal/contiguous_container_base.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a read-only, non-owning reference to a string
	//! @tparam Char character type referenced
	//! @attention the referenced string is not necessarily null-terminated!
	template<typename Char>
	class basic_string_ref final : public internal::contiguous_container_base<basic_string_ref<Char>, const Char, boost::totally_ordered2<basic_string_ref<Char>, const Char *>> {
		using base_type = internal::contiguous_container_base<basic_string_ref<Char>, const Char, boost::totally_ordered2<basic_string_ref<Char>, const Char *>>;

		struct c_str_iterator final : boost::input_iterator_helper<c_str_iterator, Char> {
			constexpr
			c_str_iterator() noexcept {}
			constexpr
			c_str_iterator(const Char * ptr) noexcept : ptr{ptr} {}

			constexpr
			decltype(auto) operator++() noexcept {
				if(ptr && !*ptr++) ptr = nullptr;
				return *this;
			}

			constexpr
			auto operator*() const noexcept -> const Char & {
				PTL_REQUIRES(ptr);
				return *ptr;
			}

			//input_iterators can also act as a range
			constexpr
			auto begin() const noexcept { return *this; }
			constexpr
			auto end() const noexcept { return c_str_iterator{}; }

			friend
			constexpr
			auto operator==(const c_str_iterator & lhs, const c_str_iterator & rhs) noexcept { return lhs.ptr == rhs.ptr; }
		private:
			const Char * ptr{nullptr};
		};
		BOOST_CONCEPT_ASSERT((boost::InputIterator<c_str_iterator>));

		const Char * first{nullptr}, * last{nullptr};
	public:
		constexpr
		basic_string_ref() noexcept =default;

		//! @brief construct basic_string_ref from c-string
		//! @param[in] str null-terminated string
		constexpr
		basic_string_ref(const Char * str) noexcept : first{str}, last{first} { if(last) while(*last) ++last; }

		//! @brief construct basic_string_ref from c-string + length
		//! @param[in] str string to reference
		//! @param[in] size size of string to reference
		//! @attention [str, size) must be a valid range!
		constexpr
		basic_string_ref(const Char * str, std::size_t size) noexcept : first{str}, last{str + size} { PTL_REQUIRES(str || (!str && !size)); }

		//! @brief construct basic_string_ref from string
		//! @param[in] str string to reference
		basic_string_ref(const std::string & str) noexcept : basic_string_ref{str.data(), str.size()} {}

		constexpr
		basic_string_ref(const basic_string_ref &) noexcept =default;

		constexpr
		auto operator=(const basic_string_ref &) noexcept -> basic_string_ref & =default;

		constexpr
		auto data() const noexcept -> const Char * { return first; }
		constexpr
		auto size() const noexcept -> std::size_t { return last - first; }
		constexpr
		auto max_size() const noexcept { return std::numeric_limits<std::size_t>::max(); }

		constexpr
		void remove_prefix(typename base_type::const_iterator pos) noexcept {
			PTL_REQUIRES(pos.ptr >= first && pos.ptr <= last);
			first = pos.ptr;
		}

		constexpr
		void remove_suffix(typename base_type::const_iterator pos) noexcept {
			PTL_REQUIRES(pos.ptr >= first && pos.ptr <= last);
			last = pos.ptr;
		}

		constexpr
		auto substr(typename base_type::const_iterator first, typename base_type::const_iterator last) const noexcept {
			auto result{*this};
			result.remove_prefix(first);
			result.remove_suffix(last);
			return result;
		}

		constexpr
		void swap(basic_string_ref & other) noexcept {
			using std::swap;
			swap(first, other.first);
			swap(last,  other.last);
		}

		friend
		constexpr
		auto operator==(const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept { return base_type::equal(lhs, rhs); }
		friend
		constexpr
		auto operator< (const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept { return base_type::less(lhs, rhs); }

		friend
		constexpr
		auto operator==(const Char * lhs, const basic_string_ref & rhs) noexcept { return base_type::equal(c_str_iterator{lhs}, rhs); }
		friend
		constexpr
		auto operator< (const Char * lhs, const basic_string_ref & rhs) noexcept { return base_type::less(c_str_iterator{lhs}, rhs); }

		friend
		decltype(auto) operator<<(std::ostream & os, const basic_string_ref & self) {
			for(auto & tmp : self) os << tmp;
			return os;
		}

		operator std::string() const { return {this->begin(), this->end()}; }
	};
	PTL_PACK_END

	using string_ref = basic_string_ref<char>;
	BOOST_CONCEPT_ASSERT((boost::RandomAccessContainer<string_ref>));
	static_assert(sizeof(string_ref)                 == 2 * sizeof(const char *), "invalid size of string_ref detected");
	static_assert(sizeof(string_ref::iterator)       ==     sizeof(const char *), "invalid size of string_ref::iterator detected");
	static_assert(sizeof(string_ref::const_iterator) ==     sizeof(const char *), "invalid size of string_ref::const_iterator detected");

	namespace literals {
		inline
		constexpr
		auto operator""_sr(const char * ptr, std::size_t) noexcept { return string_ref{ptr}; }
	}
}

namespace std {
	template<typename Char>
	struct hash<ptl::basic_string_ref<Char>> final {
		auto operator()(const ptl::basic_string_ref<Char> & self) const noexcept -> std::size_t { return boost::hash_range(self.data(),  self.data() + self.size()); }
	};
}
