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

#ifndef SPAIX_RANGE_HPP
#define SPAIX_RANGE_HPP

#include <utility>

namespace spaix {

template <class T>
using Range = std::pair<T, T>;

template <class T>
constexpr Range<T>
make_range(T min, T max)
{
  return std::make_pair(min, max);
}

} // namespace spaix

#endif // SPAIX_RANGE_HPP
