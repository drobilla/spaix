// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DISTRIBUTE_HPP
#define SPAIX_DISTRIBUTE_HPP

#include <cstddef>
#include <utility>

namespace spaix {

template<class DirEntry, class DirKey, class Entry>
static void
distribute_child(DirEntry& parent, const DirKey& new_parent_key, Entry&& child)
{
  parent.key = new_parent_key;
  parent.node->append_child(std::forward<Entry>(child));
}

template<class DirEntry, class Deposit>
static void
distribute_remaining(DirEntry& parent, Deposit&& deposit)
{
  for (size_t i = 0; i < deposit.size(); ++i) {
    parent.key |= entry_key(deposit[i]);
    parent.node->append_child(std::move(deposit[i]));
  }
}

} // namespace spaix

#endif // SPAIX_DISTRIBUTE_HPP
