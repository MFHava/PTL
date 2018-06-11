
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>
#include "requires.hpp"
#include "type_checks.hpp"

namespace ptl {
	namespace internal {
		template<typename Implementation, typename Type, typename Base = boost::operators_detail::empty_base<Implementation>>
		class contiguous_container_base : boost::totally_ordered1<Implementation, Base> {
			static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");

			constexpr
			decltype(auto) self() const noexcept { return static_cast<const Implementation &>(*this); }
			constexpr
			decltype(auto) self()       noexcept { return static_cast<      Implementation &>(*this); }

			template<typename ValueType, bool IsConst>
			struct contiguous_iterator final : boost::random_access_iterator_helper<contiguous_iterator<ValueType, IsConst>, ValueType, std::ptrdiff_t, std::conditional_t<IsConst, const ValueType, ValueType> *, std::conditional_t<IsConst, const ValueType, ValueType> &> {
				static_assert(!std::is_const<ValueType>::value, "ValueType may not be const");

				constexpr
				contiguous_iterator() noexcept =default;

				constexpr
				decltype(auto) operator++() noexcept { move(+1); return *this; }
				constexpr
				decltype(auto) operator--() noexcept { move(-1); return *this; }

				constexpr
				decltype(auto) operator*() const noexcept {
					PTL_REQUIRES(ptr);
					return *ptr;
				}

				constexpr
				decltype(auto) operator+=(std::ptrdiff_t count) noexcept { move(+count); return *this; }
				constexpr
				decltype(auto) operator-=(std::ptrdiff_t count) noexcept { move(-count); return *this; }

				constexpr
				operator contiguous_iterator<ValueType, true>() const noexcept { return contiguous_iterator<ValueType, true>{ptr}; }

				friend
				constexpr
				auto operator-(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> std::ptrdiff_t { return lhs.ptr - rhs.ptr; }
				friend
				constexpr
				auto operator==(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return lhs.ptr == rhs.ptr; }
				friend
				constexpr
				auto operator< (const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return lhs.ptr <  rhs.ptr; }
			private:
				constexpr
				operator contiguous_iterator<ValueType, false>() const noexcept { return contiguous_iterator<ValueType, false>{const_cast<ValueType *>(ptr)}; }

				friend contiguous_container_base;
				friend Implementation;

				constexpr
				void move(std::ptrdiff_t count) noexcept {
					PTL_REQUIRES(ptr);
					ptr += count;
				}

				using internal_type = std::conditional_t<IsConst, const ValueType, ValueType> *;

				explicit
				constexpr
				contiguous_iterator(internal_type ptr) noexcept : ptr{ptr} {}

				internal_type ptr{nullptr};
			};

			template<typename InputRange1, typename InputRange2>
			static
			constexpr
			auto compare(const InputRange1 & lhs, const InputRange2 & rhs) noexcept -> int {
				auto first1{lhs.begin()};
				auto first2{rhs.begin()};
				const auto last1{lhs.end()};
				const auto last2{rhs.end()};
				while(first1 != last1 && first2 != last2 && *first1 == *first2) ++first1, ++first2;
				if(first1 == last1) {
					if(first2 == last2) return 0;
					else return -1;
				} else {
					if(first2 == last2) return 1;
					else return *first1 - *first2;
				}
			}
		protected:
			template<typename InputRange1, typename InputRange2>
			static
			constexpr
			auto equal(const InputRange1 & lhs, const InputRange2 & rhs) noexcept { return compare(lhs, rhs) == 0; }

			template<typename InputRange1, typename InputRange2>
			static
			constexpr
			auto less(const InputRange1 & lhs, const InputRange2 & rhs) noexcept { return compare(lhs, rhs) <  0; }
		public:
			using value_type             = std::remove_cv_t<Type>;
			using size_type              = std::size_t;
			using difference_type        = std::ptrdiff_t;
			using reference              =       value_type &;
			using const_reference        = const value_type &;
			using pointer                =       value_type *;
			using const_pointer          = const value_type *;
		private:
			using mutable_iterator       = contiguous_iterator<value_type, false>;
			BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessIterator<mutable_iterator>));
		public:
			using const_iterator         = contiguous_iterator<value_type, true>;
			BOOST_CONCEPT_ASSERT((boost::RandomAccessIterator<const_iterator>));
			using iterator               = std::conditional_t<std::is_const<Type>::value, const_iterator, mutable_iterator>;
			using reverse_iterator       = std::reverse_iterator<      iterator>;
			using const_reverse_iterator = std::reverse_iterator<const_iterator>;

			constexpr
			decltype(auto) front() const noexcept {
				PTL_REQUIRES(!empty());
				return (*this)[0];
			}
			constexpr
			decltype(auto) front()       noexcept {
				PTL_REQUIRES(!empty());
				return (*this)[0];
			}
			constexpr
			decltype(auto) back() const noexcept {
				PTL_REQUIRES(!empty());
				return (*this)[self().size() - 1];
			}
			constexpr
			decltype(auto) back()       noexcept {
				PTL_REQUIRES(!empty());
				return (*this)[self().size() - 1];
			}

			constexpr
			decltype(auto) operator[](std::size_t index) const noexcept {
				PTL_REQUIRES(index < self().size());
				return *(begin() + index);
			}
			constexpr
			decltype(auto) operator[](std::size_t index)       noexcept {
				PTL_REQUIRES(index < self().size());
				return *(begin() + index);
			}
			constexpr
			decltype(auto) at(std::size_t index) const {
				if(index >= self().size()) throw std::out_of_range{"index out of range"};
				return (*this)[index];
			}
			constexpr
			decltype(auto) at(std::size_t index)       {
				if(index >= self().size()) throw std::out_of_range{"index out of range"};
				return (*this)[index];
			}

			constexpr
			auto empty() const noexcept -> bool { return self().size() == 0; }

			constexpr
			auto begin() const noexcept { return const_iterator{self().data()}; }
			constexpr
			auto begin()       noexcept { return iterator{self().data()}; }
			constexpr
			auto end()   const noexcept { return begin() + self().size(); }
			constexpr
			auto end()         noexcept { return begin() + self().size(); }
			constexpr
			auto cbegin() const noexcept { return begin(); }
			constexpr
			auto cend()   const noexcept { return end(); }
			constexpr
			auto rbegin()  const noexcept { return const_reverse_iterator{end()}; }
			constexpr
			auto rbegin()        noexcept { return reverse_iterator{end()}; }
			constexpr
			auto crbegin() const noexcept { return const_reverse_iterator{cend()}; }
			constexpr
			auto rend()    const noexcept { return const_reverse_iterator{begin()}; }
			constexpr
			auto rend()          noexcept { return reverse_iterator{begin()}; }
			constexpr
			auto crend()   const noexcept { return const_reverse_iterator{cbegin()}; }

			friend
			constexpr
			void swap(Implementation & lhs, Implementation & rhs) noexcept { lhs.swap(rhs); }
		};
	}
}
