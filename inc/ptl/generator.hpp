
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
		class coroutine_handle final {
			enum class mode { promise, done, resume, destroy, };

			void*(*func)(void *, mode) /*noexcept*/;
			void * ptr{nullptr};
		public:
			coroutine_handle() noexcept =default;

			template<typename Promise>
			coroutine_handle(std::coroutine_handle<Promise> handle) noexcept {
				if(!handle) return;

				func = +[](void * ptr, mode m) /*noexcept*/ -> void * {
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

			auto promise() const noexcept -> void * { return func(ptr, mode::promise); }

			auto done() const noexcept -> bool { return func(ptr, mode::done) != nullptr; }
			void resume() const /*noexcept*/ { func(ptr, mode::resume); }
			void destroy() const noexcept { if(ptr) func(ptr, mode::destroy); }

			void swap(coroutine_handle other) noexcept {
				std::swap(func, other.func);
				std::swap(ptr, other.ptr);
			}
		};
	}

	//! @brief lazy view of elements yielded by a coroutine
	//! @tparam Type element type yielded when iterating over the view
	//! @attention throwing an exception across ABI boundaries is undefined
	template<typename Type>
	class generator final : public std::ranges::view_interface<generator<Type>> {
		using reference = Type &&;
		using value = std::remove_cvref_t<Type>;
		static_assert(std::is_object_v<value>);

		using rref = std::remove_reference_t<reference> &&;
		static_assert(std::common_reference_with<reference &&, value &>);
		static_assert(std::common_reference_with<reference &&, rref &&>);
		static_assert(std::common_reference_with<rref &&, const value &>);

		struct iterator;
	public:
		class promise_type final {
			friend iterator;

			std::add_pointer_t<reference> ptr{nullptr};
		public:
			auto get_return_object() noexcept -> generator { return std::coroutine_handle<promise_type>::from_promise(*this); }

			static
			auto initial_suspend() noexcept -> std::suspend_always { return {}; }
			static
			auto final_suspend() noexcept -> std::suspend_always { return {}; }

			auto yield_value(reference val) noexcept -> std::suspend_always {
				ptr = std::addressof(val);
				return {};
			}

			auto yield_value(const std::remove_reference_t<reference> & lval) requires std::is_rvalue_reference_v<reference> && std::constructible_from<std::remove_cvref_t<reference>, const std::remove_reference_t<reference> &> {
				struct awaitable final {
					std::remove_cvref_t<reference> val;

					static
					auto await_ready() noexcept -> bool { return false; }
					void await_suspend(std::coroutine_handle<promise_type> handle) noexcept { handle.promise().ptr = std::addressof(val); }
					static
					void await_resume() noexcept {}
				};
				return awaitable{lval};
			}

			void await_transform() =delete;

			static
			void return_void() noexcept {}
			static
			void unhandled_exception() { throw; }
		};
	private:
		internal_generator::coroutine_handle handle;

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
				return static_cast<reference>(*static_cast<promise_type *>(handle.promise())->ptr);
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
			iterator(internal_generator::coroutine_handle handle) noexcept : handle{handle} {}

			internal_generator::coroutine_handle handle;
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
