// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DIRECTORYNODE_HPP
#define SPAIX_DETAIL_DIRECTORYNODE_HPP

#include <spaix/ConstStaticVectorView.hpp>
#include <spaix/StaticVectorView.hpp>
#include <spaix/detail/DatEntryType.hpp>
#include <spaix/detail/NodePointerEntry.hpp>
#include <spaix/types.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

namespace spaix::detail {

template<class Box, class DatNode, class Structure>
struct DirectoryNode {
public:
  static constexpr auto placement  = Structure::placement;
  static constexpr auto dir_fanout = Structure::dir_fanout;
  static constexpr auto dat_fanout = Structure::dat_fanout;

  using ChildCount = typename Structure::ChildCount;
  using ChildIndex = ChildCount;

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

  explicit DirectoryNode(const NodeType t)
    : _child_type{t}
  {}

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
    assert(entry.node);
    dir_children().emplace_back(std::move(entry));
    return _size;
  }

  [[nodiscard]] ChildIndex num_children() const { return _size; }

  [[nodiscard]] NodeType child_type() const { return _child_type; }

  [[nodiscard]] ConstDirChildrenView dir_children() const
  {
    assert(_child_type == NodeType::directory);
    return {_size, reinterpret_cast<const DirEntry*>(&_children)};
  }

  [[nodiscard]] ConstDatChildrenView dat_children() const
  {
    assert(_child_type == NodeType::data);
    return {_size, reinterpret_cast<const DatEntry*>(&_children)};
  }

  [[nodiscard]] DirChildrenView dir_children()
  {
    assert(_child_type == NodeType::directory);
    return {_size, reinterpret_cast<DirEntry*>(&_children)};
  }

  [[nodiscard]] DatChildrenView dat_children()
  {
    assert(_child_type == NodeType::data);
    return {_size, reinterpret_cast<DatEntry*>(&_children)};
  }

private:
  using AnyEntry = union {
    DirEntry dir;
    DatEntry dat;
  };

  static constexpr auto n_children_bytes =
    std::max(dir_fanout * sizeof(DirEntry), dat_fanout * sizeof(DatEntry));

  const NodeType _child_type; ///< Type of children nodes
  ChildCount     _size{};     ///< Number of children nodes

  struct alignas(AnyEntry) Children {
    std::array<std::byte, n_children_bytes> bytes;
  };

  /// Opaque storage for children array (array of DirEntry or DatEntry)
  Children _children{};
};

} // namespace spaix::detail

#endif // SPAIX_DETAIL_DIRECTORYNODE_HPP
