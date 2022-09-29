
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace ptl {
	namespace internal_array_ref {
		struct invalid;

		constexpr
		auto data(...) noexcept -> invalid;
	}

	//! @brief non-owning reference to array
	//! @tparam Type type of the referenced array
	template<typename Type>
	class array_ref final { //TODO: static_assert(sizeof(array_ref<T>) == 2 * sizeof(T *));
		Type * first_{nullptr}, * last_{nullptr};

		template<typename ContiguousRange>
		class is_compatible_range final {
			struct valid;
			using invalid = internal_array_ref::invalid;

			static
			constexpr
			auto rebind(Type * ) noexcept -> valid;
			static
			constexpr
			auto rebind(...) noexcept -> invalid;

			static
			constexpr
			auto test() noexcept -> bool {
				using std::data;
				using internal_array_ref::data;
				using Pointer = decltype(data(std::declval<ContiguousRange>()));
				if constexpr(std::is_same_v<Pointer, invalid>) return false;
				else {
					static_assert(std::is_pointer_v<Pointer>);
					using Rebind = decltype(rebind(Pointer{}));
					return std::is_same_v<Rebind, valid>;
				}
			}
		public:
			static
			constexpr
			bool value{test()};
		};

		template<typename ContiguousRange>
		static
		constexpr
		bool is_compatible_range_v{is_compatible_range<ContiguousRange>::value};

		template<bool IsConst>
		struct contiguous_iterator final {
			//TODO: [C++20] using iterator_concept = std::contiguous_iterator_tag;
			using iterator_category = std::random_access_iterator_tag;
			using value_type        = std::remove_cv_t<Type>;
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
			friend array_ref;

			constexpr
			contiguous_iterator(pointer ptr) noexcept : ptr{ptr} {}

			pointer ptr{nullptr};
		};
	public:
		using element_type           = Type;
		using value_type             = std::remove_cv_t<element_type>;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       element_type &;
		using const_reference        = const element_type &;
		using pointer                =       element_type *;
		using const_pointer          = const element_type *;
		using iterator               = contiguous_iterator<false>;
		using const_iterator         = contiguous_iterator<true>;
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr
		array_ref() noexcept =default;

		//! @brief construct from two pointers
		//! @param[in] first start of the referenced array
		//! @param[in] last end of the referenced array
		//! @attention [first, last) must be valid!
		constexpr
		array_ref(pointer first, pointer last) noexcept : first_{first}, last_{last} {} //TODO: [C++20] use contiguous_iterators //TODO: [C++??] precondition(first <= last);

		//! @brief construct from pointer and size
		//! @param[in] ptr start of the referenced array
		//! @param[in] count count of elements in the referenced array
		//! @attention [ptr, count) must be valid!
		constexpr
		array_ref(pointer ptr, size_type count) noexcept : array_ref{ptr, ptr + count} {} //TODO: [C++20] use contiguous_iterators + sized_sentinal

		//! @brief construct from ContiguousRange
		//! @tparam ContiguousRange compatible range type that fulfills the ContiguousRange-requirements
		//! @param[in] range range to reference
		template<typename ContiguousRange, typename = std::enable_if_t<is_compatible_range_v<ContiguousRange>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		array_ref(ContiguousRange && range) noexcept : array_ref{std::data(range), std::size(range)} {}

		constexpr
		auto data() const noexcept -> pointer { return first_; }
		[[nodiscard]]
		constexpr
		auto empty() const noexcept -> bool { return size() == 0; }
		constexpr
		auto size() const noexcept -> size_type { return last_ - first_; } 

		constexpr
		auto front() const noexcept -> reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto back() const noexcept -> reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto operator[](size_type index) const noexcept -> reference { return *(data() + index); } //TODO: [C++??] precondition(index < size());
		constexpr
		auto at(size_type index) const -> reference {
			if(index >= size()) throw std::out_of_range{"ptl::array::at - index out of range"};
			return (*this)[index];
		}

		constexpr
		auto first(size_type count) const noexcept -> array_ref { return {data(), count}; } //TODO: [C++??] precondition(count <= size());
		constexpr
		auto last(size_type count) const noexcept -> array_ref { return {data() + (size() - count), count}; } //TODO: [C++??] precondition(count <= size());
		constexpr
		auto subrange(size_type offset) const noexcept -> array_ref { return {data() + offset, size() - offset}; } //TODO: [C++??] precondition(offset <= size());
		constexpr
		auto subrange(size_type offset, size_type count) const noexcept -> array_ref { return {data() + offset, count}; } //TODO: [C++??] precondition(offset + count <= size());

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
	};

	template<typename Type, std::size_t Size>
	array_ref(Type(&)[Size]) -> array_ref<Type>;

	template<typename ContiguousRange>
	array_ref(const ContiguousRange &) -> array_ref<const typename ContiguousRange::value_type>; //TODO: [C++20] this deduction guide should be mergeable with the next one using ranges-traits...

	template<typename ContiguousRange>
	array_ref(      ContiguousRange &) -> array_ref<      typename ContiguousRange::value_type>;
}

//TODO: [C++20] mark as borrowed_range
//TODO: [C++20] mark as view
