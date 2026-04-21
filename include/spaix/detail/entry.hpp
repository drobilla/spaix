// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_ENTRY_HPP
#define SPAIX_DETAIL_ENTRY_HPP

#include <spaix/DataNode.hpp>
#include <spaix/types.hpp>

#include <memory>

namespace spaix::detail {

template<class ChildKey, class ChildNode>
struct NodePointerEntry;

template<class Key, class Node>
const auto&
entry_key(const NodePointerEntry<Key, Node>& entry) noexcept
{
  return entry.key;
}

template<class Key, class Data>
const auto&
entry_key(const DataNode<Key, Data>& entry) noexcept
{
  return entry.first;
}

template<class Key, class Data>
const auto&
entry_key(const std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return entry->first;
}

template<class Key, class Data>
auto&
entry_key(DataNode<Key, Data>& entry) noexcept
{
  return entry.first;
}

template<class Key, class Data>
auto&
entry_key(std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return entry->first;
}

template<class Key, class Data>
const auto&
entry_data(const DataNode<Key, Data>& entry) noexcept
{
  return entry.second;
}

template<class Key, class Data>
const auto&
entry_data(const std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return entry->second;
}

template<class Key, class Data>
const auto&
entry_ref(const DataNode<Key, Data>& entry) noexcept
{
  return entry;
}

template<class Key, class Data>
const auto&
entry_ref(const std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return *entry;
}

template<class Key, class Data>
const auto*
entry_ptr(const DataNode<Key, Data>& entry) noexcept
{
  return &entry;
}

template<class Key, class Data>
auto*
entry_ptr(DataNode<Key, Data>& entry) noexcept
{
  return &entry;
}

template<class Key, class Data>
auto*
entry_ptr(const std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return entry.get();
}

template<class Key, class Node>
ChildCount
entry_num_children(const NodePointerEntry<Key, Node>& entry) noexcept
{
  return entry.node->num_children();
}

template<class Key, class Data>
ChildCount
entry_num_children(const DataNode<Key, Data>&) noexcept
{
  return 0U;
}

} // namespace spaix::detail

#endif // SPAIX_DETAIL_ENTRY_HPP
