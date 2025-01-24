
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/optional_ref.hpp>

TEST_CASE("optional_ref ctor", "[optional_ref]") {
	ptl::optional_ref<int> op;
	REQUIRE(!op);
	REQUIRE(!static_cast<bool>(op));

	int val1{0};
	ptl::optional_ref<int> ref1{val1};
	REQUIRE(static_cast<bool>(ref1));
	REQUIRE(*ref1 == val1);

	ptl::optional_ref<const int> ref2{val1};
	REQUIRE(static_cast<bool>(ref2));
	REQUIRE(*ref2 == val1);

	const int val2{10};
	ptl::optional_ref<const int> ref3{val2};
	REQUIRE(static_cast<bool>(ref3));
	REQUIRE(*ref3 == val2);

	std::optional<int> mop{5};
	ptl::optional_ref<int> ref{mop};
	REQUIRE(static_cast<bool>(ref));
	REQUIRE(*ref == *mop);

	ptl::optional_ref<const int> cref0{mop};
	REQUIRE(static_cast<bool>(cref0));
	REQUIRE(*cref0 == *mop);

	const std::optional<int> cop{10};
	ptl::optional_ref<const int> cref1{cop};
	REQUIRE(static_cast<bool>(cref1));
	REQUIRE(*cref1 == *cop);
}

TEST_CASE("optional_ref value", "[optional_ref]") {
	ptl::optional_ref<int> ref;
	REQUIRE(!ref.has_value());
	REQUIRE(ref.value_or(10) == 10);
	REQUIRE_THROWS(ref.value());

	auto value{20};
	ref = value;
	REQUIRE(ref.has_value());
	REQUIRE(ref.value_or(10) == 20);
	REQUIRE_NOTHROW(ref.value());

	ref = std::nullopt;
	REQUIRE(!ref.has_value());
	REQUIRE(ref.value_or(10) == 10);
	REQUIRE_THROWS(ref.value());
}

TEST_CASE("optional_ref ctad", "[optional_ref]") {
	int v1{0};
	ptl::optional_ref op1{v1};
	static_assert(std::is_same_v<decltype(op1), ptl::optional_ref<int>>);

	const int v2{0};
	ptl::optional_ref op2{v2};
	static_assert(std::is_same_v<decltype(op2), ptl::optional_ref<const int>>);

	int * p1{nullptr};
	ptl::optional_ref op3{p1};
	static_assert(std::is_same_v<decltype(op3), ptl::optional_ref<int>>);

	const int * p2{nullptr};
	ptl::optional_ref op4{p2};
	static_assert(std::is_same_v<decltype(op4), ptl::optional_ref<const int>>);

	std::optional<int> mop{5};
	ptl::optional_ref mref{mop};
	static_assert(std::is_same_v<decltype(mref), ptl::optional_ref<      int>>);
	
	const std::optional<int> cop{5};
	ptl::optional_ref cref{cop};
	static_assert(std::is_same_v<decltype(cref), ptl::optional_ref<const int>>);
}
