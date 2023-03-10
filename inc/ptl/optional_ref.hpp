
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <optional>
#include <type_traits>

namespace ptl {
	//! @brief non-owning reference to an optional value
	//! @tparam Type type of the potentially contained object
	template<typename Type>
	class optional_ref final { //TODO: static_assert(sizeof(optional_ref<T>) == sizeof(T *));
		Type * ptr{nullptr};

		template<typename Optional>
		class is_compatible_optional final { //TODO: [C++20] replace with Concept
			struct valid;
			struct invalid;

			template<typename T>
			static
			constexpr
			auto deref(T * t) -> decltype(**t);
			static
			constexpr
			auto deref(...) -> invalid;

			template<typename T>
			static
			constexpr
			auto cast(const T * t) noexcept -> decltype(static_cast<bool>(*t));
			static
			constexpr
			auto cast(...) noexcept -> invalid;

			static
			constexpr
			auto rebind(Type *) noexcept -> valid;
			static
			constexpr
			auto rebind(...) noexcept -> invalid;

			static
			constexpr
			auto test() noexcept -> bool {
				using Reference = decltype(deref(static_cast<Optional *>(nullptr)));
				if constexpr(std::is_same_v<Reference, invalid>) return false;
				else {
					static_assert(std::is_lvalue_reference_v<Reference>);
					using Cast = decltype(cast(static_cast<Optional *>(nullptr)));
					if constexpr(std::is_same_v<Cast, invalid>) return false;
					else {
						using Rebind = decltype(rebind(static_cast<std::add_pointer_t<std::remove_reference_t<Reference>>>(nullptr)));
						return std::is_same_v<Rebind, valid>;
					}
				}
			}
		public:
			static
			constexpr
			bool value{test()};
		};

		template<typename Optional>
		static
		constexpr
		bool is_compatible_optional_v{is_compatible_optional<Optional>::value};
	public:
		constexpr
		optional_ref() noexcept =default;
		constexpr
		optional_ref(std::nullopt_t) noexcept {}

		constexpr
		optional_ref(Type & value) noexcept : ptr{std::addressof(value)} {}

		template<typename Optional, typename = std::enable_if_t<is_compatible_optional_v<Optional>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		optional_ref(Optional & op) noexcept : ptr{op ? std::addressof(*op) : nullptr} {} //TODO should be forwarding reference

		constexpr
		auto operator->() const noexcept -> Type * { return ptr; } //TODO: [C++??] precondition(*this);
		constexpr
		auto operator*() const noexcept -> Type & { return *ptr; } //TODO: [C++??] precondition(*this);

		constexpr
		explicit
		operator bool() const noexcept { return ptr; }

		constexpr
		auto has_value() const noexcept -> bool { return ptr; }

		constexpr
		auto value() const -> Type & {
			if(*this) return **this;
			throw std::bad_optional_access{};
		}

		template<typename Default, typename = std::enable_if_t<std::is_convertible_v<Default &&, Type>>>
		constexpr
		auto value_or(Default && default_value) const -> Type { return *this ? **this : static_cast<Type>(std::forward<Default>(default_value)); }

		//TODO: [C++23] and_then
		//TODO: [C++23] transform
		//TODO: [C++23] or_else
	};

	template<typename Optional>
	optional_ref(const Optional &) -> optional_ref<const std::remove_reference_t<decltype(*std::declval<Optional>())>>; //TODO: [C++20] this deduction guide should be mergeable with the next one...

	template<typename Optional>
	optional_ref(      Optional &) -> optional_ref<      std::remove_reference_t<decltype(*std::declval<Optional>())>>;
}
