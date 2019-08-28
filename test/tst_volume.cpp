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

#include "spaix/volume.hpp"

namespace spaix {
namespace test {

static void
test_volume()
{
  STATIC_CHECK((volume(TestRect{{1, 3}, {2, 5}}) == 6));
  STATIC_CHECK((volume(TestRect{{1, 1}, {2, 5}}) == 0));
  STATIC_CHECK((volume(TestRect{{1, 3}, {2, 2}}) == 0));
}

} // namespace test
} // namespace spaix

int
main()
{
  spaix::test::test_volume();
  return 0;
}
