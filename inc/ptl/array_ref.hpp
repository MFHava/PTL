
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"
#include <cassert>
#include <cstddef>
#include <ostream>
#include <iterator>
#include <stdexcept>
#include <initializer_list>

namespace ptl {
	namespace internal {//emulate C++17-features
		template<typename Container>
		constexpr auto size(const Container & c) -> decltype(c.size()) { return c.size(); }

		template<typename Type, std::size_t Size>
		constexpr auto size(const Type(&array)[Size]) noexcept -> std::size_t { return Size; }

		template<typename Container>
		constexpr auto data(      Container & c) -> decltype(c.data()) { return c.data(); }

		template<typename Container>
		constexpr auto data(const Container & c) -> decltype(c.data()) { return c.data(); }

		template<typename Type, std::size_t Size>
		constexpr auto data(Type(&array)[Size]) noexcept -> Type * { return array; }

		template<typename Type>
		constexpr auto data(std::initializer_list<Type> ilist) noexcept -> const Type * { return ilist.begin(); }
	}

	PTL_PACK_BEGIN
	//! @brief non-owning reference to array
	//! @tparam Type type of the referenced array
	template<typename Type>
	struct array_ref final {//TODO: evaluate differences to the standard!  
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

		array_ref() =default;
		array_ref(const array_ref &) =default;
		array_ref(array_ref &&) noexcept =default;
		auto operator=(const array_ref &) -> array_ref & =default;
		auto operator=(array_ref &&) noexcept -> array_ref & =default;
		~array_ref() noexcept =default;

		//! @brief construct array_ref from two pointers
		//! @param[in] first start of the referenced array
		//! @param[in] last end of the referenced array
		//! @throws std::invalid_argument iff first > last
		//! @attention [first, last) must be valid!
		array_ref(pointer first, pointer last) : first{first}, last{last} { if(first > last) throw std::invalid_argument{"array_ref requires [first, last)"}; }

		//! @brief construct array_ref from pointer and size
		//! @param[in] ptr start of the referenced array
		//! @param[in] count count of elements in the referenced array
		//! @attention [ptr, count) must be valid!
		array_ref(pointer ptr, size_type count) noexcept : array_ref{ptr, ptr + count} {}

		//! @brief construct array_ref from ContiguousRange
		//! @tparam ContiguousRange range type that fulfills the ContiguousRange-requirements
		//! @param[in] range range to reference
		template<typename ContiguousRange>
		array_ref(ContiguousRange & range) noexcept : array_ref{internal::data(range), internal::size(range)} {}

		template<typename OtherType>
		array_ref(const array_ref<OtherType> & other) noexcept : first{other.begin()}, last{other.end()} {}

		template<typename OtherType>
		array_ref(array_ref<OtherType> && other) noexcept : first{other.begin()}, last{other.end()} {}

		template<typename OtherType>
		auto operator=(const array_ref<OtherType> & other) noexcept -> array_ref & {
			first = other.begin();
			last = other.end();
			return *this;
		}

		template<typename OtherType>
		auto operator=(array_ref<OtherType> && other) noexcept -> array_ref & {
			first = other.begin();
			last = other.end();
			return *this;
		}

		auto operator[](size_type index) const noexcept -> const_reference { assert(!empty()); return first[index]; }
		auto operator[](size_type index)       noexcept ->       reference { assert(!empty()); return first[index]; }

		auto at(size_type index) const -> const_reference { return validate_index(index), (*this)[index]; }
		auto at(size_type index)       ->       reference { return validate_index(index), (*this)[index]; }

		auto size()  const noexcept -> size_type { return last - first; }
		auto empty() const noexcept -> bool { return size() == 0; }

		auto data() const noexcept -> const_pointer { return first; }
		auto data()       noexcept ->       pointer { return first; }

		auto begin()  const noexcept -> const_iterator { return first; }
		auto begin()        noexcept ->       iterator { return first; }
		auto cbegin() const noexcept -> const_iterator { return first; }

		auto end()    const noexcept -> const_iterator { return last; }
		auto end()          noexcept ->       iterator { return last; }
		auto cend()   const noexcept -> const_iterator { return last; }

		auto rbegin()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }
		auto rbegin()        noexcept ->       reverse_iterator { return       reverse_iterator{end()}; }
		auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cend()}; }

		auto rend()    const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }
		auto rend()          noexcept ->       reverse_iterator { return       reverse_iterator{begin()}; }
		auto crend()   const noexcept -> const_reverse_iterator { return const_reverse_iterator{cbegin()}; }

		friend
		void swap(array_ref & lhs, array_ref & rhs) noexcept {
			using std::swap;
			swap(lhs.first, rhs.first);
			swap(lhs.last,  rhs.last);
		}

		friend
		auto operator==(const array_ref & lhs, const array_ref & rhs) noexcept -> bool {
			if(lhs.size() != rhs.size()) return false;
			for(size_type i{0}; i < lhs.size(); ++i)
				if(lhs[i] != rhs[i])
					return false;
			return true;
		}
		friend
		auto operator!=(const array_ref & lhs, const array_ref & rhs) noexcept -> bool { return !(lhs == rhs); }
		friend
		auto operator< (const array_ref & lhs, const array_ref & rhs) noexcept -> bool {
			if(lhs.size() < rhs.size()) return true;
			if(lhs.size() > rhs.size()) return false;
			for(size_type i{0}; i < lhs.size(); ++i)
				if(!(lhs[i] < rhs[i]))
					return false;
			return true;
		}
		friend
		auto operator<=(const array_ref & lhs, const array_ref & rhs) noexcept -> bool { return !(rhs < lhs); }
		friend
		auto operator> (const array_ref & lhs, const array_ref & rhs) noexcept -> bool { return rhs < lhs; }
		friend
		auto operator>=(const array_ref & lhs, const array_ref & rhs) noexcept -> bool { return !(lhs < rhs); }

		friend
		auto operator<<(std::ostream & os, const array_ref & self) -> std::ostream & {
			os << '[';
			if(!self.empty()) {
				auto it{self.begin()};
				os << *it;
				for(++it; it != self.end(); ++it) os << ", " << *it;
			}
			return os << ']';
		}
	private:
		void validate_index(size_type index) const { if(index >= size()) throw std::out_of_range{"index out of range"}; }

		pointer first{nullptr}, last{nullptr};
	};
	PTL_PACK_END
}