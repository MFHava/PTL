
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <type_traits>

namespace ptl {
	namespace internal_function_ref {
		template<typename T>
		using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>; //TODO: [C++20] replace with std::remove_cvref_t

		class storage_t final {
			union {
				void * ptr;
				const void * cptr;
			};

			static_assert(sizeof(void *) == sizeof(void(*)())); //required for POSIX compatibility
		public:
			template<typename T>
			constexpr
			storage_t(T * ptr, std::enable_if_t<std::is_object_v<T>, int> = 0) noexcept : ptr{ptr} {} //TODO: [C++20] replace with concepts/requires-clause

			template<typename T>
			constexpr
			storage_t(const T * ptr, std::enable_if_t<std::is_object_v<T>, int> = 0) noexcept : cptr{ptr} {} //TODO: [C++20] replace with concepts/requires-clause

			template<typename T>
			storage_t(T * ptr, std::enable_if_t<std::is_function_v<T>, int> = 0) noexcept : ptr{reinterpret_cast<void *>(ptr)} {} //TODO: [C++20] replace with concepts/requires-clause

			template<typename T>
			constexpr
			auto get() const -> decltype(auto) {
				if constexpr(std::is_function_v<T>) return reinterpret_cast<T *>(ptr);
				else if constexpr(std::is_const_v<T>) return *static_cast<T *>(cptr);
				else return *static_cast<T *>(ptr);
			}
		};


		template<typename R, typename F, typename... Args, typename = std::enable_if_t<std::is_invocable_r_v<R, F, Args...>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto invoke_r(F && f, Args &&... args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>) { //TODO: [C++23] replace with std::invoke_r
			if constexpr(std::is_void_v<R>) std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
			else return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
		}


		template<typename Impl, bool Const, bool Noexcept, typename Result, typename... Args>
		struct invoker {
			template<typename T>
			static
			constexpr
			bool is_invocable_using{Noexcept ? std::is_nothrow_invocable_r_v<Result, T, Args...> : std::is_invocable_r_v<Result, T, Args...>};

			template<typename T>
			using const_ = std::conditional_t<Const, const T, T>;

			using dispatch_type = std::conditional_t<Noexcept, Result(*)(const storage_t*, Args...) noexcept, Result(*)(const storage_t*, Args...)>;

			template<typename T>
			static
			auto functor(const storage_t * ctx, Args... args) noexcept(Noexcept) -> Result { return invoke_r<Result>(ctx->template get<T>(), std::forward<Args>(args)...); }

			constexpr
			auto operator()(Args... args) const noexcept(Noexcept) -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};


		template<typename, typename>
		struct traits;

		template<typename Impl, typename Result, typename... Args>
		struct traits<Impl, Result(Args...)> : invoker<Impl, false, false, Result, Args...> {};

		template<typename Impl, typename Result, typename... Args>
		struct traits<Impl, Result(Args...) const> : invoker<Impl, true, false, Result, Args...> {};

		template<typename Impl, typename Result, typename... Args>
		struct traits<Impl, Result(Args...) noexcept> : invoker<Impl, false, true, Result, Args...> {};

		template<typename Impl, typename Result, typename... Args>
		struct traits<Impl, Result(Args...) const noexcept> : invoker<Impl, true, true, Result, Args...> {};
	}

	//! @brief non-owning reference to a function (either a plain function or a functor)
	//! @tparam Signature function signature of the referenced functor (including potential const and noexcept qualifiers)
	//! @attention throwing an exception across ABI boundaries is undefined, so consider always using the noexcept-qualifier
	template<typename... Signature>
	struct function_ref;

	template<typename Signature>
	class function_ref<Signature> final : internal_function_ref::traits<function_ref<Signature>, Signature> {
		using traits = internal_function_ref::traits<function_ref, Signature>;

		template<typename, bool, bool, typename, typename...>
		friend
		class internal_function_ref::invoker;

		internal_function_ref::storage_t storage;
		typename traits::dispatch_type dispatch;
	public:
		template<typename F, typename = std::enable_if_t<std::is_function_v<F> && traits::template is_invocable_using<F>>> //TODO: [C++20] replace with concepts/requires-clause
		function_ref(F * func) noexcept : storage{func}, dispatch{traits::template functor<F>} {} //TODO: [C++??] precondition(func);

		template<typename F, typename T = std::remove_reference_t<F>, typename = std::enable_if_t<!std::is_same_v<function_ref, internal_function_ref::remove_cvref_t<F>> && !std::is_member_pointer_v<T> && traits::template is_invocable_using<typename traits::template const_<T> &>>>//TODO: [C++20] replace with concepts/requires-clause
		constexpr
		function_ref(F && func) noexcept : storage{std::addressof(func)}, dispatch{traits::template functor<T>} {}

		constexpr
		function_ref(const function_ref &) noexcept =default;
		constexpr
		auto operator=(const function_ref &) noexcept -> function_ref & =default;

		template<typename T, typename = std::enable_if_t<!std::is_same_v<function_ref, T> && !std::is_pointer_v<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto operator=(T) -> function_ref & =delete;

		using internal_function_ref::traits<function_ref, Signature>::operator();
	};

	template<typename F>
	function_ref(F *) -> function_ref<F>;

	//TODO: static_assert(sizeof(function_ref<T>) == 2 * sizeof(void *));
}
