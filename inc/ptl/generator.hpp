
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cassert> //TODO: remove...
#include <ranges>
#include <concepts>
#include <coroutine>

namespace ptl { //TODO: how to make all of this type-erased & ABI-stable for PTL?????!
	namespace ranges {
		template<std::ranges::range Range>
		struct elements_of {
			Range range;
		};

		template<typename Range>
		elements_of(Range &&) -> elements_of<Range &&>; //TODO: necessary?!
	}

	//! @brief lazy view of elements yielded by a coroutine
	//! @tparam Reference TODO
	//! @tparam Value TODO
	template<typename Reference, typename Value = void>
	class generator final : public std::ranges::view_interface<generator<Reference, Value>> {
		using value = std::conditional_t<std::is_void_v<Value>, std::remove_cvref_t<Reference>, Value>; //exposition only
		static_assert(std::is_object_v<value> && std::is_same_v<std::remove_cvref_t<value>, value>);

		using reference = std::conditional_t<std::is_void_v<Value>, Reference &&, Reference>; //exposition only
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

			struct nested_info final {
				std::exception_ptr eptr;
				std::coroutine_handle<promise_type> bottom, parent; //"stack" navigation
			} * nested{nullptr};

			std::add_pointer_t<yielded> ptr{nullptr};
			std::coroutine_handle<promise_type> top{std::coroutine_handle<promise_type>::from_promise(*this)};
		public:
			auto get_return_object() noexcept -> generator { return std::coroutine_handle<promise_type>::from_promise(*this); }

			auto initial_suspend() const noexcept -> std::suspend_always { return {}; }
			auto final_suspend() noexcept {
				struct awaitable final {
					auto await_ready() const noexcept -> bool { return false; }
					auto await_suspend(std::coroutine_handle<promise_type> handle) noexcept -> std::coroutine_handle<> {
						if(auto nested{handle.promise().nested}) {
							auto parent{nested->parent};
							nested->bottom.promise().top = parent;
							parent.promise().top = parent;
							return parent;
						} else return std::noop_coroutine();
					}
					void await_resume() const noexcept {}
				};
				return awaitable{};
			}

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

			template<typename R2, typename V2>
			requires std::same_as<typename generator<R2, V2>::yielded, yielded>
			auto yield_value(ranges::elements_of<generator<R2, V2> &&> g) noexcept {
				struct awaitable final {
					generator<R2, V2> g;
					nested_info n;

					auto await_ready() const noexcept -> bool { return false; }
					auto await_suspend(std::coroutine_handle<promise_type> handle) {
						g.handle.promise().nested = &n;
						n.parent = handle;
						auto & parent_promise{handle.promise()};
						(n.bottom = parent_promise.nested ? parent_promise.nested->bottom : (assert(parent_promise.top == handle), parent_promise.top)).promise().top = g.handle; //TODO: remove assert...
						return g.handle;
					}
					void await_resume() const { if(n.eptr) std::rethrow_exception(n.eptr); }
				};
				return awaitable{std::move(g.range)};
			}

			template<std::ranges::input_range R>
			requires std::convertible_to<std::ranges::range_reference_t<R>, yielded>
			auto yield_value(ranges::elements_of<R> r) noexcept {
				auto wrapped{[](std::ranges::iterator_t<R> i, std::ranges::sentinel_t<R> s) -> generator<yielded, std::ranges::range_value_t<R>> { for (; i != s; ++i) co_yield static_cast<yielded>(*i); }};
				return yield_value(ranges::elements_of(wrapped(std::ranges::begin(r.range), std::ranges::end(r.range))));
			}

			void await_transform() =delete;

			void return_void() const noexcept {}
			void unhandled_exception() {
				if(nested) nested->eptr = std::current_exception();
				else throw;
			}

			//auto operator new(std::size_t size) -> void * {
			//	auto ptr{std::malloc(size)};
			//	if(!ptr) throw std::bad_alloc{};
			//	//TODO: store information for type-erase inside allocated memory block
			//	return ptr;
			//}
			//void operator delete(void * pointer, std::size_t size) noexcept {
			//	(void)size;
			//	std::free(pointer);
			//	//TODO: extract type-erased deleter from memory block and used instead of direct call to std::free
			//}
		};
	private:
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
				return static_cast<reference>(*handle.promise().top.promise().ptr);
			}

			auto operator++() -> iterator & {
				//TODO: [C++??] precondition(!handle.done());
				handle.promise().top.resume();
				return *this;
			}
			void operator++(int) { ++*this; }

			friend
			auto operator==(const iterator & i, std::default_sentinel_t) -> bool {
				return i.handle.done();
			}
		private:
			friend generator;
			iterator(std::coroutine_handle<promise_type> handle) noexcept : handle{handle} {}

			std::coroutine_handle<promise_type> handle;
		};
	public:
		generator(const generator &) =delete;
		generator(generator && other) noexcept : handle{std::exchange(other.handle, {})} {}

		auto operator=(generator other) noexcept -> generator & {
			std::swap(handle, other.handle);
			return *this;
		}

		~generator() noexcept { if(handle) handle.destroy(); }

		auto begin() -> iterator {
			//TODO: [C++??] precondition(handle­ refers to a coroutine suspended at its initial suspend point);
			handle.resume();
			return handle;
		}
		auto end() const noexcept -> std::default_sentinel_t { return std::default_sentinel; }
	private:
		friend promise_type;
		generator(std::coroutine_handle<promise_type> handle) : handle{std::move(handle)} {}

		std::coroutine_handle<promise_type> handle{nullptr}; //exposition only
	};
}
