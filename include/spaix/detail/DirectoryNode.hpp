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

#include "spaix/types.hpp"

#include <array>
#include <memory>
#include <type_traits>
#include <utility>

namespace spaix {

template <class DirKey, class DatNode, ChildCount Fanout>
struct DirectoryNode
{
public:
  using DirNode = DirectoryNode<DirKey, DatNode, Fanout>;
  using Key     = decltype(std::declval<DatNode>().key);
  using NodeKey = DirKey;

  using DatNodePtr = std::unique_ptr<DatNode>;
  using DirNodePtr = std::unique_ptr<DirNode>;

  using DatChildren = std::array<DatNodePtr, Fanout>;
  using DirChildren = std::array<DirNodePtr, Fanout>;

  DirectoryNode(const DirKey& k, const NodeType t)
    : key{std::move(k)}, child_type{t}, n_children{0}
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

  void append_child(std::unique_ptr<DatNode> child)
  {
    assert(child_type == NodeType::DAT);
    assert(n_children < Fanout);
    dat_children[n_children++] = std::move(child);
    assert(dat_children[n_children - 1]);
  }

  void append_child(std::unique_ptr<DirNode> child)
  {
    assert(child_type == NodeType::DIR);
    assert(n_children < Fanout);
    dir_children[n_children++] = std::move(child);
    assert(dir_children[n_children - 1]);
  }

  size_t num_children() const { return n_children; }

  DirKey         key;        ///< Key that contains all children
  const NodeType child_type; ///< Type of children nodes
  ChildCount     n_children; ///< Number of direct children
  union
  {
    DirChildren dir_children; ///< Directory node children
    DatChildren dat_children; ///< Data node children
  };
};

} // namespace spaix

#endif // SPAIX_DIRECTORYNODE_HPP
