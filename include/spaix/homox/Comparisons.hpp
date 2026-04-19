// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HOMOX_COMPARISONS_HPP
#define SPAIX_HOMOX_COMPARISONS_HPP

#include <spaix/homox/Point.hpp>
#include <spaix/homox/Rect.hpp>

#include <cstddef>

namespace spaix::homox {

template<class T, size_t D>
struct Comparisons {
  using Box = Rect<T, D>;

  static constexpr bool contains(const Box& parent, const Point<T, D>& child)
  {
    for (size_t d = 0U; d < D; ++d) {
      const auto& p = parent[d];
      const auto& c = child[d];

      if ((c < p.lower) || (p.upper < c)) {
        return false;
      }
    }

    return true;
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  static constexpr bool contains(const Box& parent, const Rect<T, D>& child)
  {
    for (size_t d = 0U; d < D; ++d) {
      const auto& p = parent[d];
      const auto& c = child[d];

      if ((c.lower < p.lower) || (p.upper < c.upper)) {
        return false;
      }
    }

    return true;
  }

  static constexpr bool intersects(const Box& lhs, const Point<T, D>& rhs)
  {
    for (size_t d = 0U; d < D; ++d) {
      const auto l = lhs[d];
      const auto r = rhs[d];

      if ((r < l.lower) || (l.upper < r)) {
        return false;
      }
    }

    return true;
  }

  static constexpr bool intersects(const Box& lhs, const Rect<T, D>& rhs)
  {
    for (size_t d = 0U; d < D; ++d) {
      const auto l = lhs[d];
      const auto r = rhs[d];

      if ((r.upper < l.lower) || (l.upper < r.lower)) {
        return false;
      }
    }

    return true;
  }
};

} // namespace spaix::homox

#endif // SPAIX_HOMOX_COMPARISONS_HPP
