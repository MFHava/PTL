
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <atomic>
#include <vector>
#include <numeric>
#include <boost/test/unit_test.hpp>
#include "ptl/algorithms.hpp"

BOOST_AUTO_TEST_SUITE(algorithms)

BOOST_AUTO_TEST_CASE(transform_if_1) {
	const std::vector expected{0, 4, 16, 36, 64};

	const auto pred{[](int val) { return val % 2 == 0; }};
	const auto op{[](int val) { return val * val; }};

	std::vector<int> input(10);
	std::iota(std::begin(input), std::end(input), 0);

	std::vector<int> result1;
	ptl::transform_if(input, std::back_inserter(result1), pred, op);
	BOOST_TEST(result1 == expected);

	std::vector<int> result2(expected.size());
	ptl::transform_if(std::execution::seq, input, std::begin(result2), pred, op);
	BOOST_TEST(result2 == expected);

	std::vector<int> result3(expected.size());
	ptl::transform_if(std::execution::par, input, std::begin(result3), pred, op);
	BOOST_TEST(result3 == expected);
}

BOOST_AUTO_TEST_CASE(transform_if_2) {
	const std::vector expected{18, 48, 70, 84, 90};

	const auto pred{[](int lhs, int rhs) { return lhs % 2 && rhs % 2 == 0; }};
	const auto op{[](int lhs, int rhs) { return lhs * rhs; }};

	std::vector<int> input1(10), input2(20);
	std::iota(std::begin(input1), std::end(input1), 0);
	std::iota(std::rbegin(input2), std::rend(input2), 0);

	std::vector<int> result1;
	ptl::transform_if(std::begin(input1), std::end(input1), std::begin(input2), std::back_inserter(result1), pred, op);
	BOOST_TEST(result1 == expected);

	std::vector<int> result2(expected.size());
	ptl::transform_if(std::execution::seq, std::begin(input1), std::end(input1), std::begin(input2), std::begin(result2), pred, op);
	BOOST_TEST(result2 == expected);

	std::vector<int> result3(expected.size());
	ptl::transform_if(std::execution::par, std::begin(input1), std::end(input1), std::begin(input2), std::begin(result3), pred, op);
	BOOST_TEST(result3 == expected);
}

BOOST_AUTO_TEST_CASE(for_n_1) {
	auto sum1{0};
	ptl::for_n(1'000, [&](auto val) { sum1 += val; });
	BOOST_TEST(sum1 == 499'500);

	std::atomic sum2{0};
	ptl::for_n(std::execution::seq, 1'000, [&](auto val) { sum2 += val; });
	BOOST_TEST(sum2 == 499'500);

	std::atomic sum3{0};
	ptl::for_n(std::execution::par, 1'000, [&](auto val) { sum3 += val; });
	BOOST_TEST(sum3 == 499'500);
}

BOOST_AUTO_TEST_CASE(for_n_2) {
	auto sum1{0};
	ptl::for_n(500, 1'000, [&](auto val) { sum1 += val; });
	BOOST_TEST(sum1 == 374'750);

	std::atomic sum2{0};
	ptl::for_n(std::execution::seq, 500, 1'000, [&](auto val) { sum2 += val; });
	BOOST_TEST(sum2 == 374'750);

	std::atomic sum3{0};
	ptl::for_n(std::execution::par, 500, 1'000, [&](auto val) { sum3 += val; });
	BOOST_TEST(sum3 == 374'750);
}

BOOST_AUTO_TEST_CASE(for_n_3) {
	auto sum1{0};
	ptl::for_n(0, 1'000, 7, [&](auto val) { sum1 += val; });
	BOOST_TEST(sum1 == 71'071);

	std::atomic sum2{0};
	ptl::for_n(std::execution::seq, 0, 1'000, 7, [&](auto val) { sum2 += val; });
	BOOST_TEST(sum2 == 71'071);

	std::atomic sum3{0};
	ptl::for_n(std::execution::par, 0, 1'000, 7, [&](auto val) { sum3 += val; });
	BOOST_TEST(sum3 == 71'071);
}

BOOST_AUTO_TEST_SUITE_END()
