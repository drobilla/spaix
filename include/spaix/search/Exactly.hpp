// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SEARCH_EXACTLY_HPP
#define SPAIX_SEARCH_EXACTLY_HPP

#include <type_traits>
#include <utility>

#include <spaix/concepts.hpp>

namespace spaix::search {

template<typename Comps, typename QueryKey>
#if SPAIX_USE_CONCEPTS
  requires ChecksIntersects<Comps, QueryKey>
#endif
class Exactly
{
public:
  explicit Exactly(QueryKey key) noexcept
    : _query_key{std::move(key)}
  {}

  template<class DirKey>
  [[nodiscard]] constexpr bool directory(const DirKey& k) const noexcept
  {
    return Comps::intersects(k, _query_key);
  }

  template<class DatKey>
  [[nodiscard]] constexpr bool leaf(const DatKey& k) const noexcept
  {
    return _query_key == k;
  }

private:
  std::decay_t<QueryKey> _query_key;
};

} // namespace spaix::search

#endif // SPAIX_SEARCH_EXACTLY_HPP
