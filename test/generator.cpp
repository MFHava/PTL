
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <catch.hpp>
#include <ptl/generator.hpp>

//TODO: add unit tests for generator

static_assert(!std::is_copy_constructible_v<decltype(std::declval<ptl::generator<int>>().begin())>);


auto flipflop() -> ptl::generator<int> {
	for(int i = 0; i < 8; ++i) {
		co_yield i % 2;
	}
	std::printf("\n");
}

auto iota() -> ptl::generator<int> {
	//co_yield ptl::ranges::elements_of(flipflop());

	for(int i = 0; i < 10; ++i) {
		co_yield i;
	}
	std::printf("\n");
}

auto fibonacci() -> ptl::generator<int> {
	//co_yield ptl::ranges::elements_of{iota()};

	auto a = 0, b = 1;
	for (;;) {
		co_yield std::exchange(a, std::exchange(b, a + b));
	}
	std::printf("\n");
}

TEST_CASE("generator fib", "[generator]") {
	auto fib{fibonacci()};
	for(auto i : fibonacci()) {
		if(i > 1000) break;
		std::printf("%d ", i);
	}

	//auto it{fib.begin()};
	//*it;


#if 0
	auto it{fib.begin()};
	REQUIRE(*it == 0);
	REQUIRE(*it == 1);
	REQUIRE(*it == 1);
	REQUIRE(*it == 2);
	REQUIRE(*it == 3);
	REQUIRE(*it == 5);
	REQUIRE(*it == 8);
	REQUIRE(*it == 13);
	REQUIRE(*it == 21);
	REQUIRE(*it == 34);
#endif
}
