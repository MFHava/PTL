
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/utility.hpp"
#include "internal/type_checks.hpp"
#include <limits>
#include <cassert>
#include <cstddef>
#include <ostream>
#include <iterator>
#include <stdexcept>
#include <initializer_list>
#include <boost/concept_check.hpp>

namespace ptl {
	namespace internal {//emulate C++17-features
		template<typename Container>
		constexpr
		auto size(const Container & c) -> decltype(c.size()) { return c.size(); }

		template<typename Type, std::size_t Size>
		constexpr
		auto size(const Type(&array)[Size]) noexcept -> std::size_t { return Size; }

		template<typename Container>
		constexpr
		auto data(      Container & c) -> decltype(c.data()) { return c.data(); }

		template<typename Container>
		constexpr
		auto data(const Container & c) -> decltype(c.data()) { return c.data(); }

		template<typename Type, std::size_t Size>
		constexpr
		auto data(Type(&array)[Size]) noexcept -> Type * { return array; }

		template<typename Type>
		constexpr
		auto data(std::initializer_list<Type> ilist) noexcept -> const Type * { return ilist.begin(); }
	}

	PTL_PACK_BEGIN
	//! @brief non-owning reference to array
	//! @tparam Type type of the referenced array
	template<typename Type>
	class array_ref final : public internal::random_access_container_base<array_ref<Type>,boost::totally_ordered1<array_ref<Type>>> {//TODO: evaluate differences to the standard!  
		Type * first{nullptr}, * last{nullptr};
	public:
		static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");

		using value_type             = Type;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       value_type &;
		using const_reference        = const value_type &;
		using pointer                =       value_type *;
		using const_pointer          = const value_type *;
		using iterator               = internal::random_access_iterator<array_ref<Type>,       Type>;
		BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<iterator>));
		using const_iterator         = internal::random_access_iterator<array_ref<Type>, const Type>; 
		BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<const_iterator>));
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr
		array_ref() =default;
		constexpr
		array_ref(const array_ref &) =default;
		constexpr
		auto operator=(const array_ref &) -> array_ref & =default;
		~array_ref() noexcept =default;

		//! @brief construct array_ref from two pointers
		//! @param[in] first start of the referenced array
		//! @param[in] last end of the referenced array
		//! @throws std::invalid_argument iff first > last
		//! @attention [first, last) must be valid!
		constexpr
		array_ref(pointer first, pointer last) : first{first}, last{last} { if(first > last) throw std::invalid_argument{"array_ref requires [first, last)"}; }

		//! @brief construct array_ref from pointer and size
		//! @param[in] ptr start of the referenced array
		//! @param[in] count count of elements in the referenced array
		//! @attention [ptr, count) must be valid!
		constexpr
		array_ref(pointer ptr, size_type count) noexcept : array_ref{ptr, ptr + count} {}

		//! @brief construct array_ref from ContiguousRange
		//! @tparam ContiguousRange range type that fulfills the ContiguousRange-requirements
		//! @param[in] range range to reference
		template<typename ContiguousRange>
		constexpr
		array_ref(ContiguousRange & range) noexcept : array_ref{internal::data(range), internal::size(range)} {}

		template<typename OtherType>
		constexpr
		array_ref(const array_ref<OtherType> & other) noexcept { *this = other; }

		template<typename OtherType>
		constexpr
		auto operator=(const array_ref<OtherType> & other) noexcept -> array_ref & {
			if(other.empty()) first = last = nullptr;
			else {
				first = &*other.begin();
				last = first + other.size();
			}
			return *this;
		}

		constexpr
		auto operator[](size_type index) const noexcept -> const_reference { assert(!empty()); return first[index]; }
		constexpr
		auto operator[](size_type index)       noexcept ->       reference { assert(!empty()); return first[index]; }

		constexpr
		auto size()  const noexcept -> size_type { return last - first; }
		constexpr
		auto max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

		constexpr
		auto data() const noexcept -> const_pointer { return first; }
		constexpr
		auto data()       noexcept ->       pointer { return first; }

		constexpr
		auto begin() const noexcept { return const_iterator{first}; }
		constexpr
		auto begin()       noexcept { return iterator{first}; }
		constexpr
		auto end()   const noexcept { return const_iterator{last}; }
		constexpr
		auto end()         noexcept { return iterator{last}; }

		friend
		constexpr
		void swap(array_ref & lhs, array_ref & rhs) noexcept {
			using std::swap;
			swap(lhs.first, rhs.first);
			swap(lhs.last,  rhs.last);
		}

		friend
		constexpr
		auto operator==(const array_ref & lhs, const array_ref & rhs) noexcept -> bool {
			if(lhs.size() != rhs.size()) return false;
			for(size_type i{0}; i < lhs.size(); ++i)
				if(lhs[i] != rhs[i])
					return false;
			return true;
		}
		friend
		constexpr
		auto operator< (const array_ref & lhs, const array_ref & rhs) noexcept -> bool {
			if(lhs.size() < rhs.size()) return true;
			if(lhs.size() > rhs.size()) return false;
			for(size_type i{0}; i < lhs.size(); ++i)
				if(!(lhs[i] < rhs[i]))
					return false;
			return true;
		}

		friend
		decltype(auto) operator<<(std::ostream & os, const array_ref & self) {
			os << '[';
			if(!self.empty()) {
				auto it{self.begin()};
				os << *it;
				for(++it; it != self.end(); ++it) os << ", " << *it;
			}
			return os << ']';
		}
	};
	PTL_PACK_END
}
