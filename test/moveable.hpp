
//          Copyright Michael Florian Hava.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

namespace ptl {
	namespace test {
		struct moveable final {
			bool moved{false};

			moveable() {}
			moveable(const moveable &) { throw std::runtime_error{"copy called"}; }
			moveable(moveable && other) noexcept : moved{other.moved} { other.moved = true; }

			auto operator=(const moveable &) -> moveable & { throw std::runtime_error{"copy called"}; }
			auto operator=(moveable && other) noexcept -> moveable & {
				moved = other.moved;
				other.moved = true;
				return *this;
			}

			~moveable() noexcept =default;
		};
	}
}
