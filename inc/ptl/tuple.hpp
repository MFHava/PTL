
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <type_traits>

namespace ptl {
	namespace internal_tuple {
		template<typename Type>
		inline
		constexpr
		auto is_abi_compatible_v{  //TODO: determine essential subset
			std::is_standard_layout_v<std::remove_cv_t<Type>> &&
			std::is_default_constructible_v<std::remove_cv_t<Type>> &&
			std::is_copy_constructible_v<std::remove_cv_t<Type>> &&
			std::is_nothrow_move_constructible_v<std::remove_cv_t<Type>> &&
			std::is_copy_assignable_v<std::remove_cv_t<Type>> &&
			std::is_nothrow_move_assignable_v<std::remove_cv_t<Type>> &&
			std::is_nothrow_destructible_v<std::remove_cv_t<Type>> &&
			std::is_nothrow_swappable_v<std::remove_cv_t<Type>>
		};

		template<typename...>
		struct storage_t;

		template<>
		struct storage_t<> {
			static
			constexpr
			void swap(storage_t &) noexcept {}

			static
			constexpr
			auto equal(const storage_t &) noexcept { return true; }
			static
			constexpr
			auto less(const storage_t &) noexcept { return false; }
		};

		#pragma pack(push, 1)
		template<typename Head, typename... Tail>
		struct storage_t<Head, Tail...> : storage_t<Tail...> {
			template<typename Arg, typename... Args>
			storage_t(Arg && arg, Args &&... args) : storage_t<Tail...>{std::forward<Args>(args)...}, value{std::forward<Arg>(arg)} {}

			template<std::size_t Index>
			constexpr
			auto get() const noexcept -> decltype(auto) {
				if constexpr(Index == 0) return (value);
				else return storage_t<Tail...>::template get<Index - 1>();
			}
			template<std::size_t Index>
			constexpr
			auto get()       noexcept -> decltype(auto) {
				if constexpr(Index == 0) return (value);
				else return storage_t<Tail...>::template get<Index - 1>();
			}

			constexpr
			void swap(storage_t & other) noexcept {
				using std::swap;
				swap(value, other.value);
				storage_t<Tail...>::swap(other);
			}

			constexpr
			auto equal(const storage_t & other) const noexcept { return (value == other.value) ? storage_t<Tail...>::equal(other) : false; }
			constexpr
			auto less(const storage_t & other) const noexcept { return (value < other.value) || (!(other.value < value) && storage_t<Tail...>::less(other)); }
		private:
			Head value;
		};
		#pragma pack(pop)
	}

	//! @brief a fixed-size collection of heterogeneous value
	//! @tparam Types types to store inside the tuple
	template<typename... Types>
	class [[deprecated("The PTL is mainly aimed at ABI-stable interfaces, the main use case of a generic tuple type is in a generic context. tuple was therefore deemed to be out of scope.")]] tuple final {
		static_assert((internal_tuple::is_abi_compatible_v<Types> && ...));

		//TODO: fixing default construction: missing in recursive member type?!

		internal_tuple::storage_t<Types...> storage;
	public:
		template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == sizeof...(Types) || sizeof...(Args) == 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		tuple(Args &&... args) : storage{std::forward<Args>(args)...} {}

		//TODO: [C++23] these overloads can be merged by using deducing this
		template<std::size_t Index>
		constexpr
		auto get() const &  noexcept -> decltype(auto) { return storage.template get<Index>(); }
		template<std::size_t Index>
		constexpr
		auto get()       &  noexcept -> decltype(auto) { return storage.template get<Index>(); }
		template<std::size_t Index>
		constexpr
		auto get() const && noexcept -> decltype(auto) { return std::move(get<Index>()); }
		template<std::size_t Index>
		constexpr
		auto get()       && noexcept -> decltype(auto) { return std::move(get<Index>()); }

		constexpr
		void swap(tuple & other) noexcept { storage.swap(other.storage); }
		friend
		constexpr
		void swap(tuple & lhs, tuple & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const tuple & lhs, const tuple & rhs) noexcept -> bool { return lhs.storage.equal(rhs.storage); }
		friend
		constexpr
		auto operator!=(const tuple & lhs, const tuple & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] remove as implicitly generated
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const tuple & lhs, const tuple & rhs) noexcept -> bool { return lhs.storage.less(rhs.storage); }
		friend
		constexpr
		auto operator> (const tuple & lhs, const tuple & rhs) noexcept -> bool { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const tuple & lhs, const tuple & rhs) noexcept -> bool { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const tuple & lhs, const tuple & rhs) noexcept -> bool { return !(lhs < rhs); }
	};

	template<typename... Types>
	tuple(Types...) -> tuple<Types...>;
}

namespace std {
	template<typename... Types>
	struct tuple_size<ptl::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

	template<std::size_t Index, typename Head, typename... Tail>
	struct tuple_element<Index, ptl::tuple<Head, Tail...>> : tuple_element<Index - 1, ptl::tuple<Tail...>> {};

	template<typename Head, typename... Tail>
	struct tuple_element<0, ptl::tuple<Head, Tail...>> { using type = Head; };
}
