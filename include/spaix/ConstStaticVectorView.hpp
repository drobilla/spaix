// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_CONSTSTATICVECTORVIEW_HPP
#define SPAIX_CONSTSTATICVECTORVIEW_HPP

#include <cassert>

namespace spaix {

template<class T, class Size, Size max_size>
class ConstStaticVectorView
{
public:
  static_assert(max_size);

  using iterator       = T*;
  using const_iterator = const T*;
  using value_type     = T;
  using size_type      = Size;

  constexpr ConstStaticVectorView(const Size& size, const T* array) noexcept
    : _size{size}
    , _array{array}
  {}

  ConstStaticVectorView(const ConstStaticVectorView&)            = delete;
  ConstStaticVectorView& operator=(const ConstStaticVectorView&) = delete;

  ConstStaticVectorView(ConstStaticVectorView&&) noexcept            = delete;
  ConstStaticVectorView& operator=(ConstStaticVectorView&&) noexcept = delete;

  ~ConstStaticVectorView() = default;

  [[nodiscard]] const T& back() noexcept
  {
    assert(_size > 0);
    return _array[_size - 1];
  }

  [[nodiscard]] const T& operator[](const Size index) const noexcept
  {
    return _array[index];
  }

  [[nodiscard]] Size           size() const noexcept { return _size; }
  [[nodiscard]] const_iterator begin() const noexcept { return _array; }
  [[nodiscard]] const_iterator end() const noexcept { return begin() + _size; }

private:
  const Size& _size;
  const T*    _array;
};

} // namespace spaix

#endif // SPAIX_CONSTSTATICVECTORVIEW_HPP
