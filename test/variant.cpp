
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/variant.hpp"
#include "moveable.hpp"

BOOST_AUTO_TEST_SUITE(variant)

BOOST_AUTO_TEST_CASE(ctor) {
	ptl::variant<int, double> var1;
	BOOST_TEST(!var1.valueless_by_exception());
	BOOST_TEST(ptl::get<int>(var1) == 0);

	var1 = 10.;
	BOOST_TEST(ptl::holds_alternative<double>(var1));
	BOOST_TEST(ptl::get<double>(var1) == 10.);

	var1 = 10;
	BOOST_TEST(ptl::holds_alternative<int>(var1));
	BOOST_TEST(ptl::get<int>(var1) == 10);

	struct X {
		X() {}
		X(int, int) {}
	};
	struct Y {
		Y() {}
		Y(int, int, int) {}
	};

	ptl::variant<int, double, X, Y> var2{std::in_place_type<X>, 4, 5};
	BOOST_TEST(ptl::holds_alternative<X>(var2));

	var2.emplace<Y>(1, 2, 3);
	BOOST_TEST(ptl::holds_alternative<Y>(var2));
}

BOOST_AUTO_TEST_CASE(copy) {
	ptl::variant<double, int> var{1000};
	BOOST_TEST(!var.valueless_by_exception());
	BOOST_TEST(!ptl::holds_alternative<double>(var));

	auto copy1 = var;
	BOOST_TEST(!copy1.valueless_by_exception());
	BOOST_TEST(!ptl::holds_alternative<double>(copy1));
	BOOST_TEST(ptl::get<int>(var) == ptl::get<int>(copy1));

	decltype(var) copy2; copy2 = copy1;
	BOOST_TEST(!copy2.valueless_by_exception());
	BOOST_TEST(!ptl::holds_alternative<double>(copy2));
	BOOST_TEST(ptl::get<int>(var) == ptl::get<int>(copy2));
}

BOOST_AUTO_TEST_CASE(move) {
	using ptl::test::moveable;
	ptl::variant<moveable> var1;
	decltype(var1) var2{std::move(var1)};
	BOOST_TEST( ptl::get<moveable>(var1).moved);
	BOOST_TEST(!ptl::get<moveable>(var2).moved);
	var1 = std::move(var2);
	BOOST_TEST(!ptl::get<moveable>(var1).moved);
	BOOST_TEST( ptl::get<moveable>(var2).moved);
}

BOOST_AUTO_TEST_CASE(visit) {
	ptl::variant<int, double> var;
	var.visit(
		[](int) { BOOST_TEST(true); },
		[](double) { BOOST_TEST(false); }
	);

	var = 1.5;
	var.visit(
		[](int) { BOOST_TEST(false); },
		[](double) { BOOST_TEST(true); }
	);
	BOOST_TEST(var.visit([](const auto & value) -> double { return value; }) == 1.5);

	var = 1;
	var.visit(
		[](int) { BOOST_TEST(true); },
		[](double) { BOOST_TEST(false); }
	);
	BOOST_TEST(var.visit([](const auto & value) -> double { return value; }) == 1.0);
}

BOOST_AUTO_TEST_CASE(swapping) {
	ptl::variant<int, double> var1{10}, var2{20.2};
	BOOST_TEST(ptl::holds_alternative<int>(var1));
	BOOST_TEST(ptl::holds_alternative<double>(var2));

	swap(var1, var2);
	BOOST_TEST(ptl::holds_alternative<double>(var1));
	BOOST_TEST(ptl::get<double>(var1) == 20.2);
	BOOST_TEST(ptl::holds_alternative<int>(var2));
	BOOST_TEST(ptl::get<int>(var2) == 10);

	decltype(var1) var3{20};
	swap(var2, var3);
	BOOST_TEST(ptl::holds_alternative<int>(var2));
	BOOST_TEST(ptl::get<int>(var2) == 20);
	BOOST_TEST(ptl::holds_alternative<int>(var3));
	BOOST_TEST(ptl::get<int>(var3) == 10);
}

BOOST_AUTO_TEST_CASE(comparison) {
	const ptl::variant<int, double> var1{10}, var2{10.};

	BOOST_TEST(!(var1 == var2));
	BOOST_TEST( var1 != var2);

	const auto var3{var1};

	BOOST_TEST(  var1 == var3);
	BOOST_TEST(!(var1 != var3));

	BOOST_TEST(  var1 < var2);
	BOOST_TEST(!(var1 > var2));
	BOOST_TEST(  var2 > var1);
	BOOST_TEST(!(var2 < var1));

	decltype(var1) var4{1};
	BOOST_TEST(var1 > var4);
	BOOST_TEST(var4 < var1);
}

BOOST_AUTO_TEST_SUITE_END()
