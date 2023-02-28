
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
	class string final {
		class rep_t final {
			static
			constexpr
			std::size_t sso_size{((sizeof(void * ) * 3)) - 2};

			void(*dealloc)(char *) noexcept;
			union {
				struct {
					char siz;
					char buf[sso_size + 1];
				} sso;
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

			void copy_sso(const rep_t & other) noexcept {
				sso.siz = other.sso.siz;
				std::memcpy(sso.buf, other.sso.buf, sso.siz + 1);
			}

			void move_heap(rep_t & other) noexcept {
				heap = other.heap;
				other.clear_to_sso();
			}
		public:
			rep_t() noexcept { clear_to_sso(); }

			rep_t(std::size_t capacity) {
				if(capacity > sso_size) {
					dealloc = +[](char * ptr) noexcept { delete[] ptr; }; 
					heap.ptr = new char[capacity + 1];
					heap.ptr[0] = 0;
					heap.cap = capacity;
					heap.siz = 0;
				} else clear_to_sso();
			}

			rep_t(rep_t && other) noexcept : dealloc{other.dealloc} {
				if(dealloc) move_heap(other);
				else copy_sso(other);
			}
			auto operator=(rep_t && other) noexcept -> rep_t & {
				if(this != std::addressof(other)) {
					if(dealloc) dealloc(heap.ptr);
					dealloc = other.dealloc;
					if(dealloc) move_heap(other);
					else copy_sso(other);
				}
				return *this;
			}

			~rep_t() noexcept { if(dealloc) dealloc(heap.ptr); }

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

			void swap(rep_t & other) noexcept { //TODO: [C++??] precondition(this != std::addressof(other));
				if(dealloc && other.dealloc) {
					std::swap(heap.ptr, other.heap.ptr);
					std::swap(heap.cap, other.heap.cap);
					std::swap(heap.siz, other.heap.siz);
				} else if(!dealloc && !other.dealloc) {
					std::swap(sso.siz, other.sso.siz);
					std::swap_ranges(sso.buf, sso.buf + std::max(sso.siz, other.sso.siz) + 1, other.sso.buf);
				} else {
					auto & on_heap{dealloc ? *this : other};
					auto & on_sso{dealloc ? other : *this};

					const auto tmp{on_heap.heap};

					on_heap.sso.siz = on_sso.sso.siz;
					std::copy_n(on_sso.sso.buf, on_sso.sso.siz + 1, on_heap.sso.buf);

					on_sso.heap = tmp;
				}
				std::swap(dealloc, other.dealloc);
			}
		} rep;

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
			tmp.rep = count;
			tmp.rep.set_size(count);
			std::copy_n(rhs.data(), rhs.size(), std::copy_n(lhs.data(), lhs.size(), tmp.data()));
			return tmp;
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
			*this = static_cast<std::string_view>(other);
			return *this;
		}
		auto operator=(string &&) noexcept -> string & =default;
		~string() noexcept =default;

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		string(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				std::copy(first, last, std::back_inserter(*this));
			} else {
				const auto size{std::distance(first, last)};
				rep = size;
				rep.set_size(size);
				std::copy_n(first, size, data());
			}
		}
		explicit
		string(std::string_view str) : string(str.begin(), str.end()) {}
		string(size_type count, char ch) {
			rep = count;
			rep.set_size(count);
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

		auto data() const noexcept -> const_pointer { return rep.data(); }
		auto data()       noexcept ->       pointer { return rep.data(); }
		auto c_str() const noexcept -> const_pointer { return data(); }

		[[nodiscard]]
		auto empty() const noexcept -> bool { return size() == 0; }
		auto size() const noexcept -> size_type { return rep.size(); }
		static
		auto max_size() noexcept-> size_type { return static_cast<size_type>(std::numeric_limits<difference_type>::max()) - 1; }
		auto capacity() const noexcept -> size_type { return rep.capacity(); }

		auto push_back(char ch) -> reference {
			if(size() == capacity()) reserve(size() + size() / 2);
			resize(size() + 1, ch);
			return back();
		}
		void pop_back() noexcept { rep.set_size(size() - 1); } //TODO: [C++??] precondition(!empty());

		void reserve(size_type new_capacity) {
			if(new_capacity <= capacity()) return;
			if(new_capacity > max_size()) throw std::length_error{"ptl::string::reserve - exceeding max_size"};
			rep_t tmp{new_capacity};
			tmp.set_size(size());
			if(!empty()) std::copy_n(data(), size() + 1, tmp.data());
			rep = std::move(tmp);
		}

		void resize(size_type count) { resize(count, 0); }
		void resize(size_type count, char ch) {
			const auto old{size()};
			reserve(count);
			rep.set_size(count);
			if(old < size()) std::fill(data() + old, data() + size(), ch);
		}

		void shrink_to_fit() noexcept { rep.shrink_to_fit(); }

		void clear() noexcept {
			if(empty()) return;
			rep.set_size(0);
		}

		//TODO: [C++20] starts_with(string_view)
		//TODO: [C++20] starts_with(char)
		//TODO: [C++20] end_with(string_view)
		//TODO: [C++20] end_with(char)
		//TODO: [C++23] contains(string_view)
		//TODO: [C++23] contains(char)

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		void append(InputIterator first, InputIterator last) { insert(end(), first, last); }
		void append(std::string_view str) { append(str.begin(), str.end()); }
		void append(std::initializer_list<char> ilist) { append(ilist.begin(), ilist.end()); }
		void append(size_type count, char ch) { insert(end(), count, ch); }

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		void assign(InputIterator first, InputIterator last) {
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const string tmp(first, last);
				assign(tmp.begin(), tmp.end());
			} else {
				const auto count{std::distance(first, last)};
				resize(count);
				std::copy(first, last, data());
			}
		}
		void assign(std::string_view str) { assign(str.begin(), str.end()); }
		void assign(std::initializer_list<char> ilist) { assign(ilist.begin(), ilist.end()); }
		void assign(size_type count, char ch) {
			resize(count);
			std::fill_n(data(), count, ch);
		}

		auto erase(const_iterator pos) noexcept -> iterator { return erase(pos, pos + 1); } //TODO: [C++??] precondition(pos != end());
		auto erase(const_iterator first, const_iterator last) noexcept -> iterator { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			std::copy(last, cend(), iterator{first});
			rep.set_size(size() - std::distance(first, last));
			return first;
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		auto insert(const_iterator pos, InputIterator first, InputIterator last) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			const auto index{pos - data()};
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const string tmp(first, last);
				insert(pos, tmp.begin(), tmp.end());
			} else {
				const auto old{size()};
				const auto count{std::distance(first, last)};
				resize(old + count);
				std::copy_backward(data() + index, data() + old, data() + size());
				std::copy(first, last, data() + index);
			}
			return begin() + index;
		}
		auto insert(const_iterator pos, std::string_view str) -> iterator { return insert(pos, str.begin(), str.end()); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, char ch) -> iterator { return insert(pos, std::string_view{&ch, 1}); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, std::initializer_list<char> ilist) -> iterator { return insert(pos, ilist.begin(), ilist.end()); } //TODO: [C++??] precondition(begin() <= pos && pos <= end());
		auto insert(const_iterator pos, size_type count, char ch) -> iterator { //TODO: [C++??] precondition(begin() <= pos && pos <= end());
			const auto index{pos - data()};
			const auto old{size()};
			resize(old + count);
			std::copy_backward(data() + index, data() + old, data() + size());
			std::fill_n(data() + index, count, ch);
			return begin() + index;
		}

		template<typename InputIterator, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIterator>::iterator_category>>> //TODO: [C++20] replace with concepts/requires-clause
		auto replace(const_iterator first, const_iterator last, InputIterator first2, InputIterator last2) -> string & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			if constexpr(std::is_same_v<typename std::iterator_traits<InputIterator>::iterator_category, std::input_iterator_tag>) {
				const string tmp(first2, last2);
				replace(first, last, tmp);
			} else {
				const auto dist{std::distance(first, last)};
				const auto count{std::distance(first2, last2)};
				if(dist == count) std::copy(first2, last2, iterator{first});
				else if(count < dist) erase(std::copy(first2, last2, iterator{first}), iterator{last});
				else {
					const auto index{first - data()};
					reserve(size() + (count - dist));
					insert(std::copy_n(first2, dist, data() + index), first2 + dist, last2);
				}
			}
			return *this;
		}
		auto replace(const_iterator first, const_iterator last, std::string_view str) -> string & { return replace(first, last, str.begin(), str.end()); } //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
		auto replace(const_iterator first, const_iterator last, std::initializer_list<char> ilist) -> string & { return replace(first, last, ilist.begin(), ilist.end()); } //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
		auto replace(const_iterator first, const_iterator last, size_type count, char ch) -> string & { //TODO: [C++??] precondition(begin() <= first && first <= last && last <= end());
			const auto dist{static_cast<size_type>(std::distance(first, last))};
			if(dist >= count) {
				std::fill_n(iterator{first}, count, ch);
				erase(first + count, last);
			} else {
				reserve(size() + (count - dist));
				std::fill_n(iterator{first}, dist, ch);
				insert(first + dist, count - dist, ch);
			}
			return *this;
		}

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

		void swap(string & other) noexcept { rep.swap(other.rep); }
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
