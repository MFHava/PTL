
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "compiler_detection.hpp"
#include <utility>
#include <boost/operators.hpp>

#define PTL_REQUIRES(args) ((void)0)
namespace ptl {
	namespace internal {
		template<typename Owner, typename ValueType>
		struct random_access_iterator final : boost::random_access_iterator_helper<random_access_iterator<Owner, ValueType>, ValueType, std::ptrdiff_t, ValueType *, ValueType &> {
			constexpr
			random_access_iterator() noexcept {}
			
			constexpr
			decltype(auto) operator++() noexcept { move(+1); return *this; }
			constexpr
			decltype(auto) operator--() noexcept { move(-1); return *this; }
			
			constexpr
			auto operator*() const noexcept -> ValueType & {
				PTL_REQUIRES(ptr);
				return *ptr;
			}
			
			constexpr
			decltype(auto) operator+=(std::ptrdiff_t count) noexcept { move(+count); return *this; }
			constexpr
			decltype(auto) operator-=(std::ptrdiff_t count) noexcept { move(-count); return *this; }
			
			friend
			constexpr
			auto operator-(const random_access_iterator & lhs, const random_access_iterator & rhs) noexcept -> std::ptrdiff_t { return lhs.ptr - rhs.ptr; }
			friend
			constexpr
			auto operator==(const random_access_iterator & lhs, const random_access_iterator & rhs) noexcept { return lhs.ptr == rhs.ptr; }
			friend
			constexpr
			auto operator< (const random_access_iterator & lhs, const random_access_iterator & rhs) noexcept { return lhs.ptr <  rhs.ptr; }
		private:
			friend Owner;
			
			constexpr
			void move(std::ptrdiff_t count) noexcept {
				PTL_REQUIRES(ptr);
				ptr += count;
			}
			
			explicit
			constexpr
			random_access_iterator(ValueType * ptr) noexcept : ptr{ptr} {}
			
			ValueType * ptr{nullptr};
		};

		template<typename Implementation, typename Base = boost::operators_detail::empty_base<Implementation>>
		class random_access_container_base : Base {
			constexpr
			auto self() const -> const Implementation & { return static_cast<const Implementation &>(*this); }
			constexpr
			auto self()       ->       Implementation & { return static_cast<      Implementation &>(*this); }
		public:
			constexpr
			decltype(auto) front() const noexcept { return self()[0]; }
			constexpr
			decltype(auto) front()       noexcept { return self()[0]; }
			constexpr
			decltype(auto) back() const noexcept { return self()[self().size() - 1]; }
			constexpr
			decltype(auto) back()       noexcept { return self()[self().size() - 1]; } 
			constexpr
			decltype(auto) at(std::size_t index) const {
				if(index >= self().size()) throw std::out_of_range{"index out of range"};
				return self()[index];
			}
			constexpr
			decltype(auto) at(std::size_t index)       {
				if(index >= self().size()) throw std::out_of_range{"index out of range"};
				return self()[index];
			}
			constexpr
			auto empty() const noexcept -> bool { return self().size() == 0; }
			constexpr
			auto cbegin() const noexcept { return self().begin(); }
			constexpr
			auto cend()   const noexcept { return self().end(); }
			constexpr
			auto rbegin()  const noexcept { return typename Implementation::const_reverse_iterator{self().end()}; }
			constexpr
			auto rbegin()        noexcept { return typename Implementation::      reverse_iterator{self().end()}; }
			constexpr
			auto crbegin() const noexcept { return typename Implementation::const_reverse_iterator{self().cend()}; }
			constexpr
			auto rend()    const noexcept { return typename Implementation::const_reverse_iterator{self().begin()}; }
			constexpr
			auto rend()          noexcept { return typename Implementation::      reverse_iterator{self().begin()}; }
			constexpr
			auto crend()   const noexcept { return typename Implementation::const_reverse_iterator{self().cbegin()}; }
		};

		template<typename Type>
		constexpr
		void swap(Type & lhs, Type & rhs) noexcept {
			auto tmp{std::move(lhs)};
			lhs = std::move(rhs);
			rhs = std::move(tmp);
		}
	}
}
