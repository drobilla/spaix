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

#include "TestRect.hpp"
#include "check.hpp"

#include "spaix/intersects.hpp"

namespace spaix {
namespace test {

static void
test_intersects()
{
  constexpr auto rect = TestRect{{1, 3}, {2.0f, 5.0f}};

  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {2.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{0, 3}, {2.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 4}, {2.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {1.0f, 5.0f}})));
  STATIC_CHECK((intersects(rect, TestRect{{1, 3}, {2.0f, 6.0f}})));

  STATIC_CHECK((!intersects(rect, TestRect{{0, 0}, {2.0f, 5.0f}})));
  STATIC_CHECK((!intersects(rect, TestRect{{4, 5}, {2.0f, 5.0f}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1, 3}, {0.0f, 1.0f}})));
  STATIC_CHECK((!intersects(rect, TestRect{{1, 3}, {6.0f, 7.0f}})));

  STATIC_CHECK((intersects(rect, TestPoint{1, 2.0f})));
  STATIC_CHECK((intersects(rect, TestPoint{2, 5.0f})));
  STATIC_CHECK((!intersects(rect, TestPoint{0, 2.0f})));
  STATIC_CHECK((!intersects(rect, TestPoint{1, 0.0f})));

  STATIC_CHECK((intersects(TestPoint{1, 2.0f}, rect)));
  STATIC_CHECK((intersects(TestPoint{2, 5.0f}, rect)));
  STATIC_CHECK((!intersects(TestPoint{0, 2.0f}, rect)));
  STATIC_CHECK((!intersects(TestPoint{1, 0.0f}, rect)));
}

} // namespace test
} // namespace spaix

int
main()
{
  spaix::test::test_intersects();
  return 0;
}
