
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <stdexcept>
#include <string_view>

namespace ptl {
	//! @brief a read-only, non-owning reference to a string
	//! @attention the referenced string is not guaranteed to be null-terminated!
	class string_ref final {
		const char * first{nullptr}, * last{nullptr};
	public:
		using traits_type            = std::char_traits<char>;
		using value_type             = char;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       value_type &;
		using const_reference        = const value_type &;
		using pointer                =       value_type *;
		using const_pointer          = const value_type *;
		struct iterator final {
			//TODO: [C++20] using iterator_concept = std::contiguous_iterator_tag;
			using iterator_category = std::random_access_iterator_tag;
			using value_type        = string_ref::value_type;
			using difference_type   = string_ref::difference_type;
			using pointer           = string_ref::const_pointer;
			using reference         = string_ref::const_reference;

			constexpr
			iterator() noexcept =default;

			constexpr
			auto operator++() noexcept -> iterator & { ++ptr; return *this; }
			constexpr
			auto operator++(int) noexcept -> iterator {
				auto tmp{*this};
				++*this;
				return tmp;
			}

			constexpr
			auto operator--() noexcept -> iterator & { --ptr; return *this; }
			constexpr
			auto operator--(int) noexcept -> iterator {
				auto tmp{*this};
				--*this;
				return tmp;
			}

			constexpr
			auto operator*() const noexcept -> reference { return *ptr; }
			constexpr
			auto operator->() const noexcept -> pointer { return ptr; }

			constexpr
			auto operator[](difference_type index) const noexcept -> reference { return *(*this + index); }

			constexpr
			auto operator+=(difference_type count) noexcept -> iterator & { ptr += count; return *this; }
			friend
			constexpr
			auto operator+(iterator lhs, difference_type rhs) noexcept -> iterator {
				lhs += rhs;
				return lhs;
			}
			friend
			constexpr
			auto operator+(difference_type lhs, iterator rhs) noexcept -> iterator { return rhs + lhs; }

			constexpr
			auto operator-=(difference_type count) noexcept -> iterator & { ptr -= count; return *this; }
			friend
			constexpr
			auto operator-(iterator lhs, difference_type rhs) noexcept -> iterator {
				lhs -= rhs;
				return lhs;
			}

			friend
			constexpr
			auto operator-(const iterator & lhs, const iterator & rhs) noexcept -> difference_type { return lhs.ptr - rhs.ptr; }

			friend
			constexpr
			auto operator==(const iterator & lhs, const iterator & rhs) noexcept -> bool { return lhs.ptr == rhs.ptr; }
			friend
			constexpr
			auto operator!=(const iterator & lhs, const iterator & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] remove as implicitly generated
			//TODO: [C++20] replace the ordering operators by <=>
			friend
			constexpr
			auto operator< (const iterator & lhs, const iterator & rhs) noexcept -> bool { return lhs.ptr < rhs.ptr; }
			friend
			constexpr
			auto operator> (const iterator & lhs, const iterator & rhs) noexcept -> bool { return rhs < lhs; }
			friend
			constexpr
			auto operator<=(const iterator & lhs, const iterator & rhs) noexcept -> bool { return !(lhs > rhs); }
			friend
			constexpr
			auto operator>=(const iterator & lhs, const iterator & rhs) noexcept -> bool { return !(lhs < rhs); }
		private:
			friend string_ref;

			constexpr
			iterator(pointer ptr) noexcept : ptr{ptr} {}

			pointer ptr{nullptr};
		};
		using const_iterator         = iterator;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = reverse_iterator;

		constexpr
		string_ref() noexcept =default;

		//! @brief construct from c-string
		//! @param[in] str null-terminated string
		constexpr
		string_ref(const_pointer str) noexcept : first{str}, last{str + traits_type::length(str)} {} //TODO: [C++??] precondition(str);

		//! @brief construct from c-string + length
		//! @param[in] str string to reference
		//! @param[in] size size of string to reference
		//! @attention [str, size) must be a valid range!
		constexpr
		string_ref(const_pointer str, size_type size) noexcept : first{str}, last{str + size} {} //TODO: [C++??] precondition(str || (!str && !size));

		//! @brief construct from std::string_view
		//! @param[in] str string to reference
		constexpr
		string_ref(const std::string_view & str) noexcept : string_ref{str.data(), str.size()} {}

		//TODO: [C++20] constexpr basic_string_view( It first, End last );
		//TODO: [C++23] explicit constexpr basic_string_view( R&& r );
		//TODO: [C++23] constexpr basic_string_view( std::nullptr_t ) = delete;

		constexpr
		auto data() const noexcept -> const_pointer { return first; }
		[[nodiscard]]
		constexpr
		auto empty() const noexcept -> bool { return size() == 0; }
		constexpr
		auto size() const noexcept -> size_type { return last - first; }
		static
		constexpr
		auto max_size() noexcept-> size_type { return static_cast<size_type>(std::numeric_limits<difference_type>::max()); }

		constexpr
		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto operator[](size_type index) const noexcept -> const_reference { return *(data() + index); } //TODO: [C++??] precondition(index < size());
		constexpr
		auto at(size_type index) const -> const_reference {
			if(index >= size()) throw std::out_of_range{"ptl::string_ref::at - index out of range"};
			return (*this)[index];
		}

		constexpr
		void remove_prefix(size_type count) noexcept { first += count; } //TODO: [C++??] precondition(count <= size());
		constexpr
		void remove_suffix(size_type count) noexcept { last -= count; } //TODO: [C++??] precondition(count <= size());

		constexpr
		auto substr(size_type offset) const noexcept -> string_ref { return {data() + offset, size() - offset}; } //TODO: [C++??] precondition(offset <= size());
		constexpr
		auto substr(size_type offset, size_type count) const noexcept -> string_ref { return {data() + offset, count}; } //TODO: [C++??] precondition(offset + count <= size());

		constexpr
		auto begin() const noexcept -> iterator { return data(); }
		constexpr
		auto cbegin() const noexcept -> const_iterator { return begin(); }
		constexpr
		auto end() const noexcept -> iterator { return begin() + size(); }
		constexpr
		auto cend() const noexcept -> const_iterator { return end(); }
		constexpr
		auto rbegin() const noexcept -> reverse_iterator { return reverse_iterator{end()}; }
		constexpr
		auto crbegin() const noexcept -> const_reverse_iterator { return rbegin(); }
		constexpr
		auto rend() const noexcept -> reverse_iterator { return reverse_iterator{begin()}; }
		constexpr
		auto crend() const noexcept -> const_reverse_iterator { return rend(); }

		friend
		constexpr
		auto operator==(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::string_view{lhs} == std::string_view{rhs}; }
		friend
		constexpr
		auto operator!=(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return !(lhs == rhs); }//TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return std::string_view{lhs} < std::string_view{rhs}; }
		friend
		constexpr
		auto operator> (const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const string_ref & lhs, const string_ref & rhs) noexcept -> bool { return !(lhs < rhs); }

		friend
		auto operator<<(std::ostream & os, const string_ref & self) -> std::ostream & { return os << static_cast<std::string_view>(self); }

		//! @brief convert to std::string_view
		constexpr
		operator std::string_view() const noexcept { return {data(), size()}; }
	};
	static_assert(sizeof(string_ref) == 2 * sizeof(const char *));

	//TODO: [C++20] basic_string_view(It, End) -> basic_string_view<std::iter_value_t<It>>;
	//TODO: [C++23] basic_string_view(R&&) -> basic_string_view<ranges::range_value_t<R>>;

	namespace literals {
		inline
		constexpr
		auto operator""_sr(const char * ptr, std::size_t) noexcept -> string_ref { return ptr; }
	}
}

namespace std {
	template<>
	struct hash<ptl::string_ref> {
		auto operator()(const ptl::string_ref & self) const noexcept -> std::size_t { return std::hash<std::string_view>{}(self); }
	};
}

//TODO: [C++20] mark as borrowed_range
//TODO: [C++20] mark as view
