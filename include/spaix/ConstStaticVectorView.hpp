// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_CONSTSTATICVECTORVIEW_HPP
#define SPAIX_CONSTSTATICVECTORVIEW_HPP

#include <type_traits>

namespace spaix {

template<class T, class Size, Size Capacity>
class ConstStaticVectorView
{
public:
  using iterator       = T*;
  using const_iterator = const T*;
  using value_type     = T;

  using StorageElement = typename std::aligned_storage_t<sizeof(T), alignof(T)>;

  // NOLINTNEXTLINE(*-avoid-c-arrays)
  using Storage = StorageElement[Capacity];

  constexpr ConstStaticVectorView(const Size&    size,
                                  const Storage& array) noexcept
    : _size{size}
    , _array{array}
  {}

  ConstStaticVectorView(const ConstStaticVectorView&)                = delete;
  ConstStaticVectorView& operator=(const ConstStaticVectorView&)     = delete;
  ConstStaticVectorView(ConstStaticVectorView&&) noexcept            = default;
  ConstStaticVectorView& operator=(ConstStaticVectorView&&) noexcept = default;

  ~ConstStaticVectorView() = default;

  [[nodiscard]] const T& operator[](const Size index) const noexcept
  {
    return *reinterpret_cast<const T*>(&_array[index]);
  }

  [[nodiscard]] Size size() const noexcept { return _size; }

  [[nodiscard]] const_iterator begin() const noexcept
  {
    return reinterpret_cast<const T*>(_array);
  }

  [[nodiscard]] const_iterator end() const noexcept { return begin() + _size; }

private:
  Size           _size;
  const Storage& _array;
};

} // namespace spaix

#endif // SPAIX_CONSTSTATICVECTORVIEW_HPP
