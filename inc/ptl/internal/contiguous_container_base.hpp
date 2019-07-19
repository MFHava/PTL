
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <algorithm>
#include <boost/operators.hpp>
#include <boost/concept_check.hpp>
#include "requires.hpp"
#include "type_checks.hpp"

namespace ptl::internal {
	template<typename Implementation, typename Type, typename Base = boost::operators_detail::empty_base<Implementation>>
	class contiguous_container_base : boost::totally_ordered1<Implementation, Base> {
		static_assert(internal::is_abi_compatible_v<Type>);

		constexpr
		auto self() const noexcept -> const Implementation & { return static_cast<const Implementation &>(*this); }
		constexpr
		auto self()       noexcept ->       Implementation & { return static_cast<      Implementation &>(*this); }

		template<typename ValueType, bool IsConst>
		struct contiguous_iterator final : boost::random_access_iterator_helper<contiguous_iterator<ValueType, IsConst>, ValueType, std::ptrdiff_t, std::conditional_t<IsConst, const ValueType, ValueType> *, std::conditional_t<IsConst, const ValueType, ValueType> &> {
			static_assert(!std::is_const_v<ValueType>);

			constexpr
			contiguous_iterator() noexcept =default;

			constexpr
			contiguous_iterator(const contiguous_iterator &) =default;

			~contiguous_iterator() noexcept =default;

			constexpr
			auto operator++() noexcept -> contiguous_iterator & { move(+1); return *this; }
			constexpr
			auto operator--() noexcept -> contiguous_iterator & { move(-1); return *this; }

			constexpr
			auto operator*() const noexcept -> decltype(auto) {
				PTL_REQUIRES(ptr);
				return *ptr;
			}

			constexpr
			auto operator+=(std::ptrdiff_t count) noexcept -> contiguous_iterator & { move(+count); return *this; }
			constexpr
			auto operator-=(std::ptrdiff_t count) noexcept -> contiguous_iterator & { move(-count); return *this; }

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
				PTL_REQUIRES(ptr || (!ptr && !count));
				ptr += count;
			}

			using internal_type = std::conditional_t<IsConst, const ValueType, ValueType> *;

			constexpr
			explicit
			contiguous_iterator(internal_type ptr) noexcept : ptr{ptr} {}

			internal_type ptr{nullptr};
		};
	protected:
		template<typename InputRange1, typename InputRange2>
		static
		constexpr
		auto equal(const InputRange1 & lhs, const InputRange2 & rhs) noexcept { return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }

		template<typename InputRange1, typename InputRange2>
		static
		constexpr
		auto less(const InputRange1 & lhs, const InputRange2 & rhs) noexcept { return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
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
		using iterator               = std::conditional_t<std::is_const_v<Type>, const_iterator, mutable_iterator>;
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr
		auto front() const noexcept -> decltype(auto) {
			PTL_REQUIRES(!empty());
			return (*this)[0];
		}
		constexpr
		auto front()       noexcept -> decltype(auto) {
			PTL_REQUIRES(!empty());
			return (*this)[0];
		}
		constexpr
		auto back() const noexcept -> decltype(auto) {
			PTL_REQUIRES(!empty());
			return (*this)[self().size() - 1];
		}
		constexpr
		auto back()       noexcept -> decltype(auto) {
			PTL_REQUIRES(!empty());
			return (*this)[self().size() - 1];
		}

		constexpr
		auto operator[](std::size_t index) const noexcept -> decltype(auto) {
			PTL_REQUIRES(index < self().size());
			return *(begin() + index);
		}
		constexpr
		auto operator[](std::size_t index)       noexcept -> decltype(auto) {
			PTL_REQUIRES(index < self().size());
			return *(begin() + index);
		}
		constexpr
		auto at(std::size_t index) const -> decltype(auto) {
			if(index >= self().size()) throw std::out_of_range{"index out of range"};
			return (*this)[index];
		}
		constexpr
		auto at(std::size_t index)       -> decltype(auto) {
			if(index >= self().size()) throw std::out_of_range{"index out of range"};
			return (*this)[index];
		}

		constexpr
		auto empty() const noexcept { return self().size() == 0; }

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

		friend
		constexpr
		auto operator==(const Implementation & lhs, const Implementation & rhs) noexcept { return equal(lhs, rhs); }
		friend
		constexpr
		auto operator< (const Implementation & lhs, const Implementation & rhs) noexcept { return less(lhs, rhs); }
	};
}

