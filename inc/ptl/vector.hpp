
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
#include "internal/type_checks.hpp"

namespace ptl {
	//! @brief a dynamically growing array
	//! @tparam Type element type of the array
	template<typename Type>
	class vector final { //TODO: [C++20] constexpr
		static_assert(internal::is_abi_compatible_v<Type>);

		class rep_t final {
			void(*dealloc)(Type *) noexcept{nullptr};

			Type * ptr{nullptr};
			std::size_t cap{0}, siz{0};
		public:
			rep_t() noexcept =default;

			rep_t(std::size_t capacity) {
				if(capacity == 0) return;
				dealloc = +[](Type * ptr) noexcept { std::free(ptr); };
				ptr = static_cast<Type *>(std::calloc(capacity, sizeof(Type)));
				cap = capacity;
				siz = 0;
			}

			rep_t(rep_t && other) noexcept : dealloc{std::exchange(other.dealloc, nullptr)}, ptr{std::exchange(other.ptr, nullptr)}, cap{std::exchange(other.cap, 0)}, siz{std::exchange(other.siz, 0)} {}

			auto operator=(rep_t && other) noexcept -> rep_t & {
				if(this != std::addressof(other)) {
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

			~rep_t() noexcept {
				if(!dealloc) return;
				std::destroy_n(ptr, siz);
				dealloc(ptr);
			}

			auto data() const noexcept -> const Type * { return ptr; }
			auto data()       noexcept ->       Type * { return ptr; }

			auto size() const noexcept -> std::size_t { return siz; }
			auto capacity() const noexcept -> std::size_t { return cap; }

			void set_size(std::size_t val) noexcept { siz = val; } //TODO: [C++??] precondition(val <= capacity());

			void swap(rep_t & other) noexcept { //TODO: [C++??] precondition(this != std::addressof(other));
				std::swap(ptr, other.ptr);
				std::swap(cap, other.cap);
				std::swap(siz, other.siz);
				std::swap(dealloc, other.dealloc);
			}
		} rep;

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
		vector(const vector & other) : vector(other.begin(), other.end()) {}
		vector(vector &&) noexcept =default;
		auto operator=(const vector & other) -> vector & {
			assign(other.begin(), other.end());
			return *this;
		}
		auto operator=(vector &&) noexcept -> vector & =default;
		~vector() noexcept =default;

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		vector(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				std::copy(first, last, std::back_inserter(*this));
			} else {
				const auto size{std::distance(first, last)};
				rep = size;
				std::uninitialized_copy_n(first, size, data());
				rep.set_size(size);
			}
		}

		vector(size_type count) {
			reserve(count);
			std::uninitialized_default_construct_n(data(), count);
			rep.set_size(count);
		}
		vector(size_type count, const Type & value) {
			reserve(count);
			std::uninitialized_fill_n(data(), count, value);
			rep.set_size(count);
		}

		vector(std::initializer_list<Type> ilist) : vector(ilist.begin(), ilist.end()) {}
		auto operator=(std::initializer_list<Type> ilist) -> vector & {
			assign(ilist.begin(), ilist.end());
			return *this;
		}

		auto operator[](size_type index) const noexcept -> const_reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto operator[](size_type index)       noexcept ->       reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto at(size_type index) const -> const_reference { return index < size() ? (*this)[index] : throw std::out_of_range{"ptl::vector::at - index out of range"}; }
		auto at(size_type index)       ->       reference { return index < size() ? (*this)[index] : throw std::out_of_range{"ptl::vector::at - index out of range"}; }

		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto front()       noexcept ->       reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		auto back()       noexcept ->       reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());

		auto data() const noexcept -> const_pointer { return rep.data(); }
		auto data()       noexcept ->       pointer { return rep.data(); }

		[[nodiscard]]
		auto empty() const noexcept -> bool { return size() == 0; }
		auto size() const noexcept -> size_type { return rep.size(); }
		static
		auto max_size() noexcept-> size_type { return static_cast<size_type>(std::numeric_limits<difference_type>::max()) / sizeof(Type); }
		auto capacity() const noexcept -> size_type { return rep.capacity(); }

		auto push_back(const Type & value) -> reference { return emplace_back(value); }
		auto push_back(Type && value) -> reference { return emplace_back(std::move(value)); }

		template<typename... Args>
		auto emplace_back(Args &&... args) -> reference {
			if(size() == capacity()) reserve(empty() ? 8 : size() + size() / 2);
			new(data() + size()) Type{std::forward<Args>(args)...};
			rep.set_size(size() + 1);
			return back();
		}

		void pop_back() noexcept { //TODO: [C++??] precondition(!empty());
			rep.set_size(size() - 1);
			std::destroy_at(data() + size());
		}

		void reserve(size_type new_capacity) {
			if(new_capacity <= capacity()) return;
			if(new_capacity > max_size()) throw std::length_error{"ptl::vector::reserve - exceeding max_size"};
			rep_t tmp{new_capacity};
			if(!empty()) std::uninitialized_move_n(data(), size(), tmp.data());
			tmp.set_size(size());
			rep = std::move(tmp);
		}

		void resize(size_type count) {
			if(count == size()) return;
			if(count < size()) erase(begin() + count, end());
			else {
				reserve(count);
				std::uninitialized_value_construct_n(data() + size(), count - size());
				rep.set_size(count);
			}
		}
		void resize(size_type count, const Type & value) {
			if(count == size()) return;
			if(count < size()) erase(begin() + count, end());
			else {
				reserve(count);
				std::uninitialized_fill_n(data() + size(), count - size(), value);
				rep.set_size(count);
			}
		}

		void shrink_to_fit() noexcept {
			if(size() * 2 >= capacity()) return; //TODO: better criteria for "excess memory usage"

			rep_t tmp{size()};
			std::uninitialized_move_n(data(), size(), tmp.data());
			tmp.set_size(size());
			rep = std::move(tmp);
		}

		void clear() noexcept {
			std::destroy_n(data(), size());
			rep.set_size(0);
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		void assign(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				vector tmp(first, last);
				assign(std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
			} else {
				const auto count{static_cast<size_type>(std::distance(first, last))};
				clear();
				reserve(count);
				std::uninitialized_copy_n(first, count, data());
				rep.set_size(count);
			}
		}
		void assign(std::initializer_list<Type> ilist) { assign(ilist.begin(), ilist.end()); }
		void assign(size_type count, const Type & value) {
			clear();
			reserve(count);
			std::uninitialized_fill_n(data(), count, value);
			rep.set_size(count);
		}

		auto erase(const_iterator pos) noexcept -> iterator { return erase(pos, pos + 1); } //TODO: [C++??] precondition(pos != end());
		auto erase(const_iterator first, const_iterator last) noexcept -> iterator { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			const auto count{static_cast<size_type>(std::distance(first, last))};
			std::move(last, cend(), iterator{first});
			std::destroy(data() + size() - count, data() + size());
			rep.set_size(size() - count);
			return first;
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, InputIterator first, InputIterator last) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			const auto index{pos - data()};
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				vector tmp(first, last);
				insert(pos, std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
			} else {
				//TODO: if InputIterator::value_type is nothrow assignable to Type: shift_right to make room and simply copy instead of rotating after appending
				const auto count{std::distance(first, last)};
				reserve(size() + count);
				std::uninitialized_copy_n(first, count, data() + size());
				rep.set_size(size() + count);
				std::rotate(data() + index, data() + size() - count, data() + size());
			}
			return begin() + index;
		}
		auto insert(const_iterator pos, std::initializer_list<Type> ilist) -> iterator { return insert(pos, ilist.begin(), ilist.end()); }
		auto insert(const_iterator pos, const Type & value) -> iterator { return emplace(pos, value); } 
		auto insert(const_iterator pos, Type && value) -> iterator { return emplace(pos, std::move(value)); }
		auto insert(const_iterator pos, size_type count, const Type & value) -> iterator {
			//TODO: implement without rotate if T is nothrow copyable
			const auto index{pos - data()};
			reserve(size() + count);
			std::uninitialized_fill_n(data() + size(), count, value);
			rep.set_size(size() + count);
			std::rotate(data() + index, data() + size() - count, data() + size());
			return begin() + index;
		}

		template<typename... Args>
		auto emplace(const_iterator pos, Args &&... args) -> iterator {
			//TODO: if Type is nothrow constructible from Args: shift to the right and placement new => no rotation necessary
			const auto index{pos - data()};
			emplace_back(std::forward<Args>(args)...);
			std::rotate(data() + index, data() + size() - 1, data() + size());
			return begin() + index;
		}

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

		void swap(vector & other) noexcept { rep.swap(other.rep); }
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
}
