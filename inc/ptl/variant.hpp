
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
			PTL_RELAXED_CONSTEXPR
			static
			auto dispatch(unsigned char index, const void * ptr, Visitor && visitor) -> ResultType {
				throw std::logic_error{"DESIGN-ERROR: invalid dispatch in visit detected (please report this)"};
			}
		}; 

		template<typename ResultType, typename Type, typename... Types>
		struct visit<ResultType, Type, Types...> final {
			template<typename Visitor>
			PTL_RELAXED_CONSTEXPR
			static
			auto dispatch(unsigned char index,       void * ptr, Visitor && visitor) -> ResultType {
				return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, std::forward<Visitor>(visitor))
				             : visitor(*reinterpret_cast<      Type *>(ptr));
			}

			template<typename Visitor>
			PTL_RELAXED_CONSTEXPR
			static
			auto dispatch(unsigned char index, const void * ptr, Visitor && visitor) -> ResultType {
				return index ? visit<ResultType, Types...>::dispatch(index - 1, ptr, std::forward<Visitor>(visitor))
				             : visitor(*reinterpret_cast<const Type *>(ptr));
			}
		};

		template<typename TargetType>
		struct get_if_visitor final {
			auto operator()(TargetType & val) const noexcept -> TargetType * { return &val; }

			template<typename Type>
			auto operator()(Type &) const noexcept -> TargetType * { return nullptr; }
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

		PTL_RELAXED_CONSTEXPR
		variant() : type{0} { new(data) DefaultType{}; }

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

		PTL_RELAXED_CONSTEXPR
		auto index() const noexcept -> signed char { return type; }
		PTL_RELAXED_CONSTEXPR
		auto valueless_by_exception() const noexcept -> bool { return type == variant_npos; }

		template<typename Visitor>
		PTL_RELAXED_CONSTEXPR
		auto visit(Visitor && visitor) const -> decltype(std::declval<Visitor>()(std::declval<const DefaultType &>())) {
			if(valueless_by_exception()) throw bad_variant_access{};
			return internal::visit<decltype(visit(std::forward<Visitor>(visitor))), DefaultType, Types...>::dispatch(type, data, std::forward<Visitor>(visitor));
		}
		template<typename Visitor>
		PTL_RELAXED_CONSTEXPR
		auto visit(Visitor && visitor)       -> decltype(std::declval<Visitor>()(std::declval<      DefaultType &>())) {
			if(valueless_by_exception()) throw bad_variant_access{};
			return internal::visit<decltype(visit(std::forward<Visitor>(visitor))), DefaultType, Types...>::dispatch(type, data, std::forward<Visitor>(visitor));
		}

		friend
		void swap(variant & lhs, variant & rhs) noexcept {
			if(lhs.valueless_by_exception() && rhs.valueless_by_exception()) return;
			if(lhs.type == rhs.type) lhs.visit(swapper{rhs});
			else std::swap(lhs, rhs);
		}

		friend
		PTL_RELAXED_CONSTEXPR
		auto operator==(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return false;
			if(lhs.valueless_by_exception()) return true;
			return lhs.visit(compare<std::equal_to>{rhs});
		}

		friend
		PTL_RELAXED_CONSTEXPR
		auto operator!=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.type != rhs.type) return true;
			if(lhs.valueless_by_exception()) return false;
			return lhs.visit(compare<std::not_equal_to>{rhs});
		}

		friend
		PTL_RELAXED_CONSTEXPR
		auto operator< (const variant & lhs, const variant & rhs) -> bool {
			if(rhs.valueless_by_exception()) return false;
			if(lhs.valueless_by_exception()) return true;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit(compare<std::less>{rhs});
		}

		friend
		PTL_RELAXED_CONSTEXPR
		auto operator<=(const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return true;
			if(rhs.valueless_by_exception()) return false;
			if(lhs.type < rhs.type) return true;
			if(lhs.type > rhs.type) return false;
			return lhs.visit(compare<std::less_equal>{rhs});
		}

		friend
		PTL_RELAXED_CONSTEXPR
		auto operator> (const variant & lhs, const variant & rhs) -> bool {
			if(lhs.valueless_by_exception()) return false;
			if(rhs.valueless_by_exception()) return true;
			if(lhs.type > rhs.type) return true;
			if(lhs.type < rhs.type) return false;
			return lhs.visit(compare<std::greater>{rhs});
		}

		friend
		PTL_RELAXED_CONSTEXPR
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

		unsigned char data[internal::max_sizeof<DefaultType, Types...>::value];
		signed char type{variant_npos};
	};
	PTL_PACK_END

	template<typename Type, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto holds_alternative(const variant<Types...> & self) noexcept -> bool {
		using Variant = variant<Types...>;
		const auto tmp{Variant::template type_index<Type>::value};
		if(tmp == Variant::variant_npos) return false;
		return self.index() == tmp;
	}

	template<typename Type, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto get_if(const variant<Types...> & self) noexcept -> const Type * {
		if(self.valueless_by_exception()) return nullptr;
		return self.visit(internal::get_if_visitor<const Type>{});
	}
	template<typename Type, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto get_if(      variant<Types...> & self) noexcept ->       Type * {
		if(self.valueless_by_exception()) return nullptr;
		return self.visit(internal::get_if_visitor<      Type>{});
	}

	template<typename Type, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto get(const variant<Types...> & self) -> const Type & {
		if(auto ptr = get_if<Type>(self)) return *ptr;
		throw bad_variant_access{};
	}
	template<typename Type, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto get(      variant<Types...> & self) ->       Type & {
		if(auto ptr = get_if<Type>(self)) return *ptr;
		throw bad_variant_access{};
	}

	template<typename Visitor, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto visit(Visitor && visitor, const variant<Types...> & self) -> decltype(std::declval<Visitor>()(std::declval<const typename variant<Types...>::default_type &>())) { return self.visit(std::forward<Visitor>(visitor)); }
	template<typename Visitor, typename... Types>
	PTL_RELAXED_CONSTEXPR
	auto visit(Visitor && visitor,       variant<Types...> & self) -> decltype(std::declval<Visitor>()(std::declval<      typename variant<Types...>::default_type &>())) { return self.visit(std::forward<Visitor>(visitor)); }
}
