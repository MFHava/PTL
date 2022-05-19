
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <type_traits>

namespace ptl {
	static_assert(sizeof(void *) == sizeof(void(*)()));
	static_assert(sizeof(void *) == sizeof(void(*)() noexcept));

	namespace internal_function_ref {
		template<typename>
		struct traits;

		template<typename Result, typename... Args>
		struct traits<Result(Args...)> final {
			using function = Result(Args...);
			using const_ = std::false_type;
			using noexcept_ = std::false_type;
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const> final {
			using function = Result(Args...);
			using const_ = std::true_type;
			using noexcept_ = std::false_type;
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) noexcept> final {
			using function = Result(Args...);
			using const_ = std::false_type;
			using noexcept_ = std::true_type;
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const noexcept> final {
			using function = Result(Args...);
			using const_ = std::true_type;
			using noexcept_ = std::true_type;
		};


		template<typename T>
		using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>; //TODO: [C++20] replace with std::remove_cvref_t
	}

	//! @brief non-owning reference to a function (either a plain function or a functor)
	//! @tparam Signature function signature of the referenced functor (including potential const and noexcept qualifiers)
	template<typename Signature, typename = typename internal_function_ref::traits<Signature>::function>
	struct function_ref;

	template<typename Signature, typename Result, typename... Args>
	class function_ref<Signature, Result(Args...)> final {
		using traits = internal_function_ref::traits<Signature>;

		static
		constexpr //TODO: [C++20] consteval
		bool noexcept_{typename traits::noexcept_{}};

		template<typename T>
		using const_ = std::conditional_t<typename traits::const_{}, const T, T>;

		void * functor;
		Result (*dispatch)(void *, Args...) noexcept(noexcept_);

		template<typename T>
		static
		constexpr //TODO: [C++20] consteval
		bool is_invocable_using{noexcept_ ? std::is_nothrow_invocable_r_v<Result, T, Args...> : std::is_invocable_r_v<Result, T, Args...>};
	public:
		template<typename F, typename = std::enable_if_t<std::is_function_v<F> && is_invocable_using<F>>> //TODO: [C++20] replace with concepts/requires-clause
		function_ref(F * func) noexcept : functor{reinterpret_cast<void *>(func)}, dispatch{+[](void * ctx, Args... args) noexcept(noexcept_) { return reinterpret_cast<F *>(ctx)(std::forward<Args>(args)...); }} {}

		template<typename F, typename T = std::remove_reference_t<F>, typename = std::enable_if_t<!std::is_same_v<function_ref, internal_function_ref::remove_cvref_t<F>> && !std::is_member_pointer_v<T> && is_invocable_using<const_<T> &>>>//TODO: [C++20] replace with concepts/requires-clause
		function_ref(F && func) noexcept : functor{reinterpret_cast<void *>(std::addressof(func))}, dispatch{+[](void * ctx, Args... args) noexcept(noexcept_) { return (*reinterpret_cast<const_<T> *>(ctx))(std::forward<Args>(args)...); }} {}

		constexpr
		function_ref(const function_ref &) noexcept =default;
		constexpr
		auto operator=(const function_ref &) noexcept -> function_ref & =default;

		template<typename T, typename = std::enable_if_t<!std::is_same_v<function_ref, T> && !std::is_pointer_v<T>>> //TODO: [C++20] replace with concepts/requires-clause
		auto operator=(T) -> function_ref & =delete;

		auto operator()(Args... args) const noexcept(noexcept_) -> Result { return dispatch(functor, std::forward<Args>(args)...); }
	};

	template<typename F>
	function_ref(F *) -> function_ref<F>;

	//TODO: static_assert(sizeof(function_ref<T>) == 2 * sizeof(void *));
}
