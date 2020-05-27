
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "type_list.hpp"
#include "internal/type_checks.hpp"
#include "internal/cpp20_emulation.hpp"
#include "internal/compiler_detection.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief a fixed-size collection of heterogeneous value
	//! @tparam Types types to store inside the tuple
	template<typename... Types>
	class tuple final {
		static_assert((internal::is_abi_compatible_v<Types> && ...));

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

		template<typename TL, typename Base = empty_base>
		static
		constexpr //TODO(C++20): consteval
		auto determine_storage() noexcept {
			if constexpr(TL::empty) return internal::type_identity<Base>{};
			else {
				using Head = typename TL::template at<0>;
				using Tail = typename TL::template erase_at<0>;
				constexpr auto tmp{determine_storage<Tail, Base>()};
				return internal::type_identity<base<Head, typename decltype(tmp)::type>>{};
			}
		}

		typename decltype(determine_storage<type_list<Types...>>())::type storage;

		template<std::size_t Index, typename... Ts>
		friend
		constexpr
		auto get(const tuple<Ts...> &) noexcept -> decltype(auto);

		template<std::size_t Index, typename... Ts>
		friend
		constexpr
		auto get(      tuple<Ts...> &) noexcept -> decltype(auto);
	public:
		template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == sizeof...(Types) || sizeof...(Args) == 0>>
		constexpr
		tuple(Args &&... args) : storage{std::forward<Args>(args)...} {}

		constexpr
		void swap(tuple & other) noexcept { storage.swap(other.storage); }
		friend
		constexpr
		void swap(tuple & lhs, tuple & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const tuple & lhs, const tuple & rhs) noexcept { return lhs.storage.equal(rhs.storage); }
		friend
		constexpr
		auto operator!=(const tuple & lhs, const tuple & rhs) noexcept { return !(lhs == rhs); }
		friend
		constexpr
		auto operator< (const tuple & lhs, const tuple & rhs) noexcept { return lhs.storage.less(rhs.storage); }
		friend
		constexpr
		auto operator> (const tuple & lhs, const tuple & rhs) noexcept { return rhs < lhs; }
		friend
		constexpr
		auto operator<=(const tuple & lhs, const tuple & rhs) noexcept { return !(lhs > rhs); }
		friend
		constexpr
		auto operator>=(const tuple & lhs, const tuple & rhs) noexcept { return !(lhs < rhs); }
	};
	PTL_PACK_END

	template<typename... Types>
	tuple(Types...) -> tuple<Types...>;

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(const tuple<Types...> & self) noexcept -> decltype(auto) { return self.storage.template get<Index>(); }

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(      tuple<Types...> & self) noexcept -> decltype(auto) { return self.storage.template get<Index>(); }

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(const tuple<Types...> && self) noexcept -> decltype(auto) { return std::move(get<Index>(self)); }

	template<std::size_t Index, typename... Types>
	constexpr
	auto get(      tuple<Types...> && self) noexcept -> decltype(auto) { return std::move(get<Index>(self)); }
}

namespace std {
	template<typename... Types>
	struct tuple_size<ptl::tuple<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

	template<std::size_t Index, typename Head, typename... Tail>
	struct tuple_element<Index, ptl::tuple<Head, Tail...>> : tuple_element<Index - 1, ptl::tuple<Tail...>> {};

	template<typename Head, typename... Tail>
	struct tuple_element<0, ptl::tuple<Head, Tail...>> { using type = Head; };
}
