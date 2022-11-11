// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DIRECTORYNODE_HPP
#define SPAIX_DETAIL_DIRECTORYNODE_HPP

#include "spaix/DataPlacement.hpp"
#include "spaix/StaticVector.hpp"
#include "spaix/types.hpp"

#include <cassert>
#include <memory>
#include <new>
#include <type_traits>
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

template<class DirKey, class DatNode, class Structure>
struct DirectoryNode {
public:
  static constexpr auto placement  = Structure::placement;
  static constexpr auto dir_fanout = Structure::dir_fanout;
  static constexpr auto dat_fanout = Structure::dat_fanout;

  using DirNode = DirectoryNode<DirKey, DatNode, Structure>;

  using Key     = decltype(std::declval<DatNode>().key);
  using Data    = decltype(std::declval<DatNode>().data);
  using NodeKey = DirKey;

  using DirNodePtr = std::unique_ptr<DirNode>;
  using DatNodePtr = std::unique_ptr<DatNode>;

  using DirEntry = NodePointerEntry<DirKey, DirNode>;
  using DatEntry = typename DatEntryType<DatNode, placement>::Type;

  using DirChildren = StaticVector<DirEntry, ChildCount, dir_fanout>;
  using DatChildren = StaticVector<DatEntry, ChildCount, dat_fanout>;

  explicit DirectoryNode(const NodeType t)
    : _child_type{t}
  {
    switch (_child_type) {
    case NodeType::directory:
      new (&_dir_children) DirChildren();
      break;
    case NodeType::data:
      new (&_dat_children) DatChildren();
      break;
    }
  }

  ~DirectoryNode()
  {
    switch (_child_type) {
    case NodeType::directory:
      _dir_children.~DirChildren();
      break;
    case NodeType::data:
      _dat_children.~DatChildren();
      break;
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

  ChildIndex num_children() const
  {
    return _child_type == NodeType::directory ? _dir_children.size()
                                              : _dat_children.size();
  }

  NodeType child_type() const { return _child_type; }

  const DirChildren& dir_children() const
  {
    assert(_child_type == NodeType::directory);
    return _dir_children;
  }

  const DatChildren& dat_children() const
  {
    assert(_child_type == NodeType::data);
    return _dat_children;
  }

  DirChildren& dir_children()
  {
    assert(_child_type == NodeType::directory);
    return _dir_children;
  }

  DatChildren& dat_children()
  {
    assert(_child_type == NodeType::data);
    return _dat_children;
  }

private:
  const NodeType _child_type; ///< Type of children nodes
  union {
    DirChildren _dir_children; ///< Directory node children
    DatChildren _dat_children; ///< Data node children
  };
};

template<class Key, class Node>
const auto&
entry_key(const NodePointerEntry<Key, Node>& entry)
{
  return entry.key;
}

template<class Key, class Data>
const auto&
entry_key(const DataNode<Key, Data>& entry)
{
  return entry.key;
}

template<class Key, class Data>
const auto&
entry_data(const DataNode<Key, Data>& entry)
{
  return entry.data;
}

template<class Key, class Data>
const auto&
entry_data(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return entry->data;
}

template<class Key, class Data>
const auto&
entry_key(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return entry->key;
}

template<class Key, class Data>
const auto&
entry_ref(const DataNode<Key, Data>& entry)
{
  return entry;
}

template<class Key, class Data>
const auto&
entry_ref(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return *entry;
}

template<class Key, class Data>
const auto*
entry_ptr(const DataNode<Key, Data>& entry)
{
  return &entry;
}

template<class Key, class Data>
const auto*
entry_ptr(const std::unique_ptr<DataNode<Key, Data>>& entry)
{
  return entry.get();
}

} // namespace spaix

#endif // SPAIX_DETAIL_DIRECTORYNODE_HPP
