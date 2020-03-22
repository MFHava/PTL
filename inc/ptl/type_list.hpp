
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <cstddef>

namespace ptl {
	template<typename... Types>
	struct type_list;

	inline
	constexpr
	std::ptrdiff_t not_found{-1};
}

#include "internal/type_list.hpp"

namespace ptl {
	template<typename... Types>
	struct type_list final {
		static
		constexpr
		std::ptrdiff_t size{internal::TL::size<type_list>::value};

		static
		constexpr
		bool empty{size == 0};


		template<std::ptrdiff_t Index>
		using at = typename internal::TL::at<type_list, Index>::type;


		template<std::ptrdiff_t Index, typename ToInsert>
		using insert = typename internal::TL::insert<type_list, Index, ToInsert>::type;


		template<typename ToPrepend>
		using push_front = typename internal::TL::push_front<type_list, ToPrepend>::type;

		template<typename ToAppend>
		using push_back = typename internal::TL::push_back<type_list, ToAppend>::type;


		template<std::ptrdiff_t First = 0, std::ptrdiff_t Count = size - First>
		using subset = typename internal::TL::subset<type_list, First, Count>::type;


		template<typename ToFind>
		static
		constexpr
		std::ptrdiff_t find{internal::TL::find<type_list, ToFind>::value};

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		std::ptrdiff_t find_if{internal::TL::find_if<type_list, UnaryPredicate>::value};

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		std::ptrdiff_t find_if_not{internal::TL::find_if_not<type_list, UnaryPredicate>::value};


		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		static
		constexpr
		std::ptrdiff_t find_first_of{internal::TL::find_first_of<type_list, TypeList, BinaryPredicate>::value};

		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		static
		constexpr
		std::ptrdiff_t find_last_of{internal::TL::find_last_of<type_list, TypeList, BinaryPredicate>::value};


		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		static
		constexpr
		std::ptrdiff_t find_first{internal::TL::find_first<type_list, TypeList, BinaryPredicate>::value};

		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		static
		constexpr
		std::ptrdiff_t find_last{internal::TL::find_last<type_list, TypeList, BinaryPredicate>::value};


		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		bool all_of{find_if_not<UnaryPredicate> == not_found};

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		bool any_of{find_if<UnaryPredicate> != not_found};

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		bool none_of{find_if<UnaryPredicate> == not_found};


		template<std::ptrdiff_t Index>
		using erase_at = typename internal::TL::erase_at<type_list, Index>::type;

		template<typename ToRemove>
		using erase = typename internal::TL::erase<type_list, ToRemove>::type;

		template<template<typename> typename UnaryPredicate>
		using erase_if = typename internal::TL::erase_if<type_list, UnaryPredicate>::type;


		using pop_front = erase_at<0>;
		using pop_back  = erase_at<size - 1>;


		template<template<typename, typename> typename BinaryPredicate>
		using unique_if = typename internal::TL::unique_if<type_list, BinaryPredicate>::type;

		using unique = unique_if<std::is_same>;


		template<std::ptrdiff_t Index, typename Replacement>
		using replace_at = typename internal::TL::replace_at<type_list, Index, Replacement>::type;

		template<typename ToReplace, typename Replacement>
		using replace = typename internal::TL::replace<type_list, ToReplace, Replacement>::type;

		template<template<typename> typename UnaryPredicate, typename Replacement>
		using replace_if = typename internal::TL::replace_if<type_list, UnaryPredicate, Replacement>::type;


		template<std::ptrdiff_t Index, template<typename> typename UnaryFunction>
		using transform_at = typename internal::TL::transform_at<type_list, Index, UnaryFunction>::type;

		template<template<typename> typename UnaryFunction>
		using transform = typename internal::TL::transform<type_list, UnaryFunction>::type;

		template<template<typename> typename UnaryPredicate, template<typename> typename UnaryFunction>
		using transform_if = typename internal::TL::transform_if<type_list, UnaryPredicate, UnaryFunction>::type;


		using reverse = typename internal::TL::reverse<type_list>::type;


		template<typename Type>
		static
		constexpr
		std::ptrdiff_t count{internal::TL::count<type_list, Type>::value};

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		std::ptrdiff_t count_if{internal::TL::count_if<type_list, UnaryPredicate>::value};


		template<typename TypeList>
		static
		constexpr
		std::ptrdiff_t mismatch{internal::TL::mismatch<type_list, TypeList>::value};

		template<typename TypeList, template<typename, typename> typename BinaryPredicate>
		static
		constexpr
		std::ptrdiff_t mismatch_if{internal::TL::mismatch_if<type_list, TypeList, BinaryPredicate>::value};


		template<template<typename> typename UnaryPredicate>
		using copy_if = typename internal::TL::copy_if<type_list, UnaryPredicate>::type;


		template<typename Type, std::ptrdiff_t Count = size>
		using fill = typename internal::TL::fill<type_list<>, Type, Count>::type;


		template<std::ptrdiff_t Count>
		using shift_left = typename internal::TL::shift_left<type_list, Count>::type;

		template<std::ptrdiff_t Count>
		using shift_right = typename internal::TL::shift_right<type_list, Count>::type;


		template<template<typename> typename UnaryPredicate>
		using partition = typename internal::TL::partition<type_list, UnaryPredicate>::type;

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		bool is_partitioned{internal::TL::is_partitioned<type_list, UnaryPredicate>::value};

		template<template<typename> typename UnaryPredicate>
		static
		constexpr
		std::ptrdiff_t partition_point{internal::TL::partition_point<type_list, UnaryPredicate>::value};


		template<template<typename, typename> typename BinaryPredicate>
		using sort = typename internal::TL::sort<type_list, BinaryPredicate>::type;

		template<template<typename, typename> typename BinaryPredicate>
		static
		constexpr
		bool is_sorted{internal::TL::is_sorted<type_list, BinaryPredicate>::value};


		template<typename TypeList>
		using merge = typename internal::TL::merge<type_list, TypeList>::type;


		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		static
		constexpr
		bool includes{internal::TL::includes<type_list, TypeList, BinaryPredicate>::value};

		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		using set_difference = typename internal::TL::set_difference<type_list, TypeList, BinaryPredicate>::type;

		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		using set_union = typename internal::TL::set_union<type_list, TypeList, BinaryPredicate>::type;

		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		using set_intersection = typename internal::TL::set_intersection<type_list, TypeList, BinaryPredicate>::type;

		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		using set_symmetric_difference = typename internal::TL::set_symmetric_difference<type_list, TypeList, BinaryPredicate>::type;


		template<template<typename, typename> typename BinaryPredicate>
		static
		constexpr
		std::ptrdiff_t min_element{internal::TL::min_element<type_list, BinaryPredicate>::value};

		template<template<typename, typename> typename BinaryPredicate>
		static
		constexpr
		std::ptrdiff_t max_element{internal::TL::max_element<type_list, BinaryPredicate>::value};


		template<typename TypeList, template<typename, typename> typename BinaryPredicate = std::is_same>
		static
		constexpr
		bool equal{internal::TL::equal<type_list, TypeList, BinaryPredicate>::value};
	};
}
