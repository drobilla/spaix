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

#ifndef TEST_WRITE_ROW_HPP
#define TEST_WRITE_ROW_HPP

#include <iostream>

namespace spaix {
namespace test {

template<class Last>
void
write_row(std::ostream& os, Last last)
{
  os << last << '\n';
}

template<class First, class... Rest>
void
write_row(std::ostream& os, First first, Rest... rest)
{
  os << first << '\t';
  write_row(os, std::forward<Rest>(rest)...);
}

} // namespace test
} // namespace spaix

#endif // TEST_WRITE_ROW_HPP
