
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/function_ref.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	static_assert(sizeof(void *) == sizeof(void(*)()), "implementation assumes that function pointers have the same size as object pointers");

	PTL_PACK_BEGIN
	//! @brief non-owning reference to a functor (either a plain function or a lambda)
	//! @tparam Signature function signature of the referenced functor (including potential noexcept-qualifier)
	template<typename Signature>
	class function_ref final {
		using traits = internal::function_ref_traits<Signature>;

		void * functor;
		typename traits::dispatch_type * dispatch;
	public:
		template<typename Functor, typename = std::enable_if_t<!std::is_same_v<internal::remove_cvref_t<Functor>, function_ref> && traits::template compatible<Functor>>>
		constexpr
		function_ref(Functor && func) noexcept {
			if constexpr(std::is_pointer_v<Functor>) {
				//pre-condition: func
				functor = reinterpret_cast<void *>(func);
				dispatch = traits::template dispatcher<std::remove_pointer_t<Functor>>;
			} else {
				functor = reinterpret_cast<void *>(std::addressof(func));
				dispatch = traits::template dispatcher<Functor>;
			}
		}

		constexpr
		void swap(function_ref & other) noexcept {
			std::swap(functor,  other.functor);
			std::swap(dispatch, other.dispatch);
		}
		friend
		constexpr
		void swap(function_ref & lhs, function_ref & rhs) noexcept { lhs.swap(rhs); }

		template<typename... Args, typename = std::enable_if_t<std::is_invocable_v<Signature, Args...>>>
		auto operator()(Args &&... args) const noexcept(traits::noexcept_) -> typename traits::result_type { return dispatch(functor, std::forward<Args>(args)...); }
	};
	PTL_PACK_END

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...)) -> function_ref<Result(Args...)>;

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...) noexcept) -> function_ref<Result(Args...) noexcept>;

	template<typename Lambda>
	function_ref(Lambda) -> function_ref<internal::deduce_lambda_t<decltype(&Lambda::operator())>>;
}
