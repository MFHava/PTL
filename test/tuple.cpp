
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/tuple.hpp"

static_assert(sizeof(ptl::tuple<>) == sizeof(char));
static_assert(sizeof(ptl::tuple<char, int>) == sizeof(char) + sizeof(int));

static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> & >().get<0>()),       int & >);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> & >().get<0>()), const int & >);
static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> &&>().get<0>()),       int &&>);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> &&>().get<0>()), const int &&>);
static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> & >().get<1>()),       float & >);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> & >().get<1>()), const float & >);
static_assert(std::is_same_v<decltype(std::declval<      ptl::tuple<int, float> &&>().get<1>()),       float &&>);
static_assert(std::is_same_v<decltype(std::declval<const ptl::tuple<int, float> &&>().get<1>()), const float &&>);

BOOST_AUTO_TEST_SUITE(tuple)

BOOST_AUTO_TEST_CASE(ctor) {
	ptl::tuple<int, float, bool> tup0{5, 1.F, true};
	auto tup1{tup0};
	BOOST_TEST(tup0.get<0>() == tup1.get<0>());
	BOOST_TEST(tup0.get<1>() == tup1.get<1>());
	BOOST_TEST(tup0.get<2>() == tup1.get<2>());
	decltype(tup1) tup2{1, 0.F, false};
	tup2 = tup1;
	BOOST_TEST(tup0.get<0>() == tup2.get<0>());
	BOOST_TEST(tup0.get<1>() == tup2.get<1>());
	BOOST_TEST(tup0.get<2>() == tup2.get<2>());

	const ptl::tuple<> empty0;
	auto empty1{empty0};

	decltype(empty1) empty2;
	empty2 = empty0;
}

BOOST_AUTO_TEST_CASE(swapping) {
	const ptl::tuple<int, float, bool> ref0{5, 1.F, true}, ref1{1, 0.F, false};
	auto tup0{ref0}, tup1{ref1};
	BOOST_TEST((tup0 == ref0));
	BOOST_TEST((tup1 == ref1));
	tup0.swap(tup1);
	BOOST_TEST((tup0 == ref1));
	BOOST_TEST((tup1 == ref0));
}

BOOST_AUTO_TEST_CASE(comparison) {
	ptl::tuple<int, float, bool> tup0{5, 1.F, true}, tup1{1, 0.F, false};
	BOOST_TEST((tup1 < tup0));
	tup0.get<0>() = 1;
	BOOST_TEST((tup1 < tup0));
	tup0.get<1>() = 0.F;
	BOOST_TEST((tup1 < tup0));
	tup0.get<2>() = false;
	BOOST_TEST(!(tup1 < tup0));
	BOOST_TEST((tup1 == tup0));

	const ptl::tuple<> empty0, empty1;
	BOOST_TEST(!(empty0 < empty1));
	BOOST_TEST((empty0 == empty1));
}

BOOST_AUTO_TEST_CASE(structured_binding) {
	ptl::tuple<int, float, bool> tup{5, 1.F, true};
	auto [a, b, c] = tup;
	BOOST_TEST(a == tup.get<0>());
	BOOST_TEST(b == tup.get<1>());
	BOOST_TEST(c == tup.get<2>());
}

BOOST_AUTO_TEST_CASE(ctad) {
	ptl::tuple t0;
	static_assert(std::is_same_v<decltype(t0), ptl::tuple<>>);
	ptl::tuple t1{0};
	static_assert(std::is_same_v<decltype(t1), ptl::tuple<int>>);
	ptl::tuple t2{0, 0.};
	static_assert(std::is_same_v<decltype(t2), ptl::tuple<int, double>>);
	ptl::tuple t3{0, 0., false};
	static_assert(std::is_same_v<decltype(t3), ptl::tuple<int, double, bool>>);
}

BOOST_AUTO_TEST_SUITE_END()
