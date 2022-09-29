
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <memory>
#include <cstring>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <string_view>

namespace ptl {
	//! @brief a dynamically growing string
	class string final { //TODO: [C++20] constexpr
		class storage_t final {
			static
			constexpr
			std::size_t sso_size{((sizeof(void *) * 3)) - 2};

			void(*dealloc)(char *) noexcept;
			union {
				struct {
					char siz;
					char buf[sso_size + 1];
				} sso;
				static_assert(sizeof(sso) == sizeof(void *) * 3);
				struct {
					char * ptr;
					std::size_t cap, siz; //without \0
				} heap;
			};

			void clear_to_sso() noexcept {
				dealloc = nullptr;
				sso.siz = 0;
				sso.buf[0] = 0;
			}

			void copy_sso(const storage_t & other) noexcept {
				sso.siz = other.sso.siz;
				std::memcpy(sso.buf, other.sso.buf, sso.siz + 1);
			}

			void move_heap(storage_t & other) noexcept {
				heap = other.heap;
				other.clear_to_sso();
			}
		public:
			storage_t() noexcept { clear_to_sso(); }

			storage_t(std::size_t required) {
				if(required > sso_size) {
					dealloc = +[](char * ptr) noexcept { delete[] ptr; }; 
					heap.siz = 0;
					//minimum heap allocation: 2xSSO to prevent multiple allocations on push_back/etc. after initially exceeding SSO
					heap.cap = std::max(std::size_t{2}, (required + sizeof(sso) - 1) / sizeof(sso)) * sizeof(sso);
					heap.ptr = new char[heap.cap + 1];
					heap.ptr[0] = 0;
				} else clear_to_sso();
			}

			storage_t(storage_t && other) noexcept : dealloc{other.dealloc} {
				if(dealloc) move_heap(other);
				else copy_sso(other);
			}
			auto operator=(storage_t && other) noexcept -> storage_t & {
				if(this != std::addressof(other)) { //TODO: [C++20] use [[likely]]
					if(dealloc) dealloc(heap.ptr);
					dealloc = other.dealloc;
					if(dealloc) move_heap(other);
					else copy_sso(other);
				}
				return *this;
			}

			~storage_t() noexcept { if(dealloc) dealloc(heap.ptr); }

			auto data() const noexcept -> const char * { return dealloc ? heap.ptr : sso.buf; }
			auto data()       noexcept ->       char * { return dealloc ? heap.ptr : sso.buf; }

			auto capacity() const noexcept -> std::size_t { return dealloc ? heap.cap : sso_size; }

			void set_size(std::size_t val) noexcept { //TODO: [C++??] precondition(val <= capacity());
				if(dealloc) {
					heap.siz = val;
					heap.ptr[val] = 0;
				} else {
					sso.siz = static_cast<char>(val);
					sso.buf[val] = 0;
				}
			}
			auto size() const noexcept -> std::size_t { return dealloc ? heap.siz : sso.siz; }

			void shrink_to_fit() noexcept { //only shrink from heap to sso
				if(!dealloc) return;
				const auto siz{heap.siz};

				if(siz > sso_size) return;
				const auto ptr{heap.ptr};

				sso.siz = static_cast<char>(siz);
				std::copy_n(ptr, siz + 1, sso.buf);
				dealloc(ptr);
				dealloc = nullptr;
			}

			void swap(storage_t & other) noexcept {
				if(this == std::addressof(other)) return; //TODO: [C++20] use [[unlikely]] on condition
				else if(dealloc && other.dealloc) {
					std::swap(heap.ptr, other.heap.ptr);
					std::swap(heap.cap, other.heap.cap);
					std::swap(heap.siz, other.heap.siz);
					std::swap(dealloc, other.dealloc);
				} else if(!dealloc && !other.dealloc) {
					char tmp[sso_size + 1];
					std::copy_n(sso.buf, sso.siz + 1, tmp);
					std::copy_n(other.sso.buf, other.sso.siz + 1, sso.buf);
					std::copy_n(tmp, sso.siz + 1, other.sso.buf);
					std::swap(sso.siz, other.sso.siz);
				} else {
					auto & on_heap{dealloc ? *this : other};
					auto & on_sso{dealloc ? other : *this};

					const auto tmp{on_heap.heap};

					on_heap.sso.siz = on_sso.sso.siz;
					std::copy_n(on_sso.sso.buf, on_sso.sso.siz + 1, on_heap.sso.buf);

					on_sso.heap = tmp;

					std::swap(dealloc, other.dealloc);
				}
			}
		} storage;

		template<bool IsConst>
		struct contiguous_iterator final {
			//TODO: [C++20] using iterator_concept = std::contiguous_iterator_tag;
			using iterator_category = std::random_access_iterator_tag;
			using value_type        = char;
			using difference_type   = std::ptrdiff_t;
			using pointer           = std::conditional_t<IsConst, const char, char> *;
			using reference         = std::conditional_t<IsConst, const char, char> &;

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
			friend string;

			constexpr
			contiguous_iterator(pointer ptr) noexcept : ptr{ptr} {}

			constexpr
			operator contiguous_iterator<false>() const noexcept { return contiguous_iterator<false>{const_cast<char *>(ptr)}; }

			pointer ptr{nullptr};
		};

		static
		auto concat(std::string_view lhs, std::string_view rhs) -> string {
			const auto count{lhs.size() + rhs.size()};
			string tmp;
			tmp.storage = count;
			tmp.storage.set_size(count);
			std::copy_n(rhs.data(), rhs.size(), std::copy_n(lhs.data(), lhs.size(), tmp.data()));
			return tmp;
		}

		template<typename Func>
		void append_no_aliasing(std::size_t additional_size, Func fill) {
			const auto old{size()};
			resize(old + additional_size);
			fill(old);
		}

		template<typename Func>
		void assign_no_aliasing(std::size_t required_size, Func fill) {
			resize(required_size);
			fill();
		}

		template<typename Func>
		auto insert_no_aliasing(contiguous_iterator<true> pos, std::size_t additional_size, Func fill) {
			const auto old{size()};
			const auto offset{pos.ptr - data()};
			resize(old + additional_size);
			std::copy_backward(data() + offset, data() + old, data() + size());
			fill();
			return begin() + offset;
		}

		template<typename Func>
		void replace_no_aliasing(contiguous_iterator<true> first, contiguous_iterator<true> last, std::size_t required_size, Func fill) {
			const auto offset{first.ptr - data()};
			if(const auto count{static_cast<std::size_t>(last - first)}; count < required_size) { //not enough space in target location
				const auto old{size()};
				resize(size() + (required_size - count));
				std::copy_backward(data() + offset + count, data() + old, data() + size());
				fill(offset);
			} else { //directly overwrite and shrink afterwards
				fill(offset);
				erase(first + required_size, first + count);
			}
		}
	public:
		using traits_type            = std::char_traits<char>;
		using value_type             = char;
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

		string() noexcept =default;

		string(const string & other) : string{static_cast<std::string_view>(other)} {}
		string(string &&) noexcept =default;
		auto operator=(const string & other) -> string & {
			if(this != std::addressof(other)) *this = static_cast<std::string_view>(other); //TODO: [C++20] use [[likely]] on condition
			return *this;
		}
		auto operator=(string &&) noexcept -> string & =default;
		~string() noexcept =default;

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		string(InputIterator first, InputIterator last) { std::copy(first, last, std::back_inserter(*this)); }
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		string(ForwardIterator first, ForwardIterator last) {
			const auto size{std::distance(first, last)};
			storage = size;
			storage.set_size(size);
			std::copy_n(first, size, data());
		}
		explicit
		string(std::string_view str) : string(str.begin(), str.end()) {}
		string(size_type count, char ch) {
			storage = count;
			storage.set_size(count);
			std::fill_n(data(), count, ch);
		}
		string(std::initializer_list<char> ilist) : string{ilist.begin(), ilist.end()} {}

		auto operator=(std::string_view str) -> string & {
			assign(str);
			return *this;
		}
		auto operator=(std::initializer_list<char> ilist) -> string & {
			assign(ilist);
			return *this;
		}

		auto operator+=(std::string_view str) -> string & {
			append(str);
			return *this;
		}
		friend
		auto operator+(std::string_view lhs, const string & rhs) -> string { return concat(lhs, rhs); }
		friend
		auto operator+(const string & lhs, std::string_view rhs) -> string { return concat(lhs, rhs); }
		auto operator+=(std::initializer_list<char> ilist) -> string & {
			append(ilist);
			return *this;
		}
		auto operator+=(char ch) -> string & {
			append(1, ch);
			return *this;
		}
		friend
		auto operator+(char lhs, const string & rhs) -> string { return concat({&lhs, 1}, rhs); }
		friend
		auto operator+(const string & lhs, char rhs) -> string { return concat(lhs, {&rhs, 1}); }

		auto operator+=(const string & other) -> string & {
			append(other);
			return *this;
		}
		friend
		auto operator+(const string & lhs, const string & rhs) -> string { return concat(lhs, rhs); }
 
		auto operator[](size_type index) const noexcept -> const_reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto operator[](size_type index)       noexcept ->       reference { return data()[index]; } //TODO: [C++??] precondition(index < size());
		auto at(size_type index) const -> const_reference {
			if(index >= size()) throw std::out_of_range{"ptl::string::at - index out of range"};
			return (*this)[index];
		}
		auto at(size_type index)       ->       reference {
			if(index >= size()) throw std::out_of_range{"ptl::string::at - index out of range"};
			return (*this)[index];
		}

		auto front() const noexcept -> const_reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto front()       noexcept ->       reference { return (*this)[0]; } //TODO: [C++??] precondition(!empty());
		auto back() const noexcept -> const_reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());
		auto back()       noexcept ->       reference { return (*this)[size() - 1]; } //TODO: [C++??] precondition(!empty());

		auto data() const noexcept -> const_pointer { return storage.data(); }
		auto data()       noexcept ->       pointer { return storage.data(); }
		auto c_str() const noexcept -> const_pointer { return data(); }

		[[nodiscard]]
		auto empty() const noexcept -> bool { return size() == 0; }
		auto size() const noexcept -> size_type { return storage.size(); }
		static
		auto max_size() noexcept-> size_type { return static_cast<size_type>(std::numeric_limits<difference_type>::max()) - 1; }
		auto capacity() const noexcept -> size_type { return storage.capacity(); }

		auto push_back(char ch) -> reference {
			if(size() == capacity()) reserve(size() + 1);
			resize(size() + 1, ch);
			return back();
		}
		void pop_back() noexcept { storage.set_size(size() - 1); } //TODO: [C++??] precondition(!empty());

		void reserve(size_type new_capacity) {
			if(new_capacity <= capacity()) return;
			if(new_capacity > max_size()) throw std::length_error{"ptl::string::reserve - exceeding max_size"};
			storage_t tmp{new_capacity};
			tmp.set_size(size());
			if(!empty()) std::copy_n(data(), size() + 1, tmp.data());
			storage = std::move(tmp);
		}

		void resize(size_type count) { resize(count, 0); }
		void resize(size_type count, char ch) {
			const auto old{size()};
			reserve(count);
			storage.set_size(count);
			if(old < size()) std::fill(data() + old, data() + size(), ch);
		}

		void shrink_to_fit() noexcept { storage.shrink_to_fit(); }

		void clear() noexcept {
			if(empty()) return;
			storage.set_size(0);
		}

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		void append(InputIterator first, InputIterator last) {
			const string tmp(first, last);
			append_no_aliasing(tmp.size(), [&](auto offset) { std::copy_n(tmp.data(), tmp.size(), data() + offset); });
		}
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		void append(ForwardIterator first, ForwardIterator last) {
			const auto distance{static_cast<std::size_t>(std::distance(first, last))};
			if(distance + size() <= capacity()) { //no need for allocation nor double buffering
				std::copy(first, last, data() + size());
				storage.set_size(size() + distance);
			} else { //do single allocation
				storage_t tmp{size() + distance};
				std::copy(first, last, tmp.data() + size());
				std::move(data(), data() + size(), tmp.data());
				tmp.set_size(size() + distance);
				storage = std::move(tmp);
			}
		}
		void append(std::string_view str) { append(str.begin(), str.end()); }
		void append(std::initializer_list<char> ilist) { append_no_aliasing(ilist.size(), [&](auto offset) { std::copy(ilist.begin(), ilist.end(), data() + offset); }); }
		void append(size_type count, char ch) { append_no_aliasing(count, [&](auto offset) { std::fill_n(data() + offset, count, ch); }); }

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		void assign(InputIterator first, InputIterator last) { *this = string(first, last); }
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		void assign(ForwardIterator first, ForwardIterator last) {
			if(const auto distance{static_cast<std::size_t>(std::distance(first, last))}; distance + size() <= capacity()) { //no need for allocation nor double buffering
				std::copy(first, last, data() + size());
				std::rotate(data(), data() + size(), data() + size() + distance);
				storage.set_size(distance);
			} else {
				*this = string(first, last); //TODO: this is probably sub-optimal as it falls back to InputIterator-logic
			}
		}
		void assign(std::string_view str) { assign(str.begin(), str.end()); }
		void assign(std::initializer_list<char> ilist) { assign_no_aliasing(ilist.size(), [&] { std::copy(ilist.begin(), ilist.end(), data()); }); }
		void assign(size_type count, char ch) { assign_no_aliasing(count, [&] { std::fill_n(data(), count, ch); }); }

		auto erase(const_iterator pos) noexcept -> iterator { return erase(pos, pos + 1); } //TODO: [C++??] precondition(pos != end());
		auto erase(const_iterator first, const_iterator last) noexcept -> iterator { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			std::copy(const_cast<char *>(last.ptr), data() + size(), const_cast<char *>(first.ptr));
			storage.set_size(size() - (last - first));
			return first;
		}

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, InputIterator first, InputIterator last) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			const string tmp(first, last);
			return insert_no_aliasing(pos, tmp.size(), [&, offset{pos.ptr - data()}] { std::copy_n(tmp.data(), tmp.size(), data() + offset); });
		}
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, ForwardIterator first, ForwardIterator last) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			const auto distance{static_cast<std::size_t>(std::distance(first, last))};
			const auto offset{pos.ptr - data()};
			if(distance + size() <= capacity()) { //no need for allocation nor double buffering
				std::copy(first, last, data() + size());
				std::rotate(const_cast<char *>(pos.ptr), data() + size(), data() + size() + distance);
				storage.set_size(size() + distance);
			} else { //do single allocation
				storage_t tmp{size() + distance};
				std::copy(first, last, tmp.data() + offset);
				std::move(data(), data() + offset, tmp.data());
				std::move(data() + offset, data() + size(), tmp.data() + offset + distance);
				tmp.set_size(size() + distance);
				storage = std::move(tmp);
			}
			return begin() + offset;
		}
		auto insert(const_iterator pos, std::string_view str) -> iterator { return insert(pos, str.begin(), str.end()); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, std::initializer_list<char> ilist) -> iterator { return insert_no_aliasing(pos, ilist.size(), [&, offset{pos.ptr - data()}] { std::copy(ilist.begin(), ilist.end(), data() + offset); }); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, char ch) -> iterator { return insert(pos, 1, ch); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, size_type count, char ch) -> iterator { return insert_no_aliasing(pos, count, [&, offset{pos.ptr - data()}] { std::fill_n(data() + offset, count, ch); }); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());

		template<typename InputIterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		auto replace(const_iterator first, const_iterator last, InputIterator first2, InputIterator last2) -> string & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			const string tmp(first2, last2);
			replace_no_aliasing(first, last, tmp.size(), [&](auto offset) { std::copy_n(tmp.data(), tmp.size(), data() + offset); });
			return *this;
		}
		template<typename ForwardIterator, std::enable_if_t<std::is_base_of_v<std::forward_iterator_tag, typename std::iterator_traits<ForwardIterator>::iterator_category>, int> = 0> //TODO: [C++20] replace with concepts/requires-clause
		auto replace(const_iterator first, const_iterator last, ForwardIterator first2, ForwardIterator last2) -> string & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			const auto distance{static_cast<std::size_t>(std::distance(first2, last2))};
			if(distance + size() <= capacity()) { //no need for allocation nor double buffering
				std::copy(first2, last2, data() + size());
				const auto offset_first{first.ptr - data()};
				const auto offset_mid{size() - (last - first)};
				storage.set_size(size() + distance);
				erase(first, last);
				std::rotate(data() + offset_first, data() + offset_mid, data() + size());
			} else { //do single allocation
				storage_t tmp{size() - (last - first) + distance};
				std::copy(first2, last2, tmp.data() + (first.ptr - data()));
				std::move(data(), const_cast<char *>(first.ptr), tmp.data());
				std::move(const_cast<char *>(last.ptr), data() + size(), tmp.data() + (first.ptr - data()) + distance);
				tmp.set_size(size() - (last - first) + distance);
				storage = std::move(tmp);
			}
			return *this;
		}
		auto replace(const_iterator first, const_iterator last, std::string_view str) -> string & { return replace(first, last, str.begin(), str.end()); } //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
		auto replace(const_iterator first, const_iterator last, std::initializer_list<char> ilist) -> string & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			replace_no_aliasing(first, last, ilist.size(), [&](auto offset) { std::copy(ilist.begin(), ilist.end(), data() + offset); });
			return *this;
		}
		auto replace(const_iterator first, const_iterator last, size_type count, char ch) -> string & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			replace_no_aliasing(first, last, count, [&](auto offset) { std::fill_n(data() + offset, count, ch); });
			return *this;
		}

		auto substr(size_type offset) const & -> string { return {data() + offset, data() + size()}; } //TODO: [C++??] precondition(offset <= size());
		auto substr(size_type offset) && noexcept -> string { //TODO: [C++??] precondition(offset <= size());
			erase(data(), data() + offset);
			return std::move(*this);
		}
		auto substr(size_type offset, size_type count) const & -> string { return {data() + offset, data() + offset + count}; } //TODO: [C++??] precondition(offset + count <= size());
		auto substr(size_type offset, size_type count) && noexcept -> string { //TODO: [C++??] precondition(offset + count <= size());
			erase(data() + offset + count, data() + size());
			erase(data(), data() + offset);
			return std::move(*this);
		}

		//TODO: [C++20] starts_with(string_view)
		//TODO: [C++20] starts_with(char)
		//TODO: [C++20] end_with(string_view)
		//TODO: [C++20] end_with(char)
		//TODO: [C++23] contains(string_view)
		//TODO: [C++23] contains(char)

		operator std::string_view() const noexcept {
			if(empty()) return {};
			return {data(), size()};
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

		void swap(string & other) noexcept { storage.swap(other.storage); }
		friend
		void swap(string & lhs, string & rhs) noexcept { lhs.swap(rhs); }

		//TODO: [C++20] replace the ordering operators by <=>
		friend
		auto operator< (const string & lhs, const string & rhs) noexcept -> bool { return lhs <  static_cast<std::string_view>(rhs); }
		friend
		auto operator<=(const string & lhs, const string & rhs) noexcept -> bool { return lhs <= static_cast<std::string_view>(rhs); }
		friend
		auto operator>=(const string & lhs, const string & rhs) noexcept -> bool { return lhs >= static_cast<std::string_view>(rhs); }
		friend
		auto operator> (const string & lhs, const string & rhs) noexcept -> bool { return lhs >  static_cast<std::string_view>(rhs); }
		friend
		auto operator==(const string & lhs, const string & rhs) noexcept -> bool { return lhs == static_cast<std::string_view>(rhs); }
		friend
		auto operator!=(const string & lhs, const string & rhs) noexcept -> bool { return lhs != static_cast<std::string_view>(rhs); } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		auto operator< (std::string_view lhs, const string & rhs) noexcept -> bool { return lhs <  static_cast<std::string_view>(rhs); }
		friend
		auto operator<=(std::string_view lhs, const string & rhs) noexcept -> bool { return lhs <= static_cast<std::string_view>(rhs); }
		friend
		auto operator>=(std::string_view lhs, const string & rhs) noexcept -> bool { return lhs >= static_cast<std::string_view>(rhs); }
		friend
		auto operator> (std::string_view lhs, const string & rhs) noexcept -> bool { return lhs >  static_cast<std::string_view>(rhs); }
		friend
		auto operator==(std::string_view lhs, const string & rhs) noexcept -> bool { return lhs == static_cast<std::string_view>(rhs); }
		friend
		auto operator!=(std::string_view lhs, const string & rhs) noexcept -> bool { return lhs != static_cast<std::string_view>(rhs); } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		auto operator< (const string & lhs, std::string_view rhs) noexcept -> bool { return static_cast<std::string_view>(lhs) <  rhs; }
		friend
		auto operator<=(const string & lhs, std::string_view rhs) noexcept -> bool { return static_cast<std::string_view>(lhs) <= rhs; }
		friend
		auto operator>=(const string & lhs, std::string_view rhs) noexcept -> bool { return static_cast<std::string_view>(lhs) >= rhs; }
		friend
		auto operator> (const string & lhs, std::string_view rhs) noexcept -> bool { return static_cast<std::string_view>(lhs) >  rhs; }
		friend
		auto operator==(const string & lhs, std::string_view rhs) noexcept -> bool { return static_cast<std::string_view>(lhs) == rhs; }
		friend
		auto operator!=(const string & lhs, std::string_view rhs) noexcept -> bool { return static_cast<std::string_view>(lhs) != rhs; } //TODO: [C++20] remove as implicitly generated

		friend
		auto operator<<(std::ostream & os, const string & self) -> std::ostream & { return os << static_cast<std::string_view>(self); }
	};

	static_assert(sizeof(string) == 4 * sizeof(void *));

	//TODO: [C++20] erase
	//TODO: [C++20] erase_if

	namespace literals {
		inline
		auto operator""_s(const char * ptr, std::size_t) noexcept -> string { return string{ptr}; }
	}
}

namespace std {
	template<>
	struct hash<ptl::string> {
		auto operator()(const ptl::string & self) const noexcept -> std::size_t { return std::hash<std::string_view>{}(self); }
	};
}
