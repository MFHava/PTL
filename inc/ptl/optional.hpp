
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ostream>
#include <optional>
#include "internal/type_checks.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	class optional final {
		unsigned char data[sizeof(Type)], initialized{false};

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
		auto operator->() const noexcept -> const Type * { return std::addressof(**this); } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator->()       noexcept ->       Type * { return std::addressof(**this); } //TODO: [C++??] precondition(*this);

		constexpr
		auto operator*() const & noexcept -> const Type & { return reinterpret_cast<const Type &>(data); } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator*()       & noexcept ->       Type & { return reinterpret_cast<      Type &>(data); } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator*() const && noexcept -> const Type && { return std::move(**this); } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator*()       && noexcept ->       Type && { return std::move(**this); } //TODO: [C++??] precondition(*this);

		constexpr
		explicit
		operator bool() const noexcept { return initialized; }

		constexpr
		auto has_value() const noexcept -> bool { return initialized; }

		constexpr
		auto value() const & -> const Type & { return *this ? **this : throw std::bad_optional_access{}; }
		constexpr
		auto value()       & ->       Type & { return *this ? **this : throw std::bad_optional_access{}; }
		constexpr
		auto value() const && -> const Type && { return *this ? std::move(**this) : throw std::bad_optional_access{}; }
		constexpr
		auto value()       && ->       Type && { return *this ? std::move(**this) : throw std::bad_optional_access{}; }

		template<typename Default, typename = std::enable_if_t<std::is_convertible_v<Default &&, Type>>>
		constexpr
		auto value_or(Default && default_value) const & -> Type { return *this ? **this : static_cast<Type>(std::forward<Default>(default_value)); }
		template<typename Default, typename = std::enable_if_t<std::is_convertible_v<Default &&, Type>>>
		constexpr
		auto value_or(Default && default_value)       && -> Type { return *this ? std::move(**this) : static_cast<Type>(std::forward<Default>(default_value)); }

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

		void swap(optional & other) noexcept { //TODO: does this survive self-swap?!
			if(!*this && !other) return;
			else if(*this && !other) {
				other = std::move(**this);
				reset();
			} else if(!*this && other) {
				*this = std::move(*other);
				other.reset();
			} else {
				using std::swap;
				swap(**this, *other);
			}
		}
		friend
		void swap(optional & lhs, optional & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const optional & lhs, const optional & rhs) noexcept -> bool {
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs == *rhs;
		}
		friend
		constexpr
		auto operator!=(const optional & lhs, const optional & rhs) noexcept -> bool { //TODO: [C++20] remove as implicitly generated
			if(static_cast<bool>(lhs) != static_cast<bool>(rhs)) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs != *rhs;
		}
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const optional & lhs, const optional & rhs) noexcept -> bool {
			if(static_cast<bool>(rhs) == false) return false;
			if(static_cast<bool>(lhs) == false) return true;
			return *lhs <  *rhs;
		}
		friend
		constexpr
		auto operator<=(const optional & lhs, const optional & rhs) noexcept -> bool {
			if(static_cast<bool>(lhs) == false) return true;
			if(static_cast<bool>(rhs) == false) return false;
			return *lhs <= *rhs;
		}
		friend
		constexpr
		auto operator> (const optional & lhs, const optional & rhs) noexcept -> bool {
			if(static_cast<bool>(lhs) == false) return false;
			if(static_cast<bool>(rhs) == false) return true;
			return *lhs >  *rhs;
		}
		friend
		constexpr
		auto operator>=(const optional & lhs, const optional & rhs) noexcept -> bool {
			if(static_cast<bool>(rhs) == false) return true;
			if(static_cast<bool>(lhs) == false) return false;
			return *lhs >= *rhs;
		}

		friend
		constexpr
		auto operator==(const optional & opt, std::nullopt_t) noexcept -> bool { return !opt; }
		friend
		constexpr
		auto operator!=(const optional & opt, std::nullopt_t) noexcept -> bool { return static_cast<bool>(opt); } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const optional & opt, std::nullopt_t) noexcept -> bool { return false; }
		friend
		constexpr
		auto operator<=(const optional & opt, std::nullopt_t) noexcept -> bool { return !opt; }
		friend
		constexpr
		auto operator> (const optional & opt, std::nullopt_t) noexcept -> bool { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator>=(const optional & opt, std::nullopt_t) noexcept -> bool { return true; }

		friend
		constexpr
		auto operator==(std::nullopt_t, const optional & opt) noexcept -> bool { return !opt; }
		friend
		constexpr
		auto operator!=(std::nullopt_t, const optional & opt) noexcept -> bool { return static_cast<bool>(opt); } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (std::nullopt_t, const optional & opt) noexcept -> bool { return static_cast<bool>(opt); }
		friend
		constexpr
		auto operator<=(std::nullopt_t, const optional & opt) noexcept -> bool { return true; }
		friend
		constexpr
		auto operator> (std::nullopt_t, const optional & opt) noexcept -> bool { return false; }
		friend
		constexpr
		auto operator>=(std::nullopt_t, const optional & opt) noexcept -> bool { return !opt; }

		friend
		constexpr
		auto operator==(const optional & opt, const Type & value) noexcept -> bool { return opt ? *opt == value : false; }
		friend
		constexpr
		auto operator!=(const optional & opt, const Type & value) noexcept -> bool { return opt ? *opt != value : true; } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const optional & opt, const Type & value) noexcept -> bool { return opt ? *opt <  value : true; }
		friend
		constexpr
		auto operator<=(const optional & opt, const Type & value) noexcept -> bool { return opt ? *opt <= value : true; }
		friend
		constexpr
		auto operator> (const optional & opt, const Type & value) noexcept -> bool { return opt ? *opt >  value : false; }
		friend
		constexpr
		auto operator>=(const optional & opt, const Type & value) noexcept -> bool { return opt ? *opt >= value : false; }

		friend
		constexpr
		auto operator==(const Type & value, const optional & opt) noexcept -> bool { return opt ? value == *opt : false; }
		friend
		constexpr
		auto operator!=(const Type & value, const optional & opt) noexcept -> bool { return opt ? value != *opt : true; } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const Type & value, const optional & opt) noexcept -> bool { return opt ? value <  *opt : false; }
		friend
		constexpr
		auto operator<=(const Type & value, const optional & opt) noexcept -> bool { return opt ? value <= *opt : false; }
		friend
		constexpr
		auto operator> (const Type & value, const optional & opt) noexcept -> bool { return opt ? value >  *opt : true; }
		friend
		constexpr
		auto operator>=(const Type & value, const optional & opt) noexcept -> bool { return opt ? value >= *opt : true; }
	};
	PTL_PACK_END
}
