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

#ifndef TEST_TESTRECT_HPP
#define TEST_TESTRECT_HPP

#include "Scalar.hpp"

#include "spaix/Rect.hpp"

namespace spaix {
namespace test {

using XCoord = Scalar<struct X, int>;
using YCoord = Scalar<struct Y, float>;

using XRange = Range<XCoord>;
using YRange = Range<YCoord>;

using TestRect  = Rect<XCoord, YCoord>;
using TestPoint = Point<XCoord, YCoord>;

constexpr auto operator*(const XCoord& lhs, const YCoord& rhs)
{
  return lhs.value() * rhs.value();
}

} // namespace test
} // namespace spaix

#endif // TEST_TESTRECT_HPP
