// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SEARCH_TOUCHING_HPP
#define SPAIX_SEARCH_TOUCHING_HPP

#include <type_traits>
#include <utility>

namespace spaix::search {

template<class Queries, class QueryKey>
class Touching
{
public:
  explicit Touching(QueryKey key) noexcept
    : _query_key{std::move(key)}
  {}

  template<class DirKey>
  [[nodiscard]] constexpr bool directory(const DirKey& k) const noexcept
  {
    return Queries::intersects(_query_key, k);
  }

  template<class DatKey>
  [[nodiscard]] constexpr bool leaf(const DatKey& k) const noexcept
  {
    return Queries::intersects(_query_key, k);
  }

private:
  std::decay_t<QueryKey> _query_key;
};

} // namespace spaix::search

#endif // SPAIX_SEARCH_TOUCHING_HPP
