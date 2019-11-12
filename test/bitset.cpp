
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <bitset>
#include <numeric>
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "ptl/bitset.hpp"

namespace {
	template<std::size_t Size>
	auto operator==(const std::bitset<Size> & lhs, const ptl::bitset<Size> & rhs) noexcept -> bool {
		for(std::size_t i{0}; i < Size; ++i)
			if(lhs[i] != rhs[i])
				return false;
		return true;
	}
}

BOOST_AUTO_TEST_SUITE(bitset)

BOOST_AUTO_TEST_CASE(ctor) {
	std::bitset<10> sb1;
	ptl::bitset<10> pb1;
	BOOST_TEST((sb1 == pb1));

	std::bitset<10> sb2{0b01010101};
	ptl::bitset<10> pb2{0b01010101};
	BOOST_TEST((sb2 == pb2));

	std::bitset<4> sb3{0b01010101};
	ptl::bitset<4> pb3{0b01010101};
	BOOST_TEST((sb3 == pb3));
}

BOOST_AUTO_TEST_CASE(io) {
	const ptl::bitset<5> bs1{0b1000};

	std::stringstream ss;
	ss << bs1;
	BOOST_TEST(ss.str() == "01000");

	ptl::bitset<5> b2;
	ss >> b2;
	BOOST_TEST(bs1 == b2);

	ss << "10";
	ptl::bitset<1> b4;
	ss >> b4;
	const ptl::bitset<1> b5{0b1};
	BOOST_TEST(b4 == b5);
	ss >> b4;
	const ptl::bitset<1> b6{0b0};
	BOOST_TEST(b4 == b6);
}

BOOST_AUTO_TEST_CASE(set) {
	std::bitset<10> sb;
	ptl::bitset<10> pb;

	sb.set(5);
	pb.set(5);
	sb.set(8);
	pb.set(8);
	BOOST_TEST((sb == pb));

	sb.set();
	pb.set();
	BOOST_TEST((sb == pb));

	sb.set(1, false);
	pb.set(1, false);
	BOOST_TEST((sb == pb));

	sb.reset(5);
	pb.reset(5);
	BOOST_TEST((sb == pb));

	sb.reset();
	pb.reset();
	BOOST_TEST((sb == pb));
}

BOOST_AUTO_TEST_CASE(count) {
	ptl::bitset<10> pb;
	BOOST_TEST(pb.count() == 0);
	BOOST_TEST(pb.none());
	BOOST_TEST(!pb.any());
	BOOST_TEST(!pb.all());

	pb.set(1);
	BOOST_TEST(pb.test(1));
	BOOST_TEST(!pb.test(0));
	BOOST_TEST(pb.count() == 1);
	BOOST_TEST(!pb.none());
	BOOST_TEST(pb.any());
	BOOST_TEST(!pb.all());

	pb.set(4);
	BOOST_TEST(pb.test(1));
	BOOST_TEST(!pb.test(3));
	BOOST_TEST(pb.test(4));
	BOOST_TEST(pb.count() == 2);
	BOOST_TEST(!pb.none());
	BOOST_TEST(pb.any());
	BOOST_TEST(!pb.all());

	pb.set();
	BOOST_TEST(pb.count() == 10);
	BOOST_TEST(!pb.none());
	BOOST_TEST(pb.any());
	BOOST_TEST(pb.all());

	pb.reset();
	BOOST_TEST(pb.count() == 0);
	BOOST_TEST(pb.none());
	BOOST_TEST(!pb.any());
	BOOST_TEST(!pb.all());

	pb.flip();
	BOOST_TEST(pb.count() == 10);
	BOOST_TEST(!pb.none());
	BOOST_TEST(pb.any());
	BOOST_TEST(pb.all());

	pb.flip(5);
	BOOST_TEST(pb.test(0));
	BOOST_TEST(!pb.test(5));
	BOOST_TEST(pb.count() == 9);
	BOOST_TEST(!pb.none());
	BOOST_TEST(pb.any());
	BOOST_TEST(!pb.all());
}

BOOST_AUTO_TEST_CASE(bitwise) {
	ptl::bitset<10> pb1, pb2, expected;

	for(std::size_t i{0}; i < pb1.size(); i += 2) pb1.set(i);
	for(std::size_t i{0}; i < pb2.size(); i += 3) pb2.set(i);

	expected.set();
	expected.reset(1);
	expected.reset(5);
	expected.reset(7);
	BOOST_TEST(((pb1 | pb2) == expected));

	expected.reset();
	expected.set(0);
	expected.set(6);
	BOOST_TEST(((pb1 & pb2) == expected));

	expected.reset();
	expected.set(2);
	expected.set(3);
	expected.set(4);
	expected.set(8);
	expected.set(9);
	BOOST_TEST(((pb1 ^ pb2) == expected));
}

BOOST_AUTO_TEST_CASE(shifting) {
	ptl::bitset<10> pb, expected;

	pb.set(0);
	pb <<= 1;

	expected.set(1);
	BOOST_TEST((pb == expected));

	pb <<= 7;

	expected.reset();
	expected.set(8);
	BOOST_TEST((pb == expected));

	pb >>= 3;

	expected.reset();
	expected.set(5);
	BOOST_TEST((pb == expected));

	pb >>= 9;

	expected.reset();
	BOOST_TEST((pb == expected));

	pb.set(0);
	pb <<= 10;

	BOOST_TEST((pb == expected));
}

BOOST_AUTO_TEST_CASE(swapping) {
	const ptl::bitset<10> pb1{0b1010'1010}, pb2{0b0101'0101};
	auto pb3{pb1}, pb4{pb2};
	BOOST_TEST(pb1 == pb3);
	BOOST_TEST(pb2 == pb4);

	swap(pb3, pb4);
	BOOST_TEST(pb1 == pb4);
	BOOST_TEST(pb2 == pb3);
}

BOOST_AUTO_TEST_CASE(structured_binding) {
	ptl::bitset<3> pb;
	pb.set(1);
	const auto & [a, b, c]{pb};
	BOOST_TEST(!a);
	BOOST_TEST( b);
	BOOST_TEST(!c);
}

BOOST_AUTO_TEST_SUITE_END()
