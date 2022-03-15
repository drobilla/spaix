// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DISTRIBUTE_HPP
#define SPAIX_DETAIL_DISTRIBUTE_HPP

#include <cstddef>
#include <utility>

namespace spaix::detail {

template<class DirEntry, class DirKey, class Entry>
inline void
distribute_child(DirEntry& parent, const DirKey& new_parent_key, Entry&& child)
{
  parent.key = new_parent_key;
  parent.node->append_child(std::forward<Entry>(child));
}

template<class DirEntry, class Deposit>
inline void
distribute_remaining(DirEntry& parent, Deposit&& deposit)
{
  for (size_t i = 0; i < deposit.size(); ++i) {
    parent.key |= entry_key(deposit[i]);
    parent.node->append_child(std::move(deposit[i]));
  }
}

} // namespace spaix::detail

#endif // SPAIX_DETAIL_DISTRIBUTE_HPP
