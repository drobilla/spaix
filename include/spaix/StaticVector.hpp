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

#ifndef SPAIX_STATICVECTOR_HPP
#define SPAIX_STATICVECTOR_HPP

#include <array>
#include <cassert>

namespace spaix {

template <class T, class Size, Size capacity>
class StaticVector
{
public:
  void pop_back()
  {
    assert(_size);
    _array[--_size] = T{};
  }

  template <class... Args>
  void emplace_back(Args&&... args)
  {
    assert(_size < capacity);
    _array[_size++] = T{std::forward<Args>(args)...};
  }

  bool empty() const { return _size == 0; }

  const T& back() const
  {
    assert(_size > 0);
    return _array[_size - 1];
  }

  T& back()
  {
    assert(_size > 0);
    return _array[_size - 1];
  }

  bool operator==(const StaticVector& rhs) const
  {
    for (size_t i = 0; i < _size; ++i) {
      if (!(_array[i] == rhs._array[i])) {
        return false;
      }
    }

    return true;
  }

  void clear() { _size = 0; }

private:
  Size                    _size{};
  std::array<T, capacity> _array{};
};

} // namespace spaix

#endif // SPAIX_STATICVECTOR_HPP
