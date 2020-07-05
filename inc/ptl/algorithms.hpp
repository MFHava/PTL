
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>

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
}
