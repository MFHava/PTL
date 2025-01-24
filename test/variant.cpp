
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/variant.hpp>
#include "utils.hpp"

TEST_CASE("variant ctor", "[variant]") {
	ptl::variant<int, double> var1;
	REQUIRE(var1.get<int>() == 0);

	var1 = 10.;
	REQUIRE(var1.holds<double>());
	REQUIRE(var1.get<double>() == 10.);

	var1 = 10;
	REQUIRE(var1.holds<int>());
	REQUIRE(var1.get<int>() == 10);

	struct X {
		X() {}
		X(int, int) {}
	};
	struct Y {
		Y() {}
		Y(int, int, int) {}
	};

	ptl::variant<int, double, X, Y> var2{std::in_place_type<X>, 4, 5};
	REQUIRE(var2.holds<X>());

	var2.emplace<Y>(1, 2, 3);
	REQUIRE(var2.holds<Y>());
}

TEST_CASE("variant copy", "[variant]") {
	ptl::variant<double, int> var{1000};
	REQUIRE(!var.holds<double>());

	auto copy1 = var;
	REQUIRE(!copy1.holds<double>());
	REQUIRE(var.get<int>() == copy1.get<int>());

	decltype(var) copy2; copy2 = copy1;
	REQUIRE(!copy2.holds<double>());
	REQUIRE(var.get<int>() == copy2.get<int>());
}

TEST_CASE("variant move", "[variant]") {
	using ptl::test::moveable;
	ptl::variant<moveable> var1;
	decltype(var1) var2{std::move(var1)};
	REQUIRE( var1.get<moveable>().moved);
	REQUIRE(!var2.get<moveable>().moved);
	var1 = std::move(var2);
	REQUIRE(!var1.get<moveable>().moved);
	REQUIRE( var2.get<moveable>().moved);
}

TEST_CASE("variant visit", "[variant]") {
	ptl::variant<int, double> var;
	var.visit(
		[](int &) { REQUIRE(true); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::as_const(var).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(true); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::move(var).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(true); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::move(std::as_const(var)).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(true); },
		[](const double &&) { REQUIRE(false); }
	);

	var = 1.5;
	var.visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(true); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::as_const(var).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(true); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::move(var).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(true); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::move(std::as_const(var)).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(true); }
	);
	REQUIRE(var.visit([](const auto & value) -> double { return value; }) == 1.5);

	var = 1;
	var.visit(
		[](int &) { REQUIRE(true); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::as_const(var).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(true); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::move(var).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(true); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(false); },
		[](const double &&) { REQUIRE(false); }
	);
	std::move(std::as_const(var)).visit(
		[](int &) { REQUIRE(false); },
		[](double &) { REQUIRE(false); },
		[](const int &) { REQUIRE(false); },
		[](const double &) { REQUIRE(false); },
		[](int &&) { REQUIRE(false); },
		[](double &&) { REQUIRE(false); },
		[](const int &&) { REQUIRE(true); },
		[](const double &&) { REQUIRE(false); }
	);
	REQUIRE(var.visit([](const auto & value) -> double { return value; }) == 1.0);
}

TEST_CASE("variant swapping", "[variant]") {
	ptl::variant<int, double> var1{10}, var2{20.2};
	REQUIRE(var1.holds<int>());
	REQUIRE(var2.holds<double>());

	swap(var1, var2);
	REQUIRE(var1.holds<double>());
	REQUIRE(var1.get<double>() == 20.2);
	REQUIRE(var2.holds<int>());
	REQUIRE(var2.get<int>() == 10);

	decltype(var1) var3{20};
	swap(var2, var3);
	REQUIRE(var2.holds<int>());
	REQUIRE(var2.get<int>() == 20);
	REQUIRE(var3.holds<int>());
	REQUIRE(var3.get<int>() == 10);
}

TEST_CASE("variant comparison", "[variant]") {
	const ptl::variant<int, double> var1{10}, var2{10.};

	REQUIRE(!(var1 == var2));
	REQUIRE(var1 != var2);

	const auto var3{var1};

	REQUIRE(var1 == var3);
	REQUIRE(!(var1 != var3));

	REQUIRE(var1 < var2);
	REQUIRE(!(var1 > var2));
	REQUIRE(var2 > var1);
	REQUIRE(!(var2 < var1));

	decltype(var1) var4{1};
	REQUIRE(var1 > var4);
	REQUIRE(var4 < var1);
}
