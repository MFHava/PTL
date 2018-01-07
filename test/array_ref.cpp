
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/array_ref.hpp"

#include <array>
#include <vector>

BOOST_AUTO_TEST_SUITE(array_ref)

BOOST_AUTO_TEST_CASE(ctor) {
	int a0[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ptl::array_ref<      int> r0{a0};
	ptl::array_ref<const int> r1{a0};
	BOOST_TEST(ptl::internal::size(a0) == r0.size());
	BOOST_TEST(r0[0] == a0[0]);
}

BOOST_AUTO_TEST_CASE(size) {
	std::vector<int> v0;
	ptl::array_ref<      int> r00{v0};
	ptl::array_ref<const int> r01{v0};
	BOOST_TEST(v0.size() == r00.size());
	BOOST_TEST(v0.size() == r01.size());

	std::vector<int> a0(10);
	ptl::array_ref<      int> r10{a0};
	ptl::array_ref<const int> r11{a0};
	BOOST_TEST(a0.size() == r10.size());
	BOOST_TEST(a0.size() == r11.size());
}

BOOST_AUTO_TEST_CASE(comparison) {
	const std::array<int, 0> a0{};
	const std::array<int, 3> a1{};
	const ptl::array_ref<const int> r0{a0};
	const ptl::array_ref<const int> r1{a1};
	BOOST_TEST(r0 < r1);
	BOOST_TEST(!(r0 == r1));
	BOOST_TEST(!(r1 < a1));
	BOOST_TEST(r1 == a1);
}

BOOST_AUTO_TEST_SUITE_END()
