// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_STATICVECTOR_HPP
#define SPAIX_STATICVECTOR_HPP

#include <spaix/ConstStaticVectorView.hpp>
#include <spaix/StaticVectorView.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace spaix {

template<class T, class Size, Size max_size>
class StaticVector
{
public:
  static_assert(max_size);

  static_assert(alignof(T) <= sizeof(T),
                "element alignment must not exceed its size");

  static_assert((sizeof(T) % alignof(T)) == 0U,
                "element size must be a multiple of its alignment");

  using const_iterator = const T*;
  using iterator       = T*;
  using value_type     = T;
  using size_type      = Size;

  StaticVector() noexcept = default;

  StaticVector(StaticVector&& vec) noexcept
  {
    for (Size i = 0; i < vec.size(); ++i) {
      new (&begin()[i]) T{std::move(vec[i])};
      ++_size;
    }
  }

  StaticVector(const StaticVector& vec) noexcept
  {
    for (Size i = 0; i < vec.size(); ++i) {
      new (&begin()[i]) T{vec[i]};
      ++_size;
    }
  }

  StaticVector& operator=(const StaticVector&) = delete;
  StaticVector& operator=(StaticVector&&)      = delete;

  ~StaticVector() { clear(); }

  void clear() noexcept { impl().clear(); }

  void pop_at(const size_t index) noexcept(
    std::is_nothrow_move_constructible_v<T>)
  {
    impl().pop_at(index);
  }

  void pop_back() noexcept { impl().pop_back(); }

  template<class... Args>
  void emplace_back(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args...>)
  {
    impl().emplace_back(std::forward<Args>(args)...);
  }

  [[nodiscard]] bool empty() const noexcept { return _size == 0; }

  [[nodiscard]] const T& back() const noexcept { return impl().back(); }

  [[nodiscard]] T& back() noexcept { return impl().back(); }

  [[nodiscard]] const T& operator[](const Size index) const noexcept
  {
    return impl().operator[](index);
  }

  [[nodiscard]] T& operator[](const Size index) noexcept
  {
    return impl().operator[](index);
  }

  [[nodiscard]] Size                  size() const noexcept { return _size; }
  [[nodiscard]] static constexpr Size capacity() noexcept { return max_size; }

  [[nodiscard]] iterator begin() noexcept
  {
    return reinterpret_cast<T*>(&_data);
  }

  [[nodiscard]] const_iterator begin() const noexcept
  {
    return reinterpret_cast<const T*>(&_data);
  }

  [[nodiscard]] const_iterator cbegin() const noexcept
  {
    return reinterpret_cast<const T*>(&_data);
  }

  [[nodiscard]] iterator       end() noexcept { return begin() + _size; }
  [[nodiscard]] const_iterator end() const noexcept { return begin() + _size; }
  [[nodiscard]] const_iterator cend() const noexcept { return begin() + _size; }

private:
  [[nodiscard]] StaticVectorView<T, Size, max_size> impl() noexcept
  {
    return StaticVectorView<T, Size, max_size>{_size, begin()};
  }

  [[nodiscard]] ConstStaticVectorView<T, Size, max_size> impl() const noexcept
  {
    return ConstStaticVectorView<T, Size, max_size>{_size, begin()};
  }

  Size _size{};

  struct alignas(T) Data {
    std::array<std::byte, max_size * sizeof(T)> bytes;
  };

  Data _data{};
};

template<class T, class Size, Size max_size>
[[nodiscard]] inline bool
operator<(const spaix::StaticVector<T, Size, max_size>& lhs,
          const spaix::StaticVector<T, Size, max_size>& rhs) noexcept
{
  using std::lexicographical_compare;

  return lexicographical_compare(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

} // namespace spaix

#endif // SPAIX_STATICVECTOR_HPP
