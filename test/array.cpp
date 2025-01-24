
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/array.hpp>

static_assert(sizeof(ptl::array<int, 10>) == 10 * sizeof(int));
static_assert(sizeof(ptl::array<int,  0>) == sizeof(void *));
static_assert(std::is_same_v<decltype(ptl::array{1, 2, 3, 4}), ptl::array<int, 4>>);
static_assert(std::is_same_v<decltype(ptl::array{1., 2., 3.}), ptl::array<double, 3>>);

TEST_CASE("array ctor", "[array]") {
	const ptl::array<int, 10> a0;
	for(const auto & tmp : a0) REQUIRE(tmp == 0);
	const ptl::array<int, 10> a1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	for(std::size_t i{0}; i < a1.size(); ++i) REQUIRE(static_cast<std::size_t>(a1[i]) == i);
	const ptl::array<int, 10> a2{9, 8, 7, 6, 5, 4, 3, 2, 1};
	for(std::size_t i{0}; i < a2.size(); ++i) REQUIRE(static_cast<std::size_t>(a2[i]) == (a2.size() - 1) - i);
}

TEST_CASE("array size", "[array]") {
	ptl::array<int, 10> a0;
	REQUIRE(a0.size() == 10);
	REQUIRE(a0.max_size() == 10);
	ptl::array<int,  0> a1;
	REQUIRE(a1.size() ==  0);
	REQUIRE(a1.max_size() == 0);
}

TEST_CASE("array comparison", "[array]") {
	const ptl::array<int, 3> a0{0, 1, 2}, a1{a0};
	REQUIRE(a0 == a1);
	REQUIRE(!(a0 < a1));
	const ptl::array<int, 3> a2{0, 1, 3};
	REQUIRE(a0 < a2);
	REQUIRE(!(a0 == a2));
}

TEST_CASE("array fill", "[array]") {
	ptl::array<int, 10> arr;
	arr.fill(10);
	for(const auto & tmp : arr) REQUIRE(tmp == 10);
}

TEST_CASE("array swapping", "[array]") {
	ptl::array<int, 3> a0{0, 1, 2}, a1{3, 4, 5};

	const auto tmp0{a0}, tmp1{a1};
	REQUIRE(a0 == tmp0);
	REQUIRE(a1 == tmp1);
	REQUIRE(a0 != a1);


	swap(a0, a1);
	REQUIRE(a0 == tmp1);
	REQUIRE(a1 == tmp0);
	REQUIRE(a0 != a1);
}

TEST_CASE("array structured_binding", "[array]") {
	ptl::array<int, 3> arr{0, 1, 2};
	auto [a, b, c] = arr;
	REQUIRE(a == arr[0]);
	REQUIRE(b == arr[1]);
	REQUIRE(c == arr[2]);
}

TEST_CASE("array ctad", "[array]") {
	ptl::array arr0{0};
	static_assert(std::is_same_v<decltype(arr0), ptl::array<int, 1>>);
	ptl::array arr1{0, 1};
	static_assert(std::is_same_v<decltype(arr1), ptl::array<int, 2>>);
}
