
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <string_view>

namespace ptl {
	//! @brief a read-only, non-owning reference to a string
	//! @attention the referenced string is not guaranteed to be null-terminated!
	template<typename Char, typename Traits = std::char_traits<Char>>
	class basic_string_ref final {
		const Char * ptrs[2]{nullptr, nullptr};
	public:
		using traits_type            = Traits;
		using value_type             = Char;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       value_type &;
		using const_reference        = const value_type &;
		using pointer                =       value_type *;
		using const_pointer          = const value_type *;
		struct iterator final {
			using iterator_category = std::random_access_iterator_tag; //TODO: [C++20] use contiguous_iterator_tag
			using value_type        = basic_string_ref::value_type;
			using difference_type   = basic_string_ref::difference_type;
			using pointer           = basic_string_ref::const_pointer;
			using reference         = basic_string_ref::const_reference;

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
			friend basic_string_ref;

			constexpr
			iterator(pointer ptr) noexcept : ptr{ptr} {}

			pointer ptr{nullptr};
		};
		using const_iterator         = iterator;
		using reverse_iterator       = std::reverse_iterator<iterator>;
		using const_reverse_iterator = reverse_iterator;

		//! @brief corresponding std::string_view-type
		using string_view = std::basic_string_view<Char, Traits>;

		constexpr
		basic_string_ref() noexcept =default;

		//! @brief construct from c-string
		//! @param[in] str null-terminated string
		constexpr
		basic_string_ref(const_pointer str) noexcept : ptrs{str, str + Traits::length(str)} {} //TODO: [C++??] precondition(str);

		//! @brief construct from c-string + length
		//! @param[in] str string to reference
		//! @param[in] size size of string to reference
		//! @attention [str, size) must be a valid range!
		constexpr
		basic_string_ref(const_pointer str, size_type size) noexcept : ptrs{str, str + size} {} //TODO: [C++??] precondition(str || (!str && !size));

		//! @brief construct from corresponding std::string_view-type
		//! @param[in] str string to reference
		constexpr
		basic_string_ref(const string_view & str) noexcept : basic_string_ref{str.data(), str.size()} {}

		constexpr
		auto data() const noexcept -> const_pointer { return ptrs[0]; }
		constexpr
		auto empty() const noexcept -> bool { return size() == 0; }
		constexpr
		auto size() const noexcept -> size_type { return ptrs[1] - ptrs[0]; }
		static
		constexpr
		auto max_size() noexcept-> size_type { return std::numeric_limits<size_type>::max() / sizeof(Char); }

		constexpr
		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto operator[](size_type index) const noexcept -> const_reference { return *(data() + index); } //TODO: [C++??] precondition(index < size());

		constexpr
		void remove_prefix(size_type count) noexcept { ptrs[0] += count; } //TODO: [C++??] precondition(count <= size());
		constexpr
		void remove_suffix(size_type count) noexcept { ptrs[1] -= count; } //TODO: [C++??] precondition(count <= size());

		constexpr
		auto substr(size_type offset) const noexcept -> basic_string_ref { return {data() + offset, size() - offset}; } //TODO: [C++??] precondition(offset <= size());
		constexpr
		auto substr(size_type offset, size_type count) const noexcept -> basic_string_ref { return {data() + offset, count}; } //TODO: [C++??] precondition(offset + count <= size());

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

		constexpr
		void swap(basic_string_ref & other) noexcept { for(auto i{0}; i < 2; ++i) std::swap(ptrs[i], other.ptrs[i]); }
		friend
		constexpr
		void swap(basic_string_ref & lhs, basic_string_ref & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept -> bool { return string_view{lhs} == string_view{rhs}; }
		friend
		constexpr
		auto operator!=(const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept -> bool { return !(lhs == rhs); }//TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept -> bool { return string_view{lhs} < string_view{rhs}; }
		friend
		constexpr
		auto operator> (const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept -> bool { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept -> bool { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const basic_string_ref & lhs, const basic_string_ref & rhs) noexcept -> bool { return !(lhs < rhs); }

		friend
		auto operator<<(std::basic_ostream<Char, Traits> & os, const basic_string_ref & self) -> std::basic_ostream<Char, Traits> & { return os << static_cast<string_view>(self); }

		//! @brief convert to corresponding std::string_view-type
		constexpr
		operator string_view() const noexcept { return {data(), size()}; }
	};

	using string_ref = basic_string_ref<char>;
	static_assert(sizeof(string_ref) == 2 * sizeof(const char *));
	using wstring_ref = basic_string_ref<wchar_t>;
	static_assert(sizeof(wstring_ref) == 2 * sizeof(const wchar_t *));
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201907L //TODO: [C++20] this no longer needs to be conditional
	using u8string_ref = basic_string_ref<char8_t>;
	static_assert(sizeof(u8string_ref) == 2 * sizeof(const char8_t *));
#endif
	using u16string_ref = basic_string_ref<char16_t>;
	static_assert(sizeof(u16string_ref) == 2 * sizeof(const char16_t *));
	using u32string_ref = basic_string_ref<char32_t>;
	static_assert(sizeof(u32string_ref) == 2 * sizeof(const char32_t *));

	namespace literals {
		inline
		constexpr
		auto operator""_sr(const char * ptr, std::size_t) noexcept -> string_ref { return ptr; }
		inline
		constexpr
		auto operator""_sr(const wchar_t * ptr, std::size_t) noexcept -> wstring_ref { return ptr; }
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201907L //TODO: [C++20] this no longer needs to be conditional
		inline
		constexpr
		auto operator""_sr(const char8_t * ptr, std::size_t) noexcept -> u8string_ref { return ptr; }
#endif
		inline
		constexpr
		auto operator""_sr(const char16_t * ptr, std::size_t) noexcept -> u16string_ref { return ptr; }
		inline
		constexpr
		auto operator""_sr(const char32_t * ptr, std::size_t) noexcept -> u32string_ref { return ptr; }
	}
}

namespace std {
	template<>
	struct hash<ptl::string_ref> {
		auto operator()(const ptl::string_ref & self) const noexcept -> std::size_t { return std::hash<std::string_view>{}(self); }
	};

	template<>
	struct hash<ptl::wstring_ref> {
		auto operator()(const ptl::wstring_ref & self) const noexcept -> std::size_t { return std::hash<std::wstring_view>{}(self); }
	};

#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201907L //TODO: [C++20] this no longer needs to be conditional
	template<>
	struct hash<ptl::u8string_ref> {
		auto operator()(const ptl::u8string_ref & self) const noexcept -> std::size_t { return std::hash<std::u8string_view>{}(self); }
	};
#endif

	template<>
	struct hash<ptl::u16string_ref> {
		auto operator()(const ptl::u16string_ref & self) const noexcept -> std::size_t { return std::hash<std::u16string_view>{}(self); }
	};

	template<>
	struct hash<ptl::u32string_ref> {
		auto operator()(const ptl::u32string_ref & self) const noexcept -> std::size_t { return std::hash<std::u32string_view>{}(self); }
	};
}

//TODO: [C++20] mark as borrowed_range
//TODO: [C++20] mark as view
