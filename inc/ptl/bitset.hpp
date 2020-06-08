
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <climits>
#include <istream>
#include <ostream>
#include <numeric>
#include <algorithm>
#include <functional>
#include "internal/cpp20_emulation.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	static_assert(CHAR_BIT == 8);
	//TODO: endianess

	PTL_PACK_BEGIN
	//! @brief a fixed-size sequence of bits
	//! @tparam Size size of the bitset
	template<std::size_t Size>
	class bitset final {
		constexpr
		void clear_trailing_bits() noexcept {
			constexpr unsigned char masks[]{255, 1, 3, 7, 15, 31, 63, 127};
			values[sizeof(values) - 1] &= masks[Size % 8];
		}

		[[noreturn]]
		static
		constexpr
		void throw_invalid_index() { throw std::out_of_range{"invalid index"}; }

		static
		constexpr //TODO(C++20): consteval
		auto determine_storage() noexcept {
			if constexpr(Size == 0) return internal::identity_type<unsigned char>{};
			else if constexpr(Size % 8 == 0) return internal::identity_type<unsigned char[Size / 8]>{};
			else return internal::identity_type<unsigned char[(Size / 8) + 1]>{};
		}

		typename decltype(determine_storage())::type values{};

		friend
		struct std::hash<bitset>;
	public:
		using value_type = bool;
		using size_type  = std::size_t;
		class reference final {
			friend bitset;

			bitset & self;
			size_type index;

			constexpr
			reference(bitset & self, size_type index) noexcept : self{self}, index{index} {
				//pre-condition: index < self.size()
			}
		public:
			constexpr
			reference(const reference &) =default;

			constexpr
			auto operator=(const reference & other) noexcept -> reference & { return *this = static_cast<bool>(other); }

			constexpr
			auto operator=(bool value) noexcept -> reference & {
				self.set(index, value);
				return *this;
			}

			constexpr
			operator bool() const noexcept { return self[index]; }
			constexpr
			auto operator~() const noexcept -> bool { return !static_cast<bool>(*this); }

			constexpr
			auto flip() noexcept -> reference & {
				self.flip(index);
				return *this;
			}

			constexpr
			void swap(reference & other) noexcept {
				if(index == other.index) return;
				const auto lhs{static_cast<bool>(*this)};
				const auto rhs{static_cast<bool>(other)};
				if(lhs == rhs) return;
				*this = rhs;
				other = lhs;
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
		bitset(size_type value) noexcept {
			constexpr auto size{std::min(sizeof(size_type) * 8, Size)};
			for(size_type i{0}; i < size; ++i)
				if(value & (size_type{1} << i))
					set(i);
		}

		constexpr
		auto operator[](size_type index) const noexcept -> const_reference {
			//pre-condition: index < Size
			if constexpr(Size == 0) return false;
			else return values[index / 8] & (1 << (index % 8));
		}
		constexpr
		auto operator[](size_type index)       noexcept ->       reference { return {*this, index}; }
		constexpr
		auto at(size_type index) const -> const_reference {
			if(index >= size()) throw_invalid_index();
			return (*this)[index];
		}
		constexpr
		auto at(size_type index)       ->       reference {
			if(index >= size()) throw_invalid_index();
			return (*this)[index];
		}

		constexpr
		auto test(size_type index) const -> bool { return at(index); }

		constexpr
		auto all() const noexcept -> bool {
			if constexpr(Size == 0) return true;
			else {
				constexpr unsigned char masks[]{255, 1, 3, 7, 15, 31, 63, 127};
				const auto it{values + sizeof(values) - 1};
				return std::all_of(values, it, [](auto val) { return val == 255; }) && *it == masks[Size % 8];
			}
		}
		constexpr
		auto any() const noexcept -> bool {
			if constexpr(Size == 0) return false;
			else return std::any_of(values, values + sizeof(values), [](auto val) { return val != 0; });
		}
		constexpr
		auto none() const noexcept -> bool { return !any(); }

		constexpr
		auto count() const noexcept -> size_type {
			if constexpr(Size == 0) return 0;
			else return std::accumulate(values, values + sizeof(values), size_type{}, [](auto sum, auto val) {
				constexpr unsigned char nibble_count[]{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
				return sum + (nibble_count[val >> 4] + nibble_count[val & 15]);
			});
		}

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
			if constexpr(Size != 0) std::transform(values, values + sizeof(values), other.values, values, std::bit_and{});
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
			if constexpr(Size != 0) std::transform(values, values + sizeof(values), other.values, values, std::bit_or{});
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
			if constexpr(Size != 0) std::transform(values, values + sizeof(values), other.values, values, std::bit_xor{});
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
			if constexpr(Size != 0) {
				for(size_type i{0}; i < count; ++i) {
					for(size_type j{sizeof(values) - 1}; j > 0; --j) {
						values[j] = static_cast<unsigned char>(values[j] << 1);
						values[j] = static_cast<unsigned char>(values[j] | (values[j - 1] >> 7));
					}
					values[0] = static_cast<unsigned char>(values[0] << 1);
					clear_trailing_bits();
				}
			}
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
			if constexpr(Size != 0) {
				for(size_type i{0}; i < count; ++i) {
					values[0] = static_cast<unsigned char>(values[0] >> 1);
					for(size_type j{1}; j < sizeof(values); ++j) {
						values[j - 1] = static_cast<unsigned char>(values[j - 1] | (values[j] << 7));
						values[j] = static_cast<unsigned char>(values[j] >> 1);
					}
				}
			}
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
			if constexpr(Size != 0) {
				std::fill_n(values, sizeof(values), 255);
				clear_trailing_bits();
			}
			return *this;
		}
		constexpr
		auto set(size_type index, bool value = true) -> bitset & {
			if constexpr(Size == 0) throw_invalid_index();
			else {
				if(index >= Size) throw_invalid_index();
				if(value) values[index / 8] = static_cast<unsigned char>(values[index / 8] | (1 << (index % 8)));
				else reset(index);
			}
			return *this;
		}

		constexpr
		auto reset() noexcept -> bitset & {
			if constexpr(Size != 0) std::fill_n(values, sizeof(values), 0);
			return *this;
		}
		constexpr
		auto reset(size_type index) -> bitset & {
			if constexpr(Size == 0) throw_invalid_index();
			else {
				if(index >= Size) throw_invalid_index();
				values[index / 8] = static_cast<unsigned char>(values[index / 8] & (~(1 << (index % 8))));
			}
			return *this;
		}

		constexpr
		auto flip() noexcept -> bitset & {
			if constexpr(Size != 0) {
				std::transform(values, values + sizeof(values), values, std::bit_not{});
				clear_trailing_bits();
			}
			return *this;
		}
		constexpr
		auto flip(size_type index) -> bitset & {
			if constexpr(Size == 0) throw_invalid_index();
			else {
				if(index >= Size) throw_invalid_index();
				values[index / 8] = static_cast<unsigned char>(values[index / 8] ^ (1 << (index % 8)));
			}
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
		void swap(bitset & other) noexcept {
			if constexpr(Size != 0) std::swap_ranges(values, values + sizeof(values), other.values);
		}
		friend
		constexpr
		void swap(bitset & lhs, bitset & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const bitset & lhs, const bitset & rhs) noexcept {
			if constexpr(Size == 0) return true;
			else return std::equal(lhs.values, lhs.values + sizeof(values), rhs.values);
		}
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
	struct hash<ptl::bitset<Size>> {
		auto operator()(const ptl::bitset<Size> & self) const noexcept -> std::size_t {
			if constexpr(Size == 0) return 0;
			else {
				//fnv1a
				const std::size_t basis{[&] {
					if constexpr(constexpr auto bitness{8 * sizeof(std::size_t)}; bitness == 32) {
						return 2166136261U;
					} else {
						static_assert(bitness == 64);
						return 14695981039346656037ULL;
					}
				}()};
				const std::size_t prime{[&] {
					if constexpr(constexpr auto bitness{8 * sizeof(std::size_t)}; bitness == 32) {
						return 16777619U;
					} else {
						static_assert(bitness == 64);
						return 1099511628211ULL;
					}
				}()};

				return std::accumulate(self.values, self.values + sizeof(self.values), basis, [&](auto hash, auto value) {
					hash ^= value;
					hash *= prime;
					return hash;
				});
			}
		}
	};

	template<std::size_t Size>
	struct tuple_size<ptl::bitset<Size>> : std::integral_constant<std::size_t, Size> {};

	template<std::size_t Index, std::size_t Size>
	struct tuple_element<Index, ptl::bitset<Size>> { using type = bool; };
}
