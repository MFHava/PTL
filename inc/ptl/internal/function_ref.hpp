
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl::internal {
	//emulating C++20 feature
	template<typename T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;


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
}
