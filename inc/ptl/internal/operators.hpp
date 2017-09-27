
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#define PTL_OPERATOR_POSTFIX(name, op)\
	template<typename Type, typename Base = empty_base>\
	class postfix_##name: Base {\
		friend\
		constexpr\
		auto operator op##op(Type & self, int) noexcept {\
			auto tmp{self};\
			op##op self;\
			return tmp;\
		}\
	}

#define PTL_OPERATOR_BINARY_OP(name, op)\
	template<typename Type1, typename Type2, typename Base = empty_base>\
	class name##2 : Base {\
		friend\
		constexpr\
		auto operator op(Type1 lhs, const Type2 & rhs) noexcept {\
			lhs op##= rhs;\
			return lhs;\
		}\
	};\
	template<typename Type, typename Base = empty_base>\
	class name##1 : name##2<Type, Type, Base> {}


namespace ptl {
	namespace internal {
		struct empty_base {};

		template<typename Type1, typename Type2, typename Base = empty_base>
		class comparison2 : Base {
			friend
			constexpr
			auto operator!=(const Type1 & lhs, const Type2 & rhs) noexcept { return !(lhs == rhs); }
			friend
			constexpr
			auto operator<=(const Type1 & lhs, const Type2 & rhs) noexcept { return !(rhs < lhs); }
			friend
			constexpr
			auto operator> (const Type1 & lhs, const Type2 & rhs) noexcept { return rhs < lhs; }
			friend
			constexpr
			auto operator>=(const Type1 & lhs, const Type2 & rhs) noexcept { return !(lhs < rhs); }
		};

		template<typename Type, typename Base = empty_base>
		class comparison1 : comparison2<Type, Type, Base> {};

		template<typename Type1, typename Type2, typename Base = empty_base>
		class comparison2_symmetric : comparison2<Type1, Type2, comparison2<Type2, Type1, Base>> {
			friend
			constexpr
			auto operator==(const Type1 & lhs, const Type2 & rhs) noexcept { return (rhs == lhs); }
			friend
			constexpr
			auto operator< (const Type1 & lhs, const Type2 & rhs) noexcept { return (rhs >= lhs); }
		};

		PTL_OPERATOR_POSTFIX(inc, +);
		PTL_OPERATOR_POSTFIX(dec, -);

		template<typename Type, typename Base = empty_base>
		class postfix : postfix_inc<Type, postfix_dec<Type, Base>> {};

		PTL_OPERATOR_BINARY_OP(additive,    +);
		PTL_OPERATOR_BINARY_OP(subtractive, -);
	}
}

#undef PTL_OPERATOR_BINARY_OP
#undef PTL_OPERATOR_POSTFIX