
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <algorithm>
#include <execution>
#include <type_traits>
#include "internal/cpp20_emulation.hpp"

namespace ptl {
	namespace internal {
		template<typename Type>
		inline
		constexpr
		bool is_execution_policy_v{std::is_execution_policy_v<remove_cvref_t<Type>>};

		template<typename ExecutionPolicy>
		inline
		constexpr
		bool is_sequenced_v{std::is_same_v<remove_cvref_t<ExecutionPolicy>, std::execution::sequenced_policy>};
	}

	inline
	constexpr
	class {
		template<typename ForwardIterator, typename OutputIterator, typename UnaryPredicate, typename UnaryOperation>
		auto sequenced(ForwardIterator first, ForwardIterator last, OutputIterator result, UnaryPredicate pred, UnaryOperation op) const noexcept -> OutputIterator { return (*this)(first, last, result, pred, op); }
		template<typename ForwardIterator1, typename ForwardIterator2, typename OutputIterator, typename UnaryPredicate, typename UnaryOperation>
		auto sequenced(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, OutputIterator result, UnaryPredicate pred, UnaryOperation op) const noexcept -> OutputIterator { return (*this)(first1, last1, first2, result, pred, op); }
	public:
		template<typename ExecutionPolicy, typename ForwardIterator1, typename ForwardIterator2, typename UnaryPredicate, typename UnaryOperation, typename = std::enable_if_t<internal::is_execution_policy_v<ExecutionPolicy>>>
		constexpr
		//precondition: [first, last) is valid range
		//precondition: [result, N) is valid range, N=count(x in [first, last) | pred(x) == true)
		auto operator()(ExecutionPolicy && policy, ForwardIterator1 first, ForwardIterator1 last, ForwardIterator2 result, UnaryPredicate pred, UnaryOperation op) const {
			if constexpr(internal::is_sequenced_v<ExecutionPolicy>) return sequenced(first, last, result, pred, op);
			else {
				using Pointer = typename std::iterator_traits<ForwardIterator1>::pointer;
				constexpr Pointer none{nullptr};

				std::vector<Pointer> ptrs(std::distance(first, last));
				std::transform(policy, first, last, std::begin(ptrs), [&](auto && val) { return pred(val) ? &val : none; });
				const auto end{std::remove(policy, std::begin(ptrs), std::end(ptrs), none)};
				return std::transform(std::forward<ExecutionPolicy>(policy), std::begin(ptrs), end, result, [&](auto && ptr) { return op(*ptr); });
			}
		}

		template<typename ExecutionPolicy, typename ForwardRange, typename ForwardIterator, typename UnaryPredicate, typename UnaryOperation, typename = std::enable_if_t<internal::is_execution_policy_v<ExecutionPolicy>>>
		constexpr
		//precondition: [result, N) is valid range, N=count(x in range | pred(x) == true)
		auto operator()(ExecutionPolicy && policy, ForwardRange && range, ForwardIterator result, UnaryPredicate pred, UnaryOperation op) const {
			using std::begin;
			using std::end;
			return (*this)(std::forward<ExecutionPolicy>(policy), begin(range), end(range), result, pred, op);
		}

		template<typename ExecutionPolicy, typename ForwardIterator1, typename ForwardIterator2, typename ForwardIterator3, typename BinaryPredicate, typename BinaryOperation, typename = std::enable_if_t<internal::is_execution_policy_v<ExecutionPolicy>>>
		constexpr
		//precondition: [first1, last1) is valid range
		//precondition: [first2, N) is valid range, N=distance(first1, last1)
		//precondition: [result, N) is valid range, N=count(x in [first1, last1) | pred(x) == true)
		auto operator()(ExecutionPolicy && policy, ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator3 result, BinaryPredicate pred, BinaryOperation op) const {
			if constexpr(internal::is_sequenced_v<ExecutionPolicy>) return sequenced(first1, last1, first2, result, pred, op);
			else {
				using Pointers = std::pair<typename std::iterator_traits<ForwardIterator1>::pointer, typename std::iterator_traits<ForwardIterator2>::pointer>;
				constexpr Pointers none{nullptr, nullptr};

				std::vector<Pointers> ptrs(std::distance(first1, last1));
				std::transform(policy, first1, last1, first2, std::begin(ptrs), [&](auto && val1, auto && val2) { return pred(val1, val2) ? Pointers{&val1, &val2} : none; });
				const auto end{std::remove(policy, std::begin(ptrs), std::end(ptrs), none)};
				return std::transform(std::forward<ExecutionPolicy>(policy), std::begin(ptrs), end, result, [&](auto && pair) { return op(*pair.first, *pair.second); });
			}
		}


		template<typename InputIterator, typename OutputIterator, typename UnaryPredicate, typename UnaryOperation>
		constexpr
		//precondition: [first, last) is valid range
		//precondition: [result, N) is valid range, N=count(x in [first, last) | pred(x) == true)
		auto operator()(InputIterator first, InputIterator last, OutputIterator result, UnaryPredicate pred, UnaryOperation op) const -> OutputIterator {
			for(; first != last; ++first)
				if(pred(*first))
					*result++ = op(*first);
			return result;
		}

		template<typename InputRange, typename OutputIterator, typename UnaryPredicate, typename UnaryOperation>
		constexpr
		//precondition: [result, N) is valid range, N=count(x in range | pred(x) == true)
		auto operator()(InputRange && range, OutputIterator result, UnaryPredicate pred, UnaryOperation op) const -> OutputIterator {
			using std::begin;
			using std::end;
			return (*this)(begin(range), end(range), result, pred, op);
		}

		template<typename InputIterator1, typename InputIterator2, typename OutputIterator, typename BinaryPredicate, typename BinaryOperation>
		constexpr
		//precondition: [first1, last1) is valid range
		//precondition: [first2, N) is valid range, N=distance(first1, last1)
		//precondition: [result, N) is valid range, N=count(x in [first1, last1) | pred(x) == true)
		auto operator()(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, OutputIterator result, BinaryPredicate pred, BinaryOperation op) const -> OutputIterator {
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

		template<typename Integral, typename UnaryFunction>
		void sequenced(Integral first, Integral last, Integral step, UnaryFunction f) const noexcept { (*this)(first, last, step, f); }
	public:
		template<typename ExecutionPolicy, typename Integral, typename UnaryFunction, typename = std::enable_if_t<internal::is_execution_policy_v<ExecutionPolicy>>>
		constexpr
		//precondition: first <= last
		//precondition: step > 0
		void operator()(ExecutionPolicy && policy, Integral first, Integral last, Integral step, UnaryFunction f) const {
			if constexpr(internal::is_sequenced_v<ExecutionPolicy>) sequenced(first, last, step, f);
			else std::for_each(std::forward<ExecutionPolicy>(policy), iterator<Integral>{first, step}, iterator<Integral>{last}, f);
		}

		template<typename ExecutionPolicy, typename Integral, typename UnaryFunction, typename = std::enable_if_t<internal::is_execution_policy_v<ExecutionPolicy>>>
		constexpr
		//precondition: first <= last
		void operator()(ExecutionPolicy && policy, Integral first, Integral last, UnaryFunction f) const { (*this)(std::forward<ExecutionPolicy>(policy), first, last, Integral{1}, f); }

		template<typename ExecutionPolicy, typename Integral, typename UnaryFunction, typename = std::enable_if_t<internal::is_execution_policy_v<ExecutionPolicy>>>
		constexpr
		//precondition: times >= 0
		void operator()(ExecutionPolicy && policy, Integral times, UnaryFunction f) const { (*this)(std::forward<ExecutionPolicy>(policy), Integral{0}, times, f); }


		template<typename Integral, typename UnaryFunction>
		constexpr
		//precondition: first <= last
		//precondition: step > 0
		void operator()(Integral first, Integral last, Integral step, UnaryFunction f) const { for(; first < last; first += step) f(first); }

		template<typename Integral, typename UnaryFunction>
		constexpr
		//precondition: first <= last
		void operator()(Integral first, Integral last, UnaryFunction f) const { (*this)(first, last, Integral{1}, f); }

		template<typename Integral, typename UnaryFunction>
		constexpr
		//precondition: times >= 0
		void operator()(Integral times, UnaryFunction f) const { (*this)(Integral{0}, times, f); }
	} for_n;
}
