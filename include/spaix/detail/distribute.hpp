// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DISTRIBUTE_HPP
#define SPAIX_DETAIL_DISTRIBUTE_HPP

#include <spaix/detail/DirectoryNode.hpp>
#include <spaix/types.hpp>

#include <utility>

namespace spaix::detail {

template<class DirEntry, class DirKey, class Entry>
inline ChildCount
distribute_child(DirEntry& parent, const DirKey& new_parent_key, Entry&& child)
{
  parent.key = new_parent_key;
  return parent.node->append_child(std::forward<Entry>(child));
}

template<class DirEntry, class Deposit>
inline void
distribute_remaining(DirEntry& parent, Deposit deposit)
{
  for (ChildIndex i = 0; i < deposit.size(); ++i) {
    auto& entry = deposit[i];
    parent.key |= entry_key(entry);
    parent.node->append_child(std::move(entry));
  }
}

} // namespace spaix::detail

#endif // SPAIX_DETAIL_DISTRIBUTE_HPP
