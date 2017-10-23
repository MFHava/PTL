
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/utility.hpp"
#include "internal/type_checks.hpp"
#include <ostream>

namespace ptl {
	namespace internal {
		struct nullopt_helper;
		using nullopt_t = int nullopt_helper::*;
	}

	//! @brief global constant used to indicate an optional with uninitialized state
	constexpr internal::nullopt_t nullopt{nullptr};

	PTL_PACK_BEGIN
	//! @brief an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	struct optional final {//TODO: evaluate differences to the standard!  
		static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");

		optional() noexcept =default;
		optional(internal::nullopt_t) noexcept {}

		optional(const optional & other) {
			if(!other) return;
			new(data) Type{*other};
			initialized = true;
		}
		optional(optional && other) noexcept {
			if(!other) return;
			new(data) Type{std::move(*other)};
			initialized = true;
		}

		optional(const Type & value) : initialized{true} { new(data) Type{value}; }
		optional(Type && value) noexcept : initialized{true} { new(data) Type{std::move(value)}; }

		auto operator=(internal::nullopt_t) noexcept -> optional & {
			reset();
			return *this;
		}

		auto operator=(const optional & other) -> optional & {
			if(other) *this = *other;
			else reset();
			return *this;
		}
		auto operator=(optional && other) noexcept -> optional & {
			if(other) *this = std::move(*other);
			else reset();
			return *this;
		}

		auto operator=(const Type & value) -> optional & {
			if(initialized) **this = value;
			else {
				new(data) Type{value};
				initialized = true;
			}
			return *this;
		}
		auto operator=(Type && value) noexcept -> optional & {
			if(initialized) **this = std::move(value);
			else {
				new(data) Type{std::move(value)};
				initialized = true;
			}
			return *this;
		}

		~optional() noexcept { reset(); }

		auto operator->() const -> const Type * { return &**this; }
		auto operator->()       ->       Type * { return &**this; }

		auto operator*() const -> const Type & {
			PTL_REQUIRES(initialized);
			return reinterpret_cast<const Type &>(data);
		}
		auto operator*()       ->       Type & {
			PTL_REQUIRES(initialized);
			return reinterpret_cast<      Type &>(data);
		}

		explicit operator bool() const noexcept { return  initialized; }
		auto operator!() const noexcept -> bool { return !initialized; }

		void reset() noexcept {
			if(!initialized) return;
			(**this).~Type();
			initialized = false;
		}

		friend
		void swap(optional & lhs, optional & rhs) noexcept {
			if(!lhs && !rhs) return;
			else if(lhs && !rhs) {
				rhs = std::move(*lhs);
				lhs.reset();
			} else if(!lhs && rhs) {
				lhs = std::move(*rhs);
				rhs.reset();
			} else {
				PTL_REQUIRES(lhs && rhs);
				using std::swap;
				swap(*lhs, *rhs);
			}
		}

		friend
		auto operator==(const optional & lhs, const optional & rhs) -> bool {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs == *rhs;
		}
		friend
		auto operator!=(const optional & lhs, const optional & rhs) -> bool {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs != *rhs;
		}
		friend
		auto operator< (const optional & lhs, const optional & rhs) -> bool {
			if(static_cast<bool>(rhs) == false) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs < *rhs;
		}
		friend
		auto operator<=(const optional & lhs, const optional & rhs) -> bool {
			if(static_cast<bool>(lhs) == false) return true;
			if(static_cast<bool>(rhs) == false) return false;
			return *lhs <= *rhs;
		}
		friend
		auto operator> (const optional & lhs, const optional & rhs) -> bool {
			if(static_cast<bool>(lhs) == false) return false;
			if(static_cast<bool>(rhs) == false) return true;
			return *lhs > *rhs;
		}
		friend
		auto operator>=(const optional & lhs, const optional & rhs) -> bool {
			if(static_cast<bool>(rhs) == false) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs >= *rhs;
		}

		friend
		auto operator==(const optional & opt, internal::nullopt_t) noexcept -> bool { return !opt; }
		friend
		auto operator!=(const optional & opt, internal::nullopt_t) noexcept -> bool { return static_cast<bool>(opt); }
		friend
		auto operator< (const optional & opt, internal::nullopt_t) noexcept -> bool { return false; }
		friend
		auto operator<=(const optional & opt, internal::nullopt_t) noexcept -> bool { return !opt; }
		friend
		auto operator> (const optional & opt, internal::nullopt_t) noexcept -> bool { return static_cast<bool>(opt); }
		friend
		auto operator>=(const optional & opt, internal::nullopt_t) noexcept -> bool { return true; }

		friend
		auto operator==(internal::nullopt_t, const optional & opt) noexcept -> bool { return !opt; }
		friend
		auto operator!=(internal::nullopt_t, const optional & opt) noexcept -> bool { return static_cast<bool>(opt); }
		friend
		auto operator< (internal::nullopt_t, const optional & opt) noexcept -> bool { return static_cast<bool>(opt); }
		friend
		auto operator<=(internal::nullopt_t, const optional & opt) noexcept -> bool { return true; }
		friend
		auto operator> (internal::nullopt_t, const optional & opt) noexcept -> bool { return false; }
		friend
		auto operator>=(internal::nullopt_t, const optional & opt) noexcept -> bool { return !opt; }

		friend
		auto operator==(const optional & opt, const Type & value) -> bool { return opt ? *opt == value : false; }
		friend
		auto operator!=(const optional & opt, const Type & value) -> bool { return opt ? *opt != value : true; }
		friend
		auto operator< (const optional & opt, const Type & value) -> bool { return opt ? *opt < value : true; }
		friend
		auto operator<=(const optional & opt, const Type & value) -> bool { return opt ? *opt <= value : true; }
		friend
		auto operator> (const optional & opt, const Type & value) -> bool { return opt ? *opt > value : false; }
		friend
		auto operator>=(const optional & opt, const Type & value) -> bool { return opt ? *opt >= value : false; }

		friend
		auto operator==(const Type & value, const optional & opt) -> bool { return opt ? value == *opt : false; }
		friend
		auto operator!=(const Type & value, const optional & opt) -> bool { return opt ? value != *opt : true; }
		friend
		auto operator< (const Type & value, const optional & opt) -> bool { return opt ? value < *opt : false; }
		friend
		auto operator<=(const Type & value, const optional & opt) -> bool { return opt ? value <= *opt : false; }
		friend
		auto operator> (const Type & value, const optional & opt) -> bool { return opt ? value > *opt : true; }
		friend
		auto operator>=(const Type & value, const optional & opt) -> bool { return opt ? value >= *opt : true; }

		friend
		auto operator<<(std::ostream & os, const optional & self) -> std::ostream & {
			if(!self) return os << "<nullopt>";
			return os << *self;
		}
	private:
		std::uint8_t data[sizeof(Type)], initialized{false};
	};
	PTL_PACK_END
}