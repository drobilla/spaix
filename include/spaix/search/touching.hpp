// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SEARCH_TOUCHING_HPP
#define SPAIX_SEARCH_TOUCHING_HPP

#include <spaix/intersects.hpp>

#include <type_traits>
#include <utility>

namespace spaix::search {

template<class QueryKey>
struct Touching {
  template<class DirKey>
  [[nodiscard]] constexpr bool directory(const DirKey& k) const noexcept
  {
    return intersects(key, k);
  }

  template<class DatKey>
  [[nodiscard]] constexpr bool leaf(const DatKey& k) const noexcept
  {
    return intersects(key, k);
  }

  std::decay_t<QueryKey> key;
};

/// Return a query predicate that matches items that intersect a region
template<class QueryKey>
[[nodiscard]] Touching<QueryKey>
touching(QueryKey&& key) noexcept
{
  return Touching<QueryKey>{std::forward<QueryKey>(key)};
}

} // namespace spaix::search

#endif // SPAIX_SEARCH_TOUCHING_HPP
