
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include "ptl/function_ref.hpp"

static_assert(sizeof(ptl::function_ref<int()>) == sizeof(void *) * 2);
static_assert(sizeof(ptl::function_ref<int(int)>) == sizeof(void *) * 2);
static_assert(sizeof(ptl::function_ref<int() noexcept>) == sizeof(void *) * 2);
static_assert(sizeof(ptl::function_ref<int(int) noexcept>) == sizeof(void *) * 2);

namespace {
	using free_throwing = int();
	using free_noexcept = int() noexcept;

	int func1a()                 { return 0; }
	int func1b() noexcept(false) { return 0; }
	int func2a() noexcept(true)  { return 0; }
	int func2b() noexcept        { return 0; }
}

TEST_CASE("function_ref free function ref", "[function_ref]") {
	ptl::function_ref ref1a{func1a};
	REQUIRE(ref1a() == 0);
	static_assert(std::is_same_v<decltype(ref1a), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref1b{func1b};
	REQUIRE(ref1b() == 0);
	static_assert(std::is_same_v<decltype(ref1b), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref2a{func2a};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2a), ptl::function_ref<free_noexcept>>);
	ptl::function_ref ref2b{func2b};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2b), ptl::function_ref<free_noexcept>>);
}

TEST_CASE("function_ref free function ptr", "[function_ref]") {
	ptl::function_ref ref1a{&func1a};
	REQUIRE(ref1a() == 0);
	static_assert(std::is_same_v<decltype(ref1a), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref1b{&func1b};
	REQUIRE(ref1b() == 0);
	static_assert(std::is_same_v<decltype(ref1b), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref2a{&func2a};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2a), ptl::function_ref<free_noexcept>>);
	ptl::function_ref ref2b{&func2b};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2b), ptl::function_ref<free_noexcept>>);
}

TEST_CASE("function_ref stateless lambda", "[function_ref]") {
	auto func1a = []                           { return 0; };
	auto func1b = []()         noexcept(false) { return 0; };
	auto func2a = []()         noexcept(true)  { return 0; };
	auto func2b = []()         noexcept        { return 0; };
	auto func3a = []() mutable                 { return 0; };
	auto func3b = []() mutable noexcept(false) { return 0; };
	auto func4a = []() mutable noexcept(true)  { return 0; };
	auto func4b = []() mutable noexcept        { return 0; };

	ptl::function_ref ref1a{func1a};
	REQUIRE(ref1a() == 0);
	static_assert(std::is_same_v<decltype(ref1a), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref1b{func1b};
	REQUIRE(ref1b() == 0);
	static_assert(std::is_same_v<decltype(ref1b), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref2a{func2a};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2a), ptl::function_ref<free_noexcept>>);
	ptl::function_ref ref2b{func2b};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2b), ptl::function_ref<free_noexcept>>);

	ptl::function_ref ref3a{func3a};
	REQUIRE(ref3a() == 0);
	static_assert(std::is_same_v<decltype(ref3a), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref3b{func3b};
	REQUIRE(ref3b() == 0);
	static_assert(std::is_same_v<decltype(ref3b), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref4a{func4a};
	REQUIRE(ref4a() == 0);
	static_assert(std::is_same_v<decltype(ref4a), ptl::function_ref<free_noexcept>>);
	ptl::function_ref ref4b{func4b};
	REQUIRE(ref4a() == 0);
	static_assert(std::is_same_v<decltype(ref4b), ptl::function_ref<free_noexcept>>);
}

TEST_CASE("function_ref stateful lambda", "[function_ref]") {
	auto func1a = [value{0}]                           { return value; };
	auto func1b = [value{0}]()         noexcept(false) { return value; };
	auto func2a = [value{0}]()         noexcept(true)  { return value; };
	auto func2b = [value{0}]()         noexcept        { return value; };
	auto func3a = [value{0}]() mutable                 { return value; };
	auto func3b = [value{0}]() mutable noexcept(false) { return value; };
	auto func4a = [value{0}]() mutable noexcept(true)  { return value; };
	auto func4b = [value{0}]() mutable noexcept        { return value; };

	ptl::function_ref ref1a{func1a};
	REQUIRE(ref1a() == 0);
	static_assert(std::is_same_v<decltype(ref1a), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref1b{func1b};
	REQUIRE(ref1b() == 0);
	static_assert(std::is_same_v<decltype(ref1b), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref2a{func2a};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2a), ptl::function_ref<free_noexcept>>);
	ptl::function_ref ref2b{func2b};
	REQUIRE(ref2a() == 0);
	static_assert(std::is_same_v<decltype(ref2b), ptl::function_ref<free_noexcept>>);

	ptl::function_ref ref3a{func3a};
	REQUIRE(ref3a() == 0);
	static_assert(std::is_same_v<decltype(ref3a), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref3b{func3b};
	REQUIRE(ref3b() == 0);
	static_assert(std::is_same_v<decltype(ref3b), ptl::function_ref<free_throwing>>);
	ptl::function_ref ref4a{func4a};
	REQUIRE(ref4a() == 0);
	static_assert(std::is_same_v<decltype(ref4a), ptl::function_ref<free_noexcept>>);
	ptl::function_ref ref4b{func4b};
	REQUIRE(ref4a() == 0);
	static_assert(std::is_same_v<decltype(ref4b), ptl::function_ref<free_noexcept>>);
}
