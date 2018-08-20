
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "moveable.hpp"
#include "ptl/optional.hpp"

static_assert(std::is_same_v<decltype(ptl::get(std::declval<      ptl::optional<int> &&>())),       int &&>);
static_assert(std::is_same_v<decltype(ptl::get(std::declval<const ptl::optional<int> &&>())), const int &&>);

BOOST_AUTO_TEST_SUITE(optional)

BOOST_AUTO_TEST_CASE(ctor) {
	ptl::optional<int> op1;
	BOOST_TEST(!op1);
	BOOST_TEST(!static_cast<bool>(op1));

	op1 = 1;
	BOOST_TEST(!!op1);
	BOOST_TEST(static_cast<bool>(op1));
	BOOST_TEST(*op1 == 1);

	ptl::optional<int> op2{ptl::in_place, 5};
	BOOST_TEST(!!op2);
	BOOST_TEST(static_cast<bool>(op2));
	BOOST_TEST(*op2 == 5);

	ptl::optional<int> op3;
	BOOST_TEST(!op3);
	BOOST_TEST(!static_cast<bool>(op3));

	op3.emplace(10);
	BOOST_TEST(!!op3);
	BOOST_TEST(static_cast<bool>(op3));
	BOOST_TEST(*op3 == 10);

	auto op4{ptl::make_optional<int>(20)};
	BOOST_TEST(!!op4);
	BOOST_TEST(static_cast<bool>(op4));
	BOOST_TEST(*op4 == 20);
}

BOOST_AUTO_TEST_CASE(copy) {
	ptl::optional<float> op1;
	auto op2{op1};
	BOOST_TEST(op1 == op2);
	
	ptl::optional<float> op3{5};
	op2 = op3;
	BOOST_TEST(op3 == op2);
}

namespace {
	struct moveable final {
		bool moved{false};

		moveable() {}
		moveable(const moveable &) { throw std::runtime_error{"copy called"}; }
		moveable(moveable && other) noexcept : moved{other.moved} { other.moved = true; }

		auto operator=(const moveable &) -> moveable & { throw std::runtime_error{"copy called"}; }
		auto operator=(moveable && other) noexcept -> moveable & {
			moved = other.moved;
			other.moved = true;
			return *this;
		}

		~moveable() noexcept =default;
	};
}

BOOST_AUTO_TEST_CASE(move) {
	using ptl::test::moveable;
	ptl::optional<moveable> var1{moveable{}};
	decltype(var1) var2{std::move(var1)};
	BOOST_TEST( ptl::get(var1).moved);
	BOOST_TEST(!ptl::get(var2).moved);
	var1 = std::move(var2);
	BOOST_TEST(!ptl::get(var1).moved);
	BOOST_TEST( ptl::get(var2).moved);
}

BOOST_AUTO_TEST_CASE(swapping) {
	ptl::optional<int> op1{5}, op2{10};
	swap(op1, op2);
	BOOST_TEST(*op1 == 10);
	BOOST_TEST(*op2 ==  5);

	ptl::optional<int> op3;
	swap(op1, op3);
	BOOST_TEST(!op1);
	BOOST_TEST(static_cast<bool>(op3));
	BOOST_TEST(*op3 == 10);
}

BOOST_AUTO_TEST_CASE(comparison) {
	ptl::optional<int> op1, op2{5}, op3{10}, op4, op5{10};
	BOOST_TEST(op1 != op2);
	BOOST_TEST(op1 != op3);
	BOOST_TEST(op1 == op4);
	BOOST_TEST(op1 != op5);
	BOOST_TEST(op2 <  op3);
	BOOST_TEST(op2 <  op5);
	BOOST_TEST(op3 == op5);
}

BOOST_AUTO_TEST_SUITE_END()
