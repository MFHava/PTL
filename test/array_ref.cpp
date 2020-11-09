
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <vector>
#include <boost/test/unit_test.hpp>
#include "ptl/array_ref.hpp"

BOOST_AUTO_TEST_SUITE(array_ref)

BOOST_AUTO_TEST_CASE(ctor) {
	int a0[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ptl::array_ref<      int> r0{a0};
	ptl::array_ref<const int> r1{a0};
	BOOST_TEST(std::size(a0) == r0.size());
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

BOOST_AUTO_TEST_CASE(ctad) {
	int arr[]{0};
	const int carr[]{0};

	ptl::array_ref ref0{arr};
	static_assert(std::is_same_v<decltype(ref0), ptl::array_ref<      int>>);
	ptl::array_ref ref1{carr};
	static_assert(std::is_same_v<decltype(ref1), ptl::array_ref<const int>>);

	std::vector<int> vec{0};
	const std::vector<int> cvec{0};
	
	ptl::array_ref ref2{vec};
	static_assert(std::is_same_v<decltype(ref2), ptl::array_ref<      int>>);
	ptl::array_ref ref3{cvec};
	static_assert(std::is_same_v<decltype(ref3), ptl::array_ref<const int>>);
}

BOOST_AUTO_TEST_SUITE_END()
