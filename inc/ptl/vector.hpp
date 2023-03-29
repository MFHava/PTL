
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <memory>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace ptl {
	//! @brief a dynamically growing array
	//! @tparam Type element type of the array
	template<typename Type>
	class vector final { //TODO: [C++20] constexpr
		static_assert(std::is_standard_layout_v<Type>); //TODO: this is probably too strict!
		static_assert(std::is_default_constructible_v<Type>);
		static_assert(std::is_copy_constructible_v<Type>);
		static_assert(std::is_nothrow_move_constructible_v<Type>); //TODO: add support for types without move-support?!
		static_assert(std::is_copy_assignable_v<Type>);
		static_assert(std::is_nothrow_move_assignable_v<Type>); //TODO: add support for types without move-support?!
		static_assert(std::is_nothrow_destructible_v<Type>);
		static_assert(std::is_nothrow_swappable_v<Type>);

		static
		constexpr
		std::size_t min_capacity{10},
		            max_capacity{static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(Type)};
		static_assert(min_capacity < max_capacity);

		class storage_t final {
			void(*dealloc)(Type *) noexcept{nullptr};

			Type * ptr{nullptr};
			std::size_t cap{0}, siz{0};
		public:
			storage_t() noexcept =default;

			storage_t(std::size_t capacity) {
				if(capacity == 0) return;
				if(capacity > max_size()) throw std::length_error{"ptl::vector - allocation attempting to exceed max_size"};
				capacity = std::max(min_capacity, capacity);
				dealloc = +[](Type * ptr) noexcept { std::free(ptr); };
				ptr = static_cast<Type *>(std::calloc(capacity, sizeof(Type)));
				if(!ptr) throw std::bad_alloc{};
				cap = capacity;
				siz = 0;
			}

			storage_t(storage_t && other) noexcept : dealloc{std::exchange(other.dealloc, nullptr)}, ptr{std::exchange(other.ptr, nullptr)}, cap{std::exchange(other.cap, 0)}, siz{std::exchange(other.siz, 0)} {}

			auto operator=(storage_t && other) noexcept -> storage_t & {
				if(this != std::addressof(other)) { //TODO: [C++20] use [[likely]]
					if(dealloc) {
						std::destroy_n(ptr, siz);
						dealloc(ptr);
					}
					dealloc = std::exchange(other.dealloc, nullptr);
					ptr = std::exchange(other.ptr, nullptr);
					cap = std::exchange(other.cap, 0);
					siz = std::exchange(other.siz, 0);
				}
				return *this;
			}

			~storage_t() noexcept {
				if(!dealloc) return;
				std::destroy_n(ptr, siz);
				dealloc(ptr);
			}

			auto data() const noexcept -> const Type * { return ptr; }
			auto data()       noexcept ->       Type * { return ptr; }

			auto size() const noexcept -> std::size_t { return siz; }
			auto capacity() const noexcept -> std::size_t { return cap; }

			void set_size(std::size_t val) noexcept { siz = val; } //TODO: [C++??] precondition(val <= capacity());

			void swap(storage_t & other) noexcept {
				std::swap(ptr, other.ptr);
				std::swap(cap, other.cap);
				std::swap(siz, other.siz);
				std::swap(dealloc, other.dealloc);
			}
		} storage;

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
			friend vector;

			constexpr
			contiguous_iterator(pointer ptr) noexcept : ptr{ptr} {}

			constexpr
			operator contiguous_iterator<false>() const noexcept { return contiguous_iterator<false>{const_cast<Type *>(ptr)}; }

			pointer ptr{nullptr};
		};

		template<typename Func>
		void assign_impl(std::size_t required_size, Func func) {
			if(size() + required_size <= capacity()) { //no need for allocation nor double buffering
				func(data() + size());
				std::rotate(data(), data() + size(), data() + size() + required_size);
				std::destroy_n(data() + required_size, size());
				storage.set_size(required_size);
			} else { //do single allocation
				storage_t tmp{required_size};
				func(tmp.data());
				tmp.set_size(required_size);
				storage = std::move(tmp);
			}
		}

		template<typename Func>
		auto insert_impl(contiguous_iterator<true> pos, std::size_t required_size, Func func) {
			const auto offset{pos.ptr - data()};
			if(size() + required_size <= capacity()) { //no need for allocation nor double buffering
				func(data() + size());
				std::rotate(const_cast<Type *>(pos.ptr), data() + size(), data() + size() + required_size);
				storage.set_size(size() + required_size);
			} else { //do single allocation
				storage_t tmp{size() + std::max(size(), required_size)}; //TODO: evaluate performance of growth influenced by max(size, required_size)
				func(tmp.data() + offset);
				std::uninitialized_move_n(data(), offset, tmp.data());
				std::uninitialized_move(data() + offset, data() + size(), tmp.data() + offset + required_size);
				tmp.set_size(size() + required_size);
				storage = std::move(tmp);
			}
			return begin() + offset;
		}

		template<typename Func>
		void resize_impl(std::size_t new_size, Func func) {
			if(new_size == size()) return;
			if(new_size < size()) erase(begin() + new_size, end());
			else {
				storage_t tmp{new_size};
				func(tmp.data() + size(), new_size - size());
				std::uninitialized_move_n(data(), size(), tmp.data());
				tmp.set_size(new_size);
				storage = std::move(tmp);
			}
		}
	public:
		using value_type             = Type;
		using size_type              = std::size_t;
		using difference_type        = std::ptrdiff_t;
		using reference              =       value_type &;
		using const_reference        = const value_type &;
		using pointer                =       value_type *;
		using const_pointer          = const value_type *;
		using iterator               = contiguous_iterator<false>;
		using const_iterator         = contiguous_iterator<true>;
		using reverse_iterator       = std::reverse_iterator<      iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		vector() noexcept =default;
		vector(const vector & other) : vector(other.data(), other.data() + other.size()) {}
		vector(vector &&) noexcept =default;
		auto operator=(const vector & other) -> vector & {
			if(this != std::addressof(other)) assign(other.begin(), other.end()); //TODO: [C++20] use [[likely]] on condition
			return *this;
		}
		auto operator=(vector &&) noexcept -> vector & =default;
		~vector() noexcept =default;

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		vector(InputIterator first, InputIterator last) { std::copy(first, last, std::back_inserter(*this)); }
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		vector(ForwardIterator first, ForwardIterator last) { assign(first, last); }

		vector(size_type count) { resize(count); }
		vector(size_type count, const Type & value) { resize(count, value); }

		vector(std::initializer_list<Type> ilist) : vector(ilist.begin(), ilist.end()) {}

		auto operator=(std::initializer_list<Type> ilist) -> vector & {
			assign(ilist.begin(), ilist.end());
			return *this;
		}

		auto operator[](size_type index) const noexcept -> const_reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto operator[](size_type index)       noexcept ->       reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto at(size_type index) const -> const_reference {
			if(index >= size()) throw std::out_of_range{"ptl::vector::at - index out of range"};
			return (*this)[index];
		}
		auto at(size_type index)       ->       reference {
			if(index >= size()) throw std::out_of_range{"ptl::vector::at - index out of range"};
			return (*this)[index];
		}

		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto front()       noexcept ->       reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		auto back()       noexcept ->       reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());

		auto data() const noexcept -> const_pointer { return storage.data(); }
		auto data()       noexcept ->       pointer { return storage.data(); }

		[[nodiscard]]
		auto empty() const noexcept -> bool { return size() == 0; }
		auto size() const noexcept -> size_type { return storage.size(); }
		static
		auto max_size() noexcept-> size_type { return max_capacity; }
		auto capacity() const noexcept -> size_type { return storage.capacity(); }

		auto push_back(const Type & value) -> reference { return emplace_back(value); }
		auto push_back(Type && value) -> reference { return emplace_back(std::move(value)); }
		template<typename... Args>
		auto emplace_back(Args &&... args) -> reference { return *emplace(end(), std::forward<Args>(args)...); }

		void pop_back() noexcept { //TODO: [C++??] precondition(!empty());
			storage.set_size(size() - 1);
			std::destroy_at(data() + size());
		}

		void reserve(size_type new_capacity) {
			if(new_capacity <= capacity()) return;
			storage_t tmp{new_capacity};
			if(!empty()) std::uninitialized_move_n(data(), size(), tmp.data());
			tmp.set_size(size());
			storage = std::move(tmp);
		}

		void resize(size_type count) { resize_impl(count, [](auto pos, auto count) { std::uninitialized_value_construct_n(pos, count); }); }
		void resize(size_type count, const Type & value) { resize_impl(count, [&](auto pos, auto count) { std::uninitialized_fill_n(pos, count, value); }); }

		void shrink_to_fit() noexcept {
			if(size() * 2 >= capacity()) return; //TODO: better criteria for "excess memory usage"

			storage_t tmp{size()};
			std::uninitialized_move_n(data(), size(), tmp.data());
			tmp.set_size(size());
			storage = std::move(tmp);
		}

		void clear() noexcept {
			std::destroy_n(data(), size());
			storage.set_size(0);
		}

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		void assign(InputIterator first, InputIterator last) { *this = vector(first, last); }
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		void assign(ForwardIterator first, ForwardIterator last) { assign_impl(std::distance(first, last), [&](auto pos) { std::uninitialized_copy(first, last, pos); }); }
		void assign(std::initializer_list<Type> ilist) { assign(ilist.begin(), ilist.end()); }
		void assign(size_type count, const Type & value) { assign_impl(count, [&](auto pos) { std::uninitialized_fill_n(pos, count, value); }); }

		auto erase(const_iterator pos) noexcept -> iterator { return erase(pos, pos + 1); } //TODO: [C++??] precondition(pos != end());
		auto erase(const_iterator first, const_iterator last) noexcept -> iterator { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			const auto count{static_cast<size_type>(std::distance(first, last))};
			std::move(const_cast<Type *>(last.ptr), data() + size(), const_cast<Type *>(first.ptr));
			std::destroy(data() + size() - count, data() + size());
			storage.set_size(size() - count);
			return first;
		}

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, InputIterator first, InputIterator last) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			vector tmp(first, last);
			return insert(pos, std::make_move_iterator(tmp.data()), std::make_move_iterator(tmp.data() + tmp.size()));
		}
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, ForwardIterator first, ForwardIterator last) -> iterator { return insert_impl(pos, std::distance(first, last), [&](auto pos) { std::uninitialized_copy(first, last, pos); }); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, std::initializer_list<Type> ilist) -> iterator { return insert(pos, ilist.begin(), ilist.end()); }
		auto insert(const_iterator pos, const Type & value) -> iterator { return insert(pos, 1, value); }
		auto insert(const_iterator pos, Type && value) -> iterator { return emplace(pos, std::move(value)); }
		auto insert(const_iterator pos, size_type count, const Type & value) -> iterator { return insert_impl(pos, count, [&](auto pos) { std::uninitialized_fill_n(pos, count, value); }); }
		template<typename... Args>
		auto emplace(const_iterator pos, Args &&... args) -> iterator { return insert_impl(pos, 1, [&](auto pos) { new(pos) Type{std::forward<Args>(args)...}; }); } //TODO: [C++20] use construct_at

		auto begin() const noexcept -> const_iterator { return data(); }
		auto begin()       noexcept ->       iterator { return data(); }
		auto cbegin() const noexcept -> const_iterator { return begin(); }
		auto end()   const noexcept -> const_iterator { return begin() + size(); }
		auto end()         noexcept ->       iterator { return begin() + size(); }
		auto cend()   const noexcept -> const_iterator { return end(); }
		auto rbegin()  const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }
		auto rbegin()        noexcept ->       reverse_iterator { return reverse_iterator{end()}; }
		auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{cend()}; }
		auto rend()    const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }
		auto rend()          noexcept ->       reverse_iterator { return reverse_iterator{begin()}; }
		auto crend()   const noexcept -> const_reverse_iterator { return const_reverse_iterator{cbegin()}; }

		void swap(vector & other) noexcept { storage.swap(other.storage); }
		friend
		void swap(vector & lhs, vector & rhs) noexcept { lhs.swap(rhs); }

		//TODO: [C++20] replace the ordering operators by <=>
		friend
		auto operator< (const vector & lhs, const vector & rhs) noexcept -> bool { return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
		friend
		auto operator> (const vector & lhs, const vector & rhs) noexcept -> bool { return rhs < lhs; }
		friend
		auto operator<=(const vector & lhs, const vector & rhs) noexcept -> bool { return !(lhs > rhs); }
		friend
		auto operator>=(const vector & lhs, const vector & rhs) noexcept -> bool { return !(lhs < rhs); }
		friend
		auto operator==(const vector & lhs, const vector & rhs) noexcept -> bool { return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()); }
		friend
		auto operator!=(const vector & lhs, const vector & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] remove as implicitly generated
	};

	template<typename InputIterator>
	vector(InputIterator, InputIterator) -> vector<typename std::iterator_traits<InputIterator>::value_type>;

	//TODO: [C++20] erase
	//TODO: [C++20] erase_if
}
