
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>
#include "ptl/optional.hpp"

static_assert(std::is_same<decltype(ptl::get(std::declval<      ptl::optional<int> &&>())),       int &&>::value, "unexpected r-value get");
static_assert(std::is_same<decltype(ptl::get(std::declval<const ptl::optional<int> &&>())), const int &&>::value, "unexpected r-value get");

BOOST_AUTO_TEST_SUITE(optional)
//TODO: BOOST_AUTO_TEST_CASE

//TODO: default-constructor
//TODO: copy/move constructors
//TODO: copy/move assignment
//TODO: inplace-constructor
//TODO: operator->
//TODO: operator*
//TODO: operator bool
//TODO: operator!
//TODO: emplace
//TODO: comparison
//TODO: make_optional
BOOST_AUTO_TEST_SUITE_END()
