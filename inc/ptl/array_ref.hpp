
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <limits>
#include <iterator>
#include "internal/compiler_detection.hpp"
#include "internal/contiguous_container_base.hpp"

namespace ptl {
	PTL_PACK_BEGIN
	//! @brief non-owning reference to array
	//! @tparam Type type of the referenced array
	template<typename Type>
	class array_ref final : public internal::contiguous_container_base<array_ref<Type>, Type> {
		using base_type = internal::contiguous_container_base<array_ref<Type>, Type>;

		Type * first{nullptr}, * last{nullptr};
	public:
		constexpr
		array_ref() noexcept =default;

		//! @brief construct array_ref from two pointers
		//! @param[in] first start of the referenced array
		//! @param[in] last end of the referenced array
		//! @throws std::invalid_argument iff first > last
		//! @attention [first, last) must be valid!
		constexpr
		array_ref(Type * first, Type * last) : first{first}, last{last} { if(first > last) throw std::invalid_argument{"array_ref requires [first, last)"}; }

		//! @brief construct array_ref from pointer and size
		//! @param[in] ptr start of the referenced array
		//! @param[in] count count of elements in the referenced array
		//! @attention [ptr, count) must be valid!
		constexpr
		array_ref(Type * ptr, std::size_t count) noexcept : array_ref{ptr, ptr + count} {}

		//! @brief construct array_ref from ContiguousRange
		//! @tparam ContiguousRange range type that fulfills the ContiguousRange-requirements
		//! @param[in] range range to reference
		template<typename ContiguousRange>
		constexpr
		array_ref(ContiguousRange && range) noexcept : array_ref{std::data(range), std::size(range)} {}

		constexpr
		auto data() const noexcept -> const Type * { return first; }
		constexpr
		auto data()       noexcept ->       Type * { return first; }
		constexpr
		auto size() const noexcept -> std::size_t { return last - first; }
		static
		constexpr
		auto max_size() noexcept { return std::numeric_limits<std::size_t>::max() / sizeof(Type); }

		constexpr
		void swap(array_ref & other) noexcept {
			std::swap(first, other.first);
			std::swap(last,  other.last);
		}
	};
	PTL_PACK_END

	template<typename Type, std::size_t Size>
	array_ref(Type (&)[Size]) -> array_ref<Type>;

	template<typename ContiguousRange>
	array_ref(const ContiguousRange &) -> array_ref<const typename ContiguousRange::value_type>;

	template<typename ContiguousRange>
	array_ref(      ContiguousRange &) -> array_ref<      typename ContiguousRange::value_type>;
}
