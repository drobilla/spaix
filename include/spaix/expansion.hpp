// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_EXPANSION_HPP
#define SPAIX_EXPANSION_HPP

#include "spaix/Point.hpp"
#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"

#include <algorithm>
#include <cstddef>

namespace spaix {
namespace detail {

using std::max;
using std::min;

template<class Added, size_t last_dim, class... Ts>
SPAIX_ALWAYS_INLINE constexpr auto
expansion_rec(const Rect<Ts...>& base, const Added& added, LastIndex<last_dim>)
{
  const auto l  = range<last_dim>(base);
  const auto r  = range<last_dim>(added);
  const auto lo = min(l.lower, r.lower);
  const auto hi = max(l.upper, r.upper);

  const auto old_span = span<last_dim>(base);
  const auto new_span = hi - lo;

  return (old_span < new_span) ? (new_span - old_span) : 0;
}

template<class Added, size_t dim, size_t last_dim, class... Ts>
SPAIX_ALWAYS_INLINE constexpr auto
expansion_rec(const Rect<Ts...>&            base,
              const Added&                  added,
              InclusiveIndex<dim, last_dim> index)
{
  const auto l  = range<dim>(base);
  const auto r  = range<dim>(added);
  const auto lo = min(l.lower, r.lower);
  const auto hi = max(l.upper, r.upper);

  const auto old_span = span<dim>(base);
  const auto new_span = hi - lo;

  if (!(old_span < new_span)) {
    return expansion_rec(base, added, ++index);
  }

  const auto dim_expansion = new_span - old_span;
  const auto rest          = expansion_rec(base, added, ++index);
  if (rest == 0) {
    return static_cast<decltype(dim_expansion * rest)>(dim_expansion);
  }

  return dim_expansion * rest;
}

} // namespace detail

/**
   Return the volume that would be expanded by adding `rhs` to `lhs`.

   This is different from simply expanding (with union) then comparing volumes,
   because it ignores dimensions with zero span.  Specifically, it is the
   product of all non-zero dimension spans.  This does not have an intuitive
   geometric meaning, but it is useful as a volume-like metric that can be used
   to compare changes to infinitesimally thin rectangles (like a bounding box
   around a point, or several points that are aligned along some axis).
*/
template<class... Ts>
constexpr auto
expansion(const Rect<Ts...>& base, const Rect<Ts...>& added)
{
  return detail::expansion_rec(base, added, ibegin_inclusive<Ts...>());
}

/// Return the number of dimensions expanded by adding `added` to `base`
template<class... Ts>
constexpr auto
expansion(const Rect<Ts...>& base, const Point<Ts...>& added)
{
  return detail::expansion_rec(base, added, ibegin_inclusive<Ts...>());
}

} // namespace spaix

#endif // SPAIX_EXPANSION_HPP
