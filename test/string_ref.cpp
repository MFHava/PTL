
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

BOOST_AUTO_TEST_CASE(substr) {
	const std::string_view str{"Hello World"};
	const ptl::string_ref ref{str};

	BOOST_TEST(std::string_view{ref} == str);

	auto strFirst{str};
	auto refFirst{ref};
	strFirst.remove_prefix(3);
	refFirst.remove_prefix(3);
	BOOST_TEST(refFirst.size() == strFirst.size());
	for(std::size_t i{0}; i < strFirst.size(); ++i) BOOST_TEST(refFirst[i] == strFirst[i]);

	auto strLast{str};
	auto refLast{ref};
	strLast.remove_suffix(3);
	refLast.remove_suffix(3);
	BOOST_TEST(refLast.size() == strLast.size());
	for(std::size_t i{0}; i < strLast.size(); ++i) BOOST_TEST(refLast[i] == strLast[i]);

	const auto substr0{str.substr(4)};
	const auto subref0{ref.substr(4)};
	BOOST_TEST(substr0.size() == substr0.size());
	for(std::size_t i{0}; i < substr0.size(); ++i) BOOST_TEST(subref0[i] == substr0[i]);

	const auto substr1{str.substr(5, 3)};
	const auto subref1{ref.substr(5, 3)};
	BOOST_TEST(substr1.size() == substr1.size());
	for(std::size_t i{0}; i < substr1.size(); ++i) BOOST_TEST(subref1[i] == substr1[i]);
}

BOOST_AUTO_TEST_SUITE_END()
