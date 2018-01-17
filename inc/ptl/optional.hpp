
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ostream>
#include "internal/optional.hpp"
#include "internal/requires.hpp"
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	//! @brief global constant used to indicate an optional with uninitialized state
	constexpr
	internal::nullopt_t nullopt{0};

	//! @brief tag for dispatch in constructor of optional
	constexpr
	internal::in_place_t in_place{};

	PTL_PACK_BEGIN
	//! @brief an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	class optional final {
		std::uint8_t data[sizeof(Type)], initialized{false};

		static_assert(internal::is_abi_compatible<Type>::value, "Type does not fulfill ABI requirements");
	public:
		constexpr
		optional() noexcept =default;
		constexpr
		optional(internal::nullopt_t) noexcept {}

		constexpr
		optional(const optional & other) : initialized{other.initialized} { if(other) new(data) Type{*other}; }
		constexpr
		optional(optional && other) noexcept : initialized{other.initialized} { if(other) new(data) Type{std::move(*other)}; }

		constexpr
		optional(const Type & value) : initialized{true} { new(data) Type{value}; }
		constexpr
		optional(Type && value) noexcept : initialized{true} { new(data) Type{std::move(value)}; }

		template<typename... Args>
		explicit
		optional(internal::in_place_t, Args &&... args) : initialized{true} { new(data) Type{std::forward<Args>(args)...}; }

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

		constexpr
		auto operator->() const noexcept -> const Type * { return &**this; }
		constexpr
		auto operator->()       noexcept ->       Type * { return &**this; }

		constexpr
		auto operator*() const & noexcept -> const Type & {
			PTL_REQUIRES(initialized);
			return reinterpret_cast<const Type &>(data);
		}
		constexpr
		auto operator*()       & noexcept ->       Type & {
			PTL_REQUIRES(initialized);
			return reinterpret_cast<      Type &>(data);
		}

		constexpr
		auto operator*() const && noexcept -> const Type && { return std::move(**this); }
		constexpr
		auto operator*()       && noexcept ->       Type && { return std::move(**this); }

		explicit
		operator bool() const noexcept { return initialized; }
		auto operator!() const noexcept -> bool { return !initialized; }

		void reset() noexcept {
			if(initialized) {
				(**this).~Type();
				initialized = false;
			}
		}

		template<typename... Args>
		auto emplace(Args &&... args) -> Type & {
			reset();
			new(data) Type{std::forward<Args>(args)...};
			initialized = true;
			return **this;
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
				using std::swap;
				swap(*lhs, *rhs);
			}
		}

		friend
		constexpr
		auto operator==(const optional & lhs, const optional & rhs) noexcept {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs == *rhs;
		}
		friend
		constexpr
		auto operator!=(const optional & lhs, const optional & rhs) noexcept {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs != *rhs;
		}
		friend
		constexpr
		auto operator< (const optional & lhs, const optional & rhs) noexcept {
			if(static_cast<bool>(rhs) == false) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs <  *rhs;
		}
		friend
		constexpr
		auto operator<=(const optional & lhs, const optional & rhs) noexcept {
			if(static_cast<bool>(lhs) == false) return true;
			if(static_cast<bool>(rhs) == false) return false;
			return *lhs <= *rhs;
		}
		friend
		constexpr
		auto operator> (const optional & lhs, const optional & rhs) noexcept {
			if(static_cast<bool>(lhs) == false) return false;
			if(static_cast<bool>(rhs) == false) return true;
			return *lhs >  *rhs;
		}
		friend
		constexpr
		auto operator>=(const optional & lhs, const optional & rhs) noexcept {
			if(static_cast<bool>(rhs) == false) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs >= *rhs;
		}

		friend
		constexpr
		auto operator==(const optional & opt, internal::nullopt_t) noexcept { return !opt; }
		friend
		constexpr
		auto operator!=(const optional & opt, internal::nullopt_t) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator< (const optional & opt, internal::nullopt_t) noexcept { return false; }
		friend
		constexpr
		auto operator<=(const optional & opt, internal::nullopt_t) noexcept { return !opt; }
		friend
		constexpr
		auto operator> (const optional & opt, internal::nullopt_t) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator>=(const optional & opt, internal::nullopt_t) noexcept { return true; }

		friend
		constexpr
		auto operator==(internal::nullopt_t, const optional & opt) noexcept { return !opt; }
		friend
		constexpr
		auto operator!=(internal::nullopt_t, const optional & opt) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator< (internal::nullopt_t, const optional & opt) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator<=(internal::nullopt_t, const optional & opt) noexcept { return true; }
		friend
		constexpr
		auto operator> (internal::nullopt_t, const optional & opt) noexcept { return false; }
		friend
		constexpr
		auto operator>=(internal::nullopt_t, const optional & opt) noexcept { return !opt; }

		friend
		constexpr
		auto operator==(const optional & opt, const Type & value) noexcept { return opt ? *opt == value : false; }
		friend
		constexpr
		auto operator!=(const optional & opt, const Type & value) noexcept { return opt ? *opt != value : true; }
		friend
		constexpr
		auto operator< (const optional & opt, const Type & value) noexcept { return opt ? *opt <  value : true; }
		friend
		constexpr
		auto operator<=(const optional & opt, const Type & value) noexcept { return opt ? *opt <= value : true; }
		friend
		constexpr
		auto operator> (const optional & opt, const Type & value) noexcept { return opt ? *opt >  value : false; }
		friend
		constexpr
		auto operator>=(const optional & opt, const Type & value) noexcept { return opt ? *opt >= value : false; }

		friend
		constexpr
		auto operator==(const Type & value, const optional & opt) noexcept { return opt ? value == *opt : false; }
		friend
		constexpr
		auto operator!=(const Type & value, const optional & opt) noexcept { return opt ? value != *opt : true; }
		friend
		constexpr
		auto operator< (const Type & value, const optional & opt) noexcept { return opt ? value <  *opt : false; }
		friend
		constexpr
		auto operator<=(const Type & value, const optional & opt) noexcept { return opt ? value <= *opt : false; }
		friend
		constexpr
		auto operator> (const Type & value, const optional & opt) noexcept { return opt ? value >  *opt : true; }
		friend
		constexpr
		auto operator>=(const Type & value, const optional & opt) noexcept { return opt ? value >= *opt : true; }

		friend
		decltype(auto) operator<<(std::ostream & os, const optional & self) {
			if(!self) return os << "<nullopt>";
			return os << *self;
		}
	};
	PTL_PACK_END

	//! @brief exception thrown when trying to access an optional in an invalid way
	struct bad_optional_access : std::exception {
		auto what() const noexcept -> const char * override { return "bad_optional_access"; }
	};

	template<typename Type>
	constexpr
	decltype(auto) get(const optional<Type> & self) {
		if(!self) throw bad_optional_access{};
		return *self;
	}

	template<typename Type>
	constexpr
	decltype(auto) get(      optional<Type> & self) {
		if(!self) throw bad_optional_access{};
		return *self;
	}

	template<typename Type>
	constexpr
	decltype(auto) get(const optional<Type> && self) {
		if(!self) throw bad_optional_access{};
		return *std::move(self);
	}

	template<typename Type>
	constexpr
	decltype(auto) get(      optional<Type> && self) {
		if(!self) throw bad_optional_access{};
		return *std::move(self);
	}

	template<typename Type>
	constexpr
	auto make_optional(Type && value) { return optional<std::decay_t<Type>>{std::forward<Type>(value)}; }

	template<typename Type, typename... Args>
	constexpr
	auto make_optional(Args &&... args) { return optional<Type>{in_place, std::forward<Args>(args)...}; }
}

namespace std {
	template<typename Type>
	struct hash<ptl::optional<Type>> final {
		auto operator()(const ptl::optional<Type> & self) const noexcept -> std::size_t {
			return self ? std::hash<Type>{}(*self) : 0;
		}
	};
}
