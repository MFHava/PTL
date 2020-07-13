
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
	class {
		template<typename ForwardIterator>
		class unary_iterator final {
			ForwardIterator it, end{it};
			std::vector<char>::const_iterator pred;

			using nested_traits = std::iterator_traits<ForwardIterator>;

			void move() noexcept {
				do {
					++it;
					++pred;
				} while(it != end && !*pred);
			}
		public:
			using difference_type   = typename nested_traits::difference_type;
			using value_type        = typename nested_traits::value_type;
			using pointer           = typename nested_traits::pointer;
			using reference         = typename nested_traits::reference;
			using iterator_category = std::forward_iterator_tag;

			unary_iterator() noexcept =default;
			unary_iterator(ForwardIterator it) noexcept : it{it} {}
			unary_iterator(ForwardIterator first, ForwardIterator last, const std::vector<char> & preds) noexcept : it{first}, end{last}, pred{std::begin(preds)} {
				if(first == last) return;
				if(!*pred) move();
			}

			auto operator*() const noexcept -> reference { return *it; }
			auto operator->() const noexcept { return it; }

			auto operator++() noexcept -> unary_iterator & {
				move();
				return *this;
			}
			auto operator++(int) noexcept -> unary_iterator {
				auto tmp{*this};
				move();
				return tmp;
			}

			friend
			auto operator==(const unary_iterator & lhs, const unary_iterator & rhs) noexcept -> bool { return lhs.it == rhs.it; }
			friend
			auto operator!=(const unary_iterator & lhs, const unary_iterator & rhs) noexcept -> bool { return !(lhs == rhs); }
		};

		template<typename ForwardIterator1, typename ForwardIterator2>
		class binary_iterator final {
			ForwardIterator1 it1, end{it1};
			ForwardIterator2 it2;
			std::vector<char>::const_iterator pred;

			using nested_traits1 = std::iterator_traits<ForwardIterator1>;
			using nested_traits2 = std::iterator_traits<ForwardIterator2>;

			void move() noexcept {
				do {
					++it1;
					++it2;
					++pred;
				} while(it1 != end && !*pred);
			}
		public:
			using difference_type   = std::common_type_t<typename nested_traits1::difference_type, typename nested_traits2::difference_type>;
			using value_type        = std::pair<typename nested_traits1::value_type, typename nested_traits2::value_type>;
			using pointer           = void; //TODO: this member is pretty hard to define, but is hopefully not needed for std::transform
			using reference         = std::pair<typename nested_traits1::reference, typename nested_traits2::reference>;
			using iterator_category = std::forward_iterator_tag;

			binary_iterator() noexcept =default;
			binary_iterator(ForwardIterator1 it) noexcept : it1{it} {}
			binary_iterator(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, const std::vector<char> & preds) noexcept : it1{first1}, end{last1}, it2{first2}, pred{std::begin(preds)} {
				if(first1 == last1) return;
				if(!*pred) move();
			}

			auto operator*() const noexcept -> reference { return {*it1, *it2}; }
			auto operator->() const noexcept -> pointer =delete;

			auto operator++() noexcept -> binary_iterator & {
				move();
				return *this;
			}
			auto operator++(int) noexcept -> binary_iterator {
				auto tmp{*this};
				move();
				return tmp;
			}

			friend
			auto operator==(const binary_iterator & lhs, const binary_iterator & rhs) noexcept -> bool { return lhs.it1 == rhs.it1; }
			friend
			auto operator!=(const binary_iterator & lhs, const binary_iterator & rhs) noexcept -> bool { return !(lhs == rhs); }
		};
	public:
		template<typename ExecutionPolicy, typename ForwardIterator1, typename ForwardIterator2, typename UnaryPredicate, typename UnaryOperation, typename = std::enable_if_t<std::is_execution_policy_v<internal::remove_cvref_t<ExecutionPolicy>>>>
		constexpr
		auto operator()(
			ExecutionPolicy && policy, //!< [in] execution policy
			ForwardIterator1 first,    //!< [in] begin of input range
			ForwardIterator1 last,     //!< [in] end of input range
			ForwardIterator2 result,   //!< [in] begin of destination range, may be equal to first
			UnaryPredicate pred,       //!< [in] unary predicate which returns true for elements to transform
			UnaryOperation op          //!< [in] unary operation to apply for transformation
		) const -> ForwardIterator2 {
			std::vector<char> flags(std::distance(first, last), false);
			std::transform(policy, first, last, std::begin(flags), pred);

			return std::transform(std::forward<ExecutionPolicy>(policy), unary_iterator{first, last, flags}, unary_iterator{last}, result, op);
		}

		template<typename ExecutionPolicy, typename ForwardRange, typename ForwardIterator, typename UnaryPredicate, typename UnaryOperation, typename = std::enable_if_t<std::is_execution_policy_v<internal::remove_cvref_t<ExecutionPolicy>>>>
		constexpr
		auto operator()(
			ExecutionPolicy && policy, //!< [in] execution policy
			ForwardRange && range,     //!< [in] input range
			ForwardIterator result,    //!< [in] begin of destination range, may be equal to first
			UnaryPredicate pred,       //!< [in] unary predicate which returns true for elements to transform
			UnaryOperation op          //!< [in] unary operation to apply for transformation
		) const -> ForwardIterator {
			using std::begin;
			using std::end;
			return (*this)(std::forward<ExecutionPolicy>(policy), begin(range), end(range), result, pred, op);
		}

		template<typename ExecutionPolicy, typename ForwardIterator1, typename ForwardIterator2, typename ForwardIterator3, typename BinaryPredicate, typename BinaryOperation, typename = std::enable_if_t<std::is_execution_policy_v<internal::remove_cvref_t<ExecutionPolicy>>>>
		constexpr
			auto operator()(
				ExecutionPolicy && policy, //!< [in] execution policy
				ForwardIterator1 first1,   //!< [in] begin of first range
				ForwardIterator1 last1,    //!< [in] end of first range
				ForwardIterator2 first2,   //!< [in] begin of second range - will be [first2, N) with N=distance(first1, last1)
				ForwardIterator3 result,   //!< [in] begin of destination range, may be equal to first1 or first2
				BinaryPredicate pred,      //!< [in] binary predicate which returns true for elements to transform
				BinaryOperation op         //!< [in] binary operation to apply for transformation
		) const -> ForwardIterator3 {
			std::vector<char> flags(std::distance(first1, last1), false);
			std::transform(policy, first1, last1, first2, std::begin(flags), pred);

			using Iterator = binary_iterator<ForwardIterator1, ForwardIterator2>;
			return std::transform(std::forward<ExecutionPolicy>(policy), Iterator{first1, last1, first2, flags}, Iterator{last1}, result, [&](const auto & pair) { return op(pair.first, pair.second); });
		}


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
