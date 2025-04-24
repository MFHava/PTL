
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
		//! @note contrary to @c std::coroutine_handle this class is owning
		template<typename Promise, bool Noexcept>
		class coroutine_handle final {
			enum class operation { done, resume, destroy, };
			using func_t = bool(*)(Promise *, operation) noexcept(Noexcept);

			func_t fptr{nullptr};
			Promise * ptr{nullptr};
		public:
			coroutine_handle() noexcept =default;

			coroutine_handle(std::coroutine_handle<Promise> handle) noexcept {
				if(!handle) return;

				fptr = +[](Promise * ptr, operation op) noexcept(Noexcept) -> bool {
					const auto handle{std::coroutine_handle<Promise>::from_promise(*ptr)};
					switch(op) {
						case operation::done: return handle.done();
						case operation::resume: handle.resume(); break;
						case operation::destroy: handle.destroy(); break;
					}
					return false;
				};
				ptr = std::addressof(handle.promise());
			}

			coroutine_handle(coroutine_handle && other) noexcept : fptr{std::exchange(other.fptr, {})}, ptr{std::exchange(other.ptr, {})} {}
			auto operator=(coroutine_handle && other) noexcept -> coroutine_handle & {
				std::swap(fptr, other.fptr);
				std::swap(ptr, other.ptr);
				return *this;
			}
			~coroutine_handle() noexcept { if(ptr) fptr(ptr, operation::destroy); }

			auto promise() const noexcept -> Promise & { return *ptr; } //TODO: [C++??] precondition(*this);

			auto done() const noexcept -> bool { return fptr(ptr, operation::done); } //TODO: [C++??] precondition(*this);
			void resume() const noexcept(Noexcept) { fptr(ptr, operation::resume); } //TODO: [C++??] precondition(*this);

			explicit
			operator bool() const noexcept { return ptr; }
		};
	}

	//! @brief lazy view of elements yielded by a coroutine
	//! @tparam Type element type yielded when iterating over the view
	//! @tparam Noexcept enable support for throwing from coroutines
	//! @attention throwing an exception across ABI boundaries is undefined
	template<typename Type, bool Noexcept = true>
	class generator final : public std::ranges::view_interface<generator<Type>> {
		static_assert(Noexcept, "throwing across ABI boundaries is undefined, therefore this is currently unsupported");

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

		auto valueless() const noexcept -> bool { return !handle; }

		auto begin() -> iterator { return std::move(handle); } //TODO: [C++??] precondition(not valueless());
		auto end() const noexcept -> std::default_sentinel_t { return std::default_sentinel; }
	private:
		friend promise_type;
		generator(std::coroutine_handle<promise_type> handle) : handle{handle} {}

		using handle_type = internal_generator::coroutine_handle<promise_type, Noexcept>;
		handle_type handle;

		//! @attention becomes owner of coroutine on construction
		struct iterator final {
			using value_type = value;
			using difference_type = std::ptrdiff_t;

			auto operator*() const noexcept(std::is_nothrow_copy_constructible_v<reference>) -> reference {
				//TODO: [C++??] precondition(handle && !handle.done());
				return static_cast<reference>(*handle.promise().ptr);
			}

			auto operator++() -> iterator & {
				//TODO: [C++??] precondition(handle && !handle.done());
				handle.resume();
				return *this;
			}
			void operator++(int) { ++*this; }

			friend
			auto operator==(const iterator & self, std::default_sentinel_t) noexcept -> bool { return self.handle.done(); } //TODO: [C++??] precondition(i.handle);
		private:
			friend generator;
			iterator(handle_type && handle) noexcept : handle{std::move(handle)} { this->handle.resume(); }

			handle_type handle;
		};
	};
}

