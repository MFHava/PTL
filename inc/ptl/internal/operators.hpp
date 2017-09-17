
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace ptl {
	namespace internal {
		struct empty_base {};

		template<typename Type1, typename Type2, typename Base = empty_base>
		class comparison2 : Base {
			friend
			auto operator!=(const Type1 & lhs, const Type2 & rhs) noexcept { return !(lhs == rhs); }
			friend
			auto operator<=(const Type1 & lhs, const Type2 & rhs) noexcept { return !(rhs < lhs); }
			friend
			auto operator> (const Type1 & lhs, const Type2 & rhs) noexcept { return rhs < lhs; }
			friend
			auto operator>=(const Type1 & lhs, const Type2 & rhs) noexcept { return !(lhs < rhs); }
		};

		template<typename Type, typename Base = empty_base>
		class comparison1 : comparison2<Type, Type, Base> {};

		template<typename Type1, typename Type2, typename Base = empty_base>
		class comparison2_symmetric : comparison2<Type1, Type2, comparison2<Type2, Type1, Base>> {};
	}
}