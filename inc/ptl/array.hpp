
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace ptl {
	namespace internal_array {
		template<typename Type, std::size_t Size>
		struct storage final { using type = Type[Size]; };

		template<typename Type>
		struct storage<Type, 0> { using type = Type *; };

		template<typename Type, std::size_t Size>
		using storage_t = typename storage<Type, Size>::type;
	}

	//! @brief a fixed-size array
	//! @tparam Type type of the stored array
	//! @tparam Size size of the stored array
	template<typename Type, std::size_t Size>
	class array final {
		static_assert(!std::is_const_v<Type>);
		static_assert(std::is_standard_layout_v<Type>); //TODO: this is probably too strict!
		static_assert(std::is_default_constructible_v<Type>);
		static_assert(std::is_copy_constructible_v<Type>);
		static_assert(std::is_nothrow_move_constructible_v<Type>);
		static_assert(std::is_copy_assignable_v<Type>);
		static_assert(std::is_nothrow_move_assignable_v<Type>);
		static_assert(std::is_nothrow_destructible_v<Type>);
		static_assert(std::is_nothrow_swappable_v<Type>);

		internal_array::storage_t<Type, Size> values;

		template<std::size_t Index>
		friend
		constexpr
		auto get(const array & self) noexcept -> const Type & {
			static_assert(Index < Size);
			return self[Index];
		}
		template<std::size_t Index>
		friend
		constexpr
		auto get(      array & self) noexcept ->       Type & {
			static_assert(Index < Size);
			return self[Index];
		}
		template<std::size_t Index>
		friend
		constexpr
		auto get(const array && self) noexcept -> const Type && {
			static_assert(Index < Size);
			return std::move(self[Index]);
		}
		template<std::size_t Index>
		friend
		constexpr
		auto get(      array && self) noexcept ->       Type && {
			static_assert(Index < Size);
			return std::move(self[Index]);
		}

		template<bool IsConst>
		struct contiguous_iterator final {
			//TODO: [C++20] using iterator_concept = std::contiguous_iterator_tag;
			using iterator_category = std::random_access_iterator_tag;
			using value_type        = Type;
			using difference_type   = std::ptrdiff_t;
			using pointer           = std::conditional_t<IsConst, const Type, Type> *;
			using reference         = std::conditional_t<IsConst, const Type, Type> &;

			constexpr
			contiguous_iterator() noexcept =default;

			constexpr
			auto operator++() noexcept -> contiguous_iterator & { ++ptr; return *this; }
			constexpr
			auto operator++(int) noexcept -> contiguous_iterator {
				auto tmp{*this};
				++*this;
				return tmp;
			}

			constexpr
			auto operator--() noexcept -> contiguous_iterator & { --ptr; return *this; }
			constexpr
			auto operator--(int) noexcept -> contiguous_iterator {
				auto tmp{*this};
				--*this;
				return tmp;
			}

			constexpr
			auto operator*() const noexcept -> reference { return *ptr; }
			constexpr
			auto operator->() const noexcept -> pointer { return ptr; }

			constexpr
			auto operator[](difference_type index) const noexcept -> reference { return *(*this + index); }

			constexpr
			auto operator+=(difference_type count) noexcept -> contiguous_iterator & { ptr += count; return *this; }
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
			auto operator-=(difference_type count) noexcept -> contiguous_iterator & { ptr -= count; return *this; }
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
			operator contiguous_iterator<true>() const noexcept { return contiguous_iterator<true>{ptr}; }

			friend
			constexpr
			auto operator==(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> bool { return lhs.ptr == rhs.ptr; }
			friend
			constexpr
			auto operator!=(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] remove as implicitly generated
			//TODO: [C++20] replace the ordering operators by <=>
			friend
			constexpr
			auto operator< (const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> bool { return lhs.ptr < rhs.ptr; }
			friend
			constexpr
			auto operator> (const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> bool { return rhs < lhs; }
			friend
			constexpr
			auto operator<=(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> bool { return !(lhs > rhs); }
			friend
			constexpr
			auto operator>=(const contiguous_iterator & lhs, const contiguous_iterator & rhs) noexcept -> bool { return !(lhs < rhs); }
		private:
			friend array;

			constexpr
			contiguous_iterator(pointer ptr) noexcept : ptr{ptr} {}

			pointer ptr{nullptr};
		};
	public:
		using value_type             = Type;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       Type &;
		using const_reference        = const Type &;
		using pointer                =       Type *;
		using const_pointer          = const Type *;
		using iterator               = contiguous_iterator<false>;
		using const_iterator         = contiguous_iterator<true>;
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		template<typename... Args, typename = std::enable_if_t<(std::is_convertible_v<Type, Args> &&...)>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		array(Args &&... args) : values{std::forward<Args>(args)...} {}

		constexpr
		auto data() const noexcept -> const_pointer { return values; }
		constexpr
		auto data()       noexcept ->       pointer { return values; }
		static
		constexpr
		auto size() noexcept -> size_type { return Size; }
		[[nodiscard]]
		static
		constexpr
		auto empty() noexcept -> bool { return size() == 0; }
		static
		constexpr
		auto max_size() noexcept -> size_type { return Size; }

		constexpr
		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto front()       noexcept ->       reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto back()       noexcept ->       reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());

		constexpr
		auto operator[](std::size_t index) const noexcept -> const_reference { return *(data() + index); } //TODO: [C++??] precondition(index < size());
		constexpr
		auto operator[](std::size_t index)       noexcept ->       reference { return *(data() + index); } //TODO: [C++??] precondition(index < size());
		constexpr
		auto at(std::size_t index) const -> const_reference {
			if(index >= size()) throw std::out_of_range{"index out of range"};
			return (*this)[index];
		}
		constexpr
		auto at(std::size_t index)       ->       reference {
			if(index >= size()) throw std::out_of_range{"index out of range"};
			return (*this)[index];
		}

		constexpr
		void fill(const Type & value) { std::fill(begin(), end(), value); }

		constexpr
		auto begin() const noexcept -> const_iterator { return data(); }
		constexpr
		auto begin()       noexcept ->       iterator { return data(); }
		constexpr
		auto cbegin() const noexcept -> const_iterator { return begin(); }
		constexpr
		auto end()   const noexcept -> const_iterator { return begin() + size(); }
		constexpr
		auto end()         noexcept ->       iterator { return begin() + size(); }
		constexpr
		auto cend()   const noexcept -> const_iterator { return end(); }
		constexpr
		auto rbegin()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }
		constexpr
		auto rbegin()        noexcept ->       reverse_iterator { return reverse_iterator{end()}; }
		constexpr
		auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cend()}; }
		constexpr
		auto rend()    const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }
		constexpr
		auto rend()          noexcept ->       reverse_iterator { return reverse_iterator{begin()}; }
		constexpr
		auto crend()   const noexcept -> const_reverse_iterator { return const_reverse_iterator{cbegin()}; }

		constexpr
		void swap(array & other) noexcept { std::swap_ranges(begin(), end(), other.begin()); }
		friend
		constexpr
		void swap(array & lhs, array & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const array & lhs, const array & rhs) noexcept -> bool { return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
		friend
		constexpr
		auto operator!=(const array & lhs, const array & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const array & lhs, const array & rhs) noexcept -> bool { return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
		friend
		constexpr
		auto operator> (const array & lhs, const array & rhs) noexcept -> bool { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const array & lhs, const array & rhs) noexcept -> bool { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const array & lhs, const array & rhs) noexcept -> bool { return !(lhs < rhs); }
	};

	template<typename Type, typename... Types>
	array(Type, Types...) -> array<Type, 1 + sizeof...(Types)>;
}

namespace std {
	template<typename Type, std::size_t Size>
	struct tuple_size<ptl::array<Type, Size>> : std::integral_constant<std::size_t, Size> {};

	template<std::size_t Index, typename Type, std::size_t Size>
	struct tuple_element<Index, ptl::array<Type, Size>> { using type = Type; };
}
