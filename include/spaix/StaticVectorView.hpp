// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_STATICVECTORVIEW_HPP
#define SPAIX_STATICVECTORVIEW_HPP

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace spaix {

template<class T, class Size, Size max_size>
class StaticVectorView
{
public:
  static_assert(max_size);

  using iterator       = T*;
  using const_iterator = const T*;
  using value_type     = T;
  using size_type      = Size;

  constexpr StaticVectorView(Size& size, T* const array) noexcept
    : _size{&size}
    , _array{array}
  {}

  StaticVectorView(const StaticVectorView&)            = delete;
  StaticVectorView& operator=(const StaticVectorView&) = delete;

  StaticVectorView(StaticVectorView&&) noexcept            = default;
  StaticVectorView& operator=(StaticVectorView&&) noexcept = default;

  ~StaticVectorView() = default;

  void pop_at(const size_t index) noexcept(
    std::is_nothrow_move_constructible_v<T>)
  {
    assert(index < size());
    if (index != size() - 1U) {
      auto       last = std::move(back());
      const auto i    = begin() + index;

      if constexpr (!std::is_trivially_destructible_v<T>) {
        i->~T();
      }

      new (i) T{std::move(last)};
    }

    pop_back();
  }

  void pop_back() noexcept
  {
    assert(*_size);
    --*_size;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      _array[*_size].~T();
    }
  }

  template<class... Args>
  void emplace_back(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args...>)
  {
    assert(size() < max_size);

    new (&_array[(*_size)++]) T(std::forward<Args>(args)...);
  }

  [[nodiscard]] T& back() noexcept
  {
    assert(size() > 0);
    return _array[size() - 1];
  }

  [[nodiscard]] const T& operator[](const Size index) const noexcept
  {
    assert(index < size());
    return _array[index];
  }

  [[nodiscard]] T& operator[](const Size index) noexcept
  {
    assert(index < size());
    return _array[index];
  }

  void clear() noexcept
  {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (Size i = 0; i < size(); ++i) {
        _array[i].~T();
      }
    }

    *_size = 0;
  }

  [[nodiscard]] Size           size() const noexcept { return *_size; }
  [[nodiscard]] iterator       begin() noexcept { return _array; }
  [[nodiscard]] const_iterator begin() const noexcept { return _array; }
  [[nodiscard]] iterator       end() noexcept { return begin() + size(); }
  [[nodiscard]] const_iterator end() const noexcept { return begin() + size(); }

private:
  Size* _size;
  T*    _array;
};

} // namespace spaix

#endif // SPAIX_STATICVECTORVIEW_HPP
