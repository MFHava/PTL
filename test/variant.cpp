
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define WIN32_LEAN_AND_MEAN//suppress #define interface
#include <boost/test/unit_test.hpp>
#include "ptl/variant.hpp"

BOOST_AUTO_TEST_SUITE(variant)

BOOST_AUTO_TEST_CASE(ctor) {
	ptl::variant<int, double> var;
	BOOST_TEST(!var.valueless_by_exception());
	BOOST_TEST(ptl::get<int>(var) == 0);

	var = 10.;
	BOOST_TEST(ptl::holds_alternative<double>(var));
	BOOST_TEST(ptl::get<double>(var) == 10.);

	var = 10;
	BOOST_TEST(ptl::holds_alternative<int>(var));
	BOOST_TEST(ptl::get<int>(var) == 10);
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
	ptl::variant<moveable> var1;
	decltype(var1) var2{std::move(var1)};
	BOOST_TEST( ptl::get<moveable>(var1).moved);
	BOOST_TEST(!ptl::get<moveable>(var2).moved);
	var1 = std::move(var2);
	BOOST_TEST(!ptl::get<moveable>(var1).moved);
	BOOST_TEST( ptl::get<moveable>(var2).moved);
}

namespace {
	template<typename ExpectedType>
	struct expected_type_visitor final {
		void operator()(const ExpectedType &) const { BOOST_TEST(true); }

		template<typename Type>
		void operator()(const Type &) const { BOOST_TEST(false); }
	};
}
BOOST_AUTO_TEST_CASE(visit) {
	ptl::variant<int, double> var;
	ptl::visit(expected_type_visitor<int>{}, var);

	var = 1.5;
	var.visit(expected_type_visitor<double>{});
	BOOST_TEST(ptl::visit([](const auto & value) -> double { return value; }, var) == 1.5);

	var = 1;
	var.visit(expected_type_visitor<int>{});
	BOOST_TEST(ptl::visit([](const auto & value) -> double { return value; }, var) == 1.0);
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
