
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <array>
#include <vector>
#include <catch2/catch.hpp>
#include "ptl/array_ref.hpp"

TEST_CASE("array_ref construction", "[array_ref]") {
	int a0[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ptl::array_ref<      int> r0{a0};
	ptl::array_ref<const int> r1{a0};
	REQUIRE(std::size(a0) == r0.size());
	REQUIRE(r0[0] == a0[0]);
}

TEST_CASE("array_ref size", "[array_ref]") {
	std::vector<int> v0;
	ptl::array_ref<      int> r00{v0};
	ptl::array_ref<const int> r01{v0};
	REQUIRE(v0.size() == r00.size());
	REQUIRE(v0.size() == r01.size());

	std::vector<int> a0(10);
	ptl::array_ref<      int> r10{a0};
	ptl::array_ref<const int> r11{a0};
	REQUIRE(a0.size() == r10.size());
	REQUIRE(a0.size() == r11.size());
}

TEST_CASE("array_ref subviews", "[array_ref]") {
	const int arr[]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	const ptl::array_ref<const int> ref{arr};

	const auto first{ref.first(3)};
	REQUIRE(first.size() == 3);
	REQUIRE(first[0] == 0);
	REQUIRE(first[1] == 1);
	REQUIRE(first[2] == 2);

	const auto last{ref.last(4)};
	REQUIRE(last.size() == 4);
	REQUIRE(last[0] == 6);
	REQUIRE(last[1] == 7);
	REQUIRE(last[2] == 8);
	REQUIRE(last[3] == 9);

	const auto sub0{ref.subrange(4)};
	REQUIRE(sub0.size() == 6);
	REQUIRE(sub0[0] == 4);
	REQUIRE(sub0[1] == 5);
	REQUIRE(sub0[2] == 6);
	REQUIRE(sub0[3] == 7);
	REQUIRE(sub0[4] == 8);
	REQUIRE(sub0[5] == 9);

	const auto sub1{ref.subrange(5, 3)};
	REQUIRE(sub1.size() == 3);
	REQUIRE(sub1[0] == 5);
	REQUIRE(sub1[1] == 6);
	REQUIRE(sub1[2] == 7);
}

TEST_CASE("array_ref ctad", "[array_ref]") {
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
