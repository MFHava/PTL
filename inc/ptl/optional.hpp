
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ostream>
#include <optional>
#include "internal/adl_swap.hpp"
#include "internal/requires.hpp"
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	class optional final {
		std::uint8_t data[sizeof(Type)], initialized{false};

		static_assert(internal::is_abi_compatible_v<Type>);
	public:
		constexpr
		optional() noexcept =default;
		constexpr
		optional(std::nullopt_t) noexcept {}

		constexpr
		optional(const optional & other) : initialized{other.initialized} { if(other) new(data) Type{*other}; }
		constexpr
		optional(optional && other) noexcept : initialized{other.initialized} { if(other) new(data) Type{std::move(*other)}; }

		constexpr
		optional(const Type & value) : initialized{true} { new(data) Type{value}; }
		constexpr
		optional(Type && value) noexcept : initialized{true} { new(data) Type{std::move(value)}; }

		template<typename... Args>
		constexpr
		explicit
		optional(std::in_place_t, Args &&... args) : initialized{true} { new(data) Type{std::forward<Args>(args)...}; }

		auto operator=(std::nullopt_t) noexcept -> optional & {
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

		constexpr
		explicit
		operator bool() const noexcept { return initialized; }
		constexpr
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

		void swap(optional & other) noexcept {
			if(!*this && !other) return;
			else if(*this && !other) {
				other = std::move(**this);
				reset();
			} else if(!*this && other) {
				*this = std::move(*other);
				other.reset();
			} else internal::adl_swap(**this, *other);
		}

		friend
		void swap(optional & lhs, optional & rhs) noexcept { lhs.swap(rhs); }

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
		auto operator==(const optional & opt, std::nullopt_t) noexcept { return !opt; }
		friend
		constexpr
		auto operator!=(const optional & opt, std::nullopt_t) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator< (const optional & opt, std::nullopt_t) noexcept { return false; }
		friend
		constexpr
		auto operator<=(const optional & opt, std::nullopt_t) noexcept { return !opt; }
		friend
		constexpr
		auto operator> (const optional & opt, std::nullopt_t) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator>=(const optional & opt, std::nullopt_t) noexcept { return true; }

		friend
		constexpr
		auto operator==(std::nullopt_t, const optional & opt) noexcept { return !opt; }
		friend
		constexpr
		auto operator!=(std::nullopt_t, const optional & opt) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator< (std::nullopt_t, const optional & opt) noexcept { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator<=(std::nullopt_t, const optional & opt) noexcept { return true; }
		friend
		constexpr
		auto operator> (std::nullopt_t, const optional & opt) noexcept { return false; }
		friend
		constexpr
		auto operator>=(std::nullopt_t, const optional & opt) noexcept { return !opt; }

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

	template<typename Type>
	optional(Type) -> optional<Type>;

	template<typename Type>
	constexpr
	decltype(auto) get(const optional<Type> & self) {
		if(!self) throw std::bad_optional_access{};
		return *self;
	}

	template<typename Type>
	constexpr
	decltype(auto) get(      optional<Type> & self) {
		if(!self) throw std::bad_optional_access{};
		return *self;
	}

	template<typename Type>
	constexpr
	decltype(auto) get(const optional<Type> && self) {
		if(!self) throw std::bad_optional_access{};
		return *std::move(self);
	}

	template<typename Type>
	constexpr
	decltype(auto) get(      optional<Type> && self) {
		if(!self) throw std::bad_optional_access{};
		return *std::move(self);
	}

	template<typename Type>
	constexpr
	auto make_optional(Type && value) { return optional<std::decay_t<Type>>{std::forward<Type>(value)}; }

	template<typename Type, typename... Args>
	constexpr
	auto make_optional(Args &&... args) { return optional<Type>{std::in_place, std::forward<Args>(args)...}; }
}

namespace std {
	template<typename Type>
	struct hash<ptl::optional<Type>> final {
		auto operator()(const ptl::optional<Type> & self) const noexcept -> std::size_t {
			return self ? std::hash<Type>{}(*self) : 0;
		}
	};
}
