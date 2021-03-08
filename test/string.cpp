
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstring>
#include <sstream>
#include <catch2/catch.hpp>
#include "ptl/string.hpp"
#include "utils.hpp"

using namespace std::string_literals;
using namespace ptl::literals;

TEST_CASE("string ctor", "[string]") {
	using ptl::test::input_iterator;

	const ptl::string s0;
	REQUIRE(s0.empty());
	REQUIRE(std::strlen(s0.c_str()) == 0);

	const ptl::string s1{"Hello World"};
	REQUIRE(s1 == "Hello World");

	const ptl::string s2{input_iterator{s1.begin()}, input_iterator{s1.end()}};
	REQUIRE(s2 == s1);

	const ptl::string s3{s1.begin(), s1.end()};
	REQUIRE(s3 == s1);
}

TEST_CASE("string copy", "[string]") {
	//SSO
	const ptl::string s0{"Hello World"};
	REQUIRE(s0.size() == 11);
	REQUIRE(std::strlen(s0.c_str()) == s0.size());

	auto s1{s0};
	REQUIRE(s0 == s1);

	decltype(s1) s2; s2 = s1;
	REQUIRE(s2 == s1);

	//no-SSO
	const ptl::string s3{"xxxxxxxxxxxxxxxxxxxxxxxx"};
	REQUIRE(std::strlen(s3.c_str()) == s3.size());
	REQUIRE(s3.size() == 24);

	auto s4{s3};
	REQUIRE(s4.size() == 24);
	REQUIRE(s4 == s3);

	decltype(s4) s5; s5 = s4;
	REQUIRE(s5.size() == 24);
	REQUIRE(s5 == s3);
}

TEST_CASE("string move", "[string]") {
	//SSO => move should not take place
	ptl::string s0{"Hello World"};
	REQUIRE(s0.size() == 11);
	REQUIRE(std::strlen(s0.c_str()) == s0.size());

	auto s1{std::move(s0)};
	REQUIRE(s0 == s1);

	decltype(s1) s2; s2 = std::move(s1);
	REQUIRE(s2 == s1);

	//no SSO => move should take place
	ptl::string s3{"xxxxxxxxxxxxxxxxxxxxxxxx"};
	REQUIRE(std::strlen(s3.c_str()) == s3.size());
	REQUIRE(s3.size() == 24);

	auto s4{std::move(s3)};
	REQUIRE(s3.empty());
	REQUIRE(s4.size() == 24);
	REQUIRE(s4 != s3);

	decltype(s4) s5; s5 = std::move(s4);
	REQUIRE(s4.empty());
	REQUIRE(s5.size() == 24);
	REQUIRE(s5 != s3);
}

TEST_CASE("string swapping", "[string]") {
	//SSO swap SSO
	ptl::string s0{"Hello"}, s1{"World"};
	s0.swap(s1);
	REQUIRE(s0 == "World");
	REQUIRE(s1 == "Hello");

	//SSO swap noSSO
	ptl::string s2{"Test"}, s3{"xxxxxxxxxxxxxxxxxxxxxxxx"};
	s2.swap(s3);
	REQUIRE(s2 == "xxxxxxxxxxxxxxxxxxxxxxxx");
	REQUIRE(s3 == "Test");

	//noSSO swap SSO
	ptl::string s4{"xxxxxxxxxxxxxxxxxxxxxxxx"}, s5{"Test"};
	s4.swap(s5);
	REQUIRE(s4 == "Test");
	REQUIRE(s5 == "xxxxxxxxxxxxxxxxxxxxxxxx");

	//noSSO swap noSSO
	ptl::string s6{"xxxxxxxxxxxxxxxxxxxxxxxx"}, s7{"XXXXXXXXXXXXXXXXXXXXXXXX"};
	s6.swap(s7);
	REQUIRE(s6 == "XXXXXXXXXXXXXXXXXXXXXXXX");
	REQUIRE(s7 == "xxxxxxxxxxxxxxxxxxxxxxxx");
}

TEST_CASE("string shrinking", "[string]") {
	ptl::string s0;

	const auto capacity{s0.capacity()};
	for(std::size_t i{0}; i < capacity; ++i) {
		s0.push_back('x');
		REQUIRE(s0.capacity() == capacity);
	}

	s0.shrink_to_fit();
	REQUIRE(s0.capacity() == capacity);

	s0.push_back('x');
	REQUIRE(s0.capacity() > capacity);

	s0.pop_back();
	REQUIRE(s0.capacity() > capacity);

	s0.shrink_to_fit();
	REQUIRE(s0.capacity() == capacity);
	REQUIRE(s0.capacity() == s0.size());
}

TEST_CASE("string io", "[string]") {
	const ptl::string s0{"Hello world"};

	std::stringstream ss;
	ss << s0;
	std::string s1;
	std::getline(ss, s1);
	REQUIRE(s0 == s1);
}

TEST_CASE("string clear", "[string]") {
	ptl::string s;
	REQUIRE(s.empty());

	s.clear();
	REQUIRE(s.empty());

	s = "Hello World";
	REQUIRE(!s.empty());

	s.clear();
	REQUIRE(s.empty());
}

TEST_CASE("string erase", "[string]") {
	ptl::string s{"XXXHelloXXX WorldXXX"};

	const auto it0{s.erase(std::begin(s), std::begin(s) + 3)};
	REQUIRE(it0 == std::begin(s));
	REQUIRE(s == "HelloXXX WorldXXX");
	const auto it1{s.erase(std::begin(s) + 5, std::begin(s) + 8)};
	REQUIRE(*it1 == ' ');
	REQUIRE(s == "Hello WorldXXX");
	const auto it2{s.erase(std::end(s) - 3, std::end(s))};
	REQUIRE(it2 == std::end(s));
	REQUIRE(s == "Hello World");
}

TEST_CASE("string insert", "[string]") {
	using ptl::test::input_iterator;
	const std::string x{"XXX"};

	ptl::string s{"HelloWorld"};

	const auto it0{s.insert(std::begin(s), input_iterator{std::begin(x)}, input_iterator{std::end(x)})};
	REQUIRE(it0 == std::begin(s));
	REQUIRE(s == "XXXHelloWorld");
	const auto it1{s.insert(std::begin(s) + 8, input_iterator{std::begin(x)}, input_iterator{std::end(x)})};
	REQUIRE(it1 == std::begin(s) + 8);
	REQUIRE(s == "XXXHelloXXXWorld");
	const auto it2{s.insert(std::end(s), input_iterator{std::begin(x)}, input_iterator{std::end(x)})};
	REQUIRE(it2 == std::end(s) - 3);
	REQUIRE(s == "XXXHelloXXXWorldXXX");
}

TEST_CASE("string append", "[string]") {
	using ptl::test::input_iterator;
	const std::string hello{"Hello"}, cruel{"cruel"}, world{"World"};

	ptl::string s0;
	s0.append(input_iterator{std::begin(hello)}, input_iterator{std::end(hello)});
	REQUIRE(s0 == "Hello");
	s0.append(input_iterator{std::begin(cruel)}, input_iterator{std::end(cruel)});
	REQUIRE(s0 == "Hellocruel");
	s0.append(input_iterator{std::begin(world)}, input_iterator{std::end(world)});
	REQUIRE(s0 == "HellocruelWorld");

	ptl::string s1;
	s1 += hello;
	REQUIRE(s1 == "Hello");
	s1 += cruel;
	REQUIRE(s1 == "Hellocruel");
	s1 += world;
	REQUIRE(s1 == "HellocruelWorld");

	ptl::string s2;
	s2 = s2 + hello;
	REQUIRE(s2 == "Hello");
	s2 = s2 + cruel;
	REQUIRE(s2 == "Hellocruel");
	s2 = s2 + world;
	REQUIRE(s2 == "HellocruelWorld");

	ptl::string s3;
	s3 = world + s3;
	REQUIRE(s3 == "World");
	s3 = cruel + s3;
	REQUIRE(s3 == "cruelWorld");
	s3 = hello + s3;
	REQUIRE(s3 == "HellocruelWorld");
}

TEST_CASE("string assign", "[string]") {
	using ptl::test::input_iterator;
	const std::string hello_world{"Hello World"};

	ptl::string s;
	s.assign(input_iterator{std::begin(hello_world)}, input_iterator{std::end(hello_world)});
	REQUIRE(s == "Hello World");
}

TEST_CASE("string replace", "[string]") {
	using ptl::test::input_iterator;
	const std::string hello{"Hello"}, cruel{"cruel"}, world{"World"};

	ptl::string s0{"XXX XXX XXX"};

	s0.replace(std::begin(s0), std::begin(s0) + 3, input_iterator{std::begin(hello)}, input_iterator{std::end(hello)});
	REQUIRE(s0 == "Hello XXX XXX");
	s0.replace(std::begin(s0) + 6, std::begin(s0) + 9, input_iterator{std::begin(cruel)}, input_iterator{std::end(cruel)});
	REQUIRE(s0 == "Hello cruel XXX");
	s0.replace(std::end(s0) - 3, std::end(s0), input_iterator{std::begin(world)}, input_iterator{std::end(world)});
	REQUIRE(s0 == "Hello cruel World");

	ptl::string s1{"XXXXX XXXXX XXXXX"};

	s1.replace(std::begin(s1), std::begin(s1) + 5, input_iterator{std::begin(hello)}, input_iterator{std::end(hello)});
	REQUIRE(s1 == "Hello XXXXX XXXXX");
	s1.replace(std::begin(s1) + 6, std::begin(s1) + 11, input_iterator{std::begin(cruel)}, input_iterator{std::end(cruel)});
	REQUIRE(s1 == "Hello cruel XXXXX");
	s1.replace(std::end(s1) - 5, std::end(s1), input_iterator{std::begin(world)}, input_iterator{std::end(world)});
	REQUIRE(s1 == "Hello cruel World");

	ptl::string s2{"XXXXXXX XXXXXXX XXXXXXX"};

	s2.replace(std::begin(s2), std::begin(s2) + 7, input_iterator{std::begin(hello)}, input_iterator{std::end(hello)});
	REQUIRE(s2 == "Hello XXXXXXX XXXXXXX");
	s2.replace(std::begin(s2) + 6, std::begin(s2) + 13, input_iterator{std::begin(cruel)}, input_iterator{std::end(cruel)});
	REQUIRE(s2 == "Hello cruel XXXXXXX");
	s2.replace(std::end(s2) - 7, std::end(s2), input_iterator{std::begin(world)}, input_iterator{std::end(world)});
	REQUIRE(s2 == "Hello cruel World");
}
