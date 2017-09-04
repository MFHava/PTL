
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "ptl/array_ref.hpp"

#include <array>
#include <vector>

namespace {
	void test() {
		using namespace ptl;

		std::vector<int> vec0;
		array_ref<int> ref00{vec0};
		array_ref<const int> ref01{vec0};

		const std::vector<int> vec1;
		//array_ref<int> ref10{vec1};
		array_ref<const int> ref11{vec1};


		std::array<int, 3> arr0;
		array_ref<int> ref20{arr0};
		array_ref<const int> ref21{arr0};

		const std::array<int, 3> arr1{{0, 0, 0}};
		//array_ref<int> ref30{arr1};
		array_ref<const int> ref31{arr1};

		std::array<const int, 3> arr2{{0, 0, 0}};
		//array_ref<int> ref40{arr2};
		array_ref<const int> ref41{arr2};

		const std::array<const int, 3> arr3{{0, 0, 0}};
		//array_ref<int> ref50{arr3};
		array_ref<const int> ref51{arr3};

		int car0[3];
		array_ref<int> ref60{car0};
		array_ref<const int> ref61{car0};

		const int car1[3]{0, 0, 0};
		//array_ref<int> ref70{car1};
		array_ref<const int> ref71{car1};

		array_ref<int> ref80 = ref60;
		//array_ref<int> ref81 = ref61;
		array_ref<const int> ref82 = ref60;
		array_ref<const int> ref83 = ref61;

		array_ref<const int> ref84 = std::move(ref60);
		array_ref<const int> ref85 = std::move(ref61);
	}
}