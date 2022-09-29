
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <utility>
#include <variant>
#include <type_traits>

namespace ptl {
	namespace internal_variant {
		template<typename...>
		class storage_t final {};

		template<typename Head, typename... Tail>
		class storage_t<Head, Tail...> final {
			union {
				Head val;
				storage_t<Tail...> next;
			};
		public:
			using value_type = Head;

			constexpr
			storage_t() noexcept {} //content uninitialized!

			template<std::size_t I>
			constexpr
			auto get() const noexcept { //TODO: [C++23] merge all overloads using deducing this
				if constexpr(I != 0) return next.template get<I - 1>();
				else return std::addressof(val);
			}
			template<std::size_t I>
			constexpr
			auto get()       noexcept { //TODO: [C++23] merge all overloads using deducing this
				if constexpr(I != 0) return next.template get<I - 1>();
				else return std::addressof(val);
			}

			template<std::size_t I, typename... Args>
			constexpr
			void set(Args &&... args) { //TODO: [C++??] precondition(val is uninitialized);
				if constexpr(I != 0) next.template set<I - 1>(std::forward<Args>(args)...);
				else new(std::addressof(val)) Head{std::forward<Args>(args)...}; //TODO: [C++20] use std::construct_at here
			}

			template<std::size_t I>
			constexpr
			void destroy() noexcept {
				if constexpr(I != 0) next.template destroy<I - 1>();
				else std::destroy_at(std::addressof(val));
			}
		};

		inline
		constexpr
		unsigned char not_found{255};

		template<typename T, typename Head, typename... Tail>
		constexpr //TODO: [C++20] replace with consteval
		auto determine_index(unsigned char index = 0) noexcept -> unsigned char {
			if constexpr(std::is_same_v<T, Head>) return index;
			else if constexpr(sizeof...(Tail) != 0) return determine_index<T, Tail...>(index + 1);
			else return not_found;
		}

		template<typename Head, typename... Tail>
		constexpr //TODO: [C++20] replace with consteval
		auto validate_unique() noexcept -> bool {
			if constexpr(sizeof...(Tail) == 0) return true;
			else if constexpr(determine_index<Head, Tail...>() != not_found) return false;
			else return validate_unique<Tail...>();
		}

		template<typename Head, typename... Tail>
		constexpr //TODO: [C++20] replace with consteval
		auto max_sizeof(std::size_t size = 0) noexcept -> std::size_t {
			if(sizeof(Head) > size) size = sizeof(Head);
			if constexpr(sizeof...(Tail) != 0) return max_sizeof<Tail...>(size);
			else return size;
		}

		template<typename T>
		struct type_identity final { //TODO: [C++20] replace with std::type_identity
			using type = T;
		};

		template<unsigned char Index, typename Head, typename... Tail>
		static
		constexpr //TODO: [C++20] replace with consteval
		auto determine_type() noexcept {
			if constexpr(Index == 0) return type_identity<Head>{};
			else return determine_type<Index - 1, Tail...>();
		}

		template<bool Move, typename Storage, typename Visitor, std::size_t... Indices>
		constexpr
		auto visit(std::index_sequence<Indices...>, std::size_t type, Storage & storage, Visitor && visitor) -> decltype(auto) { //TODO: [C++??] precondition(storage.get(type) is valid);
			using Type = typename Storage::value_type;
			using Tmp1 = std::conditional_t<std::is_const_v<Storage>, const Type, Type>;
			using Tmp2 = std::conditional_t<Move, Tmp1, Tmp1 &>;
			using Result = decltype(std::declval<Visitor>()(std::declval<Tmp2>()));
			using Dispatch = Result(*)(Storage &, Visitor &);
			constexpr Dispatch dispatch[]{+[](Storage & storage, Visitor & visitor) -> Result {
				auto ptr{storage.template get<Indices>()};
				if constexpr(Move) return visitor(std::move(*ptr));
				else return visitor(*ptr);
			}...};
			return dispatch[type](storage, visitor);
		}

		template<bool Move, typename Storage, typename... Visitors, std::size_t... Indices, typename = std::enable_if_t<(sizeof...(Visitors) > 1)>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(std::index_sequence<Indices...> indices, std::size_t type, Storage & storage, Visitors &&... visitors) -> decltype(auto) {  //TODO: [C++??] precondition(storage.get(type) is valid);
			struct combined_visitor : Visitors... { using Visitors::operator()...; };
			return visit<Move>(indices, type, storage, combined_visitor{std::forward<Visitors>(visitors)...});
		}

		template<std::size_t... Indices, typename Storage, typename... Args>
		constexpr
		void emplace(std::index_sequence<Indices...>, std::size_t type, Storage & storage, Args &&... args) {  //TODO: [C++??] precondition(union in storage is uninitialized);
			using Dispatch = void(*)(Storage &, Args &&...);
			constexpr Dispatch dispatch[]{+[](Storage & storage, Args &&... args) { storage.template set<Indices>(std::forward<Args>(args)...); }...};
			dispatch[type](storage, std::forward<Args>(args)...);
		}

		template<bool Move, typename Storage, std::size_t... Indices>
		constexpr
		void assign(std::index_sequence<Indices...>, std::size_t type, Storage & lhs, std::conditional_t<Move, Storage, const Storage> & rhs) noexcept(Move) { //TODO: [C++??] precondition(lhs is uninitialized);
			using Other = std::conditional_t<Move, Storage, const Storage>;
			using Dispatch = void(*)(Storage &, Other &) noexcept(Move);
			constexpr Dispatch dispatch[]{+[](Storage & lhs, Other & rhs) noexcept(Move) {
				auto ptr{rhs.template get<Indices>()};
				if constexpr(Move) lhs.template set<Indices>(std::move(*ptr));
				else lhs.template set<Indices>(*ptr);
			}...};
			dispatch[type](lhs, rhs);
		}

		template<std::size_t... Indices, typename Storage>
		constexpr
		void destroy(std::index_sequence<Indices...>, std::size_t type, Storage & storage) { //TODO: [C++??] precondition(storage.get(type) is valid);
			using Dispatch = void(*)(Storage &) noexcept;
			constexpr Dispatch dispatch[]{+[](Storage & storage) noexcept { storage.template destroy<Indices>(); }...};
			dispatch[type](storage);
		}

		template<std::size_t... Indices, typename Storage, typename Comparator>
		constexpr
		auto compare(std::index_sequence<Indices...>, std::size_t type, const Storage & lhs, const Storage & rhs, Comparator) noexcept { //TODO: [C++??] precondition(lhs.get(type) is valid && other.get(type) is valid);
			using Dispatch = bool(*)(const Storage &, const Storage &) noexcept;
			constexpr Dispatch dispatch[]{+[](const Storage & lhs, const Storage & rhs) noexcept { return Comparator{}(*lhs.template get<Indices>(), *rhs.template get<Indices>()); }...};
			return dispatch[type](lhs, rhs);
		}

		template<std::size_t... Indices, typename Storage>
		constexpr
		void swap(std::index_sequence<Indices...>, std::size_t type, Storage & lhs, Storage & rhs) noexcept { //TODO: [C++??] precondition(lhs.get(type) is valid && other.get(type) is valid);
			using Dispatch = void(*)(Storage &, Storage &) noexcept;
			constexpr Dispatch dispatch[]{+[](Storage & lhs, Storage & rhs) noexcept {
				using std::swap;
				swap(*lhs.template get<Indices>(), *rhs.template get<Indices>());
			}...};
			dispatch[type](lhs, rhs);
		}
	}

	//! @brief a type-safe union, storing one of multiple types
	//! @tparam Types all types that may be stored in the variant
	template<typename... Types>
	class variant;

	#pragma pack(push, 1)
	template<typename Head, typename... Tail>
	class variant<Head, Tail...> final {
		static_assert(sizeof...(Tail) < 254);

		static_assert(std::is_standard_layout_v<Head> && (std::is_standard_layout_v<Tail> && ...)); //TODO: this is probably too strict!
		static_assert(std::is_copy_constructible_v<Head> && (std::is_copy_constructible_v<Tail> && ...));
		static_assert(std::is_nothrow_move_constructible_v<Head> && (std::is_nothrow_move_constructible_v<Tail> && ...));
		//no checks for assignment as internally variant only ever uses construction but no assignment operators
		static_assert(std::is_nothrow_destructible_v<Head> && (std::is_nothrow_destructible_v<Tail> && ...));
		static_assert(std::is_nothrow_swappable_v<Head> && (std::is_nothrow_swappable_v<Tail> && ...));
		static_assert(internal_variant::validate_unique<Head, Tail...>());

		using storage_t = internal_variant::storage_t<Head, Tail...>;
		static_assert(sizeof(storage_t) == internal_variant::max_sizeof<Head, Tail...>());

		using indices_t = std::index_sequence_for<Head, Tail...>;

		storage_t storage;
		unsigned char type;

		template<typename T>
		static
		constexpr
		bool can_store{internal_variant::determine_index<std::remove_const_t<std::remove_reference_t<T>>, Head, Tail...>() != internal_variant::not_found}; //TODO: [C++20] replace with concepts/requires-clause
	public:
		template<typename T = Head, typename = std::enable_if_t<std::is_default_constructible_v<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		variant() : type{0} { storage.template set<0>(); }

		variant(const variant & other) : type{other.type} { internal_variant::assign<false>(indices_t{}, type, storage, other.storage); }
		variant(variant && other) noexcept : type{other.type} { internal_variant::assign<true>(indices_t{}, type, storage, other.storage); }

		template<typename T, std::size_t Index = internal_variant::determine_index<std::decay_t<T>, Head, Tail...>(), typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		variant(T && value) : type{Index} { storage.template set<Index>(std::forward<T>(value)); }

		template<std::size_t Index, typename... Args, typename = std::enable_if_t<(Index >= 0 && Index < sizeof...(Tail) + 1)>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		explicit
		variant(std::in_place_index_t<Index>, Args &&... args) : type{Index} { storage.template set<Index>(std::forward<Args>(args)...); }
		template<typename T, typename... Args, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		explicit
		variant(std::in_place_type_t<T>, Args &&... args) : variant{std::in_place_index<internal_variant::determine_index<T, Head, Tail...>()>, std::forward<Args>(args)...} {}

		auto operator=(const variant & other) -> variant & {
			if(this != &other) { //TODO: [C++20] use [[likely]]
				variant tmp{other};
				internal_variant::destroy(indices_t{}, type, storage);
				internal_variant::assign<true>(indices_t{}, tmp.type, storage, tmp.storage);
				type = other.type;
			}
			return *this;
		}
		auto operator=(variant && other) noexcept -> variant & {
			if(this != &other) { //TODO: [C++20] use [[likely]]
				internal_variant::destroy(indices_t{}, type, storage);
				internal_variant::assign<true>(indices_t{}, other.type, storage, other.storage);
				type = other.type;
			}
			return *this;
		}

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		auto operator=(T && value) -> variant & {
			variant tmp{std::forward<T>(value)};
			*this = std::move(tmp);
			return *this;
		}

		~variant() noexcept { internal_variant::destroy(indices_t{}, type, storage); }

		template<std::size_t Index, typename... Args, typename = std::enable_if_t<(Index >= 0 && Index < sizeof...(Tail) + 1)>> //TODO: [C++20] replace with concepts/requires-clause
		auto emplace(Args &&... args) -> decltype(auto) { return emplace<typename decltype(internal_variant::determine_type<Index, Head, Tail...>())::type>(std::forward<Args>(args)...); }
		template<typename T, typename... Args, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		auto emplace(Args &&... args) -> T & {
			if constexpr(std::is_nothrow_constructible_v<T, Args...>) {
				internal_variant::destroy(indices_t{}, type, storage);
				constexpr auto index{internal_variant::determine_index<std::decay_t<T>, Head, Tail...>()};
				internal_variant::emplace(indices_t{}, index, storage, std::forward<Args>(args)...);
				type = index;
			} else {
				variant tmp{std::in_place_type<T>, std::forward<Args>(args)...};
				*this = std::move(tmp);
			}
			return get<T>();
		}

		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors) const & -> decltype(auto) { return internal_variant::visit<false>(indices_t{}, type, storage, std::forward<Visitors>(visitors)...); } //TODO: [C++23] merge all overloads using deducing this
		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors)       & -> decltype(auto) { return internal_variant::visit<false>(indices_t{}, type, storage, std::forward<Visitors>(visitors)...); } //TODO: [C++23] merge all overloads using deducing this
		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors) const && -> decltype(auto) { return internal_variant::visit<true>(indices_t{}, type, storage, std::forward<Visitors>(visitors)...); } //TODO: [C++23] merge all overloads using deducing this
		template<typename... Visitors, typename = std::enable_if_t<sizeof...(Visitors) != 0>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto visit(Visitors &&... visitors)       && -> decltype(auto) { return internal_variant::visit<true>(indices_t{}, type, storage, std::forward<Visitors>(visitors)...); } //TODO: [C++23] merge all overloads using deducing this

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto holds() const noexcept -> bool { return type == internal_variant::determine_index<T, Head, Tail...>(); }

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get_if() const noexcept -> const T * { //TODO: [C++23] merge all overloads using deducing this
			if(!holds<T>()) return nullptr;
			return storage.template get<internal_variant::determine_index<T, Head, Tail...>()>();
		}
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get_if()       noexcept ->       T * { //TODO: [C++23] merge all overloads using deducing this
			if(!holds<T>()) return nullptr;
			return storage.template get<internal_variant::determine_index<T, Head, Tail...>()>();
		}

		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get() const &  -> const T & { //TODO: [C++23] merge all overloads using deducing this
			if(const auto ptr{get_if<T>()}) return *ptr;
			throw std::bad_variant_access{};
		}
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get()      &  ->       T & { //TODO: [C++23] merge all overloads using deducing this
			if(const auto ptr{get_if<T>()}) return *ptr;
			throw std::bad_variant_access{};
		}
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get() const && -> const T && { return std::move(get<T>()); } //TODO: [C++23] merge all overloads using deducing this
		template<typename T, typename = std::enable_if_t<can_store<T>>> //TODO: [C++20] replace with concepts/requires-clause
		constexpr
		auto get()       && ->       T && { return std::move(get<T>()); } //TODO: [C++23] merge all overloads using deducing this

		void swap(variant & other) noexcept {
			if(this == &other) return; //TODO: [C++20] use [[unlikely]]
			if(type == other.type) internal_variant::swap(indices_t{}, type, storage, other.storage);
			else std::swap(*this, other);
		}
		friend
		void swap(variant & lhs, variant & rhs) noexcept { lhs.swap(rhs); }

		friend
		constexpr
		auto operator==(const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type != rhs.type) return false;
			return internal_variant::compare(indices_t{}, lhs.type, lhs.storage, rhs.storage, std::equal_to<>{});
		}
		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) noexcept -> bool { //TODO: [C++20] remove as implicitly generated
			if(lhs.type != rhs.type) return true;
			return internal_variant::compare(indices_t{}, lhs.type, lhs.storage, rhs.storage, std::not_equal_to<>{});
		}
		//TODO: [C++20] replace the ordering operators by <=>
		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return internal_variant::compare(indices_t{}, lhs.type, lhs.storage, rhs.storage, std::less<>{});
		}
		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return internal_variant::compare(indices_t{}, lhs.type, lhs.storage, rhs.storage, std::less_equal<>{});
		}
		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return internal_variant::compare(indices_t{}, lhs.type, lhs.storage, rhs.storage, std::greater<>{});
		}
		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) noexcept -> bool {
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return internal_variant::compare(indices_t{}, lhs.type, lhs.storage, rhs.storage, std::greater_equal<>{});
		}
	};
	#pragma pack(pop)
}
