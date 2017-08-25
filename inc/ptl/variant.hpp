
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "internal/type_checks.hpp"
#include "internal/type_list.hpp"

#include <ostream>
#include <utility>
#include <stdexcept>

namespace ptl {
	//TODO: documentation
	struct bad_variant_access : std::exception {
		auto what() const noexcept -> const char * override { return "bad_variant_access"; }
	};

	namespace internal {
		template<typename TypeList>
		struct biggest_type;

		template<typename Head, typename Tail>
		struct biggest_type<TL::type_list<Head, Tail>> final {
			using type = typename std::conditional<
				(sizeof(Head) > sizeof(typename biggest_type<Tail>::type)),
				Head,
				typename biggest_type<Tail>::type
			>::type;
		};

		template<typename Type>
		struct biggest_type<TL::type_list<Type, TL::empty_type_list>> final {
			using type = Type;
		};

		template<typename ResultType, typename TypeList>
		struct visit final {
			template<typename Visitor>
			PTL_CXX_RELAXED_CONSTEXPR
			static auto dispatch(unsigned char index,       void * ptr, Visitor && visitor) -> ResultType {
				return index ? visit<ResultType, typename TypeList::tail>::dispatch(index - 1, ptr, std::forward<Visitor>(visitor))
				             : visitor(*reinterpret_cast<      typename TypeList::head *>(ptr));
			}

			template<typename Visitor>
			PTL_CXX_RELAXED_CONSTEXPR
			static auto dispatch(unsigned char index, const void * ptr, Visitor && visitor) -> ResultType {
				return index ? visit<ResultType, typename TypeList::tail>::dispatch(index - 1, ptr, std::forward<Visitor>(visitor))
				             : visitor(*reinterpret_cast<const typename TypeList::head *>(ptr));
			}
		};

		template<typename ResultType>
		struct visit<ResultType, TL::empty_type_list> final {
			template<typename Visitor>
			PTL_CXX_RELAXED_CONSTEXPR
			static auto dispatch(unsigned char index, const void * ptr, Visitor && visitor) -> ResultType { throw std::logic_error{"DESIGN-ERROR: invalid dispatch in visit detected (please report this)"}; }
		};

		template<typename TargetType>
		struct get_if_visitor final {
			auto operator()(TargetType & val) const noexcept -> TargetType * { return &val; }

			template<typename Type>
			auto operator()(Type &) const noexcept -> TargetType * { return nullptr; }
		};

		template<typename TypeList>
		struct is_unique;

		template<typename Head, typename Tail>
		struct is_unique<TL::type_list<Head, Tail>> final {
			enum { value = (TL::find<Tail, Head>::value == -1) && is_unique<Tail>::value };
		};

		template<>
		struct is_unique<TL::empty_type_list> final {
			enum { value = 1 };
		};
	}

	PTL_PACK_BEGIN
	//! @brief a type-safe union, storing one of multiple types
	//! @tparam Types all types that may be stored in the variant
	template<typename... Types>
	struct variant final {//TODO: evaluate differences to the standard! 
		using all_types = typename internal::TL::make_type_list<Types...>::type;
		using default_type = typename internal::TL::at<all_types, 0>::type;

		template<typename Type>
		using type_index = internal::TL::find<all_types, Type>;

		static_assert(internal::are_abi_compatible<Types...>::value, "Types do not fulfill ABI requirements");
		static_assert(sizeof...(Types) >   0, "A variant cannot store 0 types");
		static_assert(sizeof...(Types) < 128, "A variant stores at most 127 different types");
		static_assert(internal::is_unique<all_types>::value, "variant does not support duplicated types");

		static const signed char variant_npos{-1};

		PTL_CXX_RELAXED_CONSTEXPR
		variant() : type{0} { new(data) default_type{}; }

		variant(const variant & other) : type{other.type} { if(!valueless_by_exception()) other.visit(copy_ctor{data}); }
		variant(variant && other) noexcept : type{other.type} { if(!valueless_by_exception()) other.visit(move_ctor{data}); }

		template<typename Type, typename = typename std::enable_if<!std::is_same<typename std::decay<Type>::type, variant>::value>::type>
		variant(Type && value) noexcept {
			using DecayedType = typename std::decay<Type>::type;
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
		auto operator=(Type && value) -> typename std::enable_if<!std::is_same<typename std::decay<Type>::type, variant>::value, variant &>::type {
			using DecayedType = typename std::decay<Type>::type;
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

		PTL_CXX_RELAXED_CONSTEXPR
		auto index() const noexcept -> signed char { return type; }
		PTL_CXX_RELAXED_CONSTEXPR
		auto valueless_by_exception() const noexcept -> bool { return type == variant_npos; }

		template<typename Visitor>
		PTL_CXX_RELAXED_CONSTEXPR
		auto visit(Visitor && visitor) const -> decltype(std::declval<Visitor>()(std::declval<const default_type &>())) {
			if(valueless_by_exception()) throw bad_variant_access{};
			return internal::visit<decltype(visit(std::forward<Visitor>(visitor))), all_types>::dispatch(type, data, std::forward<Visitor>(visitor));
		}
		template<typename Visitor>
		PTL_CXX_RELAXED_CONSTEXPR
		auto visit(Visitor && visitor)       -> decltype(std::declval<Visitor>()(std::declval<      default_type &>())) {
			if(valueless_by_exception()) throw bad_variant_access{};
			return internal::visit<decltype(visit(std::forward<Visitor>(visitor))), all_types>::dispatch(type, data, std::forward<Visitor>(visitor));
		}

		friend
		void swap(variant & lhs, variant & rhs) noexcept {
			if(lhs.valueless_by_exception() && rhs.valueless_by_exception()) return;
			if(lhs.type == rhs.type) lhs.visit(swapper{rhs});
			else std::swap(lhs, rhs);
		}

		friend
		PTL_CXX_RELAXED_CONSTEXPR
		auto operator==(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return false;
			if(lhs.valueless_by_exception()) return true;
			return lhs.visit(compare<std::equal_to>{rhs});
		}

		friend
		PTL_CXX_RELAXED_CONSTEXPR
		auto operator!=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return true;
			if(lhs.valueless_by_exception()) return false;
			return lhs.visit(compare<std::not_equal_to>{rhs});
		}

		friend
		PTL_CXX_RELAXED_CONSTEXPR
		auto operator< (const variant & lhs, const variant & rhs) -> bool {
			if(rhs.valueless_by_exception()) return false;
			if(lhs.valueless_by_exception()) return true;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit(compare<std::less>{rhs});
		}

		friend
		PTL_CXX_RELAXED_CONSTEXPR
		auto operator<=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return true;
			if(rhs.valueless_by_exception()) return false;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit(compare<std::less_equal>{rhs});
		}

		friend
		PTL_CXX_RELAXED_CONSTEXPR
		auto operator> (const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return false;
			if(rhs.valueless_by_exception()) return true;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit(compare<std::greater>{rhs});
		}

		friend
		PTL_CXX_RELAXED_CONSTEXPR
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
			copy_ctor(void * data) : data{data} {}

			template<typename Type>
			void operator()(const Type & value) const { new(data) Type{value}; }
		private:
			void * data;
		};
		struct move_ctor final {
			move_ctor(void * data) : data{data} {}

			template<typename Type>
			void operator()(Type & value) const { new(data) Type{std::move(value)}; }
		private:
			void * data;
		};

		struct copy_assign final {
			copy_assign(void * data) : data{data} {}

			template<typename Type>
			void operator()(const Type & value) const { *reinterpret_cast<Type *>(data) = value; }
		private:
			void * data;
		};
		struct move_assign final {
			move_assign(void * data) : data{data} {}

			template<typename Type>
			void operator()(Type & value) const { *reinterpret_cast<Type *>(data) = std::move(value); }
		private:
			void * data;
		};

		struct dtor final {
			template<typename Type>
			void operator()(Type & value) const { value.~Type(); }
		};

		template<template<typename> class Comparator>
		struct compare final {
			compare(const variant & other) : other{other} {}

			template<typename Type>
			auto operator()(const Type & value) const -> bool { return other.visit(subcompare<Type>{value}); }
		private:
			template<typename ValueType>
			struct subcompare final {
				subcompare(const ValueType & lhs) : lhs{lhs} {}

				template<typename Type>
				auto operator()(const Type &) const -> bool { throw std::logic_error{"DESIGN-ERROR: invalid dispatch in subcompare detected (please report this)"}; }
				auto operator()(const ValueType & rhs) const -> bool { return Comparator<ValueType>{}(lhs, rhs); }
			private:
				const ValueType & lhs;
			};
			const variant & other;
		};

		struct swapper final {
			swapper(variant & other) : other{other} {}

			template<typename Type>
			void operator()(Type & value) { return other.visit(subswapper<Type>{value}); }
		private:
			template<typename ValueType>
			struct subswapper final {
				subswapper(ValueType & lhs) : lhs{lhs} {}

				template<typename Type>
				void operator()(Type &) { throw std::logic_error{"DESIGN-ERROR: invalid dispatch in subswapper detected (please report this)"}; }
				void operator()(ValueType & rhs) {
					using std::swap;
					swap(lhs, rhs);
				}
			private:
				ValueType & lhs;
			};
			variant & other;
		};

		struct printer final {
			printer(std::ostream & os) : os{os} {}

			template<typename Type>
			void operator()(const Type & value) const { os << value; }
		private:
			std::ostream & os;
		};

		unsigned char data[sizeof(typename internal::biggest_type<all_types>::type)];
		signed char type{variant_npos};
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto holds_alternative(const variant<Types...> & self) noexcept -> bool {
		using Variant = variant<Types...>;
		const auto tmp{Variant::template type_index<Type>::value};
		if(tmp == Variant::variant_npos) return false;
		return self.index() == tmp;
	}

	template<typename Type, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto get_if(const variant<Types...> & self) noexcept -> const Type * {
		if(self.valueless_by_exception()) return nullptr;
		return self.visit(internal::get_if_visitor<const Type>{});
	}
	template<typename Type, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto get_if(      variant<Types...> & self) noexcept ->       Type * {
		if(self.valueless_by_exception()) return nullptr;
		return self.visit(internal::get_if_visitor<      Type>{});
	}

	template<typename Type, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto get(const variant<Types...> & self) -> const Type & {
		if(auto ptr = get_if<Type>(self)) return *ptr;
		throw bad_variant_access{};
	}
	template<typename Type, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto get(      variant<Types...> & self) ->       Type & {
		if(auto ptr = get_if<Type>(self)) return *ptr;
		throw bad_variant_access{};
	}

	template<typename Visitor, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto visit(Visitor && visitor, const variant<Types...> & self) -> decltype(std::declval<Visitor>()(std::declval<const typename variant<Types...>::default_type &>())) { return self.visit(std::forward<Visitor>(visitor)); }
	template<typename Visitor, typename... Types>
	PTL_CXX_RELAXED_CONSTEXPR
	auto visit(Visitor && visitor,       variant<Types...> & self) -> decltype(std::declval<Visitor>()(std::declval<      typename variant<Types...>::default_type &>())) { return self.visit(std::forward<Visitor>(visitor)); }
}
