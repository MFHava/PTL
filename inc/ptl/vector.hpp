
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <algorithm>
#include <stdexcept>

namespace ptl {
	using T = char; //TODO: make into template parameter of vector

	class vector final { //TODO: [C++20] constexpr
		class rep_t final {
			void(*dealloc)(T *, std::size_t size) noexcept;

			T * ptr{nullptr};
			std::size_t cap{0}, siz{0};
		public:
			rep_t() noexcept =default;

			rep_t(std::size_t capacity) {
				dealloc = +[](T * ptr, std::size_t size) noexcept {
					for(; size; --size) (ptr + size)->~T();
					std::free(ptr);
				}; 
				ptr = static_cast<T *>(std::calloc(capacity, sizeof(T)));
				cap = capacity;
				siz = 0;
			}

			rep_t(rep_t && other) noexcept {
				dealloc = std::exchange(other.dealloc, nullptr);
				ptr = std::exchange(other.ptr, nullptr);
				cap = std::exchange(other.cap, 0);
				siz = std::exchange(other.siz, 0);
			}
			auto operator=(rep_t && other) noexcept -> rep_t & {
				if(dealloc) dealloc(ptr, siz);
				dealloc = std::exchange(other.dealloc, nullptr);
				ptr = std::exchange(other.ptr, nullptr);
				cap = std::exchange(other.cap, 0);
				siz = std::exchange(other.siz, 0);
				return *this;
			}

			~rep_t() noexcept { if(dealloc) dealloc(ptr, siz); }

			auto data() const noexcept -> const T * { return ptr; }
			auto data()       noexcept ->       T * { return ptr; }

			auto capacity() const noexcept -> std::size_t { return cap; }

			void set_size(std::size_t val) noexcept { //TODO: [C++??] precondition(val <= capacity());
				//TODO: create/destroy members unless always initializing
				siz = val;
			}
			auto size() const noexcept -> std::size_t { return siz; }

			void shrink_to_fit() noexcept {
				//TODO: implement
			}

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
			using value_type        = T;
			using difference_type   = std::ptrdiff_t;
			using pointer           = std::conditional_t<IsConst, const T, T> *;
			using reference         = std::conditional_t<IsConst, const T, T> &;

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
			operator contiguous_iterator<false>() const noexcept { return contiguous_iterator<false>{const_cast<T *>(ptr)}; }

			pointer ptr{nullptr};
		};
	public:
		using value_type             = T;
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
		vector(const vector & other); //TODO: implement
		vector(vector &&) noexcept =default; //TODO: implement
		auto operator=(const vector & other) -> vector &; //TODO: implement
		auto operator=(vector &&) noexcept -> vector & =default; //TODO: implement
		~vector() noexcept =default; //TODO: implement

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		vector(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				std::copy(first, last, std::back_inserter(*this));
			} else {
				//TODO: implement
				const auto size{std::distance(first, last)};
				rep = size;
				rep.set_size(size);
				std::copy_n(first, size, data());
			}
		}

		auto operator[](size_type index) const noexcept -> const_reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto operator[](size_type index)       noexcept ->       reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto at(size_type index) const -> const_reference { return index < size() ? (*this)[index] : throw std::out_of_range{"ptl::vector::at - index out of range"}; }
		auto at(size_type index)       ->       reference { return index < size() ? (*this)[index] : throw std::out_of_range{"ptl::vector::at - index out of range"}; }

		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto front()       noexcept ->       reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		auto back()       noexcept ->       reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());

		auto data() const noexcept -> const_pointer { return rep.data(); } //TODO: implement
		auto data()       noexcept ->       pointer { return rep.data(); } //TODO: implement

		auto empty() const noexcept -> bool { return size() == 0; }
		auto size() const noexcept -> size_type { return rep.size(); } //TODO: implement
		static
		auto max_size() noexcept-> size_type { return std::numeric_limits<size_type>::max() / sizeof(T); } //TODO: implement
		auto capacity() const noexcept -> size_type { return rep.capacity(); } //TODO: implement

		void push_back(T ch) { //TODO: implement
			if(size() == capacity()) reserve(size() + size() / 2);
			resize(size() + 1, ch);
		}
		void pop_back() noexcept { rep.set_size(size() - 1); }//TODO: [C++??] precondition(!empty()); //TODO: implement

		void reserve(size_type new_capacity) { //TODO: implement
			if(new_capacity <= capacity()) return;
			rep_t tmp{new_capacity};
			tmp.set_size(size());
			if(!empty()) std::copy_n(data(), size() + 1, data());
			rep = std::move(tmp);
		}

		void resize(size_type count) { resize(count, 0); }
		void resize(size_type count, T ch) { //TODO: implement
			const auto old{size()};
			reserve(count);
			rep.set_size(count);
			if(old < size()) std::fill(data() + old, data() + size(), ch);
		}

		void shrink_to_fit() noexcept { rep.shrink_to_fit(); } //TODO: implement

		void clear() noexcept { //TODO: implement
			if(empty()) return;
			rep.set_size(0);
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		void append(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const vector tmp(first, last);
				append(tmp.begin(), tmp.end());
			} else { //TODO: implement
				const auto old{size()};
				const auto count{std::distance(first, last)};
				resize(size() + count);
				std::copy(first, last, data() + old);
			}
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		void assign(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const vector tmp(first, last);
				assign(tmp.begin(), tmp.end());
			} else { //TODO: implement
				const auto count{std::distance(first, last)};
				resize(count);
				std::copy(first, last, data());
			}
		}

		auto erase(const_iterator pos) noexcept -> iterator { return erase(pos, pos + 1); } //TODO: [C++??] precondition(pos != end());
		auto erase(const_iterator first, const_iterator last) noexcept -> iterator { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			std::rotate(iterator{first}, iterator{last}, end()); //TODO: implement
			rep.set_size(size() - std::distance(first, last));
			return first;
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, InputIterator first, InputIterator last) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			const auto index{pos - data()};
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const vector tmp(first, last);
				insert(pos, tmp.begin(), tmp.end());
			} else { //TODO: implement
				const auto old{size()};
				const auto count{std::distance(first, last)};
				resize(old + count);
				std::copy(first, last, begin() + old);
				std::rotate(begin() + index, end() - count, end());
			}
			return begin() + index;
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		auto replace(const_iterator first, const_iterator last, InputIterator first2, InputIterator last2) -> vector & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const vector tmp(first2, last2);
				replace(first, last, tmp);
			} else { //TODO: implement
				const auto count{std::distance(first, last)};
				const auto count2{std::distance(first2, last2)};
				if(count == count2) std::copy(first2, last2, iterator{first});
				else if(count2 < count) erase(std::copy(first2, last2, iterator{first}), iterator{last});
				else {
					const auto index{first - data()};
					reserve(size() + (count2 - count));
					insert(std::copy_n(first2, count, data() + index), first2 + count, last2);
				}
			}
			return *this;
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

	static_assert(sizeof(vector) == 2 * sizeof(void *) + 2 * sizeof(std::size_t)); //TODO: implement
}
