
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/string_ref.hpp"

using namespace std::string_literals;
using namespace ptl::literals;

BOOST_AUTO_TEST_SUITE(string_ref)

BOOST_AUTO_TEST_CASE(ctor) {
	const char data[]{'H', 'e', 'l', 'l', 'o', '\0', 'W', 'o', 'r', 'l', 'd', '\0'};
	const std::string s{data, sizeof(data)};
	const ptl::string_ref r0{s};
	BOOST_TEST(r0.size() == sizeof(data));
	BOOST_TEST(r0[5] == '\0');

	std::stringstream ss;
	ss << r0;
	BOOST_TEST(ss.str().size() == sizeof(data));

	ptl::string_ref r1;
	BOOST_TEST(r0 != r1);
	r1 = r0;
	BOOST_TEST(r0 == r1);
}

BOOST_AUTO_TEST_CASE(size) {
	const std::string s0{"TEST"};
	const ptl::string_ref r0{"TEST"};
	BOOST_TEST(s0.size() == r0.size());
	BOOST_TEST(s0.size() == 4);

	const auto s1 = "TEST"s;
	const auto r1 = "TEST"_sr;
	BOOST_TEST(s1.size() == r1.size());
	BOOST_TEST(s1.size() == 4);
}

BOOST_AUTO_TEST_CASE(comparisons) {
	const auto r0{"abcd"_sr}, r1{"edfg"_sr};
	const char * s = "hjkl";
	BOOST_TEST(r0 < r1);
	BOOST_TEST(!(r0 == r1));
	BOOST_TEST(r1 < s);
	BOOST_TEST(!(r1 == s));
}

BOOST_AUTO_TEST_CASE(removing) {
	const ptl::string_ref r{"Hello World"};
	auto r0{r};
	r0.remove_prefix(std::begin(r0));
	BOOST_TEST(r0 == r);
	r0.remove_prefix(std::find(std::begin(r0), std::end(r0), ' ') + 1);
	BOOST_TEST(r0 == "World");

	auto r1{r};
	r1.remove_suffix(std::end(r1));
	BOOST_TEST(r1 == r);
	r1.remove_suffix(std::find(std::begin(r1), std::end(r1), ' '));
	BOOST_TEST(r1 == "Hello");

	const auto it{std::find(std::begin(r), std::end(r), ' ')};
	const auto r2{r.substr(it, it + 1)};
	BOOST_TEST(r2 == " ");
	BOOST_TEST(r2.size() == 1);
}

//TODO: hash?!

BOOST_AUTO_TEST_SUITE_END()
