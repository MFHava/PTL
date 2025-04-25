
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <concepts>
#include <coroutine>

namespace ptl {
	//! @brief lazy view of elements yielded by a coroutine
	//! @tparam Type element type yielded when iterating over the view
	//! @tparam Noexcept enable support for throwing from coroutines
	template<typename Type, bool Noexcept = true>
	class generator final {
		static_assert(Noexcept, "throwing across ABI boundaries is undefined, therefore this is currently unsupported");

		using reference = Type &&;
		using value = std::remove_cvref_t<Type>;
		static_assert(std::is_object_v<value>);

		using rref = std::remove_reference_t<reference> &&;
		static_assert(std::common_reference_with<reference &&, value &>);
		static_assert(std::common_reference_with<reference &&, rref &&>);
		static_assert(std::common_reference_with<rref &&, const value &>);

		class handle_type;
	public:
		class promise_type final {
			friend handle_type;

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

			auto yield_value(const std::remove_reference_t<reference> & lval) requires std::is_rvalue_reference_v<reference> and std::constructible_from<std::remove_cvref_t<reference>, const std::remove_reference_t<reference> &> {
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

			static
			void await_transform() =delete;
			static
			void return_void() noexcept {}
			static
			void unhandled_exception() noexcept(Noexcept) {
				if constexpr(Noexcept) std::terminate();
				else throw;
			}
		};
	private:
		friend promise_type;
		generator(std::coroutine_handle<promise_type> handle) : handle{handle} {}

		class handle_type final {
			enum class operation { done, resume, destroy, };
			using func_t = bool(*)(promise_type *, operation) noexcept(Noexcept);

			func_t fptr{nullptr};
			promise_type * ptr{nullptr};
		public:
			handle_type() noexcept =default;
			handle_type(std::coroutine_handle<promise_type> handle) noexcept {
				if(!handle) return;

				fptr = +[](promise_type * ptr, operation op) noexcept(Noexcept) -> bool {
					const auto handle{std::coroutine_handle<promise_type>::from_promise(*ptr)};
					switch(op) {
						case operation::done: return handle.done();
						case operation::resume: handle.resume(); break;
						case operation::destroy: handle.destroy(); break;
					}
					return false;
				};
				ptr = std::addressof(handle.promise());
			}

			handle_type(handle_type && other) noexcept : fptr{std::exchange(other.fptr, {})}, ptr{std::exchange(other.ptr, {})} {}
			auto operator=(handle_type && other) noexcept -> handle_type & {
				std::swap(fptr, other.fptr);
				std::swap(ptr, other.ptr);
				return *this;
			}
			~handle_type() noexcept { if(ptr) fptr(ptr, operation::destroy); }

			auto value() const noexcept -> reference { return static_cast<reference>(*ptr->ptr); } //TODO: [C++??] precondition(*this);

			auto done() const noexcept -> bool { return fptr(ptr, operation::done); } //TODO: [C++??] precondition(*this);
			void resume() const noexcept(Noexcept) { fptr(ptr, operation::resume); } //TODO: [C++??] precondition(*this);

			explicit
			operator bool() const noexcept { return ptr; }
		};

		class iterator final {
			friend generator;
			iterator(handle_type && handle) noexcept : handle{std::move(handle)} { this->handle.resume(); }

			handle_type handle;
		public:
			using value_type = value;
			using difference_type = std::ptrdiff_t;

			auto operator*() const noexcept(std::is_nothrow_copy_constructible_v<reference>) -> reference {
				//TODO: [C++??] precondition(handle && !handle.done());
				return handle.value();
			}

			auto operator++() -> iterator & {
				//TODO: [C++??] precondition(handle && !handle.done());
				handle.resume();
				return *this;
			}
			void operator++(int) { ++*this; }

			friend
			auto operator==(const iterator & self, std::default_sentinel_t) noexcept -> bool { return self.handle.done(); } //TODO: [C++??] precondition(self.handle);
		};

		handle_type handle;
	public:
		auto valueless() const noexcept -> bool { return !handle; }

		//! @attention @c this becomes @b valueless and ownership of the managed coroutine is passed to the returned iterator
		auto begin() -> iterator { return std::move(handle); } //TODO: [C++??] precondition(not valueless());
		auto end() const noexcept -> std::default_sentinel_t { return std::default_sentinel; }
	};
}
