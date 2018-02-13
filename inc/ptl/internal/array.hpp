
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>

namespace ptl {
	namespace internal {
		template<typename Type, std::size_t Size>
		struct array_storage final { using type = Type[Size]; };

		template<typename Type>
		struct array_storage<Type, 0> final { using type = Type *; };

		template<typename Type, std::size_t Size>
		using array_storage_t = typename array_storage<Type, Size>::type;
	}
}
