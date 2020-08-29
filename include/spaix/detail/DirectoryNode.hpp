/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPAIX_DIRECTORYNODE_HPP
#define SPAIX_DIRECTORYNODE_HPP

#include "spaix/DataNode.hpp"
#include "spaix/DataPlacement.hpp"
#include "spaix/StaticVector.hpp"
#include "spaix/types.hpp"

#include <memory>
#include <type_traits>
#include <utility>

namespace spaix {

template <class ChildKey, class ChildNode>
struct NodeEntry
{
  using Key  = ChildKey;
  using Node = ChildNode;

  Key                   key{};
  std::unique_ptr<Node> node{};
};

template <class DatNode, DataPlacement placement>
struct DatEntryType;

template <class DatNode>
struct DatEntryType<DatNode, DataPlacement::inlined>
{
  using Type = DatNode;

  template <class Key, class Data>
  static Type make(Key key, Data data)
  {
    return DatNode{std::move(key), std::move(data)};
  }
};

template <class DatNode>
struct DatEntryType<DatNode, DataPlacement::separate>
{
  using Type = std::unique_ptr<DatNode>;

  template <class Key, class Data>
  static Type make(Key key, Data data)
  {
    return std::unique_ptr<DatNode>(
        new DatNode{std::move(key), std::move(data)});
  }
};

template <class DirKey,
          class DatNode,
          DataPlacement Placement,
          ChildCount    DirFanout,
          ChildCount    DatFanout>
struct DirectoryNode
{
public:
  using DirNode = DirectoryNode<DirKey, DatNode, Placement, DirFanout, DatFanout>;
  using Key     = decltype(std::declval<DatNode>().key);
  using Data    = decltype(std::declval<DatNode>().data);
  using NodeKey = DirKey;

  using DirNodePtr = std::unique_ptr<DirNode>;
  using DatNodePtr = std::unique_ptr<DatNode>;

  using DirEntry = NodeEntry<DirKey, DirNode>;
  // using DatEntry = DatNodePtr;
  // using DatEntry = DatNode;
  using DatEntry = typename DatEntryType<DatNode, Placement>::Type;

  using DirChildren = StaticVector<DirEntry, ChildCount, DirFanout>;
  using DatChildren = StaticVector<DatEntry, ChildCount, DatFanout>;

  explicit DirectoryNode(const NodeType t) : child_type{t}
  {
    if (child_type == NodeType::data) {
      new (&dat_children) DatChildren();
    } else {
      new (&dir_children) DirChildren();
    }
  }

  ~DirectoryNode()
  {
    if (child_type == NodeType::data) {
      dat_children.~DatChildren();
    } else {
      dir_children.~DirChildren();
    }
  }

  DirectoryNode(const DirectoryNode&) = delete;
  DirectoryNode& operator=(const DirectoryNode&) = delete;

  DirectoryNode(DirectoryNode&&) = delete;
  DirectoryNode& operator=(DirectoryNode&&) = delete;

  static DatEntry make_dat_entry(Key key, Data data)
  {
    return DatEntryType<DatNode, Placement>::make(key, data);
  }

  void append_child(DatEntry child)
  {
    assert(child_type == NodeType::data);
    dat_children.emplace_back(std::move(child));
  }

  void append_child(DirEntry entry)
  {
    assert(child_type == NodeType::directory);
    dir_children.emplace_back(std::move(entry));
  }

  size_t num_children() const
  {
    return child_type == NodeType::directory ? dir_children.size()
                                             : dat_children.size();
  }

  const NodeType child_type; ///< Type of children nodes
  union
  {
    DirChildren dir_children; ///< Directory node children
    DatChildren dat_children; ///< Data node children
  };
};

template <class Key, class Node>
const auto&
entry_key(const NodeEntry<Key, Node>& entry)
{
  return entry.key;
}

template <class Key, class Data>
const auto&
entry_key(const DataNode<Key, Data>& entry)
{
  return entry.key;
}

template <class Key, class Data>
const auto&
entry_data(const DataNode<Key, Data>& entry)
{
  return entry.data;
}

template <class Key, class Data>
const auto&
entry_data(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return entry->data;
}

template <class Key, class Data>
const auto&
entry_key(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return entry->key;
}

template <class Key, class Data>
const auto&
entry_ref(const DataNode<Key, Data>& entry)
{
  return entry;
}

template <class Key, class Data>
const auto&
entry_ref(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return *entry;
}

template <class Key, class Data>
const auto*
entry_ptr(const DataNode<Key, Data>& entry)
{
  return &entry;
}

template <class Key, class Data>
const auto*
entry_ptr(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return entry.get();
}

} // namespace spaix

#endif // SPAIX_DIRECTORYNODE_HPP
