// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_PAGECONFIGURATION_HPP
#define SPAIX_PAGECONFIGURATION_HPP

#include "spaix/DataNode.hpp"
#include "spaix/DataPlacement.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/sizes.hpp"
#include "spaix/union.hpp"

#include <cstddef>

namespace spaix {

/**
   Configuration for an RTree based on a page size.

   This calculates optimal fanouts so that nodes fill the given page size as
   much as possible.

   @tparam page_size Size of nodes in bytes.

   @tparam MinFillDivisor Minimum fill divisor when splitting nodes.  The
   maximum fanout divided by this is the minimum number of children that a
   split node will receive.  For example, the default value of 3 means that
   split nodes must have at least a third of the maximum fanout.

   @tparam SplitAlgorithm Node splitting algorithm, spaix::LinearSplit or
   spaix::QuadraticSplit.

   @tparam InsertionAlgorithm Insert position selection algorithm.
*/
template<class SplitAlgorithm,
         class InsertionAlgorithm,
         class K,
         class D,
         size_t        page_size      = 4096u,
         unsigned      MinFillDivisor = 3u,
         DataPlacement Placement      = DataPlacement::separate>
struct PageConfiguration {
  using Box       = UnionOf<K>;
  using Insertion = InsertionAlgorithm;
  using Split     = SplitAlgorithm;

  static constexpr auto dir_fanout = page_dir_fanout<Box>(page_size);
  static constexpr auto dat_fanout =
    page_dat_fanout<K, D, Placement>(page_size);

  static constexpr auto min_fill_divisor = MinFillDivisor;
  static constexpr auto placement        = Placement;

private:
  using DatNode = DataNode<K, D>;
  using DirNode =
    DirectoryNode<Box, DatNode, Placement, dir_fanout, dat_fanout>;
  using DirEntry = typename DirNode::DirEntry;
  using DatEntry = typename DirNode::DatEntry;

  static_assert(sizeof(DirNode) <= page_size);
  static_assert(sizeof(DirNode) > page_size - sizeof(DirEntry));
  static_assert(sizeof(DirNode) > page_size - sizeof(DatEntry));
};

} // namespace spaix

#endif // SPAIX_PAGECONFIGURATION_HPP
