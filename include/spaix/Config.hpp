// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_CONFIG_HPP
#define SPAIX_CONFIG_HPP

#include <spaix/DataNode.hpp>
#include <spaix/detail/DatEntryType.hpp>
#include <spaix/detail/NodePointerEntry.hpp>
#include <spaix/types.hpp>

#include <cstddef>
#include <limits>
#include <ratio>

namespace spaix {

enum class DataPlacement : unsigned char;

using DefaultMinFillRatio = std::ratio<3, 10>;

/**
   A tree structure with specific fanouts.

   Instantiations of this are used as a template parameter for spaix::Config.
   This can be used to configure a tree with specific internal (for directory
   nodes) and leaf (for data nodes) fanouts.  To make nodes a specific size,
   use PageStructure instead.

   @tparam max_dir_fanout The maximum number of directory node children.
   @tparam max_dat_fanout The maximum number of data node children.
   @tparam data_placement Where data in the tree is stored.
*/
template<ChildCount    max_dir_fanout,
         ChildCount    max_dat_fanout,
         DataPlacement data_placement>
struct FanoutStructure {
  static constexpr auto placement  = data_placement;
  static constexpr auto dir_fanout = max_dir_fanout;
  static constexpr auto dat_fanout = max_dat_fanout;

  // Bounds used for static assertions (noops in this case)
  static constexpr auto min_dir_node_size = std::numeric_limits<size_t>::min();
  static constexpr auto max_dir_node_size = std::numeric_limits<size_t>::max();
};

/**
   A tree structure with nodes of a specific page size.

   Instantiations of this are used as a template parameter for spaix::Config.
   When used with inline data placement (and sufficiently small data types!),
   this can be used to make every node in the tree a specific page size.
   Otherwise, with separate data placement, directory nodes will be page-sized,
   but data nodes may not, since DataNode is always its "natural" size.

   @tparam B Box type that can maintain many keys.
   @tparam K Key type for data (leaf) nodes.
   @tparam D Data type for data (leaf) nodes.
   @tparam page_size Page size, the maximum size of a directory node, in bytes.
   @tparam data_placement Where data in the tree is stored.
*/
template<class B,
         class K,
         class D,
         size_t        page_size,
         DataPlacement data_placement>
struct PageStructure {
  using DirEntry = detail::NodePointerEntry<B, void>;
  using DatEntry =
    typename detail::DatEntryType<DataNode<K, D>, data_placement>::Type;

  /* We need to be careful about padding here, since keys can be any type and
     we want to cram as many children in a page as possible.  Since this
     configuration code can't depend on the actual directory node definition,
     we define simple structs with the same structure but only a single child,
     and subtract the size of the child entry.  This accounts for any padding
     the compiler inserts between the header fields (the child type and size)
     and the array of children. */

  struct DirDirectory {
    NodeType   child_type;
    ChildCount size;
    DirEntry   children;
  };

  struct DatDirectory {
    NodeType   child_type;
    ChildCount size;
    DatEntry   children;
  };

  static constexpr auto dir_overhead = sizeof(DirDirectory) - sizeof(DirEntry);
  static constexpr auto dat_overhead = sizeof(DatDirectory) - sizeof(DatEntry);
  static constexpr auto dir_space    = page_size - dir_overhead;
  static constexpr auto dat_space    = page_size - dat_overhead;

  // Configuration constants
  static constexpr auto placement  = data_placement;
  static constexpr auto dir_fanout = dir_space / sizeof(DirEntry);
  static constexpr auto dat_fanout = dat_space / sizeof(DatEntry);

  // Bounds used for static assertions
  static constexpr auto min_dir_node_size = page_size - sizeof(DirEntry);
  static constexpr auto max_dir_node_size = page_size;
};

/**
   Configuration for an RTree.

   This allows the user to specify the algorithms used, and the structure of
   the tree (fanouts and data node placement) which determines node sizes.

   @tparam TreeStructure The tree structure configuration, either a
   FanoutStructure or a PageStructure.

   @tparam SplitAlgorithm Node splitting algorithm, spaix::LinearSplit or
   spaix::QuadraticSplit.

   @tparam InsertionAlgorithm Insert position selection algorithm, generally
   spaix::LinearInsertion;

   @tparam SplitMinFillRatio Minimum fill ratio when splitting nodes.  The
   maximum fanout multiplied by this is the minimum number of children that a
   split node will receive.  For example, the default value of 3/10 means that
   split nodes must have at least 3/10th of the maximum fanout.
*/
template<class TreeStructure,
         class SplitAlgorithm,
         class InsertionAlgorithm,
         class SplitMinFillRatio = DefaultMinFillRatio>
struct Config {
  using Structure    = TreeStructure;
  using Split        = SplitAlgorithm;
  using Insertion    = InsertionAlgorithm;
  using MinFillRatio = typename SplitMinFillRatio::type;

  static_assert(MinFillRatio::num < MinFillRatio::den);

  static constexpr const auto placement  = Structure::placement;
  static constexpr auto       dir_fanout = Structure::dir_fanout;
  static constexpr auto       dat_fanout = Structure::dat_fanout;

  static constexpr auto min_dir_fanout =
    dir_fanout * MinFillRatio::num / MinFillRatio::den;

  static constexpr auto min_dat_fanout =
    dat_fanout * MinFillRatio::num / MinFillRatio::den;

  static_assert(dir_fanout > 1);
  static_assert(dat_fanout > 1);
  static_assert(min_dir_fanout >= 1);
  static_assert(min_dat_fanout >= 1);
};

} // namespace spaix

#endif // SPAIX_CONFIG_HPP
