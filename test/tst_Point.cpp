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

#include "spaix/Point.hpp"

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace spaix {
namespace test {

static void
test_point()
{
  constexpr auto point = make_point(XCoord{1}, YCoord{2});

  STATIC_CHECK((TestPoint{std::make_tuple(XCoord{1}, YCoord{2})} == point));

  // Comparison
  STATIC_CHECK((point == TestPoint{1, 2}));
  STATIC_CHECK((point != TestPoint{2, 2}));
  STATIC_CHECK((point != TestPoint{1, 3}));

  // Basic access
  STATIC_CHECK((ranges(point) == std::make_tuple(XRange{1, 1}, YRange{2, 2})));
  STATIC_CHECK((point.size() == 2));
  STATIC_CHECK((get<0>(point) == 1));
  STATIC_CHECK((get<1>(point) == 2));
  STATIC_CHECK((range<0>(point) == XRange{1, 1}));
  STATIC_CHECK((range<1>(point) == YRange{2, 2}));
  STATIC_CHECK((min<0>(point) == 1));
  STATIC_CHECK((min<1>(point) == 2));
  STATIC_CHECK((max<0>(point) == 1));
  STATIC_CHECK((max<1>(point) == 2));
  STATIC_CHECK((span<0>(point) == 0));

  std::ostringstream ss;
  ss << make_point(1, 2);
  CHECK((ss.str() == "[1, 2]"));
}

} // namespace test
} // namespace spaix

int
main()
{
  spaix::test::test_point();

  return 0;
}