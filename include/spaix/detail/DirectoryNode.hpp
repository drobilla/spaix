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

template <class DirKey, class DatNode, ChildCount Fanout>
struct DirectoryNode
{
public:
  using DirNode = DirectoryNode<DirKey, DatNode, Fanout>;
  using Key     = decltype(std::declval<DatNode>().key);
  using NodeKey = DirKey;

  using DatNodePtr = std::unique_ptr<DatNode>;
  using DirNodePtr = std::unique_ptr<DirNode>;

  using DirEntry = NodeEntry<DirKey, DirNode>;
  using DatEntry = DatNode;

  using DatChildren = StaticVector<DatEntry, ChildCount, Fanout>;
  using DirChildren = StaticVector<DirEntry, ChildCount, Fanout>;

  DirectoryNode(const NodeType t) : child_type{t}
  {
    if (child_type == NodeType::DAT) {
      new (&dat_children) DatChildren();
    } else {
      new (&dir_children) DirChildren();
    }
  }

  ~DirectoryNode()
  {
    if (child_type == NodeType::DAT) {
      dat_children.~DatChildren();
    } else {
      dir_children.~DirChildren();
    }
  }

  DirectoryNode(const DirectoryNode&) = delete;
  DirectoryNode& operator=(const DirectoryNode&) = delete;

  DirectoryNode(DirectoryNode&&) = delete;
  DirectoryNode& operator=(DirectoryNode&&) = delete;

  void append_child(DatNode&& child)
  {
    assert(child_type == NodeType::DAT);
    dat_children.emplace_back(std::move(child));
  }

  void append_child(DirEntry entry)
  {
    assert(child_type == NodeType::DIR);
    dir_children.emplace_back(std::move(entry));
  }

  size_t num_children() const
  {
    return child_type == NodeType::DIR ? dir_children.size()
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

template <class Key, class Node>
const auto&
entry_node(const NodeEntry<Key, Node>& entry)
{
  return entry.node;
}

template <class Key, class Data>
const auto&
entry_node(const DataNode<Key, Data>& entry)
{
  return entry;
}

} // namespace spaix

#endif // SPAIX_DIRECTORYNODE_HPP
