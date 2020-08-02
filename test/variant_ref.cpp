
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/variant_ref.hpp"

BOOST_AUTO_TEST_SUITE(variant_ref)

BOOST_AUTO_TEST_CASE(ctor) {
	const double val1{10.0};
	ptl::variant_ref<int, const int, const double> var0{val1};
	static_assert(sizeof(var0) == sizeof(void *) * 2);
	BOOST_TEST(var0.holds<const double>());
	BOOST_TEST(var0.get<const double>() == val1);

	const int val2{10};
	var0 = val2;
	BOOST_TEST(var0.holds<const int>());
	BOOST_TEST(var0.get<const int>() == val2);

	int val3{1};
	var0 = val3;
	BOOST_TEST(var0.holds<int>());
	BOOST_TEST(var0.get<int>() == val3);

	const ptl::variant_ref<int, double> var1{val3};
	static_assert(sizeof(var1) == sizeof(void *) * 2);
	BOOST_TEST(var1.holds<int>());
	BOOST_TEST(var1.get<int>() == val3);
	var1.get<int>() = 255;
	BOOST_TEST(var1.get<int>() == 255);
}

BOOST_AUTO_TEST_CASE(visit) {
	ptl::variant_ref<const int, const double> var{10};
	static_assert(sizeof(var) == sizeof(void *) * 2);
	var.visit(
		[](int) { BOOST_TEST(true); },
		[](double) { BOOST_TEST(false); }
	);

	var = 1.5;
	var.visit(
		[](int) { BOOST_TEST(false); },
		[](double) { BOOST_TEST(true); }
	);

	var = 1;
	var.visit(
		[](int) { BOOST_TEST(true); },
		[](double) { BOOST_TEST(false); }
	);
}

BOOST_AUTO_TEST_CASE(swapping) {
	ptl::variant_ref<const int, const double> var1{10}, var2{20.2};
	static_assert(sizeof(var1) == sizeof(void *) * 2);
	BOOST_TEST(var1.holds<const int>());
	BOOST_TEST(var2.holds<const double>());

	swap(var1, var2);
	BOOST_TEST(var1.holds<const double>());
	BOOST_TEST(var2.holds<const int>());

	decltype(var1) var3{20};
	swap(var2, var3);
	BOOST_TEST(var2.holds<const int>());
	BOOST_TEST(var3.holds<const int>());
}

BOOST_AUTO_TEST_SUITE_END()
