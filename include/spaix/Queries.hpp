// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_QUERIES_HPP
#define SPAIX_QUERIES_HPP

#include <spaix/Point.hpp>
#include <spaix/Rect.hpp>
#include <spaix/detail/attributes.hpp>
#include <spaix/detail/meta.hpp>
#include <spaix/search/touching.hpp>
#include <spaix/search/within.hpp>

#include <cstddef>

namespace spaix {
namespace detail {

template<class Parent, class Child, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr bool
contains_rec(const Parent&, const Child&, EndIndex<n_dims>)
{
  return true;
}

template<class Parent, class Child, size_t dim, size_t n_dims>
SPAIX_ALWAYS_INLINE constexpr bool
contains_rec(const Parent& parent, const Child& child, Index<dim, n_dims> index)
{
  const auto& p = range<dim>(parent);
  const auto& c = range<dim>(child);

  return (!(c.lower < p.lower) && !(p.upper < c.upper) &&
          contains_rec(parent, child, ++index));
}

template<class Lhs, class Rhs, size_t num_dims>
SPAIX_ALWAYS_INLINE constexpr bool
intersects_rec(const Lhs&, const Rhs&, EndIndex<num_dims>)
{
  return true;
}

template<class Lhs, class Rhs, size_t dim, size_t num_dims>
SPAIX_ALWAYS_INLINE constexpr bool
intersects_rec(const Lhs& lhs, const Rhs& rhs, Index<dim, num_dims> index)
{
  const auto& l = range<dim>(lhs);
  const auto& r = range<dim>(rhs);

  return (!(r.upper < l.lower) && !(l.upper < r.lower) &&
          intersects_rec(lhs, rhs, ++index));
}

} // namespace detail

template<class... Ts>
struct Queries {
  using DirKey   = Rect<Ts...>;
  using QueryKey = Rect<Ts...>;

  static constexpr bool contains(const DirKey& lhs, const Point<Ts...>& rhs)
  {
    return detail::contains_rec(lhs, rhs, rhs.ibegin());
  }

  static constexpr bool contains(const DirKey& lhs, const Rect<Ts...>& rhs)
  {
    return detail::contains_rec(lhs, rhs, rhs.ibegin());
  }

  static constexpr bool intersects(const DirKey& lhs, const Point<Ts...>& rhs)
  {
    return detail::intersects_rec(lhs, rhs, rhs.ibegin());
  }

  static constexpr bool intersects(const DirKey& lhs, const Rect<Ts...>& rhs)
  {
    return detail::intersects_rec(lhs, rhs, rhs.ibegin());
  }

  using Contained   = spaix::search::Within<Queries, DirKey>;
  using Intersected = spaix::search::Touching<Queries, DirKey>;
};

} // namespace spaix

#endif // SPAIX_QUERIES_HPP
