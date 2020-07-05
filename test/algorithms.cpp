
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <numeric>
#include <boost/test/unit_test.hpp>
#include "ptl/algorithms.hpp"

BOOST_AUTO_TEST_SUITE(algorithms)

BOOST_AUTO_TEST_CASE(transform_if_1) {
	std::vector<int> input(10);
	std::iota(std::begin(input), std::end(input), 0);

	std::vector<int> result;
	ptl::transform_if(input, std::back_inserter(result), [](int val) { return val % 2 == 0; }, [](int val) { return val * val; });

	const std::vector expected{0, 4, 16, 36, 64};
	BOOST_TEST(result == expected);
}

BOOST_AUTO_TEST_CASE(transform_if_2) {
	std::vector<int> input1(10), input2(20);
	std::iota(std::begin(input1), std::end(input1), 0);
	std::iota(std::rbegin(input2), std::rend(input2), 0);

	std::vector<int> result;
	ptl::transform_if(std::begin(input1), std::end(input1), std::begin(input2), std::back_inserter(result), [](int lhs, int rhs) { return lhs % 2 && rhs % 2 == 0; }, [](int lhs, int rhs) { return lhs * rhs; });

	const std::vector expected{18, 48, 70, 84, 90};
	BOOST_TEST(result == expected);
}

BOOST_AUTO_TEST_SUITE_END()
