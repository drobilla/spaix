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
#include <memory>
#include <type_traits>

namespace spaix {

template <class T, class Size, Size Capacity>
class StaticVector
{
public:
  StaticVector() = default;
  ~StaticVector() { clear(); }

  StaticVector(StaticVector&& vec) noexcept
  {
    for (Size i = 0; i < vec.size(); ++i) {
      new (&_array[i]) T(std::move(vec[i]));
      ++_size;
    }
  }

  StaticVector(const StaticVector& vec)
  {
    for (Size i = 0; i < vec.size(); ++i) {
      new (&_array[i]) T(vec[i]);
      ++_size;
    }
  }

	// TODO (maybe, unused...)
  StaticVector& operator=(StaticVector&) = delete;
  StaticVector& operator=(StaticVector&&) = delete;

  void pop_back()
  {
    assert(!empty());
    --_size;

    if (!std::is_trivially_destructible<T>::value) {
      reinterpret_cast<T*>(&_array[_size])->~T();
    }
  }

  template <class... Args>
  void emplace_back(Args&&... args)
  {
    assert(_size < Capacity);

    new (&_array[_size++]) T(std::forward<Args>(args)...);
  }

  bool empty() const { return _size == 0; }

  const T& back() const
  {
    assert(_size > 0);
    return *reinterpret_cast<const T*>(&_array[_size - 1]);
  }

  T& back()
  {
    assert(_size > 0);
    return *reinterpret_cast<T*>(&_array[_size - 1]);
  }

  const T& operator[](const Size index) const
  {
    return *reinterpret_cast<const T*>(&_array[index]);
  }

  T& operator[](const Size index)
  {
    return *reinterpret_cast<T*>(&_array[index]);
  }

  bool operator==(const StaticVector& rhs) const
  {
    return _size == rhs._size && std::equal(begin(), end(), rhs.begin());
  }

  void clear()
  {
    if (!std::is_trivially_destructible<T>::value) {
      for (Size i = 0; i < _size; ++i) {
        reinterpret_cast<T*>(&_array[i])->~T();
      }
    }

    _size = 0;
  }

  Size                  size() const { return _size; }
  static constexpr Size capacity() { return Capacity; }

  T*       begin() { return reinterpret_cast<T*>(_array); }
  const T* begin() const { return reinterpret_cast<const T*>(_array); }
  T*       end() { return begin() + _size; }
  const T* end() const { return begin() + _size; }

  using iterator               = T*;
  using const_iterator         = const T*;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  reverse_iterator       rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }

  reverse_iterator       rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }

private:
  using Element = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  Size    _size{};
  Element _array[Capacity];
};

} // namespace spaix

#endif // SPAIX_STATICVECTOR_HPP
