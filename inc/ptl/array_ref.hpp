
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/utility.hpp"
#include <limits>
#include <ostream>
#include <initializer_list>

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
	class array_ref final : public internal::contiguous_container_base<array_ref<Type>, Type> {//TODO: evaluate differences to the standard!
		using base_type = internal::contiguous_container_base<array_ref<Type>, Type>;

		Type * first{nullptr}, * last{nullptr};
	public:
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
		array_ref(Type * first, Type * last) : first{first}, last{last} { if(first > last) throw std::invalid_argument{"array_ref requires [first, last)"}; }

		//! @brief construct array_ref from pointer and size
		//! @param[in] ptr start of the referenced array
		//! @param[in] count count of elements in the referenced array
		//! @attention [ptr, count) must be valid!
		constexpr
		array_ref(Type * ptr, std::size_t count) noexcept : array_ref{ptr, ptr + count} {}

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
		auto data() const noexcept -> const Type * { return first; }
		constexpr
		auto data()       noexcept ->       Type * { return first; }
		constexpr
		auto size() const noexcept -> std::size_t { return last - first; }
		constexpr
		auto max_size() const noexcept { return std::numeric_limits<std::size_t>::max(); }

		constexpr
		void swap(array_ref & other) noexcept {
			using std::swap;
			swap(first, other.first);
			swap(last,  other.last);
		}

		friend
		constexpr
		auto operator==(const array_ref & lhs, const array_ref & rhs) noexcept -> bool {
			if(lhs.size() != rhs.size()) return false;
			return base_type::equal(lhs, rhs);
		}
		friend
		constexpr
		auto operator< (const array_ref & lhs, const array_ref & rhs) noexcept -> bool {
			if(lhs.size() < rhs.size()) return true;
			if(lhs.size() > rhs.size()) return false;
			return base_type::less(lhs, rhs);
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
