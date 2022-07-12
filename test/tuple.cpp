
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>
#include <ptl/tuple.hpp>

static_assert(sizeof(ptl::tuple<>) == sizeof(char));
static_assert(sizeof(ptl::tuple<char, int>) == sizeof(char) + sizeof(int));
static_assert(sizeof(ptl::tuple<long, char, int>) == sizeof(long) + sizeof(char) + sizeof(int));

static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> & >().get<0>()),       int & >);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> & >().get<0>()), const int & >);
static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> &&>().get<0>()),       int &&>);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> &&>().get<0>()), const int &&>);
static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> & >().get<1>()),       float & >);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> & >().get<1>()), const float & >);
static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> &&>().get<1>()),       float &&>);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> &&>().get<1>()), const float &&>);

TEST_CASE("tuple ctor", "[tuple]") {
	ptl::tuple<int, float, bool> tup0{5, 1.F, true};
	auto tup1{tup0};
	REQUIRE(tup0.get<0>() == tup1.get<0>());
	REQUIRE(tup0.get<1>() == tup1.get<1>());
	REQUIRE(tup0.get<2>() == tup1.get<2>());
	decltype(tup1) tup2{1, 0.F, false};
	tup2 = tup1;
	REQUIRE(tup0.get<0>() == tup2.get<0>());
	REQUIRE(tup0.get<1>() == tup2.get<1>());
	REQUIRE(tup0.get<2>() == tup2.get<2>());

	const ptl::tuple<> empty0;
	auto empty1{empty0};

	decltype(empty1) empty2;
	empty2 = empty0;
}

TEST_CASE("tuple swapping", "[tuple]") {
	const ptl::tuple<int, float, bool> ref0{5, 1.F, true}, ref1{1, 0.F, false};
	auto tup0{ref0}, tup1{ref1};
	REQUIRE((tup0 == ref0));
	REQUIRE((tup1 == ref1));
	swap(tup0, tup1);
	REQUIRE((tup0 == ref1));
	REQUIRE((tup1 == ref0));
}

TEST_CASE("tuple comparison", "[tuple]") {
	ptl::tuple<int, float, bool> tup0{5, 1.F, true}, tup1{1, 0.F, false};
	REQUIRE((tup1 < tup0));
	tup0.get<0>() = 1;
	REQUIRE((tup1 < tup0));
	tup0.get<1>() = 0.F;
	REQUIRE((tup1 < tup0));
	tup0.get<2>() = false;
	REQUIRE(!(tup1 < tup0));
	REQUIRE((tup1 == tup0));

	const ptl::tuple<> empty0, empty1;
	REQUIRE(!(empty0 < empty1));
	REQUIRE((empty0 == empty1));
}

TEST_CASE("tuple structured binding", "[tuple]") {
	ptl::tuple<int, float, bool> tup{5, 1.F, true};
	auto [a, b, c] = tup;
	REQUIRE(a == tup.get<0>());
	REQUIRE(b == tup.get<1>());
	REQUIRE(c == tup.get<2>());
}

TEST_CASE("tuple ctad", "[tuple]") {
	ptl::tuple t0;
	static_assert(std::is_same_v<decltype(t0), ptl::tuple<>>);
	ptl::tuple t1{0};
	static_assert(std::is_same_v<decltype(t1), ptl::tuple<int>>);
	ptl::tuple t2{0, 0.};
	static_assert(std::is_same_v<decltype(t2), ptl::tuple<int, double>>);
	ptl::tuple t3{0, 0., false};
	static_assert(std::is_same_v<decltype(t3), ptl::tuple<int, double, bool>>);
}
