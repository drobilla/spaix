// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DISTRIBUTE_HPP
#define SPAIX_DETAIL_DISTRIBUTE_HPP

#include <spaix/detail/entry.hpp>

#include <utility>

namespace spaix::detail {

template<class DirEntry, class DirKey, class Entry>
inline auto
distribute_child(DirEntry& parent, const DirKey& new_parent_key, Entry&& child)
{
  parent.key = new_parent_key;
  return parent.node->append_child(std::forward<Entry>(child));
}

template<class Ops, class DirEntry, class Deposit>
inline typename Deposit::size_type
distribute_remaining(DirEntry&                         parent,
                     Deposit                           deposit,
                     const typename Deposit::size_type track_index)
{
  const auto new_index = static_cast<typename Deposit::size_type>(
    parent.node->num_children() + track_index);

  for (auto i = typename Deposit::size_type{}; i < deposit.size(); ++i) {
    auto& entry = deposit[i];
    Ops::expand(parent.key, entry_key(entry));
    parent.node->append_child(std::move(entry));
  }

  return new_index;
}

} // namespace spaix::detail

#endif // SPAIX_DETAIL_DISTRIBUTE_HPP
