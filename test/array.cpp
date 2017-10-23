
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "ptl/array.hpp"

BOOST_CONCEPT_ASSERT((boost::Mutable_RandomAccessContainer<ptl::array<int, 10>>));
static_assert(sizeof(ptl::array<int, 10>) == 10 * sizeof(int), "unexpected size for array detected");

namespace {
	//TODO
}