
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ranges>
#include <concepts>
#include <coroutine>

namespace ptl {
	//! @brief lazy view of elements yielded by a coroutine
	//! @tparam Reference TODO
	//! @tparam Value TODO
	//! @attention throwing an exception across ABI boundaries is undefined, so consider only using noexcept coroutines
	template<typename Reference, typename Value = void>
	class generator final : public std::ranges::view_interface<generator<Reference, Value>> {
		using value = std::conditional_t<std::is_void_v<Value>, std::remove_cvref_t<Reference>, Value>;
		static_assert(std::is_object_v<value> && std::is_same_v<std::remove_cvref_t<value>, value>);

		using reference = std::conditional_t<std::is_void_v<Value>, Reference &&, Reference>;
		static_assert(std::is_reference_v<reference> || (std::is_object_v<reference> && std::is_same_v<std::remove_cv_t<reference>, reference> && std::copy_constructible<reference>));

		using rref = std::conditional_t<std::is_reference_v<reference>, std::remove_reference_t<reference> &&, reference>;
		static_assert(std::common_reference_with<reference &&, value &>);
		static_assert(std::common_reference_with<reference &&, rref &&>);
		static_assert(std::common_reference_with<rref &&, const value &>);

		struct iterator;
	public:
		using yielded = std::conditional_t<std::is_reference_v<reference>, reference, const reference &>;

		class promise_type final {
			friend iterator;

			using deleter = void(*)(void *);

			std::add_pointer_t<yielded> ptr{nullptr};
		public:
			auto get_return_object() noexcept -> generator { return std::coroutine_handle<promise_type>::from_promise(*this); }

			auto initial_suspend() const noexcept -> std::suspend_always { return {}; }
			auto final_suspend() noexcept -> std::suspend_always { return {}; }

			auto yield_value(yielded val) noexcept -> std::suspend_always {
				ptr = std::addressof(val);
				return {};
			}

			auto yield_value(const std::remove_reference_t<yielded> & lval) requires std::is_rvalue_reference_v<yielded> && std::constructible_from<std::remove_cvref_t<yielded>, const std::remove_reference_t<yielded> &> {
				struct awaitable final {
					std::remove_cvref_t<yielded> val;

					auto await_ready() const noexcept -> bool { return false; }
					void await_suspend(std::coroutine_handle<promise_type> handle) noexcept { handle.promise().ptr = std::addressof(val); }
					void await_resume() const noexcept {}
				};
				return awaitable{lval};
			}

			void await_transform() =delete;

			void return_void() const noexcept {}
			void unhandled_exception() { throw; }

			auto operator new(std::size_t size) -> void * {
				deleter d{std::free};
				auto ptr{std::malloc(size + sizeof(d))};
				if(!ptr) throw std::bad_alloc{};
				std::memcpy(static_cast<char *>(ptr) + size, &d, sizeof(d));
				return ptr;
			}
			void operator delete(void * ptr, std::size_t size) noexcept {
				deleter d;
				std::memcpy(&d, static_cast<char *>(ptr) + size, sizeof(d));
				d(ptr);
			}
		};
	private:
		struct vtable final {
			bool(*done)(void *) /*noexcept*/;
			void(*resume)(void *) /*TODO: noexcept*/;
			void(*destroy)(void *) /*TODO: noexcept*/;
			promise_type&(*promise)(void *) noexcept;
		};

		const vtable * vptr;
		void * ptr;

		struct iterator final {
			using value_type = value;
			using difference_type = std::ptrdiff_t;

			iterator(iterator && other) noexcept : ptr{std::exchange(other.ptr, {})}, vptr{std::exchange(other.vptr, {})} {}
			auto operator=(iterator && other) noexcept -> iterator & {
				ptr = std::exchange(other.ptr, {});
				vptr = std::exchange(other.vptr, {});
				return *this;
			}

			auto operator*() const noexcept(std::is_nothrow_copy_constructible_v<reference>) -> reference {
				//TODO: [C++??] precondition(!vptr->done(ptr));
				return static_cast<reference>(*vptr->promise(ptr).ptr);
			}

			auto operator++() -> iterator & {
				//TODO: [C++??] precondition(!vptr->done(ptr));
				vptr->resume(ptr);
				return *this;
			}
			void operator++(int) { ++*this; }

			friend
			auto operator==(const iterator & i, std::default_sentinel_t) -> bool {
				return i.vptr->done(i.ptr);
			}
		private:
			friend generator;
			iterator(const vtable * vptr, void * ptr) noexcept : vptr{vptr}, ptr{ptr} {}

			const vtable * vptr;
			void * ptr;
		};
	public:
		generator(const generator &) =delete;
		generator(generator && other) noexcept : ptr{std::exchange(other.ptr, {})}, vptr{std::exchange(other.vptr, {})} {}

		auto operator=(generator other) noexcept -> generator & {
			std::swap(ptr, other.ptr);
			std::swap(vptr, other.vptr);
			return *this;
		}

		~generator() noexcept { if(ptr) vptr->destroy(ptr); }

		auto begin() -> iterator {
			//TODO: [C++??] precondition(ptr­ refers to a coroutine suspended at its initial suspend point);
			vptr->resume(ptr);
			return {vptr, ptr};
		}
		auto end() const noexcept -> std::default_sentinel_t { return std::default_sentinel; }
	private:
		friend promise_type;
		generator(std::coroutine_handle<promise_type> handle) {
			static constexpr vtable vtable{
				+[](void * ptr) /*TODO: noexcept*/ { return std::coroutine_handle<>::from_address(ptr).done(); },
				+[](void * ptr) /*TODO: noexcept*/ { std::coroutine_handle<>::from_address(ptr).resume(); },
				+[](void * ptr) /*TODO: noexcept*/ { std::coroutine_handle<>::from_address(ptr).destroy(); },
				+[](void * ptr) noexcept -> promise_type & { return std::coroutine_handle<promise_type>::from_address(ptr).promise(); }
			};
			vptr = &vtable;
			ptr = handle.address();
		}
	};
}
