
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/array.hpp"

static_assert(sizeof(ptl::array<int, 10>) == 10 * sizeof(int));
static_assert(sizeof(ptl::array<int,  0>) == sizeof(void *));
static_assert(std::is_same_v<decltype(ptl::array{1, 2, 3, 4}), ptl::array<int, 4>>);
static_assert(std::is_same_v<decltype(ptl::array{1., 2., 3.}), ptl::array<double, 3>>);

BOOST_AUTO_TEST_SUITE(array)

BOOST_AUTO_TEST_CASE(ctor) {
	const ptl::array<int, 10> a0;
	for(const auto & tmp : a0) BOOST_TEST(tmp == 0);
	const ptl::array<int, 10> a1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	for(std::size_t i{0}; i < a1.size(); ++i) BOOST_TEST(a1[i] == i);
	const ptl::array<int, 10> a2{9, 8, 7, 6, 5, 4, 3, 2, 1};
	for(std::size_t i{0}; i < a2.size(); ++i) BOOST_TEST(a2[i] == (a2.size() - 1) - i);
}

BOOST_AUTO_TEST_CASE(size) {
	ptl::array<int, 10> a0;
	BOOST_TEST(a0.size() == 10);
	BOOST_TEST(a0.max_size() == 10);
	ptl::array<int,  0> a1;
	BOOST_TEST(a1.size() ==  0);
	BOOST_TEST(a1.max_size() == 0);
}

BOOST_AUTO_TEST_CASE(comparison) {
	const ptl::array<int, 3> a0{0, 1, 2}, a1{a0};
	BOOST_TEST(a0 == a1);
	BOOST_TEST(!(a0 < a1));
	const ptl::array<int, 3> a2{0, 1, 3};
	BOOST_TEST(a0 < a2);
	BOOST_TEST(!(a0 == a2));
}

BOOST_AUTO_TEST_CASE(fill) {
	ptl::array<int, 10> arr;
	arr.fill(10);
	for(const auto & tmp : arr) BOOST_TEST(tmp == 10);
}

BOOST_AUTO_TEST_CASE(structured_binding) {
	ptl::array<int, 3> arr{0, 1, 2};
	auto [a, b, c] = arr;
	BOOST_TEST(a == arr[0]);
	BOOST_TEST(b == arr[1]);
	BOOST_TEST(c == arr[2]);
}

BOOST_AUTO_TEST_CASE(ctad) {
	ptl::array arr0{0};
	static_assert(std::is_same_v<decltype(arr0), ptl::array<int, 1>>);
	ptl::array arr1{0, 1};
	static_assert(std::is_same_v<decltype(arr1), ptl::array<int, 2>>);
}

BOOST_AUTO_TEST_SUITE_END()
