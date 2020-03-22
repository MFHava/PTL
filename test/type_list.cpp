
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdint>
#include "ptl/type_list.hpp"
using namespace ptl;

namespace size_ {
	static_assert(type_list<>         ::size == 0);
	static_assert(type_list<int>      ::size == 1);
	static_assert(type_list<int, void>::size == 2);
}

namespace at_ {
	static_assert(std::is_same_v<
		type_list<int, void>::at<0>,
		int
	>);
	static_assert(std::is_same_v<
		type_list<int, void>::at<1>,
		void
	>);
}

namespace insert_ {
	static_assert(std::is_same_v<
		type_list<>::insert<0, int>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<int, void>::insert<0, float>,
		type_list<float, int, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, void>::insert<1, float>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, void>::insert<2, float>,
		type_list<int, void, float>
	>);
}

namespace subset_ {
	static_assert(std::is_same_v<
		type_list<>::subset<>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void, int>::subset<>,
		type_list<void, int>
	>);
	static_assert(std::is_same_v<
		type_list<void, int>::subset<1>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<void, int>::subset<0, 1>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<void, int>::subset<1, 1>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::subset<1>,
		type_list<int, float>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::subset<1, 2>,
		type_list<int, float>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::subset<2, 1>,
		type_list<float>
	>);
}

namespace find_ {
	static_assert(type_list<         >::find<  int> == not_found);
	static_assert(type_list<int      >::find<  int> == 0);
	static_assert(type_list<int, void>::find<  int> == 0);
	static_assert(type_list<         >::find< void> == not_found);
	static_assert(type_list<int      >::find< void> == not_found);
	static_assert(type_list<int, void>::find< void> == 1);
	static_assert(type_list<         >::find<float> == not_found);
	static_assert(type_list<int      >::find<float> == not_found);
	static_assert(type_list<int, void>::find<float> == not_found);
}

namespace find_first_of_ {
	static_assert(type_list<>::find_first_of<type_list<int, double, void>> == not_found);
	static_assert(type_list<float, float, float>::find_first_of<type_list<int, double, void>> == not_found);
	static_assert(type_list<float, float, float, int>::find_first_of<type_list<int, double, void>> == 3);
	static_assert(type_list<float, float, double, float, int>::find_first_of<type_list<int, double, void>> == 2);
	static_assert(type_list<void, float, double, float, int>::find_first_of<type_list<int, double, void>> == 0);
}

namespace find_last_of_ {
	static_assert(type_list<>::find_last_of<type_list<int, double, void>> == not_found);
	static_assert(type_list<float, float, float>::find_last_of<type_list<int, double, void>> == not_found);
	static_assert(type_list<float, float, float, int>::find_last_of<type_list<int, double, void>> == 3);
	static_assert(type_list<float, float, double, float, int>::find_last_of<type_list<int, double, void>> == 4);
	static_assert(type_list<void, float, double, float, int>::find_last_of<type_list<int, double, void>> == 4);
}

namespace find_first_ {
	static_assert(type_list<>::find_first<type_list<>> == not_found);
	static_assert(type_list<>::find_first<type_list<int , bool>> == not_found);
	static_assert(type_list<int>::find_first<type_list<int, bool>> == not_found);
	static_assert(type_list<int, bool>::find_first<type_list<>> == not_found);
	static_assert(type_list<int, bool>::find_first<type_list<int>> == 0);
	static_assert(type_list<int, bool>::find_first<type_list<bool>> == 1);
	static_assert(type_list<int, bool>::find_first<type_list<int, bool>> == 0);
	static_assert(type_list<bool, int, bool>::find_first<type_list<int, bool>> == 1);
	static_assert(type_list<int, bool, int, bool>::find_first<type_list<int, bool>> == 0);
	static_assert(type_list<int, bool, int, bool>::find_first<type_list<bool, int>> == 1);
	static_assert(type_list<bool, int, bool, int, bool>::find_first<type_list<bool, int>> == 0);
	static_assert(type_list<int, float, bool, int, bool>::find_first<type_list<int, bool>> == 3);
}

namespace find_last_ {
	static_assert(type_list<>::find_last<type_list<>> == not_found);
	static_assert(type_list<>::find_last<type_list<int , bool>> == not_found);
	static_assert(type_list<int>::find_last<type_list<int, bool>> == not_found);
	static_assert(type_list<int, bool>::find_last<type_list<>> == not_found);
	static_assert(type_list<int, bool>::find_last<type_list<int>> == 0);
	static_assert(type_list<int, bool>::find_last<type_list<bool>> == 1);
	static_assert(type_list<int, bool>::find_last<type_list<int, bool>> == 0);
	static_assert(type_list<bool, int, bool>::find_last<type_list<int, bool>> == 1);
	static_assert(type_list<int, bool, int, bool>::find_last<type_list<int, bool>> == 2);
	static_assert(type_list<int, bool, int, bool>::find_last<type_list<bool, int>> == 1);
	static_assert(type_list<bool, int, bool, int, bool>::find_last<type_list<bool, int>> == 2);
	static_assert(type_list<int, float, bool, int, bool>::find_last<type_list<int, bool>> == 3);
}

namespace all_of_ {
	static_assert(type_list<                >::all_of<std::is_integral> == true);
	static_assert(type_list<int, long, short>::all_of<std::is_integral> == true);
	static_assert(type_list<int, float, char>::all_of<std::is_integral> == false);
}

namespace any_of_ {
	static_assert(type_list<                >::any_of<std::is_integral> == false);
	static_assert(type_list<int, long, short>::any_of<std::is_integral> == true);
	static_assert(type_list<int, float, char>::any_of<std::is_integral> == true);
}

namespace none_of_ {
	static_assert(type_list<                >::none_of<std::is_integral> == true);
	static_assert(type_list<int, long, short>::none_of<std::is_integral> == false);
	static_assert(type_list<int, float, char>::none_of<std::is_integral> == false);
}

namespace erase_at_ {
	static_assert(std::is_same_v<
		type_list<>::erase_at<0>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::erase_at<1>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, void>::erase_at<0>,
		type_list<float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, void>::erase_at<1>,
		type_list<int, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, void>::erase_at<2>,
		type_list<int, float>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, void>::erase_at<3>,
		type_list<int, float, void>
	>);
}

namespace erase_ {
	static_assert(std::is_same_v<
		type_list<>::erase<void>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void>::erase<void>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void, int>::erase<void>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<int, void>::erase<void>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<void, void>::erase<void>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int, void, void, int>::erase<void>,
		type_list<int, int>
	>);
	static_assert(std::is_same_v<
		type_list<int, void, void, int, void>::erase<void>,
		type_list<int, int>
	>);
}

namespace unique_ {
	static_assert(std::is_same_v<
		type_list<>::unique,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int, int, void, void, int, float>::unique,
		type_list<int, void, float>
	>);
}

namespace replace_at_ {
	static_assert(std::is_same_v<
		type_list<>::replace_at<0, int>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::replace_at<1, int>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void, void, void>::replace_at<0, int>,
		type_list<int, void, void>
	>);
	static_assert(std::is_same_v<
		type_list<void, void, void>::replace_at<1, int>,
		type_list<void, int, void>
	>);
	static_assert(std::is_same_v<
		type_list<void, void, void>::replace_at<2, int>,
		type_list<void, void, int>
	>);
	static_assert(std::is_same_v<
		type_list<void, void, void>::replace_at<3, int>,
		type_list<void, void, void>
	>);
}

namespace replace_ {
	static_assert(std::is_same_v<
		type_list<>::replace<void, float>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<float, int>::replace<void, float>,
		type_list<float, int>
	>);
	static_assert(std::is_same_v<
		type_list<float, int>::replace<float, void>,
		type_list<void, int>
	>);
	static_assert(std::is_same_v<
		type_list<float, int, float, int>::replace<void, float>,
		type_list<float, int, float, int>
	>);
	static_assert(std::is_same_v<
		type_list<float, int, float, int>::replace<float, void>,
		type_list<void, int, void, int>
	>);
}

namespace {
	template<typename>
	struct make_int final {
		using type = int;
	};
}

namespace transform_at_ {
	static_assert(std::is_same_v<
		type_list<>::transform_at<0, make_int>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::transform_at<0, make_int>,
		type_list<int, void>
	>);
	static_assert(std::is_same_v<
		type_list<float>::transform_at<0, std::add_const>,
		type_list<const float>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::transform_at<1, std::add_const>,
		type_list<float, const void>
	>);
	static_assert(std::is_same_v<
		type_list<float>::transform_at<0, std::add_pointer>,
		type_list<float *>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::transform_at<1, std::add_pointer>,
		type_list<float, void *>
	>);
}

namespace transform_ {
	static_assert(std::is_same_v<
		type_list<>::transform<make_int>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<float>::transform<make_int>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::transform<make_int>,
		type_list<int, int>
	>);
	static_assert(std::is_same_v<
		type_list<float>::transform<std::add_const>,
		type_list<const float>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::transform<std::add_const>,
		type_list<const float, const void>
	>);
	static_assert(std::is_same_v<
		type_list<float>::transform<std::add_pointer>,
		type_list<float *>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::transform<std::add_pointer>,
		type_list<float *, void *>
	>);
}

namespace reverse_ {
	static_assert(std::is_same_v<
		type_list<>::reverse,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, void>::reverse,
		type_list<void, float, int>
	>);
}

namespace count_ {
	static_assert(type_list<                           >::count<int   > == 0);
	static_assert(type_list<int, int, float, float, int>::count<int   > == 3);
	static_assert(type_list<int, int, float, float, int>::count<double> == 0);
}

namespace merge_ {
	static_assert(std::is_same_v<
		type_list<>::merge<type_list<>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int>::merge<type_list<>>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<>::merge<type_list<int>>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<int>::merge<type_list<float>>,
		type_list<int, float>
	>);
}

namespace includes_ {
	static_assert(type_list<>::includes<type_list<>>);
	static_assert(!type_list<>::includes<type_list<void>>);
	static_assert(type_list<void>::includes<type_list<>>);
	static_assert(type_list<int, void>::includes<type_list<void>>);
	static_assert(type_list<int, void>::includes<type_list<int>>);
	static_assert(!type_list<int, void>::includes<type_list<float>>);
	static_assert(!type_list<int, void>::includes<type_list<int, float>>);
	static_assert(!type_list<int, void>::includes<type_list<float, int>>);
}

namespace set_difference_ {
	static_assert(std::is_same_v<
		type_list<>::set_difference<type_list<>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::set_difference<type_list<void>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int>::set_difference<type_list<void>>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_difference<type_list<>>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_difference<type_list<int>>,
		type_list<float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_difference<type_list<float, int>>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_difference<type_list<float, int, double>>,
		type_list<void>
	>);
}

namespace set_union_ {
	static_assert(std::is_same_v<
		type_list<>::set_union<type_list<>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::set_union<type_list<void>>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<int>::set_union<type_list<void>>,
		type_list<int, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_union<type_list<>>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_union<type_list<int>>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_union<type_list<float, int>>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_union<type_list<float, int, double>>,
		type_list<int, float, void, double>
	>);
}

namespace set_intersection_ {
	static_assert(std::is_same_v<
		type_list<>::set_intersection<type_list<>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::set_intersection<type_list<void>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int>::set_intersection<type_list<void>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_intersection<type_list<>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_intersection<type_list<int>>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_intersection<type_list<float, int>>,
		type_list<int, float>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_intersection<type_list<float, int, double>>,
		type_list<int, float>
	>);
}

namespace set_symmetric_difference_ {
	static_assert(std::is_same_v<
		type_list<>::set_symmetric_difference<type_list<>>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::set_symmetric_difference<type_list<void>>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<int>::set_symmetric_difference<type_list<void>>,
		type_list<int, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_symmetric_difference<type_list<>>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_symmetric_difference<type_list<int>>,
		type_list<float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_symmetric_difference<type_list<float, int>>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<int, float, int, int, void>::set_symmetric_difference<type_list<float, int, double>>,
		type_list<void, double>
	>);
}

namespace equal_ {
	static_assert( type_list<               >::equal<type_list<               >>);
	static_assert( type_list<int, int, float>::equal<type_list<int, int, float>>);
	static_assert(!type_list<int, int       >::equal<type_list<int, int, float>>);
	static_assert(!type_list<int, int, float>::equal<type_list<int, int       >>);
	static_assert(!type_list<int, bool      >::equal<type_list<bool, int      >>);
	static_assert(!type_list<bool, int      >::equal<type_list<float, int     >>);

	namespace {
		template<typename Type1, typename Type2>
		using same_size = std::bool_constant<sizeof(Type1) == sizeof(Type2)>;
	}
	
	static_assert(!type_list<  signed int>::equal<type_list<unsigned int>>);
	static_assert( type_list<  signed int>::equal<type_list<unsigned int>, same_size>);
}

namespace mismatch_ {
	static_assert(type_list<>::mismatch<type_list<>> == not_found);
	static_assert(type_list<void>::mismatch<type_list<void>> == not_found);
	static_assert(type_list<void, int>::mismatch<type_list<void, int>> == not_found);

	static_assert(type_list<>::mismatch<type_list<int>> == 0);
	static_assert(type_list<void>::mismatch<type_list<void, int>> == 1);
	static_assert(type_list<void, int>::mismatch<type_list<void, int, float>> == 2);

	static_assert(type_list<int>::mismatch<type_list<>> == 0);
	static_assert(type_list<void, int>::mismatch<type_list<void>> == 1);
	static_assert(type_list<void, int, float>::mismatch<type_list<void, int>> == 2);

	static_assert(type_list<int, void, float>::mismatch<type_list<int, float, void>> == 1);
}

namespace copy_if_ {
	static_assert(std::is_same_v<
		type_list<>::copy_if<std::is_integral>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void, float>::copy_if<std::is_integral>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void, float, int, unsigned>::copy_if<std::is_integral>,
		type_list<int, unsigned>
	>);
}

namespace fill_ {
	static_assert(std::is_same_v<
		type_list<>::fill<int, 0>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::fill<int, 1>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<>::fill<int, 2>,
		type_list<int, int>
	>);
	static_assert(std::is_same_v<
		type_list<>::fill<int, 3>,
		type_list<int, int, int>
	>);
	static_assert(std::is_same_v<
		type_list<void>::fill<int, 0>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void>::fill<int>,
		type_list<int>
	>);
	static_assert(std::is_same_v<
		type_list<void, void>::fill<int>,
		type_list<int, int>
	>);
}

namespace shift_left_ {
	static_assert(std::is_same_v<
		type_list<>::shift_left<0>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::shift_left<1>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void>::shift_left<0>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<void>::shift_left<1>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_left<0>,
		type_list<void, int, float>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_left<1>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_left<2>,
		type_list<float, void, int>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_left<3>,
		type_list<void, int, float>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_left<4>,
		type_list<void, int, float>
	>);
}

namespace shift_right_ {
	static_assert(std::is_same_v<
		type_list<>::shift_right<0>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<>::shift_right<1>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<void>::shift_right<0>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<void>::shift_right<1>,
		type_list<void>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_right<0>,
		type_list<void, int, float>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_right<1>,
		type_list<float, void, int>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_right<2>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_right<3>,
		type_list<void, int, float>
	>);
	static_assert(std::is_same_v<
		type_list<void, int, float>::shift_right<4>,
		type_list<void, int, float>
	>);
}

namespace partition_ {
	static_assert(std::is_same_v<
		type_list<>::partition<std::is_integral>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<float>::partition<std::is_integral>,
		type_list<float>
	>);
	static_assert(std::is_same_v<
		type_list<float, void>::partition<std::is_integral>,
		type_list<float, void>
	>);
	static_assert(std::is_same_v<
		type_list<int, unsigned>::partition<std::is_integral>,
		type_list<int, unsigned>
	>);
	static_assert(std::is_same_v<
		type_list<float, void, int>::partition<std::is_integral>,
		type_list<int, float, void>
	>);
	static_assert(std::is_same_v<
		type_list<unsigned, float, void, int>::partition<std::is_integral>,
		type_list<unsigned, int, float, void>
	>);
}

namespace is_partitioned_ {
	static_assert(type_list<>::is_partitioned<std::is_integral>);
	static_assert(type_list<float>::is_partitioned<std::is_integral>);
	static_assert(type_list<float, void>::is_partitioned<std::is_integral>);
	static_assert(type_list<int, unsigned>::is_partitioned<std::is_integral>);
	static_assert(!type_list<float, void, int>::is_partitioned<std::is_integral>);
	static_assert(type_list<int, float, void>::is_partitioned<std::is_integral>);
	static_assert(type_list<unsigned, int, float, void>::is_partitioned<std::is_integral>);
}

namespace partition_point_ {
	static_assert(type_list<>::partition_point<std::is_integral> == 0);
	static_assert(type_list<float>::partition_point<std::is_integral> == 0);
	static_assert(type_list<float, void>::partition_point<std::is_integral> == 0);
	static_assert(type_list<int, unsigned>::partition_point<std::is_integral> == 2);
	static_assert(type_list<int, float, void>::partition_point<std::is_integral> == 1);
	static_assert(type_list<unsigned, int, float, void>::partition_point<std::is_integral> == 2);
}

namespace {
	template<typename T1, typename T2>
	using smaller_type = std::bool_constant<(sizeof(T1) < sizeof(T2))>;
}

namespace sort_ {
	static_assert(std::is_same_v<
		type_list<>::sort<smaller_type>,
		type_list<>
	>);
	static_assert(std::is_same_v<
		type_list<std::int8_t>::sort<smaller_type>,
		type_list<std::int8_t>
	>);
	static_assert(std::is_same_v<
		type_list<std::int8_t, std::int16_t>::sort<smaller_type>,
		type_list<std::int8_t, std::int16_t>
	>);
	static_assert(std::is_same_v<
		type_list<std::int16_t, std::int8_t>::sort<smaller_type>,
		type_list<std::int8_t, std::int16_t>
	>);
	static_assert(std::is_same_v<
		type_list<std::int32_t, std::int16_t, std::int8_t>::sort<smaller_type>,
		type_list<std::int8_t, std::int16_t, std::int32_t>
	>);
	static_assert(std::is_same_v<
		type_list<std::int32_t, std::int16_t, std::int64_t, std::int8_t>::sort<smaller_type>,
		type_list<std::int8_t, std::int16_t, std::int32_t, std::int64_t>
	>);
	static_assert(std::is_same_v<
		type_list<std::int32_t, std::int16_t, std::int8_t, std::int64_t, std::int8_t>::sort<smaller_type>,
		type_list<std::int8_t, std::int8_t, std::int16_t, std::int32_t, std::int64_t>
	>);
}

namespace is_sorted_ {
	static_assert(type_list<>::is_sorted<smaller_type>);
	static_assert(type_list<std::int8_t>::is_sorted<smaller_type>);
	static_assert(type_list<std::int8_t, std::int8_t>::is_sorted<smaller_type>);
	static_assert(type_list<std::int8_t, std::int16_t>::is_sorted<smaller_type>);
	static_assert(type_list<std::int8_t, std::int16_t, std::int16_t>::is_sorted<smaller_type>);
	static_assert(!type_list<std::int64_t, std::int8_t>::is_sorted<smaller_type>);
	static_assert(!type_list<std::int64_t, std::int8_t, std::int8_t>::is_sorted<smaller_type>);
}

namespace min_element_ {
	static_assert(type_list<>::min_element<smaller_type> == not_found);
	static_assert(type_list<std::int16_t>::min_element<smaller_type> == 0);
	static_assert(type_list<std::int16_t, std::int8_t>::min_element<smaller_type> == 1);
	static_assert(type_list<std::int16_t, std::int8_t, std::int32_t>::min_element<smaller_type> == 1);
	static_assert(type_list<std::int16_t, std::int8_t, std::int32_t, std::int8_t>::min_element<smaller_type> == 1);
}

namespace max_element_ {
	static_assert(type_list<>::max_element<smaller_type> == not_found);
	static_assert(type_list<std::int16_t>::max_element<smaller_type> == 0);
	static_assert(type_list<std::int16_t, std::int8_t>::max_element<smaller_type> == 0);
	static_assert(type_list<std::int16_t, std::int8_t, std::int32_t>::max_element<smaller_type> == 2);
	static_assert(type_list<std::int16_t, std::int8_t, std::int32_t, std::int8_t>::max_element<smaller_type> == 2);
	static_assert(type_list<std::int16_t, std::int8_t, std::int32_t, std::int8_t, std::int32_t>::max_element<smaller_type> == 4);
}
