
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <ranges>
#include <concepts>
#include <coroutine>

namespace ptl {
	namespace internal_generator {
		enum class mode { promise, done, resume, destroy, };

		template<typename Promise>
		class coroutine_handle final {
			void*(*func)(void *, mode) /*noexcept*/;
			void * ptr{nullptr};
		public:
			coroutine_handle() noexcept =default;
			coroutine_handle(std::coroutine_handle<Promise> handle) {
				if(!handle) return;

				func = +[](void * ptr, mode m) /*TODO: noexcept*/ -> void * {
					const auto handle{std::coroutine_handle<Promise>::from_address(ptr)};
					switch (m) {
						case mode::promise: return &handle.promise();
						case mode::done: {
							int dummy;
							return handle.done() ? &dummy : nullptr;
						}
						case mode::resume: handle.resume(); break;
						case mode::destroy: handle.destroy(); break;
					}
					return nullptr;
				};
				ptr = handle.address();
			}

			auto done() const noexcept -> bool { return func(ptr, mode::done) != nullptr; }
			void resume() const /*TODO: noexcept*/ { func(ptr, mode::resume); }
			auto promise() const noexcept -> Promise & { return *static_cast<Promise *>(func(ptr, mode::promise)); }
			void destroy() const noexcept { if(ptr) func(ptr, mode::destroy); }

			void swap(coroutine_handle other) noexcept {
				std::swap(func, other.func);
				std::swap(ptr, other.ptr);
			}
		};
	}

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

			using deleter = void(*)(void *); //TODO: is this even necessary?!

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
		};
	private:
		internal_generator::coroutine_handle<promise_type> handle;

		struct iterator final {
			using value_type = value;
			using difference_type = std::ptrdiff_t;

			iterator(iterator && other) noexcept : handle{std::exchange(other.handle, {})} {}
			auto operator=(iterator && other) noexcept -> iterator & {
				handle = std::exchange(other.handle, {});
				return *this;
			}

			auto operator*() const noexcept(std::is_nothrow_copy_constructible_v<reference>) -> reference {
				//TODO: [C++??] precondition(!handle.done());
				return static_cast<reference>(*handle.promise().ptr);
			}

			auto operator++() -> iterator & {
				//TODO: [C++??] precondition(!handle.done());
				handle.resume();
				return *this;
			}
			void operator++(int) { ++*this; }

			friend
			auto operator==(const iterator & i, std::default_sentinel_t) -> bool {
				return i.handle.done();
			}
		private:
			friend generator;
			iterator(internal_generator::coroutine_handle<promise_type> handle) noexcept : handle{handle} {}

			internal_generator::coroutine_handle<promise_type> handle;
		};
	public:
		generator(const generator &) =delete;
		generator(generator && other) noexcept : handle{std::exchange(other.handle, {})} {}

		auto operator=(generator other) noexcept -> generator & {
			handle.swap(other.handle);
			return *this;
		}

		~generator() noexcept { handle.destroy(); }

		auto begin() -> iterator {
			//TODO: [C++??] precondition(handle­ refers to a coroutine suspended at its initial suspend point);
			handle.resume();
			return handle;
		}
		auto end() const noexcept -> std::default_sentinel_t { return std::default_sentinel; }
	private:
		friend promise_type;
		generator(std::coroutine_handle<promise_type> handle) : handle{handle} {}
	};
}
