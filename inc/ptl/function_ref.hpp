
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

	//! @brief non-owning reference to a functor (either a plain function or a lambda)
	//! @tparam Signature function signature of the referenced functor (including potential noexcept-qualifier)
	template<typename Signature>
	class function_ref;

	namespace internal { //TODO: [C++??] if noexcept-ness were deducible, this extra class would not be necessary and the code overall would be simpler
		template<bool Noexcept, typename Result, typename... Args>
		class function_ref {
			void * functor;
			Result (*dispatch)(void *, Args...) noexcept(Noexcept);
		public:
			template<typename Functor>
			constexpr
			function_ref(Functor && func) noexcept {
				if constexpr(std::is_pointer_v<Functor>) {
					//TODO: [C++??] precondition(func);
					functor = reinterpret_cast<void *>(func);
				} else functor = reinterpret_cast<void *>(std::addressof(func));
				dispatch = [](void * ctx, Args... args) noexcept(Noexcept) { return (*reinterpret_cast<std::conditional_t<std::is_pointer_v<Functor>, Functor, std::remove_reference_t<Functor> *>>(ctx))(std::forward<Args>(args)...); };
			}

			constexpr
			void swap(function_ref & other) noexcept {
				std::swap(functor,  other.functor);
				std::swap(dispatch, other.dispatch);
			}
			friend
			constexpr
			void swap(function_ref & lhs, function_ref & rhs) noexcept { lhs.swap(rhs); }

			auto operator()(Args... args) const noexcept(Noexcept) -> Result { return dispatch(functor, std::forward<Args>(args)...); }
		};
	}

	template<typename Result, typename... Args>
	struct function_ref<Result(Args...)> final : internal::function_ref<false, Result, Args...> { //TODO: static_assert(sizeof(function_ref<T>) == 2 * sizeof(void *));
		template<typename Functor, typename = std::enable_if_t<std::is_invocable_r_v<Result, Functor &, Args...>>>
		constexpr
		function_ref(Functor && func) noexcept : internal::function_ref<false, Result, Args...>{std::forward<Functor>(func)} {}
	};

	template<typename Result, typename... Args>
	struct function_ref<Result(Args...) noexcept> final : internal::function_ref<true, Result, Args...> { //TODO: static_assert(sizeof(function_ref<T>) == 2 * sizeof(void *));
		template<typename Functor, typename = std::enable_if_t<std::is_nothrow_invocable_r_v<Result, Functor &, Args...>>>
		constexpr
		function_ref(Functor && func) noexcept : internal::function_ref<true, Result, Args...>{std::forward<Functor>(func)} {}
	};

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...)) -> function_ref<Result(Args...)>;

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...) noexcept) -> function_ref<Result(Args...) noexcept>;

	namespace internal {
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

	template<typename Lambda>
	function_ref(Lambda) -> function_ref<internal::deduce_lambda_t<decltype(&Lambda::operator())>>;
}
