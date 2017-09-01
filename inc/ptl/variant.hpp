
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include <ostream>
#include <utility>
#include <stdexcept>

namespace ptl {
	//TODO: documentation
	struct bad_variant_access : std::exception {
		auto what() const noexcept -> const char * override { return "bad_variant_access"; }
	};

	namespace internal {
		template<typename Type, typename... Types>
		struct max_sizeof final {
			enum { value = sizeof(Type) > max_sizeof<Types...>::value ? sizeof(Type) : max_sizeof<Types...>::value };
		};

		template<typename Type>
		struct max_sizeof<Type> final {
			enum { value = sizeof(Type) };
		};

		template<typename ResultType, typename... Types>
		struct visit final {
			template<typename Visitor>
			constexpr
			static
			auto dispatch(unsigned char index, const void * ptr, Visitor && visitor) -> ResultType {
				throw std::logic_error{"DESIGN-ERROR: invalid dispatch in visit detected (please report this)"};
			}
		}; 

		template<typename ResultType, typename Type, typename... Types>
		struct visit<ResultType, Type, Types...> final {
			template<typename Visitor>
			constexpr
			static
			auto dispatch(unsigned char index, const void * ptr, Visitor && visitor) -> ResultType {
				return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, std::forward<Visitor>(visitor))
				             : visitor(*reinterpret_cast<const Type *>(ptr));
			}
			template<typename Visitor>
			constexpr
			static
			auto dispatch(unsigned char index,       void * ptr, Visitor && visitor) -> ResultType {
				return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, std::forward<Visitor>(visitor))
				             : visitor(*reinterpret_cast<      Type *>(ptr));
			}
		};

		template<typename TypeToFind, typename... Types>
		struct find final {
			enum { value = -1 };
		}; 

		template<typename TypeToFind, typename Type, typename... Types>
		struct find<TypeToFind, Type, Types...> final {
		private:
			enum { tmp = find<TypeToFind, Types...>::value };
		public:
			enum {
				value = std::is_same<TypeToFind, Type>::value
					? 0
					: tmp == -1
						? -1
						: 1 + tmp
			};
		};

		template<typename... Types>
		struct are_unique final {
			enum { value = 1 };
		};

		template<typename Type, typename... Types>
		struct are_unique<Type, Types...> final {
			enum { value = find<Type, Types...>::value == -1 && are_unique<Types...>::value };
		};
	}

	PTL_PACK_BEGIN
	//! @brief a type-safe union, storing one of multiple types
	//! @tparam DefaultType type that will be stored in the variant by default
	//! @tparam Types all additional types that may be stored in the variant
	template<typename DefaultType, typename... Types>
	struct variant final {//TODO: evaluate differences to the standard! 
		using default_type = DefaultType;

		template<typename TypeToFind>
		using type_index = internal::find<TypeToFind, DefaultType, Types...>;

		static_assert(internal::are_abi_compatible<DefaultType, Types...>::value, "Types do not fulfill ABI requirements");
		static_assert(1 + sizeof...(Types) < 128, "A variant stores at most 127 different types");
		static_assert(internal::are_unique<DefaultType, Types...>::value, "variant does not support duplicated types");

		static const signed char variant_npos{-1};

		constexpr
		variant() : type{0} { new(data) DefaultType{}; }

		variant(const variant & other) : type{other.type} { if(!valueless_by_exception()) other.visit(copy_ctor{data}); }
		variant(variant && other) noexcept : type{other.type} { if(!valueless_by_exception()) other.visit(move_ctor{data}); }

		template<typename Type, typename = std::enable_if_t<!std::is_same<std::decay_t<Type>, variant>::value>>
		variant(Type && value) noexcept {
			using DecayedType = std::decay_t<Type>;
			static_assert(type_index<DecayedType>::value != variant_npos, "Type is not stored in variant");
			new(data) DecayedType{std::forward<Type>(value)};
			type = type_index<DecayedType>::value;
		}

		auto operator=(const variant & other) -> variant & {
			if(other.valueless_by_exception()) reset();
			else if(type == other.type) other.visit(copy_assign{data});
			else {
				reset();
				other.visit(copy_ctor{data});
				type = other.type;
			}
			return *this;
		}
		auto operator=(variant && other) noexcept -> variant & {
			if(other.valueless_by_exception()) reset();
			else if(type == other.type) other.visit(move_assign{data});
			else {
				reset();
				other.visit(move_ctor{data});
				type = other.type;
			}
			return *this;
		}

		template<typename Type>
		auto operator=(Type && value) -> std::enable_if_t<!std::is_same<std::decay_t<Type>, variant>::value, variant &> {
			using DecayedType = std::decay_t<Type>;
			static_assert(type_index<DecayedType>::value != variant_npos, "Type is not stored in variant");
			if(type == type_index<DecayedType>::value) {
				*reinterpret_cast<DecayedType *>(data) = std::forward<Type>(value);
			} else {
				reset();
				new(data) DecayedType{std::forward<Type>(value)};
				type = type_index<DecayedType>::value;
			}
			return *this;
		}

		~variant() noexcept { reset(); }

		constexpr
		auto index() const noexcept -> signed char { return type; }
		constexpr
		auto valueless_by_exception() const noexcept -> bool { return type == variant_npos; }

		template<typename Visitor>
		constexpr
		auto visit(Visitor && visitor) const -> decltype(std::declval<Visitor>()(std::declval<const DefaultType &>())) {
			if(valueless_by_exception()) throw bad_variant_access{};
			return internal::visit<decltype(visit(std::forward<Visitor>(visitor))), DefaultType, Types...>::dispatch(type, data, std::forward<Visitor>(visitor));
		}
		template<typename Visitor>
		constexpr
		auto visit(Visitor && visitor)       -> decltype(std::declval<Visitor>()(std::declval<      DefaultType &>())) {
			if(valueless_by_exception()) throw bad_variant_access{};
			return internal::visit<decltype(visit(std::forward<Visitor>(visitor))), DefaultType, Types...>::dispatch(type, data, std::forward<Visitor>(visitor));
		}

		template<typename Type>
		constexpr
		auto get_if() const noexcept -> const Type * {
			if(valueless_by_exception() || type_index<Type>::value != index()) return nullptr;
			return reinterpret_cast<const Type *>(data);
		}
		template<typename Type>
		constexpr
		auto get_if()       noexcept ->       Type * { return const_cast<Type *>(static_cast<const variant *>(this)->get_if<Type>()); }

		template<typename Type>
		constexpr
		auto get() const -> const Type & {
			if(auto ptr = get_if<Type>()) return *ptr;
			throw bad_variant_access{};
		}
		template<typename Type>
		constexpr
		auto get()       ->       Type & { return const_cast<Type &>(static_cast<const variant *>(this)->get<Type>()); }

		friend
		void swap(variant & lhs, variant & rhs) noexcept {
			if(lhs.valueless_by_exception() && rhs.valueless_by_exception()) return;
			if(lhs.type == rhs.type) lhs.visit(swapper{rhs});
			else std::swap(lhs, rhs);
		}

		friend
		constexpr
		auto operator==(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return false;
			if(lhs.valueless_by_exception()) return true;
			return lhs.visit(compare<std::equal_to>{rhs});
		}

		friend
		constexpr
		auto operator!=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return true;
			if(lhs.valueless_by_exception()) return false;
			return lhs.visit(compare<std::not_equal_to>{rhs});
		}

		friend
		constexpr
		auto operator< (const variant & lhs, const variant & rhs) -> bool {
			if(rhs.valueless_by_exception()) return false;
			if(lhs.valueless_by_exception()) return true;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit(compare<std::less>{rhs});
		}

		friend
		constexpr
		auto operator<=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return true;
			if(rhs.valueless_by_exception()) return false;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit(compare<std::less_equal>{rhs});
		}

		friend
		constexpr
		auto operator> (const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return false;
			if(rhs.valueless_by_exception()) return true;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit(compare<std::greater>{rhs});
		}

		friend
		constexpr
		auto operator>=(const variant & lhs, const variant & rhs) -> bool {
			if(rhs.valueless_by_exception()) return true;
			if(lhs.valueless_by_exception()) return false;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit(compare<std::greater_equal>{rhs});
		}

		friend
		auto operator<<(std::ostream & os, const variant & self) -> std::ostream & {
			if(self.valueless_by_exception()) return os << "<valueless by exception>";
			self.visit(printer{os});
			return os;
		}
	private:
		void reset() noexcept {
			if(type == variant_npos) return;
			visit(dtor{});
			type = variant_npos;
		}

		struct copy_ctor final {
			void * data;

			template<typename Type>
			void operator()(const Type & value) const { new(data) Type{value}; }
		};
		struct move_ctor final {
			void * data;

			template<typename Type>
			void operator()(Type & value) const noexcept { new(data) Type{std::move(value)}; }
		};

		struct copy_assign final {
			void * data;

			template<typename Type>
			void operator()(const Type & value) const { *reinterpret_cast<Type *>(data) = value; }
		};
		struct move_assign final {
			void * data;

			template<typename Type>
			void operator()(Type & value) const noexcept { *reinterpret_cast<Type *>(data) = std::move(value); }
		};

		struct dtor final {
			template<typename Type>
			void operator()(Type & value) const noexcept { value.~Type(); }
		};

		template<template<typename> class Comparator>
		struct compare final {
			const variant & other;

			template<typename Type>
			auto operator()(const Type & value) const -> bool { return Comparator<Type>{}(value, other.template get<Type>()); }
		};

		struct swapper final {
			variant & other;

			template<typename Type>
			void operator()(Type & value) noexcept { 
				using std::swap;
				swap(value, other.template get<Type>());
			}
		};

		struct printer final {
			std::ostream & os;

			template<typename Type>
			void operator()(const Type & value) const { os << value; }
		};

		unsigned char data[internal::max_sizeof<DefaultType, Types...>::value];
		signed char type{variant_npos};
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	constexpr
	auto get_if(const variant<Types...> & self) noexcept -> const Type * { return self.template get_if<Type>(); }
	template<typename Type, typename... Types>
	constexpr
	auto get_if(      variant<Types...> & self) noexcept ->       Type * { return self.template get_if<Type>(); }

	template<typename Type, typename... Types>
	constexpr
	auto get(const variant<Types...> & self) -> const Type & { return self.template get<Type>(); }
	template<typename Type, typename... Types>
	constexpr
	auto get(      variant<Types...> & self) ->       Type & { return self.template get<Type>(); }

	template<typename Type, typename... Types>
	constexpr
	auto holds_alternative(const variant<Types...> & self) noexcept -> bool { return get_if<Type>(self); }

	template<typename Visitor, typename... Types>
	constexpr
	auto visit(Visitor && visitor, const variant<Types...> & self) -> decltype(std::declval<Visitor>()(std::declval<const typename variant<Types...>::default_type &>())) { return self.visit(std::forward<Visitor>(visitor)); }
	template<typename Visitor, typename... Types>
	constexpr
	auto visit(Visitor && visitor,       variant<Types...> & self) -> decltype(std::declval<Visitor>()(std::declval<      typename variant<Types...>::default_type &>())) { return self.visit(std::forward<Visitor>(visitor)); }
}
