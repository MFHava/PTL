
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ostream>
#include "internal/requires.hpp"
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief an empty class used to indicate an optional with uninitialized state
	struct nullopt_t {
		explicit
		constexpr
		nullopt_t(int) {}
	};
	static_assert(sizeof(nullopt_t) == 1, "invalid size of nullopt_t detected");

	//! @brief global constant used to indicate an optional with uninitialized state
	constexpr nullopt_t nullopt{0};

	//! @brief an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	struct optional final {//TODO: evaluate differences to the standard!  
		static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");

		optional() noexcept =default;
		optional(nullopt_t) noexcept {}

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

		auto operator=(nullopt_t) noexcept -> optional & {
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
		auto operator==(const optional & lhs, const optional & rhs) {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs == *rhs;
		}
		friend
		auto operator!=(const optional & lhs, const optional & rhs) {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs != *rhs;
		}
		friend
		auto operator< (const optional & lhs, const optional & rhs) {
			if(static_cast<bool>(rhs) == false) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs <  *rhs;
		}
		friend
		auto operator<=(const optional & lhs, const optional & rhs) {
			if(static_cast<bool>(lhs) == false) return true;
			if(static_cast<bool>(rhs) == false) return false;
			return *lhs <= *rhs;
		}
		friend
		auto operator> (const optional & lhs, const optional & rhs) {
			if(static_cast<bool>(lhs) == false) return false;
			if(static_cast<bool>(rhs) == false) return true;
			return *lhs >  *rhs;
		}
		friend
		auto operator>=(const optional & lhs, const optional & rhs) {
			if(static_cast<bool>(rhs) == false) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs >= *rhs;
		}

		friend
		auto operator==(const optional & opt, nullopt_t) noexcept { return !opt; }
		friend
		auto operator!=(const optional & opt, nullopt_t) noexcept { return static_cast<bool>(opt); }
		friend
		auto operator< (const optional & opt, nullopt_t) noexcept { return false; }
		friend
		auto operator<=(const optional & opt, nullopt_t) noexcept { return !opt; }
		friend
		auto operator> (const optional & opt, nullopt_t) noexcept { return static_cast<bool>(opt); }
		friend
		auto operator>=(const optional & opt, nullopt_t) noexcept { return true; }

		friend
		auto operator==(nullopt_t, const optional & opt) noexcept { return !opt; }
		friend
		auto operator!=(nullopt_t, const optional & opt) noexcept { return static_cast<bool>(opt); }
		friend
		auto operator< (nullopt_t, const optional & opt) noexcept { return static_cast<bool>(opt); }
		friend
		auto operator<=(nullopt_t, const optional & opt) noexcept { return true; }
		friend
		auto operator> (nullopt_t, const optional & opt) noexcept { return false; }
		friend
		auto operator>=(nullopt_t, const optional & opt) noexcept { return !opt; }

		friend
		auto operator==(const optional & opt, const Type & value) { return opt ? *opt == value : false; }
		friend
		auto operator!=(const optional & opt, const Type & value) { return opt ? *opt != value : true; }
		friend
		auto operator< (const optional & opt, const Type & value) { return opt ? *opt <  value : true; }
		friend
		auto operator<=(const optional & opt, const Type & value) { return opt ? *opt <= value : true; }
		friend
		auto operator> (const optional & opt, const Type & value) { return opt ? *opt >  value : false; }
		friend
		auto operator>=(const optional & opt, const Type & value) { return opt ? *opt >= value : false; }

		friend
		auto operator==(const Type & value, const optional & opt) { return opt ? value == *opt : false; }
		friend
		auto operator!=(const Type & value, const optional & opt) { return opt ? value != *opt : true; }
		friend
		auto operator< (const Type & value, const optional & opt) { return opt ? value <  *opt : false; }
		friend
		auto operator<=(const Type & value, const optional & opt) { return opt ? value <= *opt : false; }
		friend
		auto operator> (const Type & value, const optional & opt) { return opt ? value >  *opt : true; }
		friend
		auto operator>=(const Type & value, const optional & opt) { return opt ? value >= *opt : true; }

		friend
		decltype(auto) operator<<(std::ostream & os, const optional & self) {
			if(!self) return os << "<nullopt>";
			return os << *self;
		}
	private:
		std::uint8_t data[sizeof(Type)], initialized{false};
	};
	PTL_PACK_END
}

namespace std {
	template<typename Type>
	struct hash<ptl::optional<Type>> final {
		auto operator()(const ptl::optional<Type> & self) const noexcept -> std::size_t {
			return self ? std::hash<Type>{}(*self) : 0;
		}
	};
}