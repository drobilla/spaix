// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DIRECTORYNODE_HPP
#define SPAIX_DETAIL_DIRECTORYNODE_HPP

#include <spaix/ConstStaticVectorView.hpp>
#include <spaix/StaticVectorView.hpp>
#include <spaix/detail/DatEntryType.hpp>
#include <spaix/detail/NodePointerEntry.hpp>
#include <spaix/types.hpp>

#include <cassert>
#include <new>
#include <utility>

namespace spaix::detail {

template<class Box, class DatNode, class Structure>
struct DirectoryNode {
public:
  static constexpr auto placement  = Structure::placement;
  static constexpr auto dir_fanout = Structure::dir_fanout;
  static constexpr auto dat_fanout = Structure::dat_fanout;

  using Key    = typename DatNode::first_type;
  using Data   = typename DatNode::second_type;
  using DirKey = Box;

  using DirNode  = DirectoryNode<Box, DatNode, Structure>;
  using DirEntry = NodePointerEntry<DirKey, DirNode>;
  using DatEntry = typename DatEntryType<DatNode, placement>::Type;

  using DirChildrenView = StaticVectorView<DirEntry, ChildCount, dir_fanout>;
  using DatChildrenView = StaticVectorView<DatEntry, ChildCount, dat_fanout>;

  using ConstDirChildrenView =
    ConstStaticVectorView<const DirEntry, ChildCount, dir_fanout>;

  using ConstDatChildrenView =
    ConstStaticVectorView<const DatEntry, ChildCount, dat_fanout>;

  using DirChildrenStorage = typename DirChildrenView::Storage;
  using DatChildrenStorage = typename DatChildrenView::Storage;

  explicit DirectoryNode(const NodeType t)
    : _child_type{t}
  {
    if (_child_type == NodeType::directory) {
      new (&_dir_children) DirChildrenStorage();
    } else {
      new (&_dat_children) DatChildrenStorage();
    }
  }

  ~DirectoryNode()
  {
    if (_child_type == NodeType::directory) {
      dir_children().clear();
    } else {
      dat_children().clear();
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
    dat_children().emplace_back(std::move(child));
    return _size;
  }

  ChildCount append_child(DirEntry entry)
  {
    assert(_child_type == NodeType::directory);
    dir_children().emplace_back(std::move(entry));
    return _size;
  }

  [[nodiscard]] ChildIndex num_children() const { return _size; }

  [[nodiscard]] NodeType child_type() const { return _child_type; }

  [[nodiscard]] ConstDirChildrenView dir_children() const
  {
    assert(_child_type == NodeType::directory);
    return {_size, _dir_children};
  }

  [[nodiscard]] ConstDatChildrenView dat_children() const
  {
    assert(_child_type == NodeType::data);
    return {_size, _dat_children};
  }

  [[nodiscard]] DirChildrenView dir_children()
  {
    assert(_child_type == NodeType::directory);
    return {_size, _dir_children};
  }

  [[nodiscard]] DatChildrenView dat_children()
  {
    assert(_child_type == NodeType::data);
    return {_size, _dat_children};
  }

private:
  const NodeType _child_type; ///< Type of children nodes
  ChildCount     _size{};     ///< Number of children nodes
  union {
    DirChildrenStorage _dir_children; ///< Directory node children
    DatChildrenStorage _dat_children; ///< Data node children
  };
};

} // namespace spaix::detail

#endif // SPAIX_DETAIL_DIRECTORYNODE_HPP
