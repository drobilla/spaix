// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_STATICVECTOR_HPP
#define SPAIX_STATICVECTOR_HPP

#include <algorithm>
#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

namespace spaix {

template<class T, class Size, Size Capacity>
class StaticVector
{
public:
  using const_iterator = const T*;
  using iterator       = T*;
  using value_type     = T;

  StaticVector() noexcept = default;

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

  StaticVector& operator=(StaticVector&) = delete;
  StaticVector& operator=(StaticVector&&) = delete;

  void pop(T* const iter)
  {
    assert(iter < cend());
    if (iter != cend() - 1) {
      auto last = std::move(back());

      if constexpr (!std::is_trivially_destructible_v<T>) {
        iter->~T();
      }

      new (iter) T(std::move(last));
    }

    pop_back();
  }

  void pop_back()
  {
    assert(!empty());
    --_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      reinterpret_cast<T*>(&_array[_size])->~T();
    }
  }

  template<class... Args>
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

  void clear()
  {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (Size i = 0; i < _size; ++i) {
        reinterpret_cast<T*>(&_array[i])->~T();
      }
    }

    _size = 0;
  }

  Size                  size() const { return _size; }
  static constexpr Size capacity() { return Capacity; }

  iterator       begin() { return reinterpret_cast<T*>(_array); }
  const_iterator begin() const { return reinterpret_cast<const T*>(_array); }
  const_iterator cbegin() const { return reinterpret_cast<const T*>(_array); }
  iterator       end() { return begin() + _size; }
  const_iterator end() const { return begin() + _size; }
  const_iterator cend() const { return begin() + _size; }

private:
  using Element = typename std::aligned_storage_t<sizeof(T), alignof(T)>;

  Size    _size{};
  Element _array[Capacity]{};
};

template<class T, class Size, Size capacity>
bool
operator==(const spaix::StaticVector<T, Size, capacity>& lhs,
           const spaix::StaticVector<T, Size, capacity>& rhs) noexcept
{
  using std::equal;

  return lhs.size() == rhs.size() && equal(lhs.begin(), rhs.end(), rhs.begin());
}

template<class T, class Size, Size capacity>
bool
operator<(const spaix::StaticVector<T, Size, capacity>& lhs,
          const spaix::StaticVector<T, Size, capacity>& rhs) noexcept
{
  using std::lexicographical_compare;

  return lexicographical_compare(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

} // namespace spaix

#endif // SPAIX_STATICVECTOR_HPP
