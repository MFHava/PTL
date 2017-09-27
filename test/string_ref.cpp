
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define WIN32_LEAN_AND_MEAN//suppress #define interface
#include <boost/test/unit_test.hpp>
#include "ptl/string_ref.hpp"

using namespace std::string_literals;
using namespace ptl::literals;

BOOST_AUTO_TEST_SUITE(string_ref)

BOOST_AUTO_TEST_CASE(construction) {
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

BOOST_AUTO_TEST_CASE(at) {
	const ptl::string_ref r{"TEST"};
	BOOST_TEST(r[0]    == 'T');
	BOOST_TEST(r.at(0) == 'T');
	BOOST_TEST(r[1]    == 'E');
	BOOST_TEST(r.at(1) == 'E');
	BOOST_TEST(r[2]    == 'S');
	BOOST_TEST(r.at(2) == 'S');
	BOOST_TEST(r[3]    == 'T');
	BOOST_TEST(r.at(3) == 'T');
	try {
		r.at(4);
		BOOST_TEST(false);
	} catch(...) {
		BOOST_TEST(true);
	}
	BOOST_TEST(r.front() == r[0]);
	BOOST_TEST(r.back()  == r[3]);
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

BOOST_AUTO_TEST_CASE(empty) {
	const std::string s0;
	const ptl::string_ref r0;
	BOOST_TEST(s0.empty());
	BOOST_TEST(r0.empty());

	const std::string s1{""};
	const ptl::string_ref r1{""};
	BOOST_TEST(s1.empty());
	BOOST_TEST(r1.empty());
}

BOOST_AUTO_TEST_CASE(iterators) {
	const ptl::string_ref r{"TEST"};
	{
		auto it = r.begin();
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST(*it == 'E');
		++it;
		BOOST_TEST(*it == 'S');
		++it;
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST((it == r.end()));
	}
	{
		auto it = r.rbegin();
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST(*it == 'S');
		++it;
		BOOST_TEST(*it == 'E');
		++it;
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST((it == r.rend()));
	}
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

BOOST_AUTO_TEST_CASE(swapping) {
	const ptl::string_ref r{"TEST"};
	ptl::string_ref r0{"ABCD"}, r1{r};
	BOOST_TEST(r == r1);
	BOOST_TEST(r != r0);
	swap(r0, r1);
	BOOST_TEST(r != r1);
	BOOST_TEST(r == r0);
}

BOOST_AUTO_TEST_CASE(comparisons) {
	const auto r0{"abcd"_sr}, r1{"edfg"_sr};
	const char * s = "hjkl";
	BOOST_TEST(r0 <  r1);
	BOOST_TEST(r0 <= r1);
	BOOST_TEST(r1 >  r0);
	BOOST_TEST(r1 >= r0);
	BOOST_TEST(r0 != r1);
	BOOST_TEST(!(r0 == r1));
	BOOST_TEST(r1 <  s);
	BOOST_TEST(r1 <= s);
	BOOST_TEST(s >  r1);
	BOOST_TEST(s >= r1);
	BOOST_TEST(r1 != s);
	BOOST_TEST(!(r1 == s));
}

//TODO: hash?!

BOOST_AUTO_TEST_SUITE_END()