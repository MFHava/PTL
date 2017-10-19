
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/utility.hpp"
#include "internal/type_checks.hpp"
#include <string>
#include <ostream>
#include <stdexcept>
#include <functional>
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>
#include <boost/functional/hash.hpp>

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a read-only, non-owning reference to a string
	//! @tparam Char character type referenced
	//! @attention the referenced string is not necessarily null-terminated!
	template<typename Char>
	class basic_string_ref final :
		boost::totally_ordered1<basic_string_ref<Char>,
			boost::totally_ordered2<basic_string_ref<Char>, const Char *>
		>
	{
		struct c_str_iterator final : boost::input_iterator_helper<c_str_iterator, Char> {
			constexpr
			c_str_iterator() {}
			constexpr
			c_str_iterator(const Char * ptr) : ptr{ptr} {}

			constexpr
			decltype(auto) operator++() noexcept {
				if(ptr && !*ptr++) ptr = nullptr;
				return *this;
			}

			constexpr
			auto operator*() const -> const Char & {
				PTL_REQUIRES(ptr);
				return *ptr;
			}

			friend
			constexpr
			auto operator==(const c_str_iterator & lhs, const c_str_iterator & rhs) { return lhs.ptr == rhs.ptr; }
		private:
			const Char * ptr{nullptr};
		};
		BOOST_CONCEPT_ASSERT((boost::InputIterator<c_str_iterator>));

		template<typename InputIterator1, typename InputIterator2>
		constexpr
		static auto compare(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2) noexcept -> int {
			std::tie(first1, first2) = std::mismatch(first1, last1, first2, last2);
			if(first1 == last1) {
				if(first2 == last2) return 0;
				else return -1;
			} else {
				if(first2 == last2) return +1;
				else return *first1 - *first2;
			}
		}

		const Char * first{nullptr}, * last{nullptr};
	public:
		using value_type             = Char;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using pointer                =       Char *;
		using const_pointer          = const Char *;
		using reference              =       Char &;
		using const_reference        = const Char &;
		struct iterator final : public boost::random_access_iterator_helper<iterator, Char, std::ptrdiff_t, const Char *, const Char &> {
			constexpr
			iterator() noexcept {}

			constexpr
			decltype(auto) operator++() noexcept { move(+1); return *this; }
			constexpr
			decltype(auto) operator--() noexcept { move(-1); return *this; }
			
			constexpr
			auto operator*() const -> const Char & {
				PTL_REQUIRES(ptr);
				return *ptr;
			}
			
			constexpr
			decltype(auto) operator+=(std::ptrdiff_t count) { move(+count); return *this; }
			constexpr
			decltype(auto) operator-=(std::ptrdiff_t count) { move(-count); return *this; }
			
			friend
			constexpr
			auto operator-(const iterator & lhs, const iterator & rhs) -> std::ptrdiff_t { return lhs.ptr - rhs.ptr; }
			friend
			constexpr
			auto operator==(const iterator & lhs, const iterator & rhs) { return lhs.ptr == rhs.ptr; }
			friend
			constexpr
			auto operator< (const iterator & lhs, const iterator & rhs) { return lhs.ptr <  rhs.ptr; }
		private:
			friend class basic_string_ref<Char>;

			constexpr
			void move(std::ptrdiff_t count) {
				PTL_REQUIRES(ptr);
				ptr += count;
			}

			explicit
			constexpr
			iterator(const Char * ptr) : ptr{ptr} {}

			const Char * ptr{nullptr};
		};
		BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<iterator>));
		using const_iterator         = iterator;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = reverse_iterator;

		constexpr
		basic_string_ref() noexcept {}

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
		auto operator[](std::size_t index) const noexcept -> const_reference {
			PTL_REQUIRES(index < size());
			return first[index];
		}
		constexpr
		auto at(std::size_t index) const -> const_reference {
			if(index >= size()) throw std::out_of_range{"index out of range"}; 
			return (*this)[index];
		}
		constexpr
		auto front() const noexcept -> const_reference { return (*this)[0]; }
		constexpr
		auto back()  const noexcept -> const_reference { return (*this)[size() - 1]; }

		constexpr
		auto data() const noexcept -> const_pointer { return first; }

		constexpr
		auto size() const noexcept -> size_type { return last - first; }
		constexpr
		auto max_size() const noexcept { return std::numeric_limits<size_type>::max(); }
		constexpr
		auto empty() const noexcept { return size() == 0; }

		constexpr
		void remove_prefix(iterator pos) {
			PTL_REQUIRES(pos.ptr >= first && pos.ptr <= last);
			first = pos.ptr;
		}

		constexpr
		void remove_suffix(iterator pos) {
			PTL_REQUIRES(pos.ptr >= first && pos.ptr <= last);
			last = pos.ptr;
		}

		constexpr
		auto substr(iterator first, iterator last) const {
			auto result{*this};
			result.remove_prefix(first);
			result.remove_suffix(last);
			return result;
		}

		constexpr
		auto begin()  const noexcept { return const_iterator{first}; }
		constexpr
		auto cbegin() const noexcept { return begin(); }
		constexpr
		auto end()    const noexcept { return const_iterator{last}; }
		constexpr
		auto cend()   const noexcept { return end(); }

		constexpr
		auto rbegin()  const noexcept { return const_reverse_iterator{end()}; }
		constexpr
		auto crbegin() const noexcept { return rbegin(); }
		constexpr
		auto rend()    const noexcept { return const_reverse_iterator{begin()}; }
		constexpr
		auto crend()   const noexcept { return rend(); }

		friend
		constexpr
		void swap(basic_string_ref & lhs, basic_string_ref & rhs) noexcept {
			internal::swap(lhs.first, rhs.first);
			internal::swap(lhs.last,  rhs.last);
		}

		friend
		constexpr
		auto operator==(const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept { return compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) == 0; }
		friend
		constexpr
		auto operator< (const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept { return compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) <  0; }

		friend
		constexpr
		auto operator==(const Char * lhs, const basic_string_ref & rhs) noexcept { return compare(c_str_iterator{lhs}, c_str_iterator{}, rhs.begin(), rhs.end()) == 0; }
		friend
		constexpr
		auto operator< (const Char * lhs, const basic_string_ref & rhs) noexcept { return compare(c_str_iterator{lhs}, c_str_iterator{}, rhs.begin(), rhs.end()) <  0; }

		friend
		decltype(auto) operator<<(std::ostream & os, const basic_string_ref & self) {
			for(auto & tmp : self) os << tmp;
			return os;
		}
	};
	PTL_PACK_END

	using string_ref = basic_string_ref<char>;
	BOOST_CONCEPT_ASSERT((boost::RandomAccessContainer<string_ref>));
	static_assert(sizeof(string_ref)           == 2 * sizeof(const char *), "invalid size of string_ref detected");
	static_assert(sizeof(string_ref::iterator) ==     sizeof(const char *), "invalid size of string_ref::iterator detected");

	namespace literals {
		inline
		constexpr
		auto operator""_sr(const char * ptr, std::size_t) noexcept { return string_ref{ptr}; }
	}
}

template<typename Char>
struct std::hash<ptl::basic_string_ref<Char>> final {
	auto operator()(const ptl::basic_string_ref<Char> & self) const noexcept -> std::size_t { return boost::hash_range(self.data(),  self.data() + self.size()); }
};