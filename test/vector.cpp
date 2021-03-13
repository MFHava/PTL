
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include "ptl/vector.hpp"
#include "utils.hpp"

TEST_CASE("vector ctor", "[vector]") {
	using ptl::test::input_iterator;

	const ptl::vector<int> v0;
	REQUIRE(v0.empty());

	const ptl::vector v1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	REQUIRE(v1.size() == 10);
	for(auto i{0}; i < 10; ++i) REQUIRE(i == v1[i]);

	//TODO: not working in GCC
	//TODO: const ptl::vector v2(input_iterator{v1.begin()}, input_iterator{v1.end()});
	//TODO: REQUIRE(v2 == v1);

	const ptl::vector v3(v1.begin(), v1.end());
	REQUIRE(v3 == v1);
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
	v0.swap(v1);
	for(auto i{0}; i < 5; ++i) {
		REQUIRE(v0[i] == i + 5);
		REQUIRE(v1[i] == i);
	}
}

TEST_CASE("vector shrinking", "[vector]") {
	ptl::vector<std::size_t> v0;
	v0.reserve(100);

	const auto capacity{v0.capacity()};
	for(std::size_t i{0}; i < v0.capacity(); ++i) {
		REQUIRE(v0.push_back(i) == i);
		REQUIRE(v0.capacity() == capacity);
	}

	v0.shrink_to_fit();
	REQUIRE(v0.capacity() == capacity);

	REQUIRE(v0.push_back(100) == 100);
	REQUIRE(v0.capacity() > capacity);

	const auto capacity2{v0.capacity()};
	while(v0.size() * 2 > v0.capacity()) { //TODO: this is quite bad as it relies heavily on implementation details instead of the actual interface
		v0.pop_back();
		v0.shrink_to_fit();
		REQUIRE(v0.capacity() == capacity2);
	}

	REQUIRE(v0.capacity() == capacity2);
	v0.pop_back();
	v0.shrink_to_fit();
	REQUIRE(v0.capacity() == v0.size());
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
	const auto it0{v.erase(std::begin(v), std::begin(v) + 3)};
	REQUIRE(it0 == std::begin(v));
	REQUIRE(v == ptl::vector{10, 11, 12, 13, 14, 0, 0, 0, 99, 20, 21, 22, 23, 24, 0, 0, 0});
	const auto it1{v.erase(std::begin(v) + 5, std::begin(v) + 8)};
	REQUIRE(*it1 == 99);
	REQUIRE(v == ptl::vector{10, 11, 12, 13, 14, 99, 20, 21, 22, 23, 24, 0, 0, 0});
	const auto it2{v.erase(std::end(v) - 3, std::end(v))};
	REQUIRE(it2 == std::end(v));
	REQUIRE(v == ptl::vector{10, 11, 12, 13, 14, 99, 20, 21, 22, 23, 24});
}

TEST_CASE("vector insert", "[vector]") {
	using ptl::test::input_iterator;
	const std::vector x{99, 98, 97};

	ptl::vector v{10, 11, 12, 13, 14, 20, 21, 22, 23, 24};

	const auto it0{v.insert(std::begin(v), input_iterator{std::begin(x)}, input_iterator{std::end(x)})};
	REQUIRE(it0 == std::begin(v));
	REQUIRE(v == ptl::vector{99, 98, 97, 10, 11, 12, 13, 14, 20, 21, 22, 23, 24});
	const auto it1{v.insert(std::begin(v) + 8, input_iterator{std::begin(x)}, input_iterator{std::end(x)})};
	REQUIRE(it1 == std::begin(v) + 8);
	REQUIRE(v == ptl::vector{99, 98, 97, 10, 11, 12, 13, 14, 99, 98, 97, 20, 21, 22, 23, 24});
	const auto it2{v.insert(std::end(v), input_iterator{std::begin(x)}, input_iterator{std::end(x)})};
	REQUIRE(it2 == std::end(v) - 3);
	REQUIRE(v == ptl::vector{99, 98, 97, 10, 11, 12, 13, 14, 99, 98, 97, 20, 21, 22, 23, 24, 99, 98, 97});
}

TEST_CASE("vector assign", "[vector]") {
	using ptl::test::input_iterator;
	const std::vector x{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	ptl::vector<int> v;
	v.assign(input_iterator{std::begin(x)}, input_iterator{std::end(x)});
	REQUIRE(v == ptl::vector{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
}
