
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <numeric>
#include <catch.hpp>
#include "ptl/function.hpp"

namespace {
	int func1()          { return 0; }
	int func2() noexcept { return 1; }
	int func3()          { return 2; }


	class small_func {
		int val;
	public:
		small_func(int val) noexcept : val{val} {}
		auto operator()() const -> int { return val; }
	};

	class big_func {
		int val;
		int buffer[10];
	public:
		big_func(int val) noexcept : val{val} {}
		auto operator()() const -> int { return val; }
	};


	template<template<typename...> typename Function>
	void test_nullptr() {
		Function<int() noexcept> fun{nullptr};
		REQUIRE(!static_cast<bool>(fun));
		REQUIRE(!fun);
		REQUIRE(fun == nullptr);

		fun = func2;
		REQUIRE(static_cast<bool>(fun));
		REQUIRE(!!fun);
		REQUIRE(fun != nullptr);

		fun = nullptr;
		REQUIRE(!static_cast<bool>(fun));
		REQUIRE(!fun);
		REQUIRE(fun == nullptr);

		fun = func2;
		REQUIRE(static_cast<bool>(fun));
		REQUIRE(!!fun);
		REQUIRE(fun != nullptr);

		fun = static_cast<decltype(&func2)>(nullptr);
		REQUIRE(!static_cast<bool>(fun));
		REQUIRE(!fun);
		REQUIRE(fun == nullptr);
	}

	template<template<typename...> typename Function>
	void test_inplace() {
		struct functor {
			functor(int x) : x{x} {}
			functor(std::initializer_list<int> ilist) : x{std::accumulate(ilist.begin(), ilist.end(), 0)} {}
			functor(std::initializer_list<int> ilist, int x) : functor{ilist} { this->x += x; }

			auto operator()(int y) noexcept -> int { return x + y; }
		private:
			int x;
		};

		Function<int(int) noexcept> func0{std::in_place_type<functor>, 10};
		REQUIRE(func0(1) == 11);
	
		Function<int(int) noexcept> func1{std::in_place_type<functor>, {11, 12}};
		REQUIRE(func1(1) == 24);

		Function<int(int) noexcept> func2{std::in_place_type<functor>, {11, 12}, 11};
		REQUIRE(func2(1) == 35);
	}

	template<template<typename...> typename Function>
	void test_free_function() {
		Function<int()> ref1{func1};
		REQUIRE(ref1() == 0);
		Function<int() const> cref1{func1};
		REQUIRE(cref1() == 0);
		Function<int() noexcept> ref2{func2};
		REQUIRE(ref2() == 1);
		Function<int() const noexcept> cref2{func2};
		REQUIRE(cref2() == 1);
	}

	template<template<typename...> typename Function>
	void test_free_function_ptr() {
		Function<int()> ref1{&func1};
		REQUIRE(ref1() == 0);
		Function<int() const> cref1{&func1};
		REQUIRE(cref1() == 0);
		Function<int() noexcept> ref2{&func2};
		REQUIRE(ref2() == 1);
		Function<int() const noexcept> cref2{&func2};
		REQUIRE(cref2() == 1);
	}

	template<template<typename...> typename Function>
	void test_member_function_ptr() {
		struct X {
			int val;

			int func(int) { return val; }
		} x{10};

		Function<int(X *, int)> f{&X::func};

		REQUIRE(f(&x, 1) == x.val);

		Function<int(X*)> m{&X::val};
		REQUIRE(m(&x) == x.val);
	}

	template<template<typename...> typename Function>
	void test_move_ctor() {
		//EMPTY
		ptl::copyable_function<int() const> mf0;
		REQUIRE(!mf0);
		ptl::copyable_function<int() const> f0{std::move(mf0)};
		REQUIRE(!f0);
		REQUIRE(!mf0);

		//FREE FUNC
		ptl::copyable_function<int() const> mf1{func1};
		REQUIRE(mf1);
		ptl::copyable_function<int() const> f1{std::move(mf1)};
		REQUIRE(f1);
		REQUIRE(!mf1);

		//FREE FUNC PTR
		ptl::copyable_function<int() const> mf2{&func1};
		REQUIRE(mf2);
		ptl::copyable_function<int() const> f2{std::move(mf2)};
		REQUIRE(f2);
		REQUIRE(!mf2);

		//SOO
		ptl::copyable_function<int() const> mf3{std::in_place_type<small_func>, 123};
		REQUIRE(mf3);
		ptl::copyable_function<int() const> f3{std::move(mf3)};
		REQUIRE(f3);
		REQUIRE(!mf3);

		//noSOO
		ptl::copyable_function<int() const> mf4{std::in_place_type<big_func>, 123};
		REQUIRE(mf4);
		ptl::copyable_function<int() const> f4{std::move(mf4)};
		REQUIRE(f4);
		REQUIRE(!mf4);
	}

	template<template<typename...> typename Function>
	void test_move_assign() {
		//EMPTY
		ptl::copyable_function<int() const> mf0;
		REQUIRE(!mf0);
		ptl::copyable_function<int() const> f0;
		REQUIRE(!f0);
		f0 = std::move(mf0);
		REQUIRE(!f0);
		REQUIRE(!mf0);

		//FREE FUNC
		ptl::copyable_function<int() const> mf1{func1};
		REQUIRE(mf1);
		ptl::copyable_function<int() const> f1;
		REQUIRE(!f1);
		f1 = std::move(mf1);
		REQUIRE(f1);
		REQUIRE(!mf1);

		//FREE FUNC PTR
		ptl::copyable_function<int() const> mf2{&func1};
		REQUIRE(mf2);
		ptl::copyable_function<int() const> f2;
		REQUIRE(!f2);
		f2 = std::move(mf2);
		REQUIRE(f2);
		REQUIRE(!mf2);

		//SOO
		ptl::copyable_function<int() const> mf3{std::in_place_type<small_func>, 123};
		REQUIRE(mf3);
		ptl::copyable_function<int() const> f3;
		REQUIRE(!f3);
		f3 = std::move(mf3);
		REQUIRE(f3);
		REQUIRE(!mf3);

		//noSOO
		ptl::copyable_function<int() const> mf4{std::in_place_type<big_func>, 123};
		REQUIRE(mf4);
		ptl::copyable_function<int() const> f4;
		REQUIRE(!f4);
		f4 = std::move(mf4);
		REQUIRE(f4);
		REQUIRE(!mf4);
	}

	template<template<typename...> typename Function>
	void test_functor() {
		struct { auto operator()() const          -> int { return 0; } } func1;
		struct { auto operator()() const noexcept -> int { return 1; } } func2;
		struct { auto operator()()                -> int { return 2; } } func3;
		struct { auto operator()()       noexcept -> int { return 3; } } func4;

		Function<int()> ref1{func1};
		REQUIRE(ref1() == 0);
		Function<int() const> cref1{func1};
		REQUIRE(cref1() == 0);
		Function<int() noexcept> ref2{func2};
		REQUIRE(ref2() == 1);
		Function<int() const noexcept> cref2{func2};
		REQUIRE(cref2() == 1);

		Function<int()> ref3{func3};
		REQUIRE(ref3() == 2);
		static_assert(!std::is_constructible_v<Function<int() const>, decltype(func3)>);
		Function<int() noexcept> ref4{func4};
		REQUIRE(ref4() == 3);
		static_assert(!std::is_constructible_v<Function<int() const noexcept>, decltype(func4)>);
	}

	template<template<typename...> typename Function>
	void test_moved_from_state() {
		static_assert(sizeof(small_func) <= sizeof(Function<int() const>));
		static_assert(sizeof(big_func) > sizeof(Function<int() const>));

		//EMPTY
		Function<int() const> mf0;
		REQUIRE(!mf0);
		Function<int() const> f0{std::move(mf0)};
		REQUIRE(!f0);
		REQUIRE(!mf0);

		//FREE FUNC
		Function<int() const> mf1{func1};
		REQUIRE(mf1);
		Function<int() const> f1{std::move(mf1)};
		REQUIRE(f1);
		REQUIRE(!mf1);

		//FREE FUNC PTR
		Function<int() const> mf2{&func1};
		REQUIRE(mf2);
		Function<int() const> f2{std::move(mf2)};
		REQUIRE(f2);
		REQUIRE(!mf2);

		//SOO
		Function<int() const> mf3{std::in_place_type<small_func>, 123};
		REQUIRE(mf3);
		Function<int() const> f3{std::move(mf3)};
		REQUIRE(f3);
		REQUIRE(!mf3);

		//noSOO
		Function<int() const> mf4{std::in_place_type<big_func>, 123};
		REQUIRE(mf4);
		Function<int() const> f4{std::move(mf4)};
		REQUIRE(f4);
		REQUIRE(!mf4);
	}

	template<template<typename...> typename Function>
	void test_swapping() {
		static_assert(sizeof(small_func) <= sizeof(Function<int() const>));
		static_assert(sizeof(big_func) > sizeof(Function<int() const>));

		//EMPTY swap EMPTY
		Function<int() const> f0, f1;
		swap(f0, f1);
		REQUIRE(!f0);
		REQUIRE(!f1);

		//EMPTY swap FREE FUNC
		Function<int() const> f2, f3{func1};
		swap(f2, f3);
		REQUIRE(!f3);
		REQUIRE(f2);
		REQUIRE(f2() == 0);

		//EMPTY swap SOO
		Function<int() const> f4, f5{std::in_place_type<small_func>, 1234};
		swap(f4, f5);
		REQUIRE(!f5);
		REQUIRE(f4);
		REQUIRE(f4() == 1234);

		//EMPTY swap noSOO
		Function<int() const> f6, f7{std::in_place_type<big_func>, 56789};
		swap(f6, f7);
		REQUIRE(!f7);
		REQUIRE(f6);
		REQUIRE(f6() == 56789);

		//FREE FUNC swap FREE FUNC
		Function<int() const> f8{func1}, f9{func3};
		swap(f8, f9);
		REQUIRE(f8);
		REQUIRE(f8() == 2);
		REQUIRE(f9);
		REQUIRE(f9() == 0);

		//FREE FUNC swap SOO
		Function<int() const> f10{func1}, f11{std::in_place_type<small_func>, 10};
		swap(f10, f11);
		REQUIRE(f10);
		REQUIRE(f10() == 10);
		REQUIRE(f11);
		REQUIRE(f11() == 0);

		//FREE FUNC swap noSOO
		Function<int() const> f12{func1}, f13{std::in_place_type<big_func>, 10};
		swap(f12, f13);
		REQUIRE(f12);
		REQUIRE(f12() == 10);
		REQUIRE(f13);
		REQUIRE(f13() == 0);

		//noSOO swap noSOO
		Function<int() const> f14{std::in_place_type<big_func>, 1}, f15{std::in_place_type<big_func>, 2};
		swap(f14, f15);
		REQUIRE(f14);
		REQUIRE(f14() == 2);
		REQUIRE(f15);
		REQUIRE(f15() == 1);

		//noSOO swap SOO
		Function<int() const> f16{std::in_place_type<big_func>, 10}, f17{std::in_place_type<small_func>, 99};
		swap(f16, f17);
		REQUIRE(f16);
		REQUIRE(f16() == 99);
		REQUIRE(f17);
		REQUIRE(f17() == 10);

		//SSO swap SSO
		Function<int() const> f18{std::in_place_type<small_func>, 17}, f19{std::in_place_type<small_func>, 50};
		swap(f18, f19);
		REQUIRE(f18);
		REQUIRE(f18() == 50);
		REQUIRE(f19);
		REQUIRE(f19() == 17);
	}
}

TEST_CASE("function nullptr", "[function]") { test_nullptr<ptl::function>(); }
TEST_CASE("copyable_function nullptr", "[copyable_function]") { test_nullptr<ptl::copyable_function>(); }

TEST_CASE("function inplace", "[function]") { test_inplace<ptl::function>(); }
TEST_CASE("copyable_function inplace", "[copyable_function]") { test_inplace<ptl::copyable_function>(); }

TEST_CASE("function free function", "[function]") { test_free_function<ptl::function>(); }
TEST_CASE("copyable_function free function", "[copyable_function]") { test_free_function<ptl::copyable_function>(); }

TEST_CASE("function free function ptr", "[function]") { test_free_function_ptr<ptl::function>(); }
TEST_CASE("copyable_function free function ptr", "[copyable_function]") { test_free_function_ptr<ptl::copyable_function>(); }

TEST_CASE("function member function ptr", "[function]") { test_member_function_ptr<ptl::function>(); }
TEST_CASE("copyable_function member function ptr", "[copyable_function]") { test_member_function_ptr<ptl::copyable_function>(); }

TEST_CASE("function functor", "[function]") { test_functor<ptl::function>(); }
TEST_CASE("copyable_function functor", "[copyable_function]") { test_functor<ptl::copyable_function>(); }

TEST_CASE("function move ctor", "[function]") { test_move_ctor<ptl::function>(); }
TEST_CASE("copyable_function move ctor", "[copyable_function]") { test_move_ctor<ptl::copyable_function>(); }

TEST_CASE("function move assign", "[function]") { test_move_assign<ptl::function>(); }
TEST_CASE("copyable_function move assign", "[copyable_function]") { test_move_assign<ptl::copyable_function>(); }

TEST_CASE("function moved-from state", "[function]") { test_moved_from_state<ptl::function>(); }
TEST_CASE("copyable_function moved-from state", "[copyable_function]") { test_moved_from_state<ptl::copyable_function>(); }

TEST_CASE("function swapping", "[function]") { test_swapping<ptl::function>(); }
TEST_CASE("copyable_function swapping", "[copyable_function]") { test_swapping<ptl::copyable_function>(); }


TEST_CASE("copyable_function copy ctor", "[copyable_function]") {
	//EMPTY
	ptl::copyable_function<int() const> mf0;
	REQUIRE(!mf0);
	ptl::copyable_function<int() const> f0{mf0};
	REQUIRE(!f0);
	REQUIRE(!mf0);

	//FREE FUNC
	ptl::copyable_function<int() const> mf1{func1};
	REQUIRE(mf1);
	ptl::copyable_function<int() const> f1{mf1};
	REQUIRE(f1);
	REQUIRE(mf1);

	//FREE FUNC PTR
	ptl::copyable_function<int() const> mf2{&func1};
	REQUIRE(mf2);
	ptl::copyable_function<int() const> f2{mf2};
	REQUIRE(f2);
	REQUIRE(mf2);

	//SOO
	ptl::copyable_function<int() const> mf3{std::in_place_type<small_func>, 123};
	REQUIRE(mf3);
	ptl::copyable_function<int() const> f3{mf3};
	REQUIRE(f3);
	REQUIRE(mf3);

	//noSOO
	ptl::copyable_function<int() const> mf4{std::in_place_type<big_func>, 123};
	REQUIRE(mf4);
	ptl::copyable_function<int() const> f4{mf4};
	REQUIRE(f4);
	REQUIRE(mf4);
}

TEST_CASE("copyable_function copy assign", "[copyable_function]") {
	//EMPTY
	ptl::copyable_function<int() const> mf0;
	REQUIRE(!mf0);
	ptl::copyable_function<int() const> f0;
	REQUIRE(!f0);
	f0 = mf0;
	REQUIRE(!f0);
	REQUIRE(!mf0);

	//FREE FUNC
	ptl::copyable_function<int() const> mf1{func1};
	REQUIRE(mf1);
	ptl::copyable_function<int() const> f1;
	REQUIRE(!f1);
	f1 = mf1;
	REQUIRE(f1);
	REQUIRE(mf1);

	//FREE FUNC PTR
	ptl::copyable_function<int() const> mf2{&func1};
	REQUIRE(mf2);
	ptl::copyable_function<int() const> f2;
	REQUIRE(!f2);
	f2 = mf2;
	REQUIRE(f2);
	REQUIRE(mf2);

	//SOO
	ptl::copyable_function<int() const> mf3{std::in_place_type<small_func>, 123};
	REQUIRE(mf3);
	ptl::copyable_function<int() const> f3;
	REQUIRE(!f3);
	f3 = mf3;
	REQUIRE(f3);
	REQUIRE(mf3);

	//noSOO
	ptl::copyable_function<int() const> mf4{std::in_place_type<big_func>, 123};
	REQUIRE(mf4);
	ptl::copyable_function<int() const> f4;
	REQUIRE(!f4);
	f4 = mf4;
	REQUIRE(f4);
	REQUIRE(mf4);
}

TEST_CASE("copyable_function conversion", "[function] [copyable_function]") {
	//EMPTY
	ptl::copyable_function<int() const> cf0;
	REQUIRE(!cf0);
	ptl::function<int() const> f0a{cf0};
	REQUIRE(!cf0);
	REQUIRE(!f0a);
	ptl::function<int() const> f0b{std::move(cf0)};
	REQUIRE(!cf0);
	REQUIRE(!f0a);
	REQUIRE(!f0b);

	//FREE FUNC
	ptl::copyable_function<int() const> cf1{func1};
	REQUIRE(cf1);
	REQUIRE(cf1() == func1());
	ptl::function<int() const> f1a{cf1};
	REQUIRE(cf1);
	REQUIRE(cf1() == func1());
	REQUIRE(f1a);
	REQUIRE(f1a() == func1());
	ptl::function<int() const> f1b{std::move(cf1)};
	REQUIRE(!cf1);
	REQUIRE(f1a);
	REQUIRE(f1a() == func1());
	REQUIRE(f1b);
	REQUIRE(f1b() == func1());

	//FREE FUNC PTR
	ptl::copyable_function<int() const> cf2{&func1};
	REQUIRE(cf2);
	REQUIRE(cf2() == func1());
	ptl::function<int() const> f2a{cf2};
	REQUIRE(cf2);
	REQUIRE(cf2() == func1());
	REQUIRE(f2a);
	REQUIRE(f2a() == func1());
	ptl::function<int() const> f2b{std::move(cf2)};
	REQUIRE(!cf2);
	REQUIRE(f2a);
	REQUIRE(f2a() == func1());
	REQUIRE(f2b);
	REQUIRE(f2b() == func1());

	//SOO
	ptl::copyable_function<int() const> cf3{std::in_place_type<small_func>, 123};
	REQUIRE(cf3);
	REQUIRE(cf3() == 123);
	ptl::function<int() const> f3a{cf3};
	REQUIRE(cf3);
	REQUIRE(cf3() == 123);
	REQUIRE(f3a);
	REQUIRE(f3a() == 123);
	ptl::function<int() const> f3b{std::move(cf3)};
	REQUIRE(!cf3);
	REQUIRE(f3a);
	REQUIRE(f3a() == 123);
	REQUIRE(f3b);
	REQUIRE(f3b() == 123);

	//noSOO
	ptl::copyable_function<int() const> cf4{std::in_place_type<big_func>, 123};
	REQUIRE(cf4);
	REQUIRE(cf4() == 123);
	ptl::function<int() const> f4a{cf4};
	REQUIRE(cf4);
	REQUIRE(cf4() == 123);
	REQUIRE(f4a);
	REQUIRE(f4a() == 123);
	ptl::function<int() const> f4b{std::move(cf4)};
	REQUIRE(!cf4);
	REQUIRE(f4a);
	REQUIRE(f4a() == 123);
	REQUIRE(f4b);
	REQUIRE(f4b() == 123);
}


TEST_CASE("function move-only ctor", "[function]") {
	class functor final {
		int val;
	public:
		functor(int val) noexcept : val{val} {}
		functor(const functor &) =delete;
		functor(functor && other) noexcept : val{other.val} {}
		auto operator=(const functor &) -> functor & =delete;
		auto operator=(functor && other) noexcept -> functor & {
			val = other.val;
			return *this;
		}
		~functor() noexcept =default;

		auto operator()() const noexcept -> int { return val; }
	};

	ptl::function<int() const> func{std::in_place_type<functor>, 1};
	REQUIRE(func() == 1);
}

namespace {
	struct size {
		size() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(sizeof(Function<void()                  >) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void()       &          >) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void()       &&         >) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void() const            >) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void() const &          >) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void() const &&         >) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void()          noexcept>) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void()       &  noexcept>) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void()       && noexcept>) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void() const    noexcept>) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void() const &  noexcept>) == 4 * sizeof(void *));
			static_assert(sizeof(Function<void() const && noexcept>) == 4 * sizeof(void *));
		}
	} s;

	struct functor0 {
		functor0() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert( std::is_constructible_v<Function<void(int)                  >, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor0>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor0>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor0>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor0>);
		}
	} f0;

	struct functor1 {
		functor1() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) & {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor1>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int)       &&         >, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor1>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor1>);
		}
	} f1;

	struct functor2 {
		functor2() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) && {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int)       &          >, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor2>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor2>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor2>);
		}
	} f2;

	struct functor3 {
		functor3() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) const {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert( std::is_constructible_v<Function<void(int)                  >, functor3>);
			static_assert( std::is_constructible_v<Function<void(int) const            >, functor3>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor3>);
			static_assert( std::is_constructible_v<Function<void(int) const &          >, functor3>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor3>);
			static_assert( std::is_constructible_v<Function<void(int) const &&         >, functor3>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor3>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor3>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor3>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor3>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor3>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor3>);
		}
	} f3;

	struct functor4 {
		functor4() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) const & {}
		void operator()(int) const && =delete;

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor4>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor4>);
			static_assert( std::is_constructible_v<Function<void(int) const &          >, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int)       &&         >, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor4>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor4>);
		}
	} f4;

	struct functor5 {
		functor5() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) const && {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int)       &          >, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor5>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor5>);
			static_assert( std::is_constructible_v<Function<void(int) const &&         >, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor5>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor5>);
		}
	} f5;

	struct functor6 {
		functor6() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) noexcept {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert( std::is_constructible_v<Function<void(int)                  >, functor6>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor6>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor6>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor6>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor6>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor6>);
			static_assert( std::is_constructible_v<Function<void(int)          noexcept>, functor6>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor6>);
			static_assert( std::is_constructible_v<Function<void(int)       &  noexcept>, functor6>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor6>);
			static_assert( std::is_constructible_v<Function<void(int)       && noexcept>, functor6>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor6>);
		}
	} f6;

	struct functor7 {
		functor7() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) & noexcept {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor7>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int)       &&         >, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor7>);
			static_assert( std::is_constructible_v<Function<void(int)       &  noexcept>, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor7>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor7>);
		}
	} f7;

	struct functor8 {
		functor8() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) && noexcept {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int)       &          >, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor8>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor8>);
			static_assert( std::is_constructible_v<Function<void(int)       && noexcept>, functor8>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor8>);
		}
	} f8;

	struct functor9 {
		functor9() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) const noexcept {}

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert( std::is_constructible_v<Function<void(int)                  >, functor9>);
			static_assert( std::is_constructible_v<Function<void(int) const            >, functor9>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor9>);
			static_assert( std::is_constructible_v<Function<void(int) const &          >, functor9>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor9>);
			static_assert( std::is_constructible_v<Function<void(int) const &&         >, functor9>);
			static_assert( std::is_constructible_v<Function<void(int)          noexcept>, functor9>);
			static_assert( std::is_constructible_v<Function<void(int) const    noexcept>, functor9>);
			static_assert( std::is_constructible_v<Function<void(int)       &  noexcept>, functor9>);
			static_assert( std::is_constructible_v<Function<void(int) const &  noexcept>, functor9>);
			static_assert( std::is_constructible_v<Function<void(int)       && noexcept>, functor9>);
			static_assert( std::is_constructible_v<Function<void(int) const && noexcept>, functor9>);
		}
	} f9;

	struct functor10 {
		functor10() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) const & noexcept {}
		void operator()(int) const && noexcept =delete;

		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor10>);
			static_assert( std::is_constructible_v<Function<void(int)       &          >, functor10>);
			static_assert( std::is_constructible_v<Function<void(int) const &          >, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int)       &&         >, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int) const &&         >, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor10>);
			static_assert( std::is_constructible_v<Function<void(int)       &  noexcept>, functor10>);
			static_assert( std::is_constructible_v<Function<void(int) const &  noexcept>, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int)       && noexcept>, functor10>);
			static_assert(!std::is_constructible_v<Function<void(int) const && noexcept>, functor10>);
		}
	} f10;

	struct functor11 {
		functor11() {
			validate<ptl::function>();
			validate<ptl::copyable_function>();
		}

		void operator()(int) const && noexcept {}
	
		template<template<typename...> typename Function>
		static
		void validate() {
			static_assert(!std::is_constructible_v<Function<void(int)                  >, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int) const            >, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int)       &          >, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int) const &          >, functor11>);
			static_assert( std::is_constructible_v<Function<void(int)       &&         >, functor11>);
			static_assert( std::is_constructible_v<Function<void(int) const &&         >, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int)          noexcept>, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int) const    noexcept>, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int)       &  noexcept>, functor11>);
			static_assert(!std::is_constructible_v<Function<void(int) const &  noexcept>, functor11>);
			static_assert( std::is_constructible_v<Function<void(int)       && noexcept>, functor11>);
			static_assert( std::is_constructible_v<Function<void(int) const && noexcept>, functor11>);
		}
	} f11;
}
