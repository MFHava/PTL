
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/vector.hpp>
#include "utils.hpp"

//TODO: redesign unit tests for new implementations

TEST_CASE("vector ctor", "[vector]") {
	using ptl::test::input_iterator;

	const ptl::vector<int> v0;
	REQUIRE(v0.empty());

	const ptl::vector v1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	REQUIRE(v1.size() == 10);
	for(auto i{0}; i < 10; ++i) REQUIRE(i == v1[i]);

	const ptl::vector v2(input_iterator{v1.cbegin()}, input_iterator{v1.cend()});
	REQUIRE(v2 == v1);

	const ptl::vector v3(v1.cbegin(), v1.cend());
	REQUIRE(v3 == v1);

	const ptl::vector<int> v4(10);
	REQUIRE(v4.size() == 10);
	REQUIRE(v4 == ptl::vector{0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

	const ptl::vector<int> v5(10, 1);
	REQUIRE(v5.size() == 10);
	REQUIRE(v5 == ptl::vector{1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
}

TEST_CASE("vector copy", "[vector]") {
	const ptl::vector v0{9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
	REQUIRE(v0.size() == 10);

	auto v1{v0};
	REQUIRE(v0 == v1);

	decltype(v1) v2; v2 = v1;
	REQUIRE(v2 == v1);
}

TEST_CASE("vector move", "[vector]") {
	ptl::vector v0{9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
	REQUIRE(v0.size() == 10);

	auto v1{std::move(v0)};
	REQUIRE(v0.empty());
	REQUIRE(v1.size() == 10);
	REQUIRE(v1 != v0);

	decltype(v1) v2; v2 = std::move(v1);
	REQUIRE(v1.empty());
	REQUIRE(v2.size() == 10);
	REQUIRE(v2 != v1);
}

TEST_CASE("vector swapping", "[vector]") {
	ptl::vector v0{0, 1, 2, 3, 4}, v1{5, 6, 7, 8, 9};
	swap(v0, v1);
	for(auto i{0}; i < 5; ++i) {
		REQUIRE(v0[i] == i + 5);
		REQUIRE(v1[i] == i);
	}
}

TEST_CASE("vector resize", "[vector]") {
	ptl::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9};

	v.resize(3);
	REQUIRE(v == ptl::vector{1, 2, 3});

	v.resize(5);
	REQUIRE(v == ptl::vector{1, 2, 3, 0, 0});

	v.resize(5);
	REQUIRE(v == ptl::vector{1, 2, 3, 0, 0});

	v.resize(2, 10);
	REQUIRE(v == ptl::vector{1, 2});

	v.resize(5, 10);
	REQUIRE(v == ptl::vector{1, 2, 10, 10, 10});

	v.resize(5, 20);
	REQUIRE(v == ptl::vector{1, 2, 10, 10, 10});
}

TEST_CASE("vector clear", "[vector]") {
	ptl::vector<int> v;
	REQUIRE(v.empty());

	v.clear();
	REQUIRE(v.empty());

	REQUIRE(v.push_back(0) == 0);
	REQUIRE(!v.empty());

	v.clear();
	REQUIRE(v.empty());
}

TEST_CASE("vector erase", "[vector]") {
	ptl::vector v{0, 0, 0, 10, 11, 12, 13, 14, 0, 0, 0, 99, 20, 21, 22, 23, 24, 0, 0, 0};
	const auto it0{v.erase(v.cbegin(), v.cbegin() + 3)};
	REQUIRE(it0 == v.begin());
	REQUIRE(v == ptl::vector{10, 11, 12, 13, 14, 0, 0, 0, 99, 20, 21, 22, 23, 24, 0, 0, 0});
	const auto it1{v.erase(v.cbegin() + 5, v.cbegin() + 8)};
	REQUIRE(*it1 == 99);
	REQUIRE(v == ptl::vector{10, 11, 12, 13, 14, 99, 20, 21, 22, 23, 24, 0, 0, 0});
	const auto it2{v.erase(v.cend() - 3, v.cend())};
	REQUIRE(it2 == v.end());
	REQUIRE(v == ptl::vector{10, 11, 12, 13, 14, 99, 20, 21, 22, 23, 24});
}

TEST_CASE("vector insert", "[vector]") {
	using ptl::test::input_iterator;
	const std::vector x{99, 98, 97};

	ptl::vector v{10, 11, 12, 13, 14, 20, 21, 22, 23, 24};

	const auto it0{v.insert(v.cbegin(), input_iterator{x.cbegin()}, input_iterator{x.cend()})};
	REQUIRE(it0 == v.begin());
	REQUIRE(v == ptl::vector{99, 98, 97, 10, 11, 12, 13, 14, 20, 21, 22, 23, 24});
	const auto it1{v.insert(v.cbegin() + 8, input_iterator{x.cbegin()}, input_iterator{x.cend()})};
	REQUIRE(it1 == v.begin() + 8);
	REQUIRE(v == ptl::vector{99, 98, 97, 10, 11, 12, 13, 14, 99, 98, 97, 20, 21, 22, 23, 24});
	const auto it2{v.insert(v.cend(), input_iterator{x.cbegin()}, input_iterator{x.cend()})};
	REQUIRE(it2 == v.end() - 3);
	REQUIRE(v == ptl::vector{99, 98, 97, 10, 11, 12, 13, 14, 99, 98, 97, 20, 21, 22, 23, 24, 99, 98, 97});

	const auto it3{v.insert(v.cbegin(), 2, 55)};
	REQUIRE(it3 == v.begin());
	REQUIRE(v == ptl::vector{55, 55, 99, 98, 97, 10, 11, 12, 13, 14, 99, 98, 97, 20, 21, 22, 23, 24, 99, 98, 97});
	const auto it4{v.insert(v.cbegin() + 10, 2, 55)};
	REQUIRE(it4 == v.begin() + 10);
	REQUIRE(v == ptl::vector{55, 55, 99, 98, 97, 10, 11, 12, 13, 14, 55, 55, 99, 98, 97, 20, 21, 22, 23, 24, 99, 98, 97});
	const auto it5{v.insert(v.cend(), 2, 55)};
	REQUIRE(it5 == v.end() - 2);
	REQUIRE(v == ptl::vector{55, 55, 99, 98, 97, 10, 11, 12, 13, 14, 55, 55, 99, 98, 97, 20, 21, 22, 23, 24, 99, 98, 97, 55, 55});
}

TEST_CASE("vector assign", "[vector]") {
	using ptl::test::input_iterator;
	const std::vector x{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	ptl::vector<int> v;
	v.assign(input_iterator{x.cbegin()}, input_iterator{x.cend()});
	REQUIRE(v == ptl::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

	v = {1, 2, 3};
	REQUIRE(v == ptl::vector{1, 2, 3});

	v.assign(4, 1);
	REQUIRE(v == ptl::vector{1, 1, 1, 1});
}
