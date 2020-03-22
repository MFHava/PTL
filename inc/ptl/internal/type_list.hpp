
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl::internal::TL {
	template<std::ptrdiff_t Value>
	using index_constant = std::integral_constant<std::ptrdiff_t, Value>;

	using invalid_index = index_constant<not_found>;
	using zero_index = index_constant<0>;


	template<typename TypeList>
	struct size;

	template<typename... Types>
	struct size<type_list<Types...>> : index_constant<sizeof...(Types)> {};


	template<typename TypeList, std::ptrdiff_t Index>
	struct at;

	template<typename Head, typename... Tail>
	struct at<type_list<Head, Tail...>, 0> {
		using type = Head;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Index>
	struct at<type_list<Head, Tail...>, Index> {
		using type = typename at<
			type_list<Tail...>,
			Index - 1
		>::type;
	};


	template<typename TypeList1, typename TypeList2>
	struct merge;

	template<typename... Types1, typename... Types2>
	struct merge<type_list<Types1...>, type_list<Types2...>> {
		using type = type_list<Types1..., Types2...>;
	};


	template<typename TypeList, typename ToAppend>
	struct push_back {
		using type = typename merge<
			TypeList,
			type_list<ToAppend>
		>::type;
	};


	template<typename TypeList, typename ToPrepend>
	struct push_front {
		using type = typename merge<
			type_list<ToPrepend>,
			TypeList
		>::type;
	};


	template<typename TypeList>
	struct reverse {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail>
	struct reverse<type_list<Head, Tail...>> {
		using type = typename push_back<
			typename reverse<
				type_list<Tail...>
			>::type,
			Head
		>::type;
	};


	template<typename TypeList, std::ptrdiff_t First, std::ptrdiff_t Count, bool DoNothing = (First < 0 || Count <= 0)>
	struct subset {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Count>
	struct subset<type_list<Head, Tail...>, 0, Count, false> {
		using type = typename push_front<
			typename subset<
				type_list<Tail...>,
				0,
				Count - 1
			>::type,
			Head
		>::type;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t First, std::ptrdiff_t Count>
	struct subset<type_list<Head, Tail...>, First, Count, false> {
		using type = typename subset<
			type_list<Tail...>,
			First - 1,
			Count
		>::type;
	};


	template<typename TypeList, std::ptrdiff_t Index, typename ToInsert>
	struct insert {
		using type = type_list<ToInsert>;
	};

	template<typename Head, typename... Tail, typename ToInsert>
	struct insert<type_list<Head, Tail...>, 0, ToInsert> {
		using type = type_list<ToInsert, Head, Tail...>;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Index, typename ToInsert>
	struct insert<type_list<Head, Tail...>, Index, ToInsert> {
		using type = typename push_front<
			typename insert<
				type_list<Tail...>,
				Index - 1,
				ToInsert
			>::type,
			Head
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate>
	struct find_if : invalid_index {};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate>
	struct find_if<type_list<Head, Tail...>, UnaryPredicate> {
	private:
		static
		constexpr
		bool match{UnaryPredicate<Head>::value};

		static
		constexpr
		std::ptrdiff_t tmp{find_if<type_list<Tail...>, UnaryPredicate>::value};
	public:
		static
		constexpr
		std::ptrdiff_t value{match ? 0 : tmp == not_found ? not_found : 1 + tmp};
	};


	template<typename TypeList, typename ToFind>
	struct find {
	private:
		template<typename T>
		using Predicate = std::is_same<T, ToFind>;
	public:
		static
		constexpr
		std::ptrdiff_t value{find_if<TypeList, Predicate>::value};
	};


	template<typename TypeList, template<typename> typename UnaryPredicate>
	struct find_if_not {
	private:
		template<typename T>
		using Predicate = std::bool_constant<!UnaryPredicate<T>::value>;
	public:
		static
		constexpr
		std::ptrdiff_t value{find_if<TypeList, Predicate>::value};
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct find_first_of {
	private:
		template<typename T>
		struct MetaPredicate {
		private:
			template<typename ToFind>
			using Predicate = BinaryPredicate<T, ToFind>;
		public:
			static
			constexpr
			bool value{find_if<TypeList2, Predicate>::value != not_found};
		};
	public:
		static
		constexpr
		std::ptrdiff_t value{find_if<TypeList1, MetaPredicate>::value};
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct find_last_of {
	private:
		static
		constexpr
		std::ptrdiff_t tmp{
			find_first_of<
				typename reverse<
					TypeList1
				>::type,
				typename reverse<
					TypeList2
				>::type,
				BinaryPredicate
			>::value
		};
	public:
		static
		constexpr
		std::ptrdiff_t value{tmp == not_found ? not_found : size<TypeList1>::value - tmp - 1};
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate, std::ptrdiff_t Index = 0, bool Matching = false>
	struct find_first : index_constant<Matching && (size<TypeList1>::value || !size<TypeList2>::value) ? Index : not_found> {};

	template<typename Head1, typename... Tail1, typename Head2, typename... Tail2, template<typename, typename> typename BinaryPredicate, std::ptrdiff_t Index>
	struct find_first<type_list<Head1, Tail1...>, type_list<Head2, Tail2...>, BinaryPredicate, Index, true> {
		static
		constexpr
		std::ptrdiff_t value{BinaryPredicate<Head1, Head2>::value ? find_first<type_list<Tail1...>, type_list<Tail2...>, BinaryPredicate, Index, true>::value : not_found};
	};

	template<typename Head1, typename... Tail1, typename Head2, typename... Tail2, template<typename, typename> typename BinaryPredicate, std::ptrdiff_t Index>
	struct find_first<type_list<Head1, Tail1...>, type_list<Head2, Tail2...>, BinaryPredicate, Index, false> {
	private:
		static
		constexpr
		bool tmp{BinaryPredicate<Head1, Head2>::value && find_first<type_list<Tail1...>, type_list<Tail2...>, BinaryPredicate, Index, true>::value != not_found};
	public:
		static
		constexpr
		std::ptrdiff_t value{tmp ? Index : find_first<type_list<Tail1...>, type_list<Head2, Tail2...>, BinaryPredicate, Index + 1, false>::value};
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct find_last {
	private:
		static
		constexpr
		std::ptrdiff_t tmp{find_first<typename reverse<TypeList1>::type, typename reverse<TypeList2>::type, BinaryPredicate>::value};
	public:
		static
		constexpr
		std::ptrdiff_t value{tmp == not_found ? not_found : size<TypeList1>::value - tmp - size<TypeList2>::value};
	};


	template<typename TypeList, std::ptrdiff_t Index>
	struct erase_at {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail>
	struct erase_at<type_list<Head, Tail...>, 0> {
		using type = type_list<Tail...>;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Index>
	struct erase_at<type_list<Head, Tail...>, Index> {
		using type = typename push_front<
			typename erase_at<
				type_list<Tail...>,
				Index - 1
			>::type,
			Head
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate>
	struct erase_if {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate>
	struct erase_if<type_list<Head, Tail...>, UnaryPredicate> {
	private:
		static
		constexpr
		bool match{UnaryPredicate<Head>::value};

		using tail = typename erase_if<
			type_list<Tail...>,
			UnaryPredicate
		>::type;
	public:
		using type = std::conditional_t<
			match,
			tail,
			typename push_front<
				tail,
				Head
			>::type
		>;
	};


	template<typename TypeList, typename ToRemove>
	struct erase {
	private:
		template<typename T>
		using Predicate = std::is_same<T, ToRemove>;
	public:
		using type = typename erase_if<
			TypeList,
			Predicate
		>::type;
	};


	template<typename TypeList, template<typename, typename> typename BinaryPredicate>
	struct unique_if {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, template<typename, typename> typename BinaryPredicate>
	struct unique_if<type_list<Head, Tail...>, BinaryPredicate> {
	private:
		template<typename T>
		using Predicate = BinaryPredicate<T, Head>;
	public:
		using type = typename push_front<
			typename erase_if<
				typename unique_if<
					type_list<Tail...>,
					BinaryPredicate
				>::type,
				Predicate
			>::type,
			Head
		>::type;
	};


	template<typename TypeList, std::ptrdiff_t Index, typename Replacement>
	struct replace_at {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, typename Replacement>
	struct replace_at<type_list<Head, Tail...>, 0, Replacement> {
		using type = type_list<Replacement, Tail...>;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Index, typename Replacement>
	struct replace_at<type_list<Head, Tail...>, Index, Replacement> {
		using type = typename push_front<
			typename replace_at<
				type_list<Tail...>,
				Index - 1,
				Replacement
			>::type,
			Head
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate, typename Replacement>
	struct replace_if {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate, typename Replacement>
	struct replace_if<type_list<Head, Tail...>, UnaryPredicate, Replacement> {
	private:
		static
		constexpr
		bool match{UnaryPredicate<Head>::value};

		using head = std::conditional_t<
			match,
			Replacement,
			Head
		>;

		using tail = typename replace_if<
			type_list<Tail...>,
			UnaryPredicate,
			Replacement
		>::type;
	public:
		using type = typename push_front<
			tail,
			head
		>::type;
	};


	template<typename TypeList, typename ToReplace, typename Replacement>
	struct replace {
	private:
		template<typename T>
		struct Predicate : std::is_same<T, ToReplace> {};
	public:
		using type = typename replace_if<
			TypeList,
			Predicate,
			Replacement
		>::type;
	};


	template<typename TypeList, std::ptrdiff_t Index, template<typename> typename UnaryFunction>
	struct transform_at {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, template<typename> typename UnaryFunction>
	struct transform_at<type_list<Head, Tail...>, 0, UnaryFunction> {
		using type = type_list<
			typename UnaryFunction<Head>::type,
			Tail...
		>;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Index, template<typename> typename UnaryFunction>
	struct transform_at<type_list<Head, Tail...>, Index, UnaryFunction> {
		using type = typename push_front<
			typename transform_at<
				type_list<Tail...>,
				Index - 1,
				UnaryFunction
			>::type,
			Head
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate, template<typename> typename UnaryFunction>
	struct transform_if {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate, template<typename> typename UnaryFunction>
	struct transform_if<type_list<Head, Tail...>, UnaryPredicate, UnaryFunction> {
	private:
		static
		constexpr
		bool match{UnaryPredicate<Head>::value};

		using head = std::conditional_t<
			match,
			typename UnaryFunction<Head>::type,
			Head
		>;
	public:
		using type = typename push_front<
			typename transform_if<
				type_list<Tail...>,
				UnaryPredicate,
				UnaryFunction
			>::type,
			head
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryFunction>
	struct transform {
	private:
		template<typename>
		using Predicate = std::true_type;
	public:
		using type = typename transform_if<
			TypeList,
			Predicate,
			UnaryFunction
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate>
	struct count_if : zero_index {};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate>
	struct count_if<type_list<Head, Tail...>, UnaryPredicate> : index_constant<count_if<type_list<Tail...>, UnaryPredicate>::value + (UnaryPredicate<Head>::value ? 1 : 0)> {};


	template<typename TypeList, typename Type>
	struct count {
	private:
		template<typename T>
		using Predicate = std::is_same<T, Type>;
	public:
		static
		constexpr
		std::ptrdiff_t value{count_if<TypeList, Predicate>::value};
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct mismatch_if : std::conditional_t<size<TypeList1>::value || size<TypeList2>::value, zero_index, invalid_index> {};

	template<typename Head1, typename... Tail1, typename Head2, typename... Tail2, template<typename, typename> typename BinaryPredicate>
	struct mismatch_if<type_list<Head1, Tail1...>, type_list<Head2, Tail2...>, BinaryPredicate> {
	private:
		static
		constexpr
		bool match{BinaryPredicate<Head1, Head2>::value};

		static
		constexpr
		std::ptrdiff_t tmp{mismatch_if<type_list<Tail1...>, type_list<Tail2...>, BinaryPredicate>::value};
	public:
		static
		constexpr
		std::ptrdiff_t value{match ? 0 : tmp == not_found ? not_found : tmp+ 1};
	};


	template<typename TypeList1, typename TypeList2>
	struct mismatch {
	private:
		template<typename Type1, typename Type2>
		using Predicate = std::bool_constant<!std::is_same_v<Type1, Type2>>;
	public:
		static
		constexpr
		std::ptrdiff_t value{mismatch_if<TypeList1, TypeList2, Predicate>::value};
	};


	template<typename TypeList, template<typename> typename UnaryPredicate>
	struct copy_if {
		using type = type_list<>;
	};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate>
	struct copy_if<type_list<Head, Tail...>, UnaryPredicate> {
	private:
		static
		constexpr
		bool match{UnaryPredicate<Head>::value};

		using tail = typename copy_if<
			type_list<Tail...>,
			UnaryPredicate
		>::type;
	public:
		using type = std::conditional_t<
			match,
			typename push_front<
				tail,
				Head
			>::type,
			tail
		>;
	};


	template<typename TypeList, typename Type, std::ptrdiff_t Count>
	struct fill {
		using type = typename push_front<
			typename fill<
				TypeList,
				Type,
				Count - 1
			>::type,
			Type
		>::type;
	};

	template<typename TypeList, typename Type>
	struct fill<TypeList, Type, 0> {
		using type = TypeList;
	};


	template<typename TypeList, std::ptrdiff_t Count, bool DoNothing = (Count <= 0 || Count >= size<TypeList>::value)>
	struct shift_left {
		using type = TypeList;
	};

	template<typename Head, typename... Tail, std::ptrdiff_t Count>
	struct shift_left<type_list<Head, Tail...>, Count, false> {
		using type = typename shift_left<
			type_list<Tail..., Head>,
			Count - 1
		>::type;
	};


	template<typename TypeList, std::ptrdiff_t Count>
	struct shift_right {
		using type = typename reverse<
			typename shift_left<
				typename reverse<TypeList>::type,
				Count
			>::type
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate, typename TrueList = type_list<>, typename FalseList = type_list<>>
	struct partition {
		using type = typename merge<
			TrueList,
			FalseList
		>::type;
	};

	template<typename Head, typename... Tail, template<typename> typename UnaryPredicate, typename TrueList, typename FalseList>
	struct partition<type_list<Head, Tail...>, UnaryPredicate, TrueList, FalseList> {
		static
		constexpr
		bool match{UnaryPredicate<Head>::value};

		using tmp = typename push_back<
			std::conditional_t<
				match,
				TrueList,
				FalseList
			>,
			Head
		>::type;
	public:
		using type = typename partition<
			type_list<Tail...>,
			UnaryPredicate,
			std::conditional_t<
				match,
				tmp,
				TrueList
			>,
			std::conditional_t<
				match,
				FalseList,
				tmp
			>
		>::type;
	};


	template<typename TypeList, template<typename> typename UnaryPredicate>
	using is_partitioned = std::bool_constant<
		std::is_same_v<
			TypeList,
			typename partition<
				TypeList,
				UnaryPredicate
			>::type
		>
	>;


	template<typename TypeList, template<typename> typename UnaryPredicate>
	struct partition_point {
	private:
		static_assert(is_partitioned<TypeList, UnaryPredicate>::value);

		template<typename T>
		using Predicate = std::bool_constant<!UnaryPredicate<T>::value>;

		static
		constexpr
		std::ptrdiff_t index{find_if<TypeList, Predicate>::value};
	public:
		static
		constexpr
		std::ptrdiff_t value{index != not_found ? index : size<TypeList>::value};
	};


	template<typename ToSort, template<typename, typename> typename BinaryPredicate, typename Sorted = type_list<>>
	struct sort {
		using type = Sorted;
	};

	template<typename Head, typename... Tail, template<typename, typename> typename BinaryPredicate, typename Sorted>
	struct sort<type_list<Head, Tail...>, BinaryPredicate, Sorted> {
	private:
		template<typename T>
		using Predicate = std::bool_constant<BinaryPredicate<T, Head>::value>;

		static
		constexpr
		std::ptrdiff_t tmp{find_if_not<Sorted, Predicate>::value}, index{tmp == not_found ? size<Sorted>::value : tmp};
	public:
		using type = typename sort<
			type_list<Tail...>,
			BinaryPredicate,
			typename insert<
				Sorted,
				index,
				Head
			>::type
		>::type;
	};


	template<typename TypeList, template<typename, typename> typename BinaryPredicate>
	struct is_sorted : std::true_type {};

	template<typename Type1, typename Type2, typename... Tail, template<typename, typename> typename BinaryPredicate>
	struct is_sorted<type_list<Type1, Type2, Tail...>, BinaryPredicate> : std::bool_constant<BinaryPredicate<Type2, Type1>::value ? false : is_sorted<type_list<Type2, Tail...>, BinaryPredicate>::value> {};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct includes : std::true_type {};

	template<typename TypeList, typename Head, typename... Tail, template<typename, typename> typename BinaryPredicate>
	struct includes<TypeList, type_list<Head, Tail...>, BinaryPredicate> {
	private:
		template<typename T>
		using Predicate = BinaryPredicate<T, Head>;
	public:
		static
		constexpr
		bool value{(find_if<TypeList, Predicate>::value == not_found) ? false : includes<TypeList, type_list<Tail...>, BinaryPredicate>::value};
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct set_difference {
	private:
		template<typename T>
		struct MetaPredicate {
			template<typename ToFind>
			using Predicate = BinaryPredicate<T, ToFind>;
		public:
			static
			constexpr
			bool value{find_if<TypeList2, Predicate>::value != not_found};
		};
	public:
		using type = typename unique_if<
			typename erase_if<
				TypeList1,
				MetaPredicate
			>::type,
			BinaryPredicate
		>::type;
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct set_union {
		using type = typename unique_if<
			typename merge<
				TypeList1,
				TypeList2
			>::type,
			BinaryPredicate
		>::type;
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct set_intersection {
	private:
		template<typename T, typename TypeList>
		struct MetaPredicate {
		private:
			template<typename ToFind>
			using Predicate = BinaryPredicate<T, ToFind>;
		public:
			static
			constexpr
			bool value{find_if<TypeList, Predicate>::value == not_found};
		};

		template<typename T>
		using LeftPredicate = MetaPredicate<T, TypeList2>;

		template<typename T>
		using RightPredicate = MetaPredicate<T, TypeList1>;

		using left = typename erase_if<
			TypeList1,
			LeftPredicate
		>::type;

		using right = typename erase_if<
			TypeList2,
			RightPredicate
		>::type;
	public:
		using type = typename unique_if<
			typename merge<
				left,
				right
			>::type,
			BinaryPredicate
		>::type;
	};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct set_symmetric_difference {
	private:
		template<typename T, typename TypeList>
		struct MetaPredicate {
		private:
			template<typename ToFind>
			using Predicate = BinaryPredicate<T, ToFind>;
		public:
			static
			constexpr
			bool value{find_if<TypeList, Predicate>::value != not_found};
		};

		template<typename T>
		using LeftPredicate = MetaPredicate<T, TypeList2>;

		template<typename T>
		using RightPredicate = MetaPredicate<T, TypeList1>;

		using left = typename erase_if<
			TypeList1,
			LeftPredicate
		>::type;

		using right = typename erase_if<
			TypeList2,
			RightPredicate
		>::type;
	public:
		using type = typename unique_if<
			typename merge<
				left,
				right
			>::type,
			BinaryPredicate
		>::type;
	};


	template<typename TypeList, template<typename, typename> typename BinaryPredicate>
	struct min_element {
	private:
		using min = typename at<
			typename sort<
				TypeList,
				BinaryPredicate
			>::type,
			0
		>::type;
	public:
		static
		constexpr
		std::ptrdiff_t value{find<TypeList, min>::value};
	};

	template<template<typename, typename> typename BinaryPredicate>
	struct min_element<type_list<>, BinaryPredicate> : invalid_index {};


	template<typename TypeList, template<typename, typename> typename BinaryPredicate>
	struct max_element {
	private:
		using max = typename at<
			typename sort<
				TypeList,
				BinaryPredicate
			>::type,
			size<TypeList>::value - 1
		>::type;

		static
		constexpr
		std::ptrdiff_t tmp{find<typename reverse<TypeList>::type, max>::value};
	public:
		static
		constexpr
		std::ptrdiff_t value{size<TypeList>::value - tmp - 1};
	};

	template<template<typename, typename> typename BinaryPredicate>
	struct max_element<type_list<>, BinaryPredicate> : invalid_index {};


	template<typename TypeList1, typename TypeList2, template<typename, typename> typename BinaryPredicate>
	struct equal : std::bool_constant<!size<TypeList1>::value && !size<TypeList2>::value> {};

	template<typename Head1, typename... Tail1, typename Head2, typename... Tail2, template<typename, typename> typename BinaryPredicate>
	struct equal<type_list<Head1, Tail1...>, type_list<Head2, Tail2...>, BinaryPredicate> : std::bool_constant<BinaryPredicate<Head1, Head2>::value && equal<type_list<Tail1...>, type_list<Tail2...>, BinaryPredicate>::value> {};
}
