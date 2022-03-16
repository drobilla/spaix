// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TRAVERSE_TOUCHING_HPP
#define SPAIX_TRAVERSE_TOUCHING_HPP

#include "spaix/intersects.hpp"

#include <utility>

namespace spaix::traverse {

template<class QueryKey>
struct Touching {
  template<class DirKey>
  constexpr bool directory(const DirKey& k) const
  {
    return intersects(key, k);
  }

  template<class DatKey>
  constexpr bool leaf(const DatKey& k) const
  {
    return intersects(key, k);
  }

  const QueryKey key;
};

/// Return a query predicate that matches items that intersect a region
template<class QueryKey>
Touching<QueryKey>
touching(QueryKey&& key)
{
  return Touching<QueryKey>{std::forward<QueryKey>(key)};
}

} // namespace spaix::traverse

#endif // SPAIX_TRAVERSE_TOUCHING_HPP
