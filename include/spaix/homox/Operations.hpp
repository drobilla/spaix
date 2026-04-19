// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HOMOX_OPERATIONS_HPP
#define SPAIX_HOMOX_OPERATIONS_HPP

#include <spaix/homox/Point.hpp>
#include <spaix/homox/Rect.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>

namespace spaix::homox {

/**
   Operations used to measure and manipulate geometry.

   This implements all of the operations required by any of the insertion and
   split algorithms.
*/
template<class T, size_t D>
struct Operations {
  static_assert(D > 0, "at least one dimension is required");

  /// The union of several data keys
  using Box = Rect<T, D>;

  /// A number that measures volume (a product of all dimensions)
  using Volume = decltype(std::declval<T>() * std::declval<T>());

  /// Return the lower bound of a point in a dimension
  template<size_t dim>
  static constexpr T lower(const Point<T, D>& point)
  {
    return get<dim>(point);
  }

  /// Return the upper bound of a point in a dimension
  template<size_t dim>
  static constexpr T upper(const Point<T, D>& point)
  {
    return get<dim>(point);
  }

  /// Return the lower bound of a rect in a dimension
  template<size_t dim>
  static constexpr T lower(const Rect<T, D>& rect)
  {
    return get<dim>(rect).lower;
  }

  /// Return the upper bound of a rect in a dimension
  template<size_t dim>
  static constexpr T upper(const Rect<T, D>& rect)
  {
    return get<dim>(rect).upper;
  }

  /// Return the volume of a point (always zero)
  template<class... Ts>
  static constexpr Volume volume(const Point<T, D>&)
  {
    return {};
  }

  /// Return the volume of a rect
  static constexpr Volume volume(const Rect<T, D>& rect)
  {
    if (rect[0].upper <= rect[0].lower) {
      return {};
    }

    Volume result{rect[0].upper - rect[0].lower};

    for (size_t d = 1; d < D; ++d) {
      if (rect[d].upper <= rect[d].lower) {
        return {};
      }

      result *= (rect[d].upper - rect[d].lower);
    }

    return result;
  }

  /// Return the volume of a point
  static constexpr Volume volume(const Point<T, D>&) { return {}; }

  /// Return the geometric union of a rect and a point
  static constexpr Rect<T, D> unify(const Rect<T, D>&  lhs,
                                    const Point<T, D>& rhs)
  {
    using std::max;
    using std::min;

    Rect<T, D> result;

    for (size_t d = 0; d < D; ++d) {
      result[d].lower = min(lhs[d].lower, rhs[d]);
      result[d].upper = max(lhs[d].upper, rhs[d]);
    }

    return result;
  }

  /// Return the geometric union of a rect and a rect
  static constexpr Rect<T, D> unify(const Rect<T, D>& lhs,
                                    const Rect<T, D>& rhs)
  {
    using std::max;
    using std::min;

    Rect<T, D> result;

    for (size_t d = 0; d < D; ++d) {
      result[d].lower = min(lhs[d].lower, rhs[d].lower);
      result[d].upper = max(lhs[d].upper, rhs[d].upper);
    }

    return result;
  }

  /// Expand a rect to include a point
  static void expand(Rect<T, D>& parent, const Point<T, D>& child)
  {
    using std::max;
    using std::min;

    for (size_t d = 0; d < D; ++d) {
      parent[d].lower = min(parent[d].lower, child[d]);
      parent[d].upper = max(parent[d].upper, child[d]);
    }
  }

  /// Expand a rect to include another
  static void expand(Rect<T, D>& parent, const Rect<T, D>& child)
  {
    using std::max;
    using std::min;

    for (size_t d = 0; d < D; ++d) {
      parent[d].lower = min(parent[d].lower, child[d].lower);
      parent[d].upper = max(parent[d].upper, child[d].upper);
    }
  }
};

} // namespace spaix::homox

#endif // SPAIX_HOMOX_OPERATIONS_HPP
