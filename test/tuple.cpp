
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/tuple.hpp"

static_assert(sizeof(ptl::tuple<>) == sizeof(char));
static_assert(sizeof(ptl::tuple<char, int>) == sizeof(char) + sizeof(int));

static_assert(std::is_same_v<decltype(ptl::get<0>(std::declval<      ptl::tuple<int, float> &>())),       int &>);
static_assert(std::is_same_v<decltype(ptl::get<0>(std::declval<const ptl::tuple<int, float> &>())), const int &>);
static_assert(std::is_same_v<decltype(ptl::get<0>(std::declval<      ptl::tuple<int, float> &&>())),       int &&>);
static_assert(std::is_same_v<decltype(ptl::get<0>(std::declval<const ptl::tuple<int, float> &&>())), const int &&>);
static_assert(std::is_same_v<decltype(ptl::get<1>(std::declval<      ptl::tuple<int, float> &>())),       float &>);
static_assert(std::is_same_v<decltype(ptl::get<1>(std::declval<const ptl::tuple<int, float> &>())), const float &>);
static_assert(std::is_same_v<decltype(ptl::get<1>(std::declval<      ptl::tuple<int, float> &&>())),       float &&>);
static_assert(std::is_same_v<decltype(ptl::get<1>(std::declval<const ptl::tuple<int, float> &&>())), const float &&>);

BOOST_AUTO_TEST_SUITE(tuple)

BOOST_AUTO_TEST_CASE(ctor) {
	ptl::tuple<int, float, bool> tup0{5, 1.F, true};
	auto tup1{tup0};
	BOOST_TEST(ptl::get<0>(tup0) == ptl::get<0>(tup1));
	BOOST_TEST(ptl::get<1>(tup0) == ptl::get<1>(tup1));
	BOOST_TEST(ptl::get<2>(tup0) == ptl::get<2>(tup1));
	decltype(tup1) tup2{1, 0.F, false};
	tup2 = tup1;
	BOOST_TEST(ptl::get<0>(tup0) == ptl::get<0>(tup2));
	BOOST_TEST(ptl::get<1>(tup0) == ptl::get<1>(tup2));
	BOOST_TEST(ptl::get<2>(tup0) == ptl::get<2>(tup2));

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
	ptl::get<0>(tup0) = 1;
	BOOST_TEST((tup1 < tup0));
	ptl::get<1>(tup0) = 0.F;
	BOOST_TEST((tup1 < tup0));
	ptl::get<2>(tup0) = false;
	BOOST_TEST(!(tup1 < tup0));
	BOOST_TEST((tup1 == tup0));

	const ptl::tuple<> empty0, empty1;
	BOOST_TEST(!(empty0 < empty1));
	BOOST_TEST((empty0 == empty1));
}

BOOST_AUTO_TEST_CASE(structured_binding) {
	ptl::tuple<int, float, bool> tup{5, 1.F, true};
	auto [a, b, c] = tup;
	BOOST_TEST(a == ptl::get<0>(tup));
	BOOST_TEST(b == ptl::get<1>(tup));
	BOOST_TEST(c == ptl::get<2>(tup));
}

BOOST_AUTO_TEST_SUITE_END()
