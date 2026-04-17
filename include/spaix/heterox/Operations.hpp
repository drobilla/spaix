// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HETEROX_OPERATIONS_HPP
#define SPAIX_HETEROX_OPERATIONS_HPP

#include <spaix/detail/Index.hpp>
#include <spaix/detail/attributes.hpp>
#include <spaix/heterox/Point.hpp>
#include <spaix/heterox/Rect.hpp>
#include <spaix/heterox/detail/meta.hpp>
#include <spaix/types.hpp>

#include <algorithm>
#include <cstddef>
#include <tuple>

namespace spaix::heterox {
namespace detail {

using std::max;
using std::min;

template<class... Ts, size_t last_dim>
constexpr auto
volume_rec(const Rect<Ts...>& rect, detail::LastIndex<last_dim>) noexcept
{
  return span<last_dim>(rect);
}

template<class... Ts, size_t dim, size_t last_dim>
constexpr auto
volume_rec(const Rect<Ts...>&                    rect,
           detail::InclusiveIndex<dim, last_dim> index) noexcept
{
  const auto r = range<dim>(rect);

  if (r.lower < r.upper) {
    return (r.upper - r.lower) * volume_rec(rect, ++index);
  }

  return detail::DifferenceOf<detail::Nth<dim + 1U, Ts...>,
                              detail::Nth<dim + 1U, Ts...>>{};
}

template<class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
unify_rec(const Lhs&, const Rhs&, ::spaix::detail::EndIndex<n_dims>) noexcept
{
  return std::make_tuple();
}

template<class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr auto
unify_rec(const Lhs&                          lhs,
          const Rhs&                          rhs,
          ::spaix::detail::Index<dim, n_dims> index) noexcept
{
  const auto l  = range<dim>(lhs);
  const auto r  = range<dim>(rhs);
  const auto lo = min(l.lower, r.lower);
  const auto hi = max(l.upper, r.upper);

  return std::tuple_cat(std::make_tuple(make_dim_range(lo, hi)),
                        unify_rec(lhs, rhs, ++index));
}

template<class Lhs, class Rhs, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr void
expand_rec(Lhs&, const Rhs&, ::spaix::detail::EndIndex<n_dims>) noexcept
{}

template<class Lhs, class Rhs, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr void
expand_rec(Lhs&                                lhs,
           const Rhs&                          rhs,
           ::spaix::detail::Index<dim, n_dims> index) noexcept
{
  auto&      l = range<dim>(lhs);
  const auto r = range<dim>(rhs);

  l.lower = min(l.lower, r.lower);
  l.upper = max(l.upper, r.upper);

  expand_rec(lhs, rhs, ++index);
}

} // namespace detail

/**
   Operations used to measure and manipulate geometry.

   This implements all of the operations required by any of the insertion and
   split algorithms.
*/
template<typename T0, typename... Tn>
struct Operations {
  /// A number that measures volume (a product of all dimensions)
  using Volume = detail::ProductOf<T0, Tn...>;

  /// Return the lower bound of a point in a dimension
  template<size_t dim>
  static constexpr detail::Nth<dim, T0, Tn...> lower(
    const Point<T0, Tn...>& point) noexcept
  {
    return get<dim>(point);
  }

  /// Return the upper bound of a point in a dimension
  template<size_t dim>
  static constexpr detail::Nth<dim, T0, Tn...> upper(
    const Point<T0, Tn...>& point) noexcept
  {
    return get<dim>(point);
  }

  /// Return the lower bound of a rect in a dimension
  template<size_t dim>
  static constexpr detail::Nth<dim, T0, Tn...> lower(
    const Rect<T0, Tn...>& rect) noexcept
  {
    return get<dim>(rect).lower;
  }

  /// Return the upper bound of a rect in a dimension
  template<size_t dim>
  static constexpr detail::Nth<dim, T0, Tn...> upper(
    const Rect<T0, Tn...>& rect) noexcept
  {
    return get<dim>(rect).upper;
  }

  /// Return the volume of a point (always zero) noexcept
  static constexpr Volume volume(const Point<T0, Tn...>&) { return {}; }

  /// Return the volume of a rect
  static constexpr Volume volume(const Rect<T0, Tn...>& rect) noexcept
  {
    return detail::volume_rec(rect, detail::ibegin_inclusive<T0, Tn...>());
  }

  /// Return the geometric union of a rect and a point
  static constexpr Rect<T0, Tn...> unify(const Rect<T0, Tn...>&  lhs,
                                         const Point<T0, Tn...>& rhs) noexcept
  {
    return Rect<T0, Tn...>{detail::unify_rec(lhs, rhs, rhs.ibegin())};
  }

  /// Return the geometric union of a rect and a rect
  static constexpr Rect<T0, Tn...> unify(const Rect<T0, Tn...>& lhs,
                                         const Rect<T0, Tn...>& rhs) noexcept
  {
    return Rect<T0, Tn...>{detail::unify_rec(lhs, rhs, rhs.ibegin())};
  }

  /// Expand a rect to include a point
  static void expand(Rect<T0, Tn...>&        parent,
                     const Point<T0, Tn...>& child) noexcept
  {
    detail::expand_rec(parent, child, child.ibegin());
  }

  /// Expand a rect to include another
  static void expand(Rect<T0, Tn...>&       parent,
                     const Rect<T0, Tn...>& child) noexcept
  {
    detail::expand_rec(parent, child, child.ibegin());
  }
};

} // namespace spaix::heterox

#endif // SPAIX_HETEROX_OPERATIONS_HPP
