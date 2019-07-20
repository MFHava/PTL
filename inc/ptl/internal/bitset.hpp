
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "fnv.hpp"
#include "adl_swap.hpp"
#include "requires.hpp"

namespace ptl::internal {
	template<std::size_t Size>
	class bitset_storage final {
		static
		constexpr
		std::size_t bits{8}, div{Size / bits}, mod{Size % bits};

		using storage_type = std::uint8_t;
		static_assert(sizeof(storage_type) * 8 == bits);

		static
		constexpr
		auto popcnt(const std::uint8_t & val) noexcept -> std::size_t {
			constexpr std::uint8_t nibble_count[]{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
			return nibble_count[val >> 4] + nibble_count[val & 15];
		}

		constexpr
		void clear_trailing_bits() noexcept {
			constexpr auto mask{[] {
				storage_type result{0};
				if constexpr(mod != 0) {
					const auto value{1 << mod};
					result |= value;
					for(std::size_t i{1}; i < (bits - mod); ++i) {
						result = static_cast<storage_type>(result << 1);
						result |= value;
					}
				}
				return ~result;
			}()};
			values[sizeof(values) - 1] = static_cast<storage_type>(values[sizeof(values) - 1] & mask);//clear trailing bits
		}

		storage_type values[mod ? div + 1 : div]{};
	public:
		constexpr
		bitset_storage() noexcept =default;
		constexpr
		bitset_storage(const bitset_storage &) noexcept =default;
		constexpr
		auto operator=(const bitset_storage &) noexcept -> bitset_storage & =default;

		constexpr
		auto operator[](std::size_t index) const noexcept -> bool {
			PTL_REQUIRES(index < Size);
			return values[index / bits] & (1 << (index % bits));
		}

		constexpr
		auto all() const noexcept -> bool {
			if constexpr(sizeof(values) > 1)
				for(std::size_t i{0}; i < sizeof(values) - 1; ++i)
					if(values[i] != std::numeric_limits<storage_type>::max())
						return false;
			constexpr auto mask{[] {
				storage_type result{0};
				if constexpr(mod != 0) {
					const auto value{1 << mod};
					result |= value;
					for(std::size_t i{1}; i < (bits - mod); ++i) {
						result = static_cast<storage_type>(result << 1);
						result |= value;
					}
				}
				return static_cast<storage_type>(~result);
			}()};
			return values[sizeof(values) - 1] == mask;
		}

		constexpr
		auto any() const noexcept -> bool {
			for(const auto & value : values)
				if(value)
					return true;
			return false;
		}

		constexpr
		auto count() const noexcept -> std::size_t {
			std::size_t result{0};
			for(const auto & value : values) result += popcnt(value);
			return result;
		}

		constexpr
		void operator&=(const bitset_storage & other) noexcept { for(std::size_t i{0}; i < sizeof(values); ++i) values[i] &= other.values[i]; }
		constexpr
		void operator|=(const bitset_storage & other) noexcept { for(std::size_t i{0}; i < sizeof(values); ++i) values[i] |= other.values[i]; }
		constexpr
		void operator^=(const bitset_storage & other) noexcept { for(std::size_t i{0}; i < sizeof(values); ++i) values[i] ^= other.values[i]; }

		constexpr
		void operator<<=(std::size_t count) noexcept {
			for(std::size_t i{0}; i < count; ++i) {
				for(std::size_t j{sizeof(values) - 1}; j > 0; --j) {
					values[j] = static_cast<storage_type>(values[j] << 1);
					values[j] |= static_cast<storage_type>(values[j - 1] >> (bits - 1));
				}
				values[0] = static_cast<storage_type>(values[0] << 1);
				clear_trailing_bits();
			}
		}
		constexpr
		void operator>>=(std::size_t count) noexcept {
			for(std::size_t i{0}; i < count; ++i) {
				values[0] = static_cast<storage_type>(values[0] >> 1);
				for(std::size_t j{1}; j < sizeof(values); ++j) {
					values[j - 1] |= static_cast<storage_type>(values[j] << (bits - 1));
					values[j] = static_cast<storage_type>(values[j] >> 1);
				}
			}
		}

		constexpr
		void set() noexcept {
			for(auto & value : values) value = static_cast<storage_type>(~0);
			clear_trailing_bits();
		}
		constexpr
		void set(std::size_t index, bool value = true) {
			if(index >= Size) throw std::out_of_range{"invalid index"};
			if(value) values[index / bits] |= static_cast<storage_type>(1 << (index % bits));
			else reset(index);
		}

		constexpr
		void reset() noexcept {
			for(auto & d : values) d = 0;
		}
		constexpr
		void reset(std::size_t index) {
			if(index >= Size) throw std::out_of_range{"invalid index"};
			values[index / bits] &= static_cast<storage_type>(~(1 << (index % bits)));
		}

		constexpr
		void flip() noexcept {
			for(auto & value : values) value = static_cast<storage_type>(~value);
			clear_trailing_bits();
		}
		constexpr
		void flip(std::size_t index) {
			if(index >= Size) throw std::out_of_range{"invalid index"};
			values[index / bits] ^= static_cast<storage_type>(1 << (index % bits));
		}

		constexpr
		void swap(bitset_storage & other) noexcept { for(std::size_t i{0}; i < sizeof(values); ++i) internal::adl_swap(values[i], other.values[i]); }

		constexpr
		auto hash() const noexcept -> std::size_t {
			static_assert(std::is_same_v<storage_type, std::uint8_t>);
			return fnv1a(values);
		}

		friend
		constexpr
		auto operator==(const bitset_storage & lhs, const bitset_storage & rhs) noexcept -> bool {
			for(std::size_t i{0}; i < sizeof(lhs.values); ++i)
				if(lhs.values[i] != rhs.values[i])
					return false;
			return true;
		}
	};

	template<>
	struct bitset_storage<0> final {
		constexpr
		bitset_storage() noexcept =default;
		constexpr
		bitset_storage(const bitset_storage &) noexcept =default;
		constexpr
		auto operator=(const bitset_storage &) noexcept -> bitset_storage & =default;

		constexpr
		auto operator[](std::size_t) const noexcept -> bool { return false; }

		constexpr
		auto all() const noexcept -> bool { return true; }

		constexpr
		auto any() const noexcept -> bool { return false; }

		constexpr
		auto count() const noexcept -> std::size_t { return 0; }

		constexpr
		void operator&=(const bitset_storage &) noexcept {}
		constexpr
		void operator|=(const bitset_storage &) noexcept {}
		constexpr
		void operator^=(const bitset_storage &) noexcept {}

		constexpr
		void operator<<=(std::size_t) noexcept {}
		constexpr
		void operator>>=(std::size_t) noexcept {}

		constexpr
		void set() noexcept {}
		constexpr
		void set(std::size_t, bool) =delete;

		constexpr
		void reset() noexcept {}
		constexpr
		void reset(std::size_t) =delete;

		constexpr
		void flip() noexcept {}
		constexpr
		void flip(std::size_t) =delete;

		constexpr
		void swap(bitset_storage &) noexcept {}

		constexpr
		auto hash() const noexcept -> std::size_t { return 0; }

		friend
		constexpr
		auto operator==(const bitset_storage &, const bitset_storage &) noexcept -> bool { return true; }
	};
}
