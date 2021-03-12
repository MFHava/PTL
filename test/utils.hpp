
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <iterator>
#include <stdexcept>

namespace ptl::test {
	struct moveable final {
		bool moved{false};

		moveable() noexcept =default;
		moveable(const moveable &) { throw std::runtime_error{"copy called"}; }
		moveable(moveable && other) noexcept : moved{other.moved} { other.moved = true; }

		auto operator=(const moveable &) -> moveable & { throw std::runtime_error{"copy called"}; }
		auto operator=(moveable && other) noexcept -> moveable & {
			moved = other.moved;
			other.moved = true;
			return *this;
		}
	};

	template<typename Iterator>
	class input_iterator final {
		Iterator it;
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type        = typename Iterator::value_type;
		using difference_type   = std::ptrdiff_t;
		using pointer           = typename Iterator::pointer;
		using reference         = typename Iterator::reference;

		input_iterator() noexcept =default;
		input_iterator(Iterator it) noexcept : it{it} {}

		auto operator++() noexcept -> input_iterator & { ++it; return *this; }
		auto operator++(int) noexcept -> input_iterator {
			auto tmp{*this};
			++*this;
			return tmp;
		}

		auto operator*() const noexcept -> reference { return *it; }
		auto operator->() const noexcept -> pointer { return &**this; }

		friend
		auto operator==(const input_iterator & lhs, const input_iterator & rhs) noexcept -> bool { return lhs.it == rhs.it; }
		friend
		auto operator!=(const input_iterator & lhs, const input_iterator & rhs) noexcept -> bool { return !(lhs == rhs); } //TODO: [C++20] implicitly generated
	};
}
