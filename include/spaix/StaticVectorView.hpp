// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_STATICVECTORVIEW_HPP
#define SPAIX_STATICVECTORVIEW_HPP

#include <new>
#include <type_traits>
#include <utility>

namespace spaix {

template<class T, class Size, Size Capacity>
class StaticVectorView
{
public:
  using iterator       = T*;
  using const_iterator = const T*;
  using value_type     = T;

  using ElementStorage = typename std::aligned_storage_t<sizeof(T), alignof(T)>;

  // NOLINTNEXTLINE(*-avoid-c-arrays)
  using Storage = ElementStorage[Capacity];

  constexpr StaticVectorView(Size& size, Storage& array) noexcept
    : _size{size}
    , _array{array}
  {}

  StaticVectorView(const StaticVectorView&)                = delete;
  StaticVectorView& operator=(const StaticVectorView&)     = delete;
  StaticVectorView(StaticVectorView&&) noexcept            = default;
  StaticVectorView& operator=(StaticVectorView&&) noexcept = default;

  ~StaticVectorView() = default;

  template<class... Args>
  void emplace_back(Args&&... args)
  {
    assert(_size < Capacity);

    new (&_array[_size++]) T(std::forward<Args>(args)...);
  }

  [[nodiscard]] const T& operator[](const Size index) const noexcept
  {
    return *reinterpret_cast<const T*>(&_array[index]);
  }

  [[nodiscard]] T& operator[](const Size index) noexcept
  {
    return *reinterpret_cast<T*>(&_array[index]);
  }

  void clear() noexcept
  {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (Size i = 0; i < _size; ++i) {
        reinterpret_cast<T*>(&_array[i])->~T();
      }
    }

    _size = 0;
  }

  [[nodiscard]] Size size() const noexcept { return _size; }

  [[nodiscard]] iterator begin() noexcept
  {
    return reinterpret_cast<T*>(_array);
  }

  [[nodiscard]] const_iterator begin() const noexcept
  {
    return reinterpret_cast<const T*>(_array);
  }

  [[nodiscard]] iterator       end() noexcept { return begin() + _size; }
  [[nodiscard]] const_iterator end() const noexcept { return begin() + _size; }

private:
  Size&    _size;
  Storage& _array;
};

} // namespace spaix

#endif // SPAIX_STATICVECTORVIEW_HPP
