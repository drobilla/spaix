/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#undef NDEBUG

#include "Scalar.hpp"
#include "TestRect.hpp"
#include "check.hpp"

#include "spaix/Rect.hpp"
#include "spaix/union.hpp"

namespace spaix {
namespace test {

static void
test_union()
{
  constexpr auto rect = TestRect{{1, 3}, {2, 5}};

  STATIC_CHECK(((rect | TestRect{{2, 4}, {1, 5}}) == TestRect{{1, 4}, {1, 5}}));

  STATIC_CHECK(((rect | TestPoint{0, 2}) == TestRect{{0, 3}, {2, 5}}));
  STATIC_CHECK(((rect | TestPoint{4, 2}) == TestRect{{1, 4}, {2, 5}}));
  STATIC_CHECK(((rect | TestPoint{1, 1}) == TestRect{{1, 3}, {1, 5}}));
  STATIC_CHECK(((rect | TestPoint{1, 6}) == TestRect{{1, 3}, {2, 6}}));

  STATIC_CHECK(((TestPoint{0, 2} | rect) == TestRect{{0, 3}, {2, 5}}));
  STATIC_CHECK(((TestPoint{4, 2} | rect) == TestRect{{1, 4}, {2, 5}}));
  STATIC_CHECK(((TestPoint{1, 1} | rect) == TestRect{{1, 3}, {1, 5}}));
  STATIC_CHECK(((TestPoint{1, 6} | rect) == TestRect{{1, 3}, {2, 6}}));

  STATIC_CHECK(((TestPoint{1, 2} | TestPoint{3, 5}) == rect));

  auto mut = TestRect{{1, 3}, {2, 5}};
  mut |= TestRect{{2, 4}, {1, 5}};
  CHECK((mut == TestRect{{1, 4}, {1, 5}}));
}

} // namespace test
} // namespace spaix

int
main()
{
  spaix::test::test_union();
  return 0;
}
