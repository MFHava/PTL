
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

BOOST_AUTO_TEST_CASE(ctor_from_string) {
	const char data[]{'H', 'e', 'l', 'l', 'o', '\0', 'W', 'o', 'r', 'l', 'd', '\0'};
	const std::string s{data, sizeof(data)};
	const ptl::string_ref v{s};
	BOOST_TEST(v.size() == sizeof(data));
	BOOST_TEST(v[5] == '\0');

	std::stringstream ss;
	ss << v;
	BOOST_TEST(ss.str().size() == sizeof(data));
}

BOOST_AUTO_TEST_CASE(at) {
	const ptl::string_ref v{"TEST"};
	BOOST_TEST(v[0] == 'T');
	BOOST_TEST(v.at(0) == 'T');
	BOOST_TEST(v[1] == 'E');
	BOOST_TEST(v.at(1) == 'E');
	BOOST_TEST(v[2] == 'S');
	BOOST_TEST(v.at(2) == 'S');
	BOOST_TEST(v[3] == 'T');
	BOOST_TEST(v.at(3) == 'T');
	try {
		v.at(4);
		BOOST_TEST(false);
	} catch(...) {
		BOOST_TEST(true);
	}
}

BOOST_AUTO_TEST_CASE(size) {
	const std::string s0{"TEST"};
	const ptl::string_ref v0{"TEST"};
	BOOST_TEST(s0.size() == v0.size());
	BOOST_TEST(s0.size() == 4);

	const auto s1 = "TEST"s;
	const auto v1 = "TEST"_sr;
	BOOST_TEST(s1.size() == v1.size());
	BOOST_TEST(s1.size() == 4);
}

BOOST_AUTO_TEST_CASE(empty) {
	const std::string s0;
	const ptl::string_ref v0;
	BOOST_TEST(s0.empty());
	BOOST_TEST(v0.empty());

	const std::string s1{""};
	const ptl::string_ref v1{""};
	BOOST_TEST(s1.empty());
	BOOST_TEST(v1.empty());
}

//TODO: data & c_str

BOOST_AUTO_TEST_CASE(iterators) {
	const ptl::string_ref v{"TEST"};
	{
		auto it = v.begin();
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST(*it == 'E');
		++it;
		BOOST_TEST(*it == 'S');
		++it;
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST((it == v.end()));
	}
	{
		auto it = v.rbegin();
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST(*it == 'S');
		++it;
		BOOST_TEST(*it == 'E');
		++it;
		BOOST_TEST(*it == 'T');
		++it;
		BOOST_TEST((it == v.rend()));
	}
}

BOOST_AUTO_TEST_CASE(swapping) {
	const ptl::string_ref v0{"TEST"};
	ptl::string_ref v1{"ABCD"}, v2{v0};
	BOOST_TEST((v0 == v2));
	BOOST_TEST((v0 != v1));
	swap(v1, v2);
	BOOST_TEST((v0 != v2));
	BOOST_TEST((v0 == v1));
}

//TODO: comparison string_ref & string_ref

//TODO: comparison string_ref & const char *
//TODO: comparison const char * & string_ref

BOOST_AUTO_TEST_SUITE_END()