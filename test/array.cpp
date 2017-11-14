
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define WIN32_LEAN_AND_MEAN//suppress #define interface
#include <boost/test/unit_test.hpp>
#include "ptl/array.hpp"

BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessContainer<ptl::array<int, 10>>));
static_assert(sizeof(ptl::array<int, 10>) == 10 * sizeof(int), "unexpected size for array detected");

BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessContainer<ptl::array<int,  0>>));
static_assert(sizeof(ptl::array<int,  0>) == sizeof(void *), "unexpected size for array detected");

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
	ptl::array<int,  0> a1;
	BOOST_TEST(a1.size() ==  0);
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

//TODO: get, other helpers?!

BOOST_AUTO_TEST_SUITE_END()