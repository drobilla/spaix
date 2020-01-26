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

#include <limits>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace spaix {
namespace test {

static void
test_rect()
{
  constexpr auto rect = make_rect(XRange{1, 3}, YRange{2.0f, 5.0f});

  STATIC_CHECK((Rect<int, int>{} ==
                Rect<int, int>{{std::numeric_limits<int>::max(),
                                std::numeric_limits<int>::lowest()},
                               {std::numeric_limits<int>::max(),
                                std::numeric_limits<int>::lowest()}}));

  STATIC_CHECK((Rect<float, float>{} ==
                Rect<float, float>{{std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::lowest()},
                                   {std::numeric_limits<float>::max(),
                                    std::numeric_limits<float>::lowest()}}));

  STATIC_CHECK((TestRect{TestPoint{1, 2.0f}} ==
                TestRect{{1, 1}, {2.0f, 2.0f}}));

  STATIC_CHECK((TestRect{std::make_tuple(XRange{1, 3}, YRange{2.0f, 5.0f})} ==
                rect));

  // Comparison
  STATIC_CHECK((rect == make_rect(XRange{1, 3}, YRange{2.0f, 5.0f})));
  STATIC_CHECK((rect != make_rect(XRange{2, 3}, YRange{2.0f, 5.0f})));
  STATIC_CHECK((rect != make_rect(XRange{1, 3}, YRange{4.0f, 5.0f})));

  // Basic access
  STATIC_CHECK((ranges(rect) == std::make_tuple(XRange{1, 3},
                                                YRange{2.0f, 5.0f})));
  STATIC_CHECK((rect.size() == 2));
  STATIC_CHECK((get<0>(rect) == XRange{1, 3}));
  STATIC_CHECK((get<1>(rect) == YRange{2.0f, 5.0f}));
  STATIC_CHECK((range<0>(rect) == XRange{1, 3}));
  STATIC_CHECK((range<1>(rect) == YRange{2.0f, 5.0f}));
  STATIC_CHECK((min<0>(rect) == 1));
  STATIC_CHECK((min<1>(rect) == 2.0f));
  STATIC_CHECK((max<0>(rect) == 3));
  STATIC_CHECK((max<1>(rect) == 5.0f));
  STATIC_CHECK((span<0>(rect) == 2));
  STATIC_CHECK((span<1>(rect) == 3.0f));

  // Printing
  std::ostringstream ss;
  ss << make_rect(std::make_pair(1, 2), std::make_pair(3, 4));
  CHECK((ss.str() == "[[1, 2], [3, 4]]"));
}

} // namespace test
} // namespace spaix

int
main()
{
  spaix::test::test_rect();
  return 0;
}
