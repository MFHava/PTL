
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>
#include "cpp20_emulation.hpp"

namespace ptl::internal {
	template<typename Func>
	struct deduce_lambda;

	template<typename Func>
	using deduce_lambda_t = typename deduce_lambda<Func>::type;

	template<typename Result, typename Class, typename... Args>
	struct deduce_lambda<Result(Class::*)(Args...)> final {
		using type = Result(Args...);
	};

	template<typename Result, typename Class, typename... Args>
	struct deduce_lambda<Result(Class::*)(Args...) const> final {
		using type = Result(Args...);
	};

	template<typename Result, typename Class, typename... Args>
	struct deduce_lambda<Result(Class::*)(Args...) noexcept> final {
		using type = Result(Args...) noexcept;
	};

	template<typename Result, typename Class, typename... Args>
	struct deduce_lambda<Result(Class::*)(Args...) const noexcept> final {
		using type = Result(Args...) noexcept;
	};


	template<typename Signature>
	struct function_ref_traits;

	template<typename Result, typename... Args>
	struct function_ref_traits<Result(Args...)> {
		static
		constexpr
		bool noexcept_{false};

		using result_type = Result;
		using dispatch_type = result_type(void *, Args...);

		template<typename Functor>
		static
		auto dispatcher(void * ctx, Args... args) -> result_type { return (*reinterpret_cast<remove_cvref_t<Functor> *>(ctx))(std::forward<Args>(args)...); }

		template<typename Functor>
		static
		constexpr
		bool compatible{std::is_invocable_r_v<result_type, Functor &, Args...>};
	};

	template<typename Result, typename... Args>
	struct function_ref_traits<Result(Args...) noexcept> {
		static
		constexpr
		bool noexcept_{true};

		using result_type = Result;
		using dispatch_type = result_type(void *, Args...) noexcept;

		template<typename Functor>
		static
		auto dispatcher(void * ctx, Args... args) noexcept -> result_type { return (*reinterpret_cast<remove_cvref_t<Functor> *>(ctx))(std::forward<Args>(args)...); }

		template<typename Functor>
		static
		constexpr
		bool compatible{std::is_nothrow_invocable_r_v<result_type, Functor &, Args...>};
	};
}
