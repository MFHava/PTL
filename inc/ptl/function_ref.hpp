
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <type_traits>

namespace ptl {
	//! @brief non-owning reference to a function (either a plain function or a functor)
	//! @tparam Signature function signature of the referenced functor (including potential noexcept-qualifier)
	template<typename Signature>
	struct function_ref;

	namespace internal_function_ref { 
		static_assert(sizeof(void *) == sizeof(void(*)()));
		static_assert(sizeof(void *) == sizeof(void(*)() noexcept));

		template<typename>
		struct is_function_ref_specialization : std::false_type {};

		template<typename Signature>
		struct is_function_ref_specialization<function_ref<Signature>> : std::true_type {};

		template<typename T>
		inline
		constexpr
		bool is_function_ref_specialization_v{is_function_ref_specialization<T>::value};

		template<bool Noexcept, typename Result, typename... Args> //TODO: [C++??] if noexcept-ness were deducible, this extra class would not be necessary and the code overall would be simpler
		class function_ref {
			void * functor;
			Result (*dispatch)(void *, Args...) noexcept(Noexcept);

			template<typename Functor>
			static
			constexpr //TODO: [C++20] consteval
			auto is_compatible_functor() noexcept -> bool {
				if constexpr(is_function_ref_specialization_v<std::decay_t<Functor>>) return false;
				if constexpr(Noexcept) return std::is_nothrow_invocable_r_v<Result, Functor &, Args...>;
				else return std::is_invocable_r_v<Result, Functor &, Args...>;
			}
		public:
			template<typename Functor, typename = std::enable_if_t<is_compatible_functor<Functor>()>> //TODO: [C++20] replace with concepts/requires-clause
			constexpr
			function_ref(Functor && func) noexcept {
				if constexpr(std::is_pointer_v<Functor>) { //TODO: [C++??] precondition(func);
					functor = reinterpret_cast<void *>(func);
					dispatch = +[](void * ctx, Args... args) noexcept(Noexcept) { return reinterpret_cast<Functor>(ctx)(std::forward<Args>(args)...); };
				} else {
					functor = reinterpret_cast<void *>(std::addressof(func));
					dispatch = +[](void * ctx, Args... args) noexcept(Noexcept) { return (*reinterpret_cast<std::remove_reference_t<Functor> *>(ctx))(std::forward<Args>(args)...); };
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

			auto operator()(Args... args) const noexcept(Noexcept) -> Result { return dispatch(functor, std::forward<Args>(args)...); }
		};

		template<typename Func>
		struct deduce_functor;

		#define PTL_INTERNAL_FUNCTION_REF_DEFINE_DEDUCTION(Const, Noexcept)\
			template<typename Result, typename Class, typename... Args>\
			struct deduce_functor<Result(Class::*)(Args...) Const Noexcept> final {\
				using type = Result(Args...) Noexcept;\
			}
		PTL_INTERNAL_FUNCTION_REF_DEFINE_DEDUCTION(     ,         );
		PTL_INTERNAL_FUNCTION_REF_DEFINE_DEDUCTION(const,         );
		PTL_INTERNAL_FUNCTION_REF_DEFINE_DEDUCTION(     , noexcept);
		PTL_INTERNAL_FUNCTION_REF_DEFINE_DEDUCTION(const, noexcept);
		#undef PTL_INTERNAL_FUNCTION_REF_DEFINE_DEDUCTION

		//TODO: static_assert(sizeof(function_ref<T>) == 2 * sizeof(void *));
	}

	#define PTL_FUNCTION_REF_DEFINE_SPECIALIZATION(Noexcept, FNoexcept)\
		template<typename Result, typename... Args>\
		struct function_ref<Result(Args...) Noexcept> final : internal_function_ref::function_ref<FNoexcept, Result, Args...> {\
			using internal_function_ref::function_ref<FNoexcept, Result, Args...>::function_ref;\
		}
	PTL_FUNCTION_REF_DEFINE_SPECIALIZATION(        , 0);
	PTL_FUNCTION_REF_DEFINE_SPECIALIZATION(noexcept, 1);
	#undef PTL_FUNCTION_REF_DEFINE_SPECIALIZATION

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...)) -> function_ref<Result(Args...)>;

	template<typename Result, typename... Args>
	function_ref(Result(*)(Args...) noexcept) -> function_ref<Result(Args...) noexcept>;

	template<typename Lambda>
	function_ref(Lambda) -> function_ref<typename internal_function_ref::deduce_functor<decltype(&Lambda::operator())>::type>;
}
