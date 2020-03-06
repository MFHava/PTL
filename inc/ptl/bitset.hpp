
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <istream>
#include <ostream>
#include <algorithm>
#include "internal/bitset.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a fixed-size sequence of bits
	//! @tparam Size size of the bitset
	template<std::size_t Size>
	class bitset final {
		internal::bitset_storage<Size> storage;

		friend
		struct std::hash<bitset>;
	public:
		using value_type = bool;
		using size_type  = std::size_t;
		class reference final {
			friend bitset;

			bitset * ptr;
			size_type index;

			constexpr
			reference(bitset & ptr, size_type index) noexcept : ptr{&ptr}, index{index} { PTL_REQUIRES(index < ptr.size()); }
		public:
			reference(const reference &) =default;

			constexpr
			auto operator=(const reference & other) noexcept -> reference & { return *this = static_cast<bool>(other); }

			constexpr
			auto operator=(bool value) noexcept -> reference & {
				ptr->set(index, value);
				return *this;
			}
			~reference() noexcept =default;

			constexpr
			operator bool() const noexcept { return static_cast<const bitset &>(*ptr)[index]; }
			constexpr
			auto operator~() const noexcept -> bool { return !static_cast<bool>(*this); }

			constexpr
			auto flip() noexcept -> reference & {
				ptr->flip(index);
				return *this;
			}

			constexpr
			void swap(reference & other) noexcept {
				PTL_REQUIRES(ptr == other.ptr);
				if(index == other.index) return;
				const auto this_set{static_cast<bool>(*this)};
				const auto other_set{static_cast<bool>(other)};
				ptr->set(index, other_set);
				other.ptr->set(other.index, this_set);
			}
			friend
			constexpr
			void swap(reference lhs, reference rhs) noexcept { lhs.swap(rhs); }
		};
		using const_reference = bool;

		constexpr
		bitset() noexcept =default;

		explicit
		constexpr
		bitset(std::uint64_t value) noexcept {
			const auto size{std::min(size_type{64}, Size)};
			for(size_type i{0}; i < size; ++i)
				if(value & (std::uint64_t{1} << i))
					set(i);
		}

		constexpr
		bitset(const bitset &) noexcept =default;
		constexpr
		auto operator=(const bitset &) noexcept -> bitset & =default;

		constexpr
		auto operator[](size_type index) const noexcept -> const_reference { return storage[index]; }
		constexpr
		auto operator[](size_type index)       noexcept ->       reference { return {*this, index}; }
		constexpr
		auto at(size_type index) const -> const_reference {
			if(index >= size()) throw std::out_of_range{"invalid index"};
			return (*this)[index];
		}
		constexpr
		auto at(size_type index)       ->       reference {
			if(index >= size()) throw std::out_of_range{"invalid index"};
			return (*this)[index];
		}

		constexpr
		auto test(size_type index) const -> bool { return at(index); }

		constexpr
		auto all() const noexcept -> bool { return storage.all(); }
		constexpr
		auto any() const noexcept -> bool { return storage.any(); }
		constexpr
		auto none() const noexcept -> bool { return !any(); }

		constexpr
		auto count() const noexcept -> size_type { return storage.count(); }

		static
		constexpr
		auto size() noexcept -> size_type { return Size; }
		static
		constexpr
		auto empty() noexcept -> bool { return size() == 0; }
		static
		constexpr
		auto max_size() noexcept -> size_type { return size(); }

		constexpr
		auto operator&=(const bitset & other) noexcept -> bitset & {
			storage &= other.storage;
			return *this;
		}
		friend
		constexpr
		auto operator&(bitset lhs, const bitset & rhs) noexcept -> bitset {
			lhs &= rhs;
			return lhs;
		}

		constexpr
		auto operator|=(const bitset & other) noexcept -> bitset & {
			storage |= other.storage;
			return *this;
		}
		friend
		constexpr
		auto operator|(bitset lhs, const bitset & rhs) noexcept -> bitset {
			lhs |= rhs;
			return lhs;
		}

		constexpr
		auto operator^=(const bitset & other) noexcept -> bitset & {
			storage ^= other.storage;
			return *this;
		}
		friend
		constexpr
		auto operator^(bitset lhs, const bitset & rhs) noexcept -> bitset {
			lhs ^= rhs;
			return lhs;
		}

		constexpr
		auto operator~() const noexcept -> bitset {
			auto tmp{*this};
			tmp.flip();
			return tmp;
		}

		constexpr
		auto operator<<=(size_type count) noexcept -> bitset & {
			storage <<= count;
			return *this;
		}
		friend
		constexpr
		auto operator<<(bitset lhs, size_type rhs) noexcept -> bitset {
			lhs <<= rhs;
			return lhs;
		}

		constexpr
		auto operator>>=(size_type count) noexcept -> bitset & {
			storage >>= count;
			return *this;
		}
		friend
		constexpr
		auto operator>>(bitset lhs, size_type rhs) noexcept -> bitset {
			lhs >>= rhs;
			return lhs;
		}

		constexpr
		auto set() noexcept -> bitset & {
			storage.set();
			return *this;
		}
		constexpr
		auto set(size_type index, bool value = true) -> bitset & {
			storage.set(index, value);
			return *this;
		}

		constexpr
		auto reset() noexcept -> bitset & {
			storage.reset();
			return *this;
		}
		constexpr
		auto reset(size_type index) -> bitset & {
			storage.reset(index);
			return *this;
		}

		constexpr
		auto flip() noexcept -> bitset & {
			storage.flip();
			return *this;
		}
		constexpr
		auto flip(size_type index) -> bitset & {
			storage.flip(index);
			return *this;
		}

		friend
		auto operator<<(std::ostream & os, const bitset & self) -> std::ostream & {
			for(size_type i{Size}; i != 0; --i) os << (self.test(i - 1) ? '1' : '0');
			return os;
		}
		friend
		auto operator>>(std::istream & is,       bitset & self) -> std::istream & {
			if(const std::istream::sentry s{is}; s) {
				auto extracted{false};
				for(size_type i{0}; i < Size; ++i) {
					if(const auto c{is.get()}; c == '0' || c == '1') {
						if(!extracted) {
							extracted = true;
							self.reset();
						}
						self <<= 1;
						if(c == '1') self.set(0);
					} else {
						is.unget();
						if(!extracted) is.setstate(std::ios::failbit);
						break;
					}
				}
			}
			return is;
		}

		constexpr
		void swap(bitset & other) noexcept { storage.swap(other.storage); }
		friend
		constexpr
		void swap(bitset & lhs, bitset & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const bitset & lhs, const bitset & rhs) noexcept { return lhs.storage == rhs.storage; }
		friend
		constexpr
		auto operator!=(const bitset & lhs, const bitset & rhs) noexcept { return !(lhs == rhs); }
	};
	PTL_PACK_END

	template<std::size_t Index, std::size_t Size>
	constexpr
	auto get(const bitset<Size> & self) noexcept -> bool {
		static_assert(Index < Size);
		return self[Index];
	}
}

namespace std {
	template<std::size_t Size>
	struct hash<ptl::bitset<Size>> final {
		auto operator()(const ptl::bitset<Size> & self) const noexcept -> std::size_t { return self.storage.hash(); }
	};

	template<std::size_t Size>
	struct tuple_size<ptl::bitset<Size>> : std::integral_constant<std::size_t, Size> {};

	template<std::size_t Index, std::size_t Size>
	struct tuple_element<Index, ptl::bitset<Size>> { using type = bool; };
}
