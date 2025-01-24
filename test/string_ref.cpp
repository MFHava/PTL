
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/string_ref.hpp>

using namespace std::string_literals;
using namespace ptl::literals;

TEST_CASE("string_ref ctor", "[string_ref]") {
	const char data[]{'H', 'e', 'l', 'l', 'o', '\0', 'W', 'o', 'r', 'l', 'd', '\0'};
	const std::string s{data, sizeof(data)};
	const ptl::string_ref r0{s};
	REQUIRE(r0.size() == sizeof(data));
	REQUIRE(r0[5] == '\0');

	std::stringstream ss;
	ss << r0;
	REQUIRE(ss.str().size() == sizeof(data));

	ptl::string_ref r1;
	REQUIRE(r0 != r1);
	r1 = r0;
	REQUIRE(r0 == r1);
}

TEST_CASE("string_ref size", "[string_ref]") {
	const std::string s0{"TEST"};
	const ptl::string_ref r0{"TEST"};
	REQUIRE(s0.size() == r0.size());
	REQUIRE(s0.size() == 4);

	const auto s1 = "TEST"s;
	const auto r1 = "TEST"_sr;
	REQUIRE(s1.size() == r1.size());
	REQUIRE(s1.size() == 4);
}

TEST_CASE("string_ref substr", "[string_ref]") {
	const std::string_view str{"Hello World"};
	const ptl::string_ref ref{str};

	REQUIRE((std::string_view{ref} == str));

	auto strFirst{str};
	auto refFirst{ref};
	strFirst.remove_prefix(3);
	refFirst.remove_prefix(3);
	REQUIRE(refFirst.size() == strFirst.size());
	for(std::size_t i{0}; i < strFirst.size(); ++i) REQUIRE(refFirst[i] == strFirst[i]);

	auto strLast{str};
	auto refLast{ref};
	strLast.remove_suffix(3);
	refLast.remove_suffix(3);
	REQUIRE(refLast.size() == strLast.size());
	for(std::size_t i{0}; i < strLast.size(); ++i) REQUIRE(refLast[i] == strLast[i]);

	const auto substr0{str.substr(4)};
	const auto subref0{ref.substr(4)};
	REQUIRE(substr0.size() == substr0.size());
	for(std::size_t i{0}; i < substr0.size(); ++i) REQUIRE(subref0[i] == substr0[i]);

	const auto substr1{str.substr(5, 3)};
	const auto subref1{ref.substr(5, 3)};
	REQUIRE(substr1.size() == substr1.size());
	for(std::size_t i{0}; i < substr1.size(); ++i) REQUIRE(subref1[i] == substr1[i]);
}
