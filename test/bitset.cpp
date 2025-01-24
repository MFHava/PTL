
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <bitset>
#include <numeric>
#include <sstream>
#include <catch2/catch_all.hpp> //TODO: use more specific headers
#include <ptl/bitset.hpp>

namespace ptl{
	template<std::size_t Size, typename Tag>
	auto operator==(const std::bitset<Size> & lhs, const bitset<Size, Tag> & rhs) noexcept -> bool {
		for(std::size_t i{0}; i < Size; ++i)
			if(lhs[i] != rhs[i])
				return false;
		return true;
	}
}

TEST_CASE("bitset ctor", "[bitset]") {
	std::bitset<10> sb1;
	ptl::bitset<10> pb1;
	REQUIRE(sb1 == pb1);

	std::bitset<10> sb2{0b01010101};
	ptl::bitset<10> pb2{0b01010101};
	REQUIRE(sb2 == pb2);

	std::bitset<4> sb3{0b01010101};
	ptl::bitset<4> pb3{0b01010101};
	REQUIRE(sb3 == pb3);
}

TEST_CASE("bitset io", "[bitset]") {
	const ptl::bitset<5> bs1{0b1000};

	std::stringstream ss;
	ss << bs1;
	REQUIRE(ss.str() == "01000");

	ptl::bitset<5> b2;
	ss >> b2;
	REQUIRE(bs1 == b2);

	ss << "10";
	ptl::bitset<1> b4;
	ss >> b4;
	const ptl::bitset<1> b5{0b1};
	REQUIRE(b4 == b5);
	ss >> b4;
	const ptl::bitset<1> b6{0b0};
	REQUIRE(b4 == b6);
}

TEST_CASE("bitset set", "[bitset]") {
	std::bitset<10> sb;
	ptl::bitset<10> pb;

	sb.set(5);
	pb.set(5);
	sb.set(8);
	pb.set(8);
	REQUIRE(sb == pb);

	sb.set();
	pb.set();
	REQUIRE(sb == pb);

	sb.set(1, false);
	pb.set(1, false);
	REQUIRE(sb == pb);

	sb.reset(5);
	pb.reset(5);
	REQUIRE(sb == pb);

	sb.reset();
	pb.reset();
	REQUIRE(sb == pb);
}

TEST_CASE("bitset count", "[bitset]") {
	ptl::bitset<10> pb;
	REQUIRE(pb.count() == 0);
	REQUIRE(pb.none());
	REQUIRE(!pb.any());
	REQUIRE(!pb.all());

	pb.set(1);
	REQUIRE(pb.test(1));
	REQUIRE(!pb.test(0));
	REQUIRE(pb.count() == 1);
	REQUIRE(!pb.none());
	REQUIRE(pb.any());
	REQUIRE(!pb.all());

	pb.set(4);
	REQUIRE(pb.test(1));
	REQUIRE(!pb.test(3));
	REQUIRE(pb.test(4));
	REQUIRE(pb.count() == 2);
	REQUIRE(!pb.none());
	REQUIRE(pb.any());
	REQUIRE(!pb.all());

	pb.set();
	REQUIRE(pb.count() == 10);
	REQUIRE(!pb.none());
	REQUIRE(pb.any());
	REQUIRE(pb.all());

	pb.reset();
	REQUIRE(pb.count() == 0);
	REQUIRE(pb.none());
	REQUIRE(!pb.any());
	REQUIRE(!pb.all());

	pb.flip();
	REQUIRE(pb.count() == 10);
	REQUIRE(!pb.none());
	REQUIRE(pb.any());
	REQUIRE(pb.all());

	pb.flip(5);
	REQUIRE(pb.test(0));
	REQUIRE(!pb.test(5));
	REQUIRE(pb.count() == 9);
	REQUIRE(!pb.none());
	REQUIRE(pb.any());
	REQUIRE(!pb.all());
}

TEST_CASE("bitset bitwise", "[bitset]") {
	ptl::bitset<10> pb1, pb2, expected;

	for(std::size_t i{0}; i < pb1.size(); i += 2) pb1.set(i);
	for(std::size_t i{0}; i < pb2.size(); i += 3) pb2.set(i);

	expected.set();
	expected.reset(1);
	expected.reset(5);
	expected.reset(7);
	REQUIRE((pb1 | pb2) == expected);

	expected.reset();
	expected.set(0);
	expected.set(6);
	REQUIRE((pb1 & pb2) == expected);

	expected.reset();
	expected.set(2);
	expected.set(3);
	expected.set(4);
	expected.set(8);
	expected.set(9);
	REQUIRE((pb1 ^ pb2) == expected);
}

TEST_CASE("bitset shifting", "[bitset]") {
	ptl::bitset<10> pb, expected;

	pb.set(0);
	pb <<= 1;

	expected.set(1);
	REQUIRE(pb == expected);

	pb <<= 7;

	expected.reset();
	expected.set(8);
	REQUIRE(pb == expected);

	pb >>= 3;

	expected.reset();
	expected.set(5);
	REQUIRE(pb == expected);

	pb >>= 9;

	expected.reset();
	REQUIRE(pb == expected);

	pb.set(0);
	pb <<= 10;

	REQUIRE(pb == expected);
}

TEST_CASE("bitset swapping", "[bitset]") {
	const ptl::bitset<10> pb1{0b1010'1010}, pb2{0b0101'0101};
	auto pb3{pb1}, pb4{pb2};
	REQUIRE(pb1 == pb3);
	REQUIRE(pb2 == pb4);

	swap(pb3, pb4);
	REQUIRE(pb1 == pb4);
	REQUIRE(pb2 == pb3);
}

TEST_CASE("bitset structured binding", "[bitset]") {
	ptl::bitset<3> pb;
	pb.set(1);
	const auto & [a, b, c]{pb};
	REQUIRE(!a);
	REQUIRE( b);
	REQUIRE(!c);
}

namespace {
	struct tag;
}

namespace ptl {
	template<>
	inline
	constexpr
	bool enable_bitset_operator_bool<tag>{true};
}

TEST_CASE("bitset operator bool", "[bitset]") {
	ptl::bitset<4, tag> pb;
	REQUIRE(!pb);
	pb.set(0);
	REQUIRE(pb);
	pb.reset(0);
	REQUIRE(!pb);
	pb.set();
	REQUIRE(pb);
	pb.reset();
	REQUIRE(!pb);
}
