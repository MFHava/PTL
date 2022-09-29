
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <optional>
#include <type_traits>

namespace ptl {
	#pragma pack(push, 1)
	//! @brief an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	class optional final {
		static_assert(std::is_standard_layout_v<Type>); //TODO: this is probably too strict!
		static_assert(std::is_copy_constructible_v<Type>);
		static_assert(std::is_nothrow_move_constructible_v<Type>);
		static_assert(std::is_copy_assignable_v<Type>);
		static_assert(std::is_nothrow_move_assignable_v<Type>);
		static_assert(std::is_nothrow_destructible_v<Type>);
		static_assert(std::is_nothrow_swappable_v<Type>);

		union {
			Type val;
		};
		unsigned char initialized{false};
	public:
		constexpr
		optional() noexcept {}
		constexpr
		optional(std::nullopt_t) noexcept {}

		constexpr
		optional(const optional & other) : initialized{other.initialized} { if(other) new(std::addressof(val)) Type{other.val}; } //TODO: [C++20] use std::construct_at
		constexpr
		optional(optional && other) noexcept : initialized{other.initialized} { if(other) new(std::addressof(val)) Type{std::move(other.val)}; } //TODO: [C++20] use std::construct_at

		constexpr
		optional(const Type & value) : initialized{true} { new(std::addressof(val)) Type{value}; } //TODO: [C++20] use std::construct_at
		constexpr
		optional(Type && value) noexcept : initialized{true} { new(std::addressof(val)) Type{std::move(value)}; } //TODO: [C++20] use std::construct_at

		template<typename... Args>
		constexpr
		explicit
		optional(std::in_place_t, Args &&... args) : initialized{true} { new(std::addressof(val)) Type{std::forward<Args>(args)...}; } //TODO: [C++20] use std::construct_at

		constexpr
		auto operator=(std::nullopt_t) noexcept -> optional & {
			reset();
			return *this;
		}

		constexpr
		auto operator=(const optional & other) -> optional & {
			if(this != &other) { //TODO: [C++20] use [[unlikely]]
				if(other) *this = *other;
				else reset();
			}
			return *this;
		}
		constexpr
		auto operator=(optional && other) noexcept -> optional & {
			if(this != &other) { //TODO: [C++20] use [[unlikely]]
				if(other) *this = std::move(*other);
				else reset();
			}
			return *this;
		}

		constexpr
		auto operator=(const Type & value) -> optional & {
			if(initialized) **this = value;
			else {
				new(std::addressof(val)) Type{value}; //TODO: [C++20] use std::construct_at
				initialized = true;
			}
			return *this;
		}
		constexpr
		auto operator=(Type && value) noexcept -> optional & {
			if(initialized) **this = std::move(value);
			else {
				new(std::addressof(val)) Type{std::move(value)}; //TODO: [C++20] use std::construct_at
				initialized = true;
			}
			return *this;
		}

		//TODO: [C++20] constexpr
		~optional() noexcept { reset(); }

		constexpr
		auto operator->() const noexcept -> const Type * { return std::addressof(**this); } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator->()       noexcept ->       Type * { return std::addressof(**this); } //TODO: [C++??] precondition(*this);

		constexpr
		auto operator*() const & noexcept -> const Type & { return val; } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator*()       & noexcept ->       Type & { return val; } //TODO: [C++??] precondition(*this);
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
		auto value() const & -> const Type & {
			if(*this) return **this;
			throw std::bad_optional_access{};
		}
		constexpr
		auto value()       & ->       Type & {
			if(*this) return **this;
			throw std::bad_optional_access{};
		}
		constexpr
		auto value() const && -> const Type && {
			if(*this) return std::move(**this);
			throw std::bad_optional_access{};
		}
		constexpr
		auto value()       && ->       Type && {
			if(*this) return std::move(**this);
			throw std::bad_optional_access{};
		}

		template<typename Default, typename = std::enable_if_t<std::is_convertible_v<Default &&, Type>>>
		constexpr
		auto value_or(Default && default_value) const & -> Type { return *this ? **this : static_cast<Type>(std::forward<Default>(default_value)); }
		template<typename Default, typename = std::enable_if_t<std::is_convertible_v<Default &&, Type>>>
		constexpr
		auto value_or(Default && default_value)       && -> Type { return *this ? std::move(**this) : static_cast<Type>(std::forward<Default>(default_value)); }

		//TODO: [C++23] and_then
		//TODO: [C++23] transform
		//TODO: [C++23] or_else

		constexpr
		void reset() noexcept {
			if(initialized) {
				std::destroy_at(std::addressof(val));
				initialized = false;
			}
		}

		template<typename... Args>
		constexpr
		auto emplace(Args &&... args) -> Type & {
			if(initialized) val = Type{std::forward<Args>(args)...};
			else {
				new(std::addressof(val)) Type{std::forward<Args>(args)...}; //TODO: [C++20] use std::construct_at
				initialized = true;
			}
			return **this;
		}

		constexpr
		void swap(optional & other) noexcept {
			if(this == &other) return; //TODO: [C++20] use [[unlikely]]
			if(!*this && !other) return;
			else if(*this && !other) {
				new(std::addressof(other.val)) Type{std::move(val)}; //TODO: [C++20] use std::construct_at
				other.initialized = true;
				std::destroy_at(std::addressof(val));
				initialized = false;
			} else if(!*this && other) {
				new(std::addressof(val)) Type{std::move(other.val)}; //TODO: [C++20] use std::construct_at
				initialized = true;
				std::destroy_at(std::addressof(other.val));
				other.initialized = false;
			} else {
				using std::swap;
				swap(val, other.val);
			}
		}
		friend
		constexpr
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

		//TODO: [C++20] the following overloads are no longer needed as C++20 has symmetrical comparisons...
		friend
		constexpr
		auto operator==(std::nullopt_t, const optional & opt) noexcept -> bool { return !opt; }
		friend
		constexpr
		auto operator!=(std::nullopt_t, const optional & opt) noexcept -> bool { return static_cast<bool>(opt); }
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

		//TODO: [C++20] the following overloads are no longer needed as C++20 has symmetrical comparisons...
		friend
		constexpr
		auto operator==(const Type & value, const optional & opt) noexcept -> bool { return opt ? value == *opt : false; }
		friend
		constexpr
		auto operator!=(const Type & value, const optional & opt) noexcept -> bool { return opt ? value != *opt : true; }
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
	#pragma pack(pop)
}
