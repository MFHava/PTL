
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <memory>
#include "internal/adl_swap.hpp"
#include "internal/requires.hpp"
#include "internal/function_ref.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief non-owning reference to a functor (either a plain function or a lambda)
	//! @tparam Signature function signature of the referenced functor (including potential noexcept-qualifier)
	template<typename Signature>
	class function_ref;

	template<typename Result, typename... Args>
	class function_ref<Result(Args...)> final {//unfortunately you can't deduce noexcept-ness at the moment, so we need to split the implementation among two almost identical partial specialization...
		using dispatch_func = Result(*)(void *, Args...);

		void * functor;
		dispatch_func dispatch;

		template<typename Functor>
		static
		auto dispatcher(void * ctx, Args... args) -> Result {
			PTL_REQUIRES(ctx);
			return (*reinterpret_cast<internal::remove_cvref_t<Functor> *>(ctx))(std::forward<Args>(args)...);
		}

		template<typename Functor>
		using parameter_validation = std::bool_constant<!std::is_same_v<internal::remove_cvref_t<Functor>, function_ref> && std::is_invocable_r_v<Result, Functor &, Args...>>;
	public:
		template<typename Functor, typename = std::enable_if_t<parameter_validation<Functor>::value>>
		constexpr
		function_ref(Functor && func) {
			if constexpr(std::is_pointer_v<Functor>) PTL_REQUIRES(func);
			functor = reinterpret_cast<void *>(std::addressof(func));
			dispatch = function_ref::dispatcher<Functor>;
		}

		constexpr
		function_ref(const function_ref &) =default;
		constexpr
		auto operator=(const function_ref &) -> function_ref & =default;
		~function_ref() noexcept =default;

		constexpr
		void swap(function_ref & other) noexcept {
			internal::adl_swap(functor,  other.functor);
			internal::adl_swap(dispatch, other.dispatch);
		}
		friend
		constexpr
		void swap(function_ref & lhs, function_ref & rhs) noexcept { lhs.swap(rhs); }

		auto operator()(Args... args) const -> Result {
			PTL_REQUIRES(functor);
			PTL_REQUIRES(dispatch);
			return dispatch(functor, std::forward<Args>(args)...);
		}
	};

	template<typename Result, typename... Args>
	class function_ref<Result(Args...) noexcept> final {
		using dispatch_func = Result(*)(void *, Args...) noexcept;

		void * functor;
		dispatch_func dispatch;

		template<typename Functor>
		static
		auto dispatcher(void * ctx, Args... args) noexcept -> Result {
			PTL_REQUIRES(ctx);
			return (*reinterpret_cast<internal::remove_cvref_t<Functor> *>(ctx))(std::forward<Args>(args)...);
		}

		template<typename Functor>
		using parameter_validation = std::bool_constant<!std::is_same_v<internal::remove_cvref_t<Functor>, function_ref> && std::is_nothrow_invocable_r_v<Result, Functor &, Args...>>;
	public:
		template<typename Functor, typename = std::enable_if_t<parameter_validation<Functor>::value>>
		constexpr
		function_ref(Functor && func) {
			if constexpr(std::is_pointer_v<Functor>) PTL_REQUIRES(func);
			functor = reinterpret_cast<void *>(std::addressof(func));
			dispatch = function_ref::dispatcher<Functor>;
		}

		constexpr
		function_ref(const function_ref &) =default;
		constexpr
		auto operator=(const function_ref &) -> function_ref & =default;
		~function_ref() noexcept =default;

		constexpr
		void swap(function_ref & other) noexcept {
			internal::adl_swap(functor,  other.functor);
			internal::adl_swap(dispatch, other.dispatch);
		}
		friend
		constexpr
		void swap(function_ref & lhs, function_ref & rhs) noexcept { lhs.swap(rhs); }

		auto operator()(Args... args) const noexcept -> Result {
			PTL_REQUIRES(functor);
			PTL_REQUIRES(dispatch);
			return dispatch(functor, std::forward<Args>(args)...);
		}
	};
	PTL_PACK_END

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...)) -> function_ref<Result(Args...)>;

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...) noexcept) -> function_ref<Result(Args...) noexcept>;

	template<typename Lambda>
	function_ref(Lambda) -> function_ref<internal::deduce_lambda_t<decltype(&Lambda::operator())>>;
}
