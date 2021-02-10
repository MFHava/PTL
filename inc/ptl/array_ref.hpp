
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <iterator>

namespace ptl {
	//! @brief non-owning reference to array
	//! @tparam Type type of the referenced array
	template<typename Type>
	class array_ref final { //TODO: static_assert(sizeof(array_ref<T>) == 2 * sizeof(T *));
		Type * ptrs[2]{nullptr, nullptr};

		template<typename ContiguousRange>
		class is_compatible_range final {
			struct valid;
			struct invalid;

			static
			constexpr
			auto data(...) noexcept -> invalid;

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
	public:
		using element_type     = Type;
		using value_type       = std::remove_cv_t<element_type>;
		using size_type        = std::size_t;
		using difference_type  = std::ptrdiff_t;
		using reference        =       element_type &;
		using const_reference  = const element_type &;
		using pointer          =       element_type *;
		using const_pointer    = const element_type *;
		struct iterator final {
			//TODO: [C++20] using iterator_concept = std::contiguous_iterator_tag;
			using iterator_category = std::random_access_iterator_tag;
			using value_type        = array_ref::value_type;
			using difference_type   = array_ref::difference_type;
			using pointer           = array_ref::pointer;
			using reference         = array_ref::reference;

			constexpr
			iterator() noexcept =default;

			constexpr
			auto operator++() noexcept -> iterator & { ++ptr; return *this; }
			constexpr
			auto operator++(int) noexcept -> iterator {
				auto tmp{*this};
				++*this;
				return tmp;
			}

			constexpr
			auto operator--() noexcept -> iterator & { --ptr; return *this; }
			constexpr
			auto operator--(int) noexcept -> iterator {
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
			auto operator+=(difference_type count) noexcept -> iterator & { ptr += count; return *this; }
			friend
			constexpr
			auto operator+(iterator lhs, difference_type rhs) noexcept -> iterator {
				lhs += rhs;
				return lhs;
			}
			friend
			constexpr
			auto operator+(difference_type lhs, iterator rhs) noexcept -> iterator { return rhs + lhs; }

			constexpr
			auto operator-=(difference_type count) noexcept -> iterator & { ptr -= count; return *this; }
			friend
			constexpr
			auto operator-(iterator lhs, difference_type rhs) noexcept -> iterator {
				lhs -= rhs;
				return lhs;
			}

			friend
			constexpr
			auto operator-(const iterator & lhs, const iterator & rhs) noexcept -> difference_type { return lhs.ptr - rhs.ptr; }

			friend
			constexpr
			auto operator==(const iterator & lhs, const iterator & rhs) noexcept -> bool { return lhs.ptr == rhs.ptr; }
			friend
			constexpr
			auto operator!=(const iterator & lhs, const iterator & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] remove as implicitly generated
			//TODO: [C++20] replace the ordering operators by <=>
			friend
			constexpr
			auto operator< (const iterator & lhs, const iterator & rhs) noexcept -> bool { return lhs.ptr < rhs.ptr; }
			friend
			constexpr
			auto operator> (const iterator & lhs, const iterator & rhs) noexcept -> bool { return rhs < lhs; }
			friend
			constexpr
			auto operator<=(const iterator & lhs, const iterator & rhs) noexcept -> bool { return !(lhs > rhs); }
			friend
			constexpr
			auto operator>=(const iterator & lhs, const iterator & rhs) noexcept -> bool { return !(lhs < rhs); }
		private:
			friend array_ref;

			constexpr
			iterator(pointer ptr) noexcept : ptr{ptr} {}

			pointer ptr{nullptr};
		};
		using reverse_iterator = std::reverse_iterator<iterator>;

		constexpr
		array_ref() noexcept =default;

		//! @brief construct from two pointers
		//! @param[in] first start of the referenced array
		//! @param[in] last end of the referenced array
		//! @attention [first, last) must be valid!
		constexpr
		array_ref(pointer first, pointer last) noexcept : ptrs{first, last} {} //TODO: [C++20] use contiguous_iterators //TODO: [C++??] precondition(first <= last);

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
		auto data() const noexcept -> pointer { return ptrs[0]; }
		constexpr
		auto empty() const noexcept -> bool { return size() == 0; }
		constexpr
		auto size() const noexcept -> size_type { return ptrs[1] - ptrs[0]; } 

		constexpr
		auto front() const noexcept -> reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto back() const noexcept -> reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		constexpr
		auto operator[](size_type index) const noexcept -> reference { return *(data() + index); } //TODO: [C++??] precondition(index < size());

		constexpr
		auto first(size_type count) const noexcept -> array_ref { return {data(), count}; } //TODO: [C++??] precondition(count <= size());
		constexpr
		auto last(size_type count) const noexcept -> array_ref { return {data() + (size() - count), count}; } //TODO: [C++??] precondition(count <= size());
		constexpr
		auto subrange(size_type offset) const noexcept -> array_ref { return {data() + offset, size() - offset}; } //TODO: [C++??] precondition(offset <= size());
		constexpr
		auto subrange(size_type offset, size_type count) const noexcept -> array_ref { return {data() + offset, count}; } //TODO: [C++??] precondition(offset + count <= size());

		constexpr
		auto begin() const noexcept -> iterator { return data(); }
		constexpr
		auto end() const noexcept -> iterator { return begin() + size(); }
		constexpr
		auto rbegin() const noexcept -> reverse_iterator { return reverse_iterator{end()}; }
		constexpr
		auto rend() const noexcept -> reverse_iterator { return reverse_iterator{begin()}; }

		constexpr
		void swap(array_ref & other) noexcept { for(auto i{0}; i < 2; ++i) std::swap(ptrs[i], other.ptrs[i]); }
		friend
		constexpr
		void swap(array_ref & lhs, array_ref & rhs) noexcept { lhs.swap(rhs); }
	};

	template<typename Type, std::size_t Size>
	array_ref(Type (&)[Size]) -> array_ref<Type>;

	template<typename ContiguousRange>
	array_ref(const ContiguousRange &) -> array_ref<const typename ContiguousRange::value_type>;

	template<typename ContiguousRange>
	array_ref(      ContiguousRange &) -> array_ref<      typename ContiguousRange::value_type>;
}

//TODO: [C++20] mark as borrowed_range
//TODO: [C++20] mark as view
