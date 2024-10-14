// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DIRECTORYNODE_HPP
#define SPAIX_DETAIL_DIRECTORYNODE_HPP

#include "spaix/DataPlacement.hpp"
#include "spaix/StaticVector.hpp"
#include "spaix/types.hpp"

#include <cassert>
#include <memory>
#include <new>
#include <utility>

namespace spaix {

template<class Key, class Data>
struct DataNode;

template<class ChildKey, class ChildNode>
struct NodePointerEntry {
  using Key  = ChildKey;
  using Node = ChildNode;

  Key                   key{};
  std::unique_ptr<Node> node{};
};

template<class DatNode, DataPlacement placement>
struct DatEntryType;

template<class DatNode>
struct DatEntryType<DatNode, DataPlacement::inlined> {
  using Type = DatNode;

  template<class Key, class Data>
  static Type make(Key key, Data data)
  {
    return DatNode{std::move(key), std::move(data)};
  }
};

template<class DatNode>
struct DatEntryType<DatNode, DataPlacement::separate> {
  using Type = std::unique_ptr<DatNode>;

  template<class Key, class Data>
  static Type make(Key key, Data data)
  {
    return std::unique_ptr<DatNode>(
      new DatNode{std::move(key), std::move(data)});
  }
};

template<class Box, class DatNode, class Structure>
struct DirectoryNode {
public:
  static constexpr auto placement  = Structure::placement;
  static constexpr auto dir_fanout = Structure::dir_fanout;
  static constexpr auto dat_fanout = Structure::dat_fanout;

  using Key    = decltype(std::declval<DatNode>().key);
  using Data   = decltype(std::declval<DatNode>().data);
  using DirKey = Box;

  using DirNode  = DirectoryNode<Box, DatNode, Structure>;
  using DirEntry = NodePointerEntry<DirKey, DirNode>;
  using DatEntry = typename DatEntryType<DatNode, placement>::Type;

  using DirChildren = StaticVector<DirEntry, ChildCount, dir_fanout>;
  using DatChildren = StaticVector<DatEntry, ChildCount, dat_fanout>;

  explicit DirectoryNode(const NodeType t)
    : _child_type{t}
  {
    if (_child_type == NodeType::directory) {
      new (&_dir_children) DirChildren();
    } else {
      new (&_dat_children) DatChildren();
    }
  }

  ~DirectoryNode()
  {
    if (_child_type == NodeType::directory) {
      _dir_children.~DirChildren();
    } else {
      _dat_children.~DatChildren();
    }
  }

  DirectoryNode(const DirectoryNode&)            = delete;
  DirectoryNode& operator=(const DirectoryNode&) = delete;

  DirectoryNode(DirectoryNode&&)            = delete;
  DirectoryNode& operator=(DirectoryNode&&) = delete;

  static DatEntry make_dat_entry(Key key, Data data)
  {
    return DatEntryType<DatNode, placement>::make(key, data);
  }

  ChildCount append_child(DatEntry child)
  {
    assert(_child_type == NodeType::data);
    _dat_children.emplace_back(std::move(child));
    return _dat_children.size();
  }

  ChildCount append_child(DirEntry entry)
  {
    assert(_child_type == NodeType::directory);
    _dir_children.emplace_back(std::move(entry));
    return _dir_children.size();
  }

  [[nodiscard]] ChildIndex num_children() const
  {
    return _child_type == NodeType::directory ? _dir_children.size()
                                              : _dat_children.size();
  }

  [[nodiscard]] NodeType child_type() const { return _child_type; }

  [[nodiscard]] const DirChildren& dir_children() const
  {
    assert(_child_type == NodeType::directory);
    return _dir_children;
  }

  [[nodiscard]] const DatChildren& dat_children() const
  {
    assert(_child_type == NodeType::data);
    return _dat_children;
  }

  [[nodiscard]] DirChildren& dir_children()
  {
    assert(_child_type == NodeType::directory);
    return _dir_children;
  }

  [[nodiscard]] DatChildren& dat_children()
  {
    assert(_child_type == NodeType::data);
    return _dat_children;
  }

private:
  union {
    DirChildren _dir_children; ///< Directory node children
    DatChildren _dat_children; ///< Data node children
  };
  const NodeType _child_type; ///< Type of children nodes
};

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
  return entry.key;
}

template<class Key, class Data>
const auto&
entry_data(const DataNode<Key, Data>& entry) noexcept
{
  return entry.data;
}

template<class Key, class Data>
const auto&
entry_data(const std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return entry->data;
}

template<class Key, class Data>
const auto&
entry_key(const std::unique_ptr<DataNode<Key, Data>>& entry) noexcept
{
  return entry->key;
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
const auto*
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

} // namespace spaix

#endif // SPAIX_DETAIL_DIRECTORYNODE_HPP
