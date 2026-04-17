// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HOMOX_QUERIES_HPP
#define SPAIX_HOMOX_QUERIES_HPP

#include <spaix/homox/Point.hpp>
#include <spaix/homox/Rect.hpp>

#include <cstddef>
#include <utility>

namespace spaix::homox {

template<class T, size_t D>
struct Queries {
  using DirKey   = Rect<T, D>;
  using QueryKey = Rect<T, D>;

  static constexpr bool contains(const DirKey& parent, const Point<T, D>& child)
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

  static constexpr bool contains(const DirKey& parent, const Rect<T, D>& child)
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

  static constexpr bool intersects(const DirKey& lhs, const Point<T, D>& rhs)
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

  static constexpr bool intersects(const DirKey& lhs, const Rect<T, D>& rhs)
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

  struct Everything {
    static constexpr bool directory(const DirKey&) { return true; }

    template<class DatKey>
    static constexpr bool leaf(const DatKey&)
    {
      return true;
    }
  };

  struct Contained {
    explicit Contained(QueryKey key)
      : _query_key{std::move(key)}
    {}

    constexpr bool directory(const DirKey& k) const
    {
      return intersects(_query_key, k);
    }

    template<class DatKey>
    constexpr bool leaf(const DatKey& k) const
    {
      return contains(_query_key, k);
    }

  private:
    const QueryKey _query_key;
  };

  struct Intersected {
    explicit Intersected(QueryKey key)
      : _query_key{std::move(key)}
    {}

    constexpr bool directory(const DirKey& k) const
    {
      return intersects(_query_key, k);
    }

    template<class DatKey>
    constexpr bool leaf(const DatKey& k) const
    {
      return intersects(_query_key, k);
    }

  private:
    const QueryKey _query_key;
  };
};

} // namespace spaix::homox

#endif // SPAIX_HOMOX_QUERIES_HPP
