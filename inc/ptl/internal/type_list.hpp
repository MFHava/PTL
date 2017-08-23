
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl {
	namespace internal {
		namespace TL {
			struct empty_type_list;

			template<typename Head, typename Tail>
			struct type_list {
				using head = Head;
				using tail = Tail;
			};

			template<typename... Types>
			struct make_type_list;

			template<>
			struct make_type_list<> {
				using type = empty_type_list;
			};

			template<typename Type, typename... Types>
			struct make_type_list<Type, Types...> {
				using type = type_list<
					Type,
					typename make_type_list<Types...>::type
				>;
			};

			template<typename TypeList, typename Type>
			struct append;

			template<>
			struct append<empty_type_list, empty_type_list> {
				using type = empty_type_list;
			};

			template<typename Type>
			struct append<empty_type_list, Type> {
				using type = type_list<
					Type,
					empty_type_list
				>;
			};

			template<typename Head, typename Tail>
			struct append<empty_type_list, type_list<Head, Tail>> {
				using type = type_list<
					Head,
					Tail
				>;
			};

			template<typename Head, typename Tail, typename Type>
			struct append<type_list<Head, Tail>, Type> {
				using type = type_list<
					Head,
					typename append<
						Tail,
						Type
					>::type
				>;
			};

			template<typename TypeList, typename Type>
			struct erase_all;

			template<typename Type>
			struct erase_all<empty_type_list, Type> {
				using type = empty_type_list;
			};

			template<typename Head, typename Tail>
			struct erase_all<type_list<Head, Tail>, Head> {
				using type = typename erase_all<
					Tail,
					Head
				>::type;
			};

			template<typename Head, typename Tail, typename Type>
			struct erase_all<type_list<Head, Tail>, Type> {
				using type = type_list<
					Head,
					typename erase_all<
						Tail,
						Type
					>::type
				>;
			};

			template<typename TypeList>
			struct unique;

			template<>
			struct unique<empty_type_list> {
				using type = empty_type_list;
			};

			template<typename Head, typename Tail>
			struct unique<type_list<Head, Tail>> {
				using type = type_list<
					Head,
					typename erase_all<
						typename unique<Tail>::type,
						Head
					>::type
				>;
			};

			template<typename TypeList, typename Type, template<typename, typename> class BinaryPredicate>
			struct find_if;

			template<typename Type, template<typename, typename> class BinaryPredicate>
			struct find_if<empty_type_list, Type, BinaryPredicate> {
				enum { value = -1 };
			};

			template<typename Head, typename Tail, typename Type, template<typename, typename> class BinaryPredicate>
			struct find_if<type_list<Head, Tail>, Type, BinaryPredicate> {
			private:
				enum {
					match = BinaryPredicate<
						Head,
						Type
					>::value
				};
				enum {
					tmp = find_if<
						Tail,
						Type,
						BinaryPredicate
					>::value
				};
			public:
				enum {
					value = match
						? 0
						: tmp == -1
							? -1
							: 1 + tmp
				};
			};

			template<typename TypeList, typename Type>
			using find = find_if<TypeList, Type, std::is_same>;

			template<typename TypeList, int Index>
			struct at;

			template<typename Head, typename Tail>
			struct at<type_list<Head, Tail>, 0> {
				using type = Head;
			};

			template<typename Head, typename Tail, int Index>
			struct at<type_list<Head, Tail>, Index> {
				using type = typename at<
					Tail,
					Index - 1
				>::type;
			};

			template<typename TypeList>
			struct size;

			template<>
			struct size<empty_type_list> {
				enum { value = 0 };
			};

			template<typename Head, typename Tail>
			struct size<type_list<Head, Tail>> {
				enum { value = 1 + size<Tail>::value };
			};
		}
	}
}
