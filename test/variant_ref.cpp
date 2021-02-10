
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include "ptl/variant_ref.hpp"

TEST_CASE("variant_ref ctor", "[variant_ref]") {
	const double val1{10.0};
	ptl::variant_ref<int, const int, const double> var0{val1};
	static_assert(sizeof(var0) == sizeof(void *) * 2);
	REQUIRE(var0.holds<const double>());
	REQUIRE(var0.get<const double>() == val1);

	const int val2{10};
	var0 = val2;
	REQUIRE(var0.holds<const int>());
	REQUIRE(var0.get<const int>() == val2);

	int val3{1};
	var0 = val3;
	REQUIRE(var0.holds<int>());
	REQUIRE(var0.get<int>() == val3);

	const ptl::variant_ref<int, double> var1{val3};
	static_assert(sizeof(var1) == sizeof(void *) * 2);
	REQUIRE(var1.holds<int>());
	REQUIRE(var1.get<int>() == val3);
	var1.get<int>() = 255;
	REQUIRE(var1.get<int>() == 255);
}

TEST_CASE("variant_ref visit", "[variant_ref]") {
	ptl::variant_ref<const int, const double> var{10};
	static_assert(sizeof(var) == sizeof(void *) * 2);
	var.visit(
		[](int) { REQUIRE(true); },
		[](double) { REQUIRE(false); }
	);

	var = 1.5;
	var.visit(
		[](int) { REQUIRE(false); },
		[](double) { REQUIRE(true); }
	);

	var = 1;
	var.visit(
		[](int) { REQUIRE(true); },
		[](double) { REQUIRE(false); }
	);
}

TEST_CASE("variant_ref swapping", "[variant_ref]") {
	ptl::variant_ref<const int, const double> var1{10}, var2{20.2};
	static_assert(sizeof(var1) == sizeof(void *) * 2);
	REQUIRE(var1.holds<const int>());
	REQUIRE(var2.holds<const double>());

	swap(var1, var2);
	REQUIRE(var1.holds<const double>());
	REQUIRE(var2.holds<const int>());

	decltype(var1) var3{20};
	swap(var2, var3);
	REQUIRE(var2.holds<const int>());
	REQUIRE(var3.holds<const int>());
}
