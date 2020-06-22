
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <algorithm>
#include <stdexcept>

namespace ptl::internal {
	template<typename Implementation, typename Type>
	class contiguous_container_base {
		constexpr
		auto self() const noexcept -> const Implementation & { return static_cast<const Implementation &>(*this); }
		constexpr
		auto self()       noexcept ->       Implementation & { return static_cast<      Implementation &>(*this); }

		template<typename ValueType, bool IsConst>
		struct contiguous_iterator final {
			static_assert(!std::is_const_v<ValueType>);

			using iterator_category = std::random_access_iterator_tag;
			using value_type        = ValueType;
			using difference_type   = std::ptrdiff_t;
			using pointer           = std::conditional_t<IsConst, const ValueType, ValueType> *;
			using reference         = std::conditional_t<IsConst, const ValueType, ValueType> &;

			constexpr
			contiguous_iterator() noexcept =default;

			constexpr
			auto operator++() noexcept -> contiguous_iterator & {
				move(+1);
				return *this;
			}
			constexpr
			auto operator++(int) noexcept -> contiguous_iterator {
				auto tmp{*this};
				++*this;
				return tmp;
			}

			constexpr
			auto operator--() noexcept -> contiguous_iterator & {
				move(-1);
				return *this;
			}
			constexpr
			auto operator--(int) noexcept -> contiguous_iterator {
				auto tmp{*this};
				--*this;
				return tmp;
			}

			constexpr
			auto operator*() const noexcept -> reference {
				//pre-condition: ptr
				return *ptr;
			}
			constexpr
			auto operator->() const noexcept -> pointer {
				//pre-condition: ptr
				return ptr;
			}

			constexpr
			auto operator[](difference_type index) const noexcept -> reference { return *(*this + index); }

			constexpr
			auto operator+=(difference_type count) noexcept -> contiguous_iterator & {
				move(+count);
				return *this;
			}
			friend
			constexpr
			auto operator+(contiguous_iterator lhs, difference_type rhs) noexcept -> contiguous_iterator {
				lhs += rhs;
				return lhs;
			}
			friend
			constexpr
			auto operator+(difference_type lhs, contiguous_iterator rhs) noexcept -> contiguous_iterator { return rhs + lhs; }

			constexpr
			auto operator-=(difference_type count) noexcept -> contiguous_iterator & {
				move(-count);
				return *this;
			}
			friend
			constexpr
			auto operator-(contiguous_iterator lhs, difference_type rhs) noexcept -> contiguous_iterator {
				lhs -= rhs;
				return lhs;
			}

			friend
			constexpr
			auto operator-(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> difference_type { return lhs.ptr - rhs.ptr; }

			constexpr
			operator contiguous_iterator<ValueType, true>() const noexcept { return contiguous_iterator<ValueType, true>{ptr}; }

			friend
			constexpr
			auto operator==(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return lhs.ptr == rhs.ptr; }
			friend
			constexpr
			auto operator!=(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return !(lhs == rhs); }
			friend
			constexpr
			auto operator< (const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return lhs.ptr <  rhs.ptr; }
			friend
			constexpr
			auto operator> (const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return rhs < lhs; }
			friend
			constexpr
			auto operator<=(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return !(lhs > rhs); }
			friend
			constexpr
			auto operator>=(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept { return !(lhs < rhs); }
		private:
			constexpr
			operator contiguous_iterator<ValueType, false>() const noexcept { return contiguous_iterator<ValueType, false>{const_cast<ValueType *>(ptr)}; }

			friend contiguous_container_base;

			constexpr
			void move(difference_type count) noexcept {
				//pre-condition: ptr || (!ptr && !count)
				ptr += count;
			}

			constexpr
			explicit
			contiguous_iterator(pointer ptr) noexcept : ptr{ptr} {}

			pointer ptr{nullptr};
		};
	protected:
		constexpr
		contiguous_container_base() noexcept =default;
	public:
		using value_type             = std::remove_cv_t<Type>;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       Type &;
		using const_reference        = const Type &;
		using pointer                =       Type *;
		using const_pointer          = const Type *;
	private:
		using mutable_iterator       = contiguous_iterator<value_type, false>;
	public:
		using const_iterator         = contiguous_iterator<value_type, true>;
		using iterator               = std::conditional_t<std::is_const_v<Type>, const_iterator, mutable_iterator>;
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr
		auto front() const noexcept -> const_reference {
			//pre-condition: !empty()
			return (*this)[0];
		}
		constexpr
		auto front()       noexcept ->       reference {
			//pre-condition: !empty()
			return (*this)[0];
		}
		constexpr
		auto back() const noexcept -> const_reference {
			//pre-condition: !empty()
			return (*this)[self().size() - 1];
		}
		constexpr
		auto back()       noexcept ->       reference {
			//pre-condition: !empty()
			return (*this)[self().size() - 1];
		}

		constexpr
		auto operator[](std::size_t index) const noexcept -> const_reference {
			//pre-condition: index < self().size()
			return *(begin() + index);
		}
		constexpr
		auto operator[](std::size_t index)       noexcept ->       reference {
			//pre-condition: index < self().size()
			return *(begin() + index);
		}
		constexpr
		auto at(std::size_t index) const -> const_reference {
			if(index >= self().size()) throw std::out_of_range{"index out of range"};
			return (*this)[index];
		}
		constexpr
		auto at(std::size_t index)       ->       reference {
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
		auto operator==(const Implementation & lhs, const Implementation & rhs) noexcept { return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
		friend
		constexpr
		auto operator!=(const Implementation & lhs, const Implementation & rhs) noexcept { return !(lhs == rhs); }
		friend
		constexpr
		auto operator< (const Implementation & lhs, const Implementation & rhs) noexcept { return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
		friend
		constexpr
		auto operator> (const Implementation & lhs, const Implementation & rhs) noexcept { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const Implementation & lhs, const Implementation & rhs) noexcept { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const Implementation & lhs, const Implementation & rhs) noexcept { return !(lhs < rhs); }
	};
}

