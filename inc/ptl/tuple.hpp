
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"

namespace ptl {
	//! @brief a fixed-size collection of heterogeneous value
	//! @tparam Types types to store inside the tuple
	#pragma pack(push, 1)
	template<typename... Types>
	class tuple final {
		static_assert((internal::is_abi_compatible_v<Types> && ...));

		template<typename T>
		struct type_identity final { using type = T; }; //TODO: [C++20] replace with std::type_identity

		template<typename Type, typename Base>
		struct base : Base {
			template<typename Arg, typename... Args>
			base(Arg && arg, Args &&... args) : Base{std::forward<Args>(args)...}, value{std::forward<Arg>(arg)} {}

			template<std::size_t Index>
			constexpr
			auto get() const noexcept -> decltype(auto) {
				if constexpr(Index == 0) return (value);
				else return Base::template get<Index - 1>();
			}
			template<std::size_t Index>
			constexpr
			auto get()       noexcept -> decltype(auto) {
				if constexpr(Index == 0) return (value);
				else return Base::template get<Index - 1>();
			}

			constexpr
			void swap(base & other) noexcept {
				using std::swap;
				swap(value, other.value);
				Base::swap(other);
			}

			constexpr
			auto equal(const base & other) const noexcept { return (value == other.value) ? Base::equal(other) : false; }
			constexpr
			auto less(const base & other) const noexcept { return (value < other.value) || (!(other.value < value) && Base::less(other)); }
		private:
			Type value;
		};

		template<typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto determine_storage() noexcept {
			if constexpr(sizeof...(Ts) == 0) {
				struct empty_base {
					static
					constexpr
					void swap(empty_base &) noexcept {}

					static
					constexpr
					auto equal(const empty_base &) noexcept { return true; }
					static
					constexpr
					auto less(const empty_base &) noexcept { return false; }
				};
				return type_identity<empty_base>{};
			} else return determine_storage_impl<Ts...>();
		}

		template<typename T, typename... Ts>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto determine_storage_impl() noexcept { return type_identity<base<T, typename decltype(determine_storage<Ts...>())::type>>{}; }

		typename decltype(determine_storage<Types...>())::type storage;
	public:
		template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == sizeof...(Types) || sizeof...(Args) == 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		tuple(Args &&... args) : storage{std::forward<Args>(args)...} {}

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
	#pragma pack(pop)

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
