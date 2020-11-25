
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include <boost/optional.hpp>
#include "ptl/optional_ref.hpp"
#include "ptl/optional.hpp"

BOOST_AUTO_TEST_SUITE(optional_ref)

BOOST_AUTO_TEST_CASE(ctor) {
	ptl::optional_ref<int> op;
	BOOST_TEST(!op);
	BOOST_TEST(!static_cast<bool>(op));

	int val1{0};
	ptl::optional_ref<int> ref1{val1};
	BOOST_TEST(static_cast<bool>(ref1));
	BOOST_TEST(*ref1 == val1);

	ptl::optional_ref<const int> ref2{val1};
	BOOST_TEST(static_cast<bool>(ref2));
	BOOST_TEST(*ref2 == val1);

	const int val2{10};
	ptl::optional_ref<const int> ref3{val2};
	BOOST_TEST(static_cast<bool>(ref3));
	BOOST_TEST(*ref3 == val2);

#define PTL_CONSTRUCTION_FROM_OPTIONAL(_1, _2, Optional) {\
	Optional<int> mop{5};\
	ptl::optional_ref<int> ref{mop};\
	BOOST_TEST(static_cast<bool>(ref));\
	BOOST_TEST(*ref == *mop);\
\
	ptl::optional_ref<const int> cref0{mop};\
	BOOST_TEST(static_cast<bool>(cref0));\
	BOOST_TEST(*cref0 == *mop);\
\
	const Optional<int> cop{10};\
	ptl::optional_ref<const int> cref1{cop};\
	BOOST_TEST(static_cast<bool>(cref1));\
	BOOST_TEST(*cref1 == *cop);\
}
	BOOST_PP_SEQ_FOR_EACH(PTL_CONSTRUCTION_FROM_OPTIONAL, _, BOOST_PP_VARIADIC_TO_SEQ(std::optional, ptl::optional, boost::optional))
#undef PTL_CONSTRUCTION_FROM_OPTIONAL
}

BOOST_AUTO_TEST_CASE(swapping) {
	int v1{5}, v2{10};
	ptl::optional_ref<int> op1{v1}, op2{v2};
	swap(op1, op2);
	BOOST_TEST(*op1 == 10);
	BOOST_TEST(*op2 ==  5);

	ptl::optional_ref<int> op3;
	swap(op1, op3);
	BOOST_TEST(!op1);
	BOOST_TEST(static_cast<bool>(op3));
	BOOST_TEST(*op3 == 10);
}

BOOST_AUTO_TEST_CASE(ctad) {
	int v1{0};
	ptl::optional_ref op1{v1};
	static_assert(std::is_same_v<decltype(op1), ptl::optional_ref<int>>);

	const int v2{0};
	ptl::optional_ref op2{v2};
	static_assert(std::is_same_v<decltype(op2), ptl::optional_ref<const int>>);

	int * p1{nullptr};
	ptl::optional_ref op3{p1};
	static_assert(std::is_same_v<decltype(op3), ptl::optional_ref<int>>);

	const int * p2{nullptr};
	ptl::optional_ref op4{p2};
	static_assert(std::is_same_v<decltype(op4), ptl::optional_ref<const int>>);

#define PTL_CTAD_FROM_OPTIONAL(_1, _2, Optional) {\
	Optional<int> mop{5};\
	ptl::optional_ref mref{mop};\
	static_assert(std::is_same_v<decltype(mref), ptl::optional_ref<      int>>);\
	\
	const Optional<int> cop{5};\
	ptl::optional_ref cref{cop};\
	static_assert(std::is_same_v<decltype(cref), ptl::optional_ref<const int>>);\
}
	BOOST_PP_SEQ_FOR_EACH(PTL_CTAD_FROM_OPTIONAL, _, BOOST_PP_VARIADIC_TO_SEQ(std::optional, ptl::optional, boost::optional))
#undef PTL_CTAD_FROM_OPTIONAL
}

BOOST_AUTO_TEST_SUITE_END()
