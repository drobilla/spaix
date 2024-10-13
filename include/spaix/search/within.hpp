// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SEARCH_WITHIN_HPP
#define SPAIX_SEARCH_WITHIN_HPP

#include "spaix/contains.hpp"
#include "spaix/intersects.hpp"

#include <utility>

namespace spaix::search {

template<typename QueryKey>
struct Within {
  template<class DirKey>
  [[nodiscard]] constexpr bool directory(const DirKey& k) const noexcept
  {
    return intersects(key, k);
  }

  template<class DatKey>
  [[nodiscard]] constexpr bool leaf(const DatKey& k) const noexcept
  {
    return contains(key, k);
  }

  const QueryKey key;
};

/// Return a query predicate that matches items contained within a region
template<class QueryKey>
[[nodiscard]] Within<QueryKey>
within(QueryKey&& key) noexcept
{
  return Within<QueryKey>{std::forward<QueryKey>(key)};
}

} // namespace spaix::search

#endif // SPAIX_SEARCH_WITHIN_HPP
