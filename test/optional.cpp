
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>
#include "ptl/optional.hpp"
#include "utils.hpp"

TEST_CASE("optional ctor", "[optional]") {
	ptl::optional<int> op1;
	REQUIRE(!static_cast<bool>(op1));

	op1 = 1;
	REQUIRE(static_cast<bool>(op1));
	REQUIRE(*op1 == 1);

	ptl::optional<int> op2{std::in_place, 5};
	REQUIRE(static_cast<bool>(op2));
	REQUIRE(*op2 == 5);

	ptl::optional<int> op3;
	REQUIRE(!static_cast<bool>(op3));

	op3.emplace(10);
	REQUIRE(static_cast<bool>(op3));
	REQUIRE(*op3 == 10);
}

TEST_CASE("optional copy", "[optional]") {
	ptl::optional<float> op1;
	auto op2{op1};
	REQUIRE(op1 == op2);
	
	ptl::optional<float> op3{5};
	op2 = op3;
	REQUIRE(op3 == op2);
}

TEST_CASE("optional move", "[optional]") {
	using ptl::test::moveable;
	ptl::optional<moveable> var1{moveable{}};
	decltype(var1) var2{std::move(var1)};
	REQUIRE( var1->moved);
	REQUIRE(!var2->moved);
	var1 = std::move(var2);
	REQUIRE(!var1->moved);
	REQUIRE( var2->moved);
}

TEST_CASE("optional swapping", "[optional]") {
	ptl::optional<int> op1{5}, op2{10};
	swap(op1, op2);
	REQUIRE(*op1 == 10);
	REQUIRE(*op2 ==  5);

	ptl::optional<int> op3;
	swap(op1, op3);
	REQUIRE(!op1);
	REQUIRE(static_cast<bool>(op3));
	REQUIRE(*op3 == 10);
}

TEST_CASE("optional comparison", "[optional]") {
	ptl::optional<int> op1, op2{5}, op3{10}, op4, op5{10};
	REQUIRE(op1 != op2);
	REQUIRE(op1 != op3);
	REQUIRE(op1 == op4);
	REQUIRE(op1 != op5);
	REQUIRE(op2 <  op3);
	REQUIRE(op2 <  op5);
	REQUIRE(op3 == op5);
}

TEST_CASE("optional ctad", "[optional]") {
	ptl::optional op{0};
	static_assert(std::is_same_v<decltype(op), ptl::optional<int>>);
}
