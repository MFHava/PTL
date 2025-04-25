
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/generator.hpp>

static_assert(!std::is_copy_constructible_v<decltype(std::declval<ptl::generator<int>>().begin())>);
static_assert(std::is_move_constructible_v<ptl::generator<int>>);
static_assert(std::is_move_assignable_v<ptl::generator<int>>);
static_assert(not std::is_copy_constructible_v<ptl::generator<int>>);
static_assert(not std::is_copy_assignable_v<ptl::generator<int>>);

TEST_CASE("generator valueless", "[generator]") {
	auto g{[]() -> ptl::generator<int> { co_return; }()};
	REQUIRE(not g.valueless());
	const auto it{g.begin()};
	REQUIRE(g.valueless());
}

TEST_CASE("generator ownership", "[generator]") {
	auto it{[] {
		auto g{[]() -> ptl::generator<int> {
			for(int i{0};; ++i) co_yield i;
		}()};
		return g.begin();
	}()};

	REQUIRE(*it == 0);
	++it;
	REQUIRE(*it == 1);
	++it;
	REQUIRE(*it == 2);
}

TEST_CASE("generator iteration value", "[generator]") {
	auto g{[]() -> ptl::generator<int> {
		for(auto i{0}; i < 10; ++i)
			co_yield i;
	}()};

	const int vals[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	REQUIRE(std::ranges::equal(g, vals));
}

TEST_CASE("generator iteration ref", "[generator]") {
	auto g{[]() -> ptl::generator<int &> {
		for(auto i{0}; i < 10; ++i)
			co_yield i;
	}()};

	const int vals[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	REQUIRE(std::ranges::equal(g, vals));
}

TEST_CASE("generator iteration cref", "[generator]") {
	auto g{[]() -> ptl::generator<const int &> {
		for(auto i{0}; i < 10; ++i)
			co_yield i;
	}()};

	const int vals[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	REQUIRE(std::ranges::equal(g, vals));
}
