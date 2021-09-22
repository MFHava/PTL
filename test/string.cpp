
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

	const ptl::string s2{input_iterator{s1.cbegin()}, input_iterator{s1.cend()}};
	REQUIRE(s2 == s1);

	const ptl::string s3{s1.cbegin(), s1.cend()};
	REQUIRE(s3 == s1);

	const ptl::string s4(5, 'x');
	REQUIRE(s4 == "xxxxx");

	const auto ilist = {'a', 'b', 'c', 'd', 'e'};
	const ptl::string s5(ilist);
	REQUIRE(s5 == "abcde");
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
	swap(s0, s1);
	REQUIRE(s0 == "World");
	REQUIRE(s1 == "Hello");

	//SSO swap noSSO
	ptl::string s2{"Test"}, s3{"xxxxxxxxxxxxxxxxxxxxxxxx"};
	swap(s2, s3);
	REQUIRE(s2 == "xxxxxxxxxxxxxxxxxxxxxxxx");
	REQUIRE(s3 == "Test");

	//noSSO swap SSO
	ptl::string s4{"xxxxxxxxxxxxxxxxxxxxxxxx"}, s5{"Test"};
	swap(s4, s5);
	REQUIRE(s4 == "Test");
	REQUIRE(s5 == "xxxxxxxxxxxxxxxxxxxxxxxx");

	//noSSO swap noSSO
	ptl::string s6{"xxxxxxxxxxxxxxxxxxxxxxxx"}, s7{"XXXXXXXXXXXXXXXXXXXXXXXX"};
	swap(s6, s7);
	REQUIRE(s6 == "XXXXXXXXXXXXXXXXXXXXXXXX");
	REQUIRE(s7 == "xxxxxxxxxxxxxxxxxxxxxxxx");
}

TEST_CASE("string shrinking", "[string]") {
	ptl::string s0;

	const auto capacity{s0.capacity()};
	for(std::size_t i{0}; i < capacity; ++i) {
		REQUIRE(s0.push_back(static_cast<char>('a' + i)) == static_cast<char>('a' + i));
		REQUIRE(s0.capacity() == capacity);
	}

	s0.shrink_to_fit();
	REQUIRE(s0.capacity() == capacity);

	REQUIRE(s0.push_back('X') == 'X');
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

	const auto it0{s.erase(s.cbegin(), s.cbegin() + 3)};
	REQUIRE(it0 == s.begin());
	REQUIRE(s == "HelloXXX WorldXXX");
	const auto it1{s.erase(s.cbegin() + 5, s.cbegin() + 8)};
	REQUIRE(*it1 == ' ');
	REQUIRE(s == "Hello WorldXXX");
	const auto it2{s.erase(s.cend() - 3, s.cend())};
	REQUIRE(it2 == s.end());
	REQUIRE(s == "Hello World");
}

TEST_CASE("string insert", "[string]") {
	using ptl::test::input_iterator;
	const std::string x{"XXX"};

	ptl::string s{"HelloWorld"};

	const auto it0{s.insert(s.cbegin(), input_iterator{x.cbegin()}, input_iterator{x.cend()})};
	REQUIRE(it0 == s.begin());
	REQUIRE(s == "XXXHelloWorld");
	const auto it1{s.insert(s.cbegin() + 8, input_iterator{x.cbegin()}, input_iterator{x.cend()})};
	REQUIRE(it1 == s.begin() + 8);
	REQUIRE(s == "XXXHelloXXXWorld");
	const auto it2{s.insert(s.cend(), input_iterator{x.cbegin()}, input_iterator{x.cend()})};
	REQUIRE(it2 == s.end() - 3);
	REQUIRE(s == "XXXHelloXXXWorldXXX");

	const auto it5{s.insert(s.cbegin(), 2, 'Y')};
	REQUIRE(it5 == s.begin());
	REQUIRE(s == "YYXXXHelloXXXWorldXXX");
	const auto it6{s.insert(s.cbegin() + 10, 2, 'Y')};
	REQUIRE(it6 == s.begin() + 10);
	REQUIRE(s == "YYXXXHelloYYXXXWorldXXX");
	const auto it7{s.insert(s.cend(), 2, 'Y')};
	REQUIRE(it7 == s.end() - 2);
	REQUIRE(s == "YYXXXHelloYYXXXWorldXXXYY");
}

TEST_CASE("string append", "[string]") {
	using ptl::test::input_iterator;
	const std::string hello{"Hello"}, cruel{"cruel"}, world{"World"};

	ptl::string s0;
	s0.append(input_iterator{hello.cbegin()}, input_iterator{hello.cend()});
	REQUIRE(s0 == "Hello");
	s0.append(input_iterator{cruel.cbegin()}, input_iterator{cruel.cend()});
	REQUIRE(s0 == "Hellocruel");
	s0.append(input_iterator{world.cbegin()}, input_iterator{world.cend()});
	REQUIRE(s0 == "HellocruelWorld");

	s0.append(3, 'X');
	REQUIRE(s0 == "HellocruelWorldXXX");

	ptl::string s1;
	s1 += hello;
	REQUIRE(s1 == "Hello");
	s1 += cruel;
	REQUIRE(s1 == "Hellocruel");
	s1 += world;
	REQUIRE(s1 == "HellocruelWorld");
	s1 += '!';
	REQUIRE(s1 == "HellocruelWorld!");

	ptl::string s2;
	s2 = s2 + hello;
	REQUIRE(s2 == "Hello");
	s2 = s2 + cruel;
	REQUIRE(s2 == "Hellocruel");
	s2 = s2 + world;
	REQUIRE(s2 == "HellocruelWorld");
	s2 = s2 + '!';
	REQUIRE(s2 == "HellocruelWorld!");

	ptl::string s3;
	s3 = '!' + s3;
	REQUIRE(s3 == "!");
	s3 = world + s3;
	REQUIRE(s3 == "World!");
	s3 = cruel + s3;
	REQUIRE(s3 == "cruelWorld!");
	s3 = hello + s3;
	REQUIRE(s3 == "HellocruelWorld!");
}

TEST_CASE("string assign", "[string]") {
	using ptl::test::input_iterator;
	const std::string hello_world{"Hello World"};

	ptl::string s;
	s.assign(input_iterator{hello_world.cbegin()}, input_iterator{hello_world.cend()});
	REQUIRE(s == "Hello World");

	s.assign(3, 'X');
	REQUIRE(s == "XXX");
}

TEST_CASE("string replace", "[string]") {
	using ptl::test::input_iterator;
	const std::string hello{"Hello"}, cruel{"cruel"}, world{"World"};

	ptl::string s0{"XXX XXX XXX"};
	s0.replace(s0.cbegin(), s0.cbegin() + 3, input_iterator{hello.cbegin()}, input_iterator{hello.cend()});
	REQUIRE(s0 == "Hello XXX XXX");
	s0.replace(s0.cbegin() + 6, s0.cbegin() + 9, input_iterator{cruel.cbegin()}, input_iterator{cruel.cend()});
	REQUIRE(s0 == "Hello cruel XXX");
	s0.replace(s0.cend() - 3, s0.cend(), input_iterator{world.cbegin()}, input_iterator{world.cend()});
	REQUIRE(s0 == "Hello cruel World");

	ptl::string s1{"XXXXX XXXXX XXXXX"};

	s1.replace(s1.cbegin(), s1.cbegin() + 5, input_iterator{hello.cbegin()}, input_iterator{hello.cend()});
	REQUIRE(s1 == "Hello XXXXX XXXXX");
	s1.replace(s1.cbegin() + 6, s1.cbegin() + 11, input_iterator{cruel.cbegin()}, input_iterator{cruel.cend()});
	REQUIRE(s1 == "Hello cruel XXXXX");
	s1.replace(s1.cend() - 5, s1.cend(), input_iterator{world.cbegin()}, input_iterator{world.cend()});
	REQUIRE(s1 == "Hello cruel World");

	ptl::string s2{"XXXXXXX XXXXXXX XXXXXXX"};

	s2.replace(s2.cbegin(), s2.cbegin() + 7, input_iterator{hello.cbegin()}, input_iterator{hello.cend()});
	REQUIRE(s2 == "Hello XXXXXXX XXXXXXX");
	s2.replace(s2.cbegin() + 6, s2.cbegin() + 13, input_iterator{cruel.cbegin()}, input_iterator{cruel.cend()});
	REQUIRE(s2 == "Hello cruel XXXXXXX");
	s2.replace(s2.cend() - 7, s2.cend(), input_iterator{world.cbegin()}, input_iterator{world.cend()});
	REQUIRE(s2 == "Hello cruel World");


	ptl::string s3{"A A A"};
	s3.replace(s3.cbegin(), s3.cbegin() + 1, 3, 'X');
	REQUIRE(s3 == "XXX A A");
	s3.replace(s3.cbegin() + 4, s3.cbegin() + 5, 3, 'X');
	REQUIRE(s3 == "XXX XXX A");
	s3.replace(s3.cbegin() + 8, s3.cend(), 3, 'X');
	REQUIRE(s3 == "XXX XXX XXX");

	ptl::string s4{"AAA AAA AAA"};
	s4.replace(s4.cbegin(), s4.cbegin() + 3, 3, 'X');
	REQUIRE(s4 == "XXX AAA AAA");
	s4.replace(s4.cbegin() + 4, s4.cbegin() + 7, 3, 'X');
	REQUIRE(s4 == "XXX XXX AAA");
	s4.replace(s4.cbegin() + 8, s4.cend(), 3, 'X');
	REQUIRE(s4 == "XXX XXX XXX");

	ptl::string s5{"AAA AAA AAA"};
	s5.replace(s5.cbegin(), s5.cbegin() + 3, 1, 'X');
	REQUIRE(s5 == "X AAA AAA");
	s5.replace(s5.cbegin() + 2, s5.cbegin() + 5, 1, 'X');
	REQUIRE(s5 == "X X AAA");
	s5.replace(s5.cbegin() + 4, s5.cend(), 1, 'X');
	REQUIRE(s5 == "X X X");
}
