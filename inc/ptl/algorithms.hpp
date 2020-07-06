
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cassert>
#include <iterator>
#include <algorithm>
#include <execution>
#include <type_traits>
#include "internal/cpp20_emulation.hpp"

namespace ptl {
	inline
	constexpr
	struct {
		template<typename InputIterator, typename OutputIterator, typename UnaryPredicate, typename UnaryOperation>
		constexpr
		auto operator()(
			InputIterator first,   //!< [in] begin of input range
			InputIterator last,    //!< [in] end of input range
			OutputIterator result, //!< [in] begin of destination range, may be equal to first
			UnaryPredicate pred,   //!< [in] unary predicate which returns true for elements to transform
			UnaryOperation op      //!< [in] unary operation to apply for transformation
		) const -> OutputIterator {
			for(; first != last; ++first)
				if(pred(*first))
					*result++ = op(*first);
			return result;
		}

		template<typename InputRange, typename OutputIterator, typename UnaryPredicate, typename UnaryOperation>
		constexpr
		auto operator()(
			InputRange && range,   //!< [in] input range
			OutputIterator result, //!< [in] begin of destination range, may be equal to begin(range)
			UnaryPredicate pred,   //!< [in] unary predicate which returns true for elements to transform
			UnaryOperation op      //!< [in] unary operation to apply for transformation
		) const -> OutputIterator {
			using std::begin;
			using std::end;
			return (*this)(begin(range), end(range), result, pred, op);
		}

		template<typename InputIterator1, typename InputIterator2, typename OutputIterator, typename BinaryPredicate, typename BinaryOperation>
		constexpr
		auto operator()(
			InputIterator1 first1, //!< [in] begin of first range
			InputIterator1 last1,  //!< [in] end of first range
			InputIterator2 first2, //!< [in] begin of second range - will be [first2, N) with N=distance(first1, last1)
			OutputIterator result, //!< [in] begin of destination range, may be equal to first1 or first2
			BinaryPredicate pred,  //!< [in] binary predicate which returns true for elements to transform
			BinaryOperation op     //!< [in] binary operation to apply for transformation
		) const -> OutputIterator {
			for(; first1 != last1; ++first1, (void)++first2)
				if(pred(*first1, *first2))
					*result++ = op(*first1, *first2);
			return result;
		}
	} transform_if;


	inline
	constexpr
	class {
		template<typename Integral>
		class iterator final {
			Integral pos{0}, inc{0};
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type        = Integral;
			using difference_type   = std::ptrdiff_t;
			using pointer           = const Integral *;
			using reference         = const Integral &;

			constexpr
			iterator() noexcept =default;
			explicit
			constexpr
			iterator(Integral last) noexcept : pos{last} {}
			constexpr
			iterator(Integral val, Integral step) noexcept : pos{val}, inc{step} {}

			constexpr
			auto operator++() noexcept -> iterator & {
				pos += inc;
				return *this;
			}
			constexpr
			auto operator++(int) noexcept -> iterator {
				auto tmp{*this};
				++*this;
				return tmp;
			}

			constexpr
			auto operator*() const noexcept -> reference { return pos; }
			constexpr
			auto operator->() const noexcept -> pointer =delete;

			friend
			constexpr
			auto operator==(const iterator & lhs, const iterator & rhs) noexcept { return (lhs.pos >= rhs.pos); }
			friend
			constexpr
			auto operator!=(const iterator & lhs, const iterator & rhs) noexcept { return !(lhs == rhs); }
		};
	public:
		template<typename ExecutionPolicy, typename Integral, typename UnaryFunction, typename = std::enable_if_t<std::is_execution_policy_v<internal::remove_cvref_t<ExecutionPolicy>>>>
		constexpr
		void operator()(
			ExecutionPolicy && policy, //!< [in] execution policy
			Integral first,            //!< [in] first index
			Integral last,             //!< [in] last index
			Integral step,             //!< [in] stride between successive calls to f
			UnaryFunction f            //!< [in] function to be called for every index in range
		) const {
			assert(first <= last && step > Integral{0});
			std::for_each(std::forward<ExecutionPolicy>(policy), iterator<Integral>{first, step}, iterator<Integral>{last}, f);
		}

		template<typename ExecutionPolicy, typename Integral, typename UnaryFunction, typename = std::enable_if_t<std::is_execution_policy_v<internal::remove_cvref_t<ExecutionPolicy>>>>
		constexpr
		void operator()(
			ExecutionPolicy && policy, //!< [in] execution policy
			Integral first,            //!< [in] first index
			Integral last,             //!< [in] last index
			UnaryFunction f            //!< [in] function to be called for every index in range
		) const {
			assert(first <= last);
			std::for_each(std::forward<ExecutionPolicy>(policy), iterator<Integral>{first, Integral{1}}, iterator<Integral>{last}, f);
		}

		template<typename ExecutionPolicy, typename Integral, typename UnaryFunction, typename = std::enable_if_t<std::is_execution_policy_v<internal::remove_cvref_t<ExecutionPolicy>>>>
		constexpr
		void operator()(
			ExecutionPolicy && policy, //!< [in] execution policy
			Integral count,            //!< [in] count of loop iterations - range will be [0, count)
			UnaryFunction f            //!< [in] function to be called for every index in range
		) const {
			assert(count >= Integral{0});
			std::for_each(std::forward<ExecutionPolicy>(policy), iterator<Integral>{Integral{0}, Integral{1}}, iterator<Integral>{count}, f);
		}

		template<typename Integral, typename UnaryFunction>
		constexpr
		void operator()(
			Integral first, //!< [in] first index
			Integral last,  //!< [in] last index
			Integral step,  //!< [in] stride between successive calls to f
			UnaryFunction f //!< [in] function to be called for every index in range
		) const {
			assert(first <= last && step > Integral{0});
			for(; first < last; first += step) f(first);
		}

		template<typename Integral, typename UnaryFunction>
		constexpr
		void operator()(
			Integral first, //!< [in] first index
			Integral last,  //!< [in] last index
			UnaryFunction f //!< [in] function to be called for every index in range
		) const {
			assert(first <= last);
			for(; first < last; ++first) f(first);
		}

		template<typename Integral, typename UnaryFunction>
		constexpr
		void operator()(
			Integral count, //!< [in] count of loop iterations - range will be [0, count)
			UnaryFunction f //!< [in] function to be called for every index in range
		) const {
			for(Integral i{0}; i < count; ++i) f(i);
		}
	} for_n;
}
