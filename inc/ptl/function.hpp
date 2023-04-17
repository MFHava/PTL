
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <utility>
#include <type_traits>

namespace ptl {
	namespace internal_function {
		template<typename R, typename F, typename... Args, typename = std::enable_if_t<std::is_invocable_r_v<R, F, Args...>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto invoke_r(F && f, Args &&... args) noexcept(std::is_nothrow_invocable_r_v<R, F, Args...>) { //TODO: [C++23] replace with std::invoke_r
			if constexpr(std::is_void_v<R>) std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
			else return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
		}


		template<typename T>
		using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>; //TODO: [C++20] replace with std::remove_cvref_t


		union storage_t {
			void * ptr;
			char sbo[sizeof(void * ) * 3];
		};

		enum class policy { move_only, copyable, };
		enum class mode { dtor, destructive_move, copy, };


		template<typename T>
		inline
		constexpr
		bool sbo{sizeof(T) <= sizeof(storage_t::sbo) && std::is_nothrow_move_constructible_v<T>};


		template<typename Dispatch>
		struct vtable final {
			void (*manage)(storage_t *, storage_t *, mode);
			Dispatch dispatch;
			bool noexcept_copyable;

			void dtor(storage_t * self) const noexcept { manage(self, nullptr, mode::dtor); }
			void destructive_move(storage_t * from, storage_t * to) const noexcept { manage(from, to, mode::destructive_move); }
			void copy(const storage_t * from, storage_t * to) const { manage(const_cast<storage_t *>(from), to, mode::copy); }

			template<policy Policy, typename Traits, typename T, typename... A>
			static
			auto init_functor(storage_t & storage, A &&... args) -> const vtable * {
				static_assert(Policy == policy::move_only || std::is_copy_constructible_v<T>);
				static_assert(std::is_nothrow_destructible_v<T>);

				if constexpr(sbo<T>) new(storage.sbo) T{std::forward<A>(args)...};
				else storage.ptr = new T{std::forward<A>(args)...};
				static constexpr vtable vtable{
					+[](storage_t * from, storage_t * to, mode m) {
						if constexpr(sbo<T>) {
							switch(m) {
								case mode::copy:
									if constexpr(Policy == policy::copyable) new(to->sbo) T{*reinterpret_cast<const T *>(from->sbo)};
									else std::terminate(); //TODO: [C++23] use std::unreachable
									break;
								case mode::destructive_move:
									new(to->sbo) T{std::move(*reinterpret_cast<T *>(from->sbo))};
									[[fallthrough]];
								case mode::dtor:
									reinterpret_cast<T *>(from->sbo)->~T();
									break;
							}
						} else {
							switch(m) {
								case mode::dtor:
									delete reinterpret_cast<T *>(from->ptr);
									break;
								case mode::destructive_move:
									to->ptr = std::exchange(from->ptr, nullptr);
									break;
								case mode::copy:
									if constexpr(Policy == policy::copyable) to->ptr = new T{*reinterpret_cast<const T *>(from->ptr)};
									else std::terminate(); //TODO: [C++23] use std::unreachable
									break;
							}
						}
					},
					&Traits::template functor<T, sbo<T>>,
					sbo<T> && std::is_nothrow_copy_constructible_v<T>
				};
				return &vtable;
			}

			static
			auto init_empty() noexcept -> const vtable * {
				static constexpr vtable vtable{+[](storage_t *, storage_t *, mode) {}, nullptr, true};
				return &vtable;
			}
		};


		template<bool Const, bool Noexcept, bool Move, typename Result, typename... Args>
		class invoker {
			template<typename T>
			using const_ = std::conditional_t<Const, const T, T>;

			template<typename T>
			using move_ = std::conditional_t<Move, T &&, T &>;

			template<typename T>
			static
			auto move(T && t) noexcept -> decltype(auto) {
				if constexpr(Move) return std::move(t);
				else return (t);
			}

			template<typename T, bool SBO>
			static
			auto get(const_<storage_t> * ctx) noexcept -> move_<const_<T>> { return move(*reinterpret_cast<const_<T> *>(SBO ? ctx->sbo : ctx->ptr)); }
		public:
			using dispatch_type = Result(*)(const_<storage_t> *, Args...); //no conditional noexcept-qualifier to support efficient rebinding of vtables from noexcept to throwing...

			template<typename T, bool SBO>
			static
			auto functor(const_<storage_t> * ctx, Args... args) noexcept(Noexcept) -> Result { return invoke_r<Result>(get<T, SBO>(ctx), std::forward<Args>(args)...); }
		};


		template<typename>
		struct traits;

		template<typename Result, typename... Args>
		struct traits<Result(Args...)> final : invoker<false, false, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_invocable_r_v<Result, VT, Args...> && std::is_invocable_r_v<Result, VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const> final : invoker<true, false, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_invocable_r_v<Result, const VT, Args...> && std::is_invocable_r_v<Result, const VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) noexcept> final : invoker<false, true, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_nothrow_invocable_r_v<Result, VT, Args...> && std::is_nothrow_invocable_r_v<Result, VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const noexcept> final : invoker<true, true, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_nothrow_invocable_r_v<Result, const VT, Args...> && std::is_nothrow_invocable_r_v<Result, const VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) &> final : invoker<false, false, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_invocable_r_v<Result, VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const &> final : invoker<true, false, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_invocable_r_v<Result, const VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) & noexcept> final : invoker<false, true, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_nothrow_invocable_r_v<Result, VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const & noexcept> final : invoker<true, true, false, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_nothrow_invocable_r_v<Result, const VT &, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) &&> final : invoker<false, false, true, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_invocable_r_v<Result, VT &&, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const &&> final : invoker<true, false, true, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_invocable_r_v<Result, const VT &&, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) && noexcept> final : invoker<false, true, true, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_nothrow_invocable_r_v<Result, VT &&, Args...>};
		};

		template<typename Result, typename... Args>
		struct traits<Result(Args...) const && noexcept> final : invoker<true, true, true, Result, Args...> {
			template<typename VT>
			static
			constexpr
			bool is_callable_from{std::is_nothrow_invocable_r_v<Result, const VT &&, Args...>};
		};


		template<typename Impl, typename Signature>
		struct function_call; //TODO: [C++23] once switching to deducing this, this class can be merged back with traits...

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...)> {
			auto operator()(Args... args) -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) const> {
			auto operator()(Args... args) const -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) noexcept> {
			auto operator()(Args... args) noexcept -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) const noexcept> {
			auto operator()(Args... args) const noexcept -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) &> {
			auto operator()(Args... args) & -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) const &> {
			auto operator()(Args... args) const & -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) & noexcept> {
			auto operator()(Args... args) & noexcept -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) const & noexcept> {
			auto operator()(Args... args) const & noexcept -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) &&> {
			auto operator()(Args... args) && -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) const &&> {
			auto operator()(Args... args) const && -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) && noexcept> {
			auto operator()(Args... args) && noexcept -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};

		template<typename Impl, typename Result, typename... Args>
		struct function_call<Impl, Result(Args...) const && noexcept> {
			auto operator()(Args... args) const && noexcept -> Result { //TODO: [C++23] use deducing this instead of CRTP
				auto & self{*static_cast<const Impl *>(this)};
				return self.vptr->dispatch(&self.storage, std::forward<Args>(args)...);
			}
		};


		template<policy Policy, typename... Signatures>
		class function;


		template<typename>
		struct is_function_specialization : std::false_type {};

		template<policy Policy, typename... Ts>
		struct is_function_specialization<function<Policy, Ts...>> : std::true_type {};

		template<typename T>
		inline
		constexpr
		bool is_function_specialization_v{is_function_specialization<T>::value};


		template<typename>
		struct is_in_place_type_t_specialization : std::false_type {};

		template<typename T>
		struct is_in_place_type_t_specialization<std::in_place_type_t<T>> : std::true_type {};

		template<typename T>
		inline
		constexpr
		bool is_in_place_type_t_specialization_v{is_in_place_type_t_specialization<T>::value};


		template<typename Signature>
		struct add_noexcept { using type = Signature; };

		template<typename Result, typename... Args>
		struct add_noexcept<Result(Args...)> { using type = Result(Args...) noexcept; };

		template<typename Result, typename... Args>
		struct add_noexcept<Result(Args...) const> { using type = Result(Args...) const noexcept; };

		template<typename Result, typename... Args>
		struct add_noexcept<Result(Args...) &> { using type = Result(Args...) & noexcept; };

		template<typename Result, typename... Args>
		struct add_noexcept<Result(Args...) const &> { using type = Result(Args...) const & noexcept; };

		template<typename Result, typename... Args>
		struct add_noexcept<Result(Args...) &&> { using type = Result(Args...) && noexcept; };

		template<typename Result, typename... Args>
		struct add_noexcept<Result(Args...) const &&> { using type = Result(Args...) const && noexcept; };


		template<typename Signature>
		using add_noexcept_t = typename add_noexcept<Signature>::type;


		template<typename Signature>
		struct remove_noexcept { using type = Signature; };

		template<typename Result, typename... Args>
		struct remove_noexcept<Result(Args...) noexcept> { using type = Result(Args...); };

		template<typename Result, typename... Args>
		struct remove_noexcept<Result(Args...) const noexcept> { using type = Result(Args...) const; };

		template<typename Result, typename... Args>
		struct remove_noexcept<Result(Args...) & noexcept> { using type = Result(Args...) &; };

		template<typename Result, typename... Args>
		struct remove_noexcept<Result(Args...) const & noexcept> { using type = Result(Args...) const &; };

		template<typename Result, typename... Args>
		struct remove_noexcept<Result(Args...) && noexcept> { using type = Result(Args...) &&; };

		template<typename Result, typename... Args>
		struct remove_noexcept<Result(Args...) const && noexcept> { using type = Result(Args...) const &&; };

		template<typename Signature>
		using remove_noexcept_t = typename remove_noexcept<Signature>::type;


		template<typename Signature, typename F>
		inline
		constexpr
		bool can_copy_unwrap_v{std::is_same_v<remove_cvref_t<F>, function<policy::copyable, Signature>> ||
		                       std::is_same_v<remove_cvref_t<F>, function<policy::copyable, add_noexcept_t<Signature>>>};

		template<policy Policy, typename Signature, typename F>
		inline
		constexpr
		bool can_move_unwrap_v{
			std::is_same_v<F, function<policy::copyable, Signature>> ||
			std::is_same_v<F, function<policy::copyable, add_noexcept_t<Signature>>> || (
				Policy == policy::move_only && (
					std::is_same_v<F, function<policy::move_only, Signature>> ||
					std::is_same_v<F, function<policy::move_only, add_noexcept_t<Signature>>>
				)
			)
		};


		template<policy Policy, typename Signature>
		class function<Policy, Signature> final : function_call<function<Policy, Signature>, Signature> {
			using traits_t = traits<Signature>;
			using vtable_t = vtable<typename traits_t::dispatch_type>;
			friend function_call<function, Signature>;

			template<policy P, typename... S>
			friend
			class function;

			const vtable_t * vptr;
			storage_t storage;
		public:
			function() noexcept : vptr{vtable_t::init_empty()} {}
			function(std::nullptr_t) noexcept : function{} {}

			template<typename F, typename = std::enable_if_t<(!std::is_same_v<function, remove_cvref_t<F>> && !is_in_place_type_t_specialization_v<remove_cvref_t<F>> && traits_t::template is_callable_from<std::decay_t<F>>)>> //TODO: [C++20] replace with concepts/requires-clause
			function(F && func) {
				using VT = std::decay_t<F>;
				static_assert(std::is_constructible_v<VT, F>);
				if constexpr(can_move_unwrap_v<Policy, Signature, F>) { //prevent double-wrapping (moving)
					vptr = func.vptr;
					func.vptr->destructive_move(&func.storage, &storage);
					func.vptr = vtable_t::init_empty();
				} else if constexpr(can_copy_unwrap_v<Signature, F>) { //prevent double-wrapping (copying)
					vptr = func.vptr;
					func.vptr->copy(&func.storage, &storage);
				} else if constexpr(std::is_function_v<std::remove_pointer_t<F>> || std::is_member_pointer_v<F> || is_function_specialization_v<remove_cvref_t<F>>) {
					vptr = func ? vtable_t::template init_functor<Policy, traits_t, VT>(storage, std::forward<F>(func)) : vtable_t::init_empty();
				} else vptr = vtable_t::template init_functor<Policy, traits_t, VT>(storage, std::forward<F>(func));
			}

			template<typename T, typename... A, typename = std::enable_if_t<(std::is_constructible_v<std::decay_t<T>, A &&...> && traits_t::template is_callable_from<std::decay_t<T>>)>> //TODO: [C++20] replace with concepts/requires-clause
			explicit
			function(std::in_place_type_t<T>, A &&... args) {
				static_assert(std::is_same_v<T, std::decay_t<T>>);
				vptr = vtable_t::template init_functor<Policy, traits_t, T>(storage, std::forward<A>(args)...);
			}

			template<typename T, typename U, typename... A, typename = std::enable_if_t<(std::is_constructible_v<std::decay_t<T>, std::initializer_list<U> &, A &&...> && traits_t::template is_callable_from<std::decay_t<T>>)>> //TODO: [C++20] replace with concepts/requires-clause
			explicit
			function(std::in_place_type_t<T>, std::initializer_list<U> ilist, A &&... args) {
				static_assert(std::is_same_v<T, std::decay_t<T>>);
				vptr = vtable_t::template init_functor<Policy, traits_t, T>(storage, ilist, std::forward<A>(args)...);
			}

			function(const function & other) {
				static_assert(Policy == policy::copyable); //TODO: [C++20] replace with concepts/requires-clause
				vptr = other.vptr;
				vptr->copy(&other.storage, &storage);
			}
			//TODO: [C++20] function(const function &) requires(Policy == policy::move_only) =delete;

			function(function && other) noexcept {
				vptr = other.vptr;
				other.vptr->destructive_move(&other.storage, &storage);
				other.vptr = vtable_t::init_empty();
			}

			auto operator=(const function & other) -> function & {
				static_assert(Policy == policy::copyable); //TODO: [C++20] replace with concepts/requires-clause
				if(this != &other) [[likely]] {
					if(other.vptr->noexcept_copyable) {
						vptr->dtor(&storage);
						other.vptr->copy(&other.storage, &storage);
					} else {
						storage_t tmp;
						other.vptr->copy(&other.storage, &tmp);
						vptr->dtor(&storage);
						other.vptr->destructive_move(&tmp, &storage);
					}
					vptr = other.vptr;
				}
				return *this;
			}
			//TODO: [C++20] auto operator=(const function &) -> function & requires(Policy == policy::move_only) =delete;

			auto operator=(function && other) noexcept -> function & {
				if(this != &other) [[likely]] {
					vptr->dtor(&storage);
					vptr = other.vptr;
					other.vptr->destructive_move(&other.storage, &storage);
					other.vptr = vtable_t::init_empty();
				}
				return *this;
			}
			auto operator=(std::nullptr_t) noexcept -> function & {
				if(*this) {
					vptr->dtor(&storage);
					vptr = vtable_t::init_empty();
				}
				return *this;
			}

			template<typename F>
			auto operator=(F && func) -> function & {
				function{std::forward<F>(func)}.swap(*this);
				return *this;
			}

			~function() noexcept { vptr->dtor(&storage); }

			using function_call<function, Signature>::operator();

			explicit
			operator bool() const noexcept { return vptr->dispatch; }

			void swap(function & other) noexcept {
				if(this == &other) [[unlikely]] return;

				storage_t tmp;
				vptr->destructive_move(&storage, &tmp);
				other.vptr->destructive_move(&other.storage, &storage);
				vptr->destructive_move(&tmp, &other.storage);
				std::swap(vptr, other.vptr);
			}
			friend
			void swap(function & lhs, function & rhs) noexcept { lhs.swap(rhs); }

			friend
			auto operator==(const function & self, std::nullptr_t) noexcept -> bool { return !self; }
			friend
			auto operator==(std::nullptr_t, const function & self) noexcept -> bool { return !self; } //TODO: [C++20] remove as operator== is symmetric
			friend
			auto operator!=(const function & self, std::nullptr_t) noexcept -> bool { return !!self; } //TODO: [C++20] remove as operator!= is automatically generated
			friend
			auto operator!=(std::nullptr_t, const function & self) noexcept -> bool { return !!self; } //TODO: [C++20] remove as operator!= is automatically generated
		};
	}

	//! @brief move-only function wrapper
	//! @tparam Signature function signature of the contained functor (including potential const-, ref- and noexcept-qualifiers)
	//! @attention throwing an exception across ABI boundaries is undefined, so consider always using the noexcept-qualifier
	template<typename... Signature>
	using function = internal_function::function<internal_function::policy::move_only, Signature...>;


	//! @brief copyable function wrapper
	//! @tparam Signature function signature of the contained functor (including potential const-, ref- and noexcept-qualifiers)
	//! @attention throwing an exception across ABI boundaries is undefined, so consider always using the noexcept-qualifier
	template<typename... Signature>
	using copyable_function = internal_function::function<internal_function::policy::copyable, Signature...>;
}
