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

#ifndef SPAIX_PAGECONFIGURATION_HPP
#define SPAIX_PAGECONFIGURATION_HPP

#include "spaix/DataPlacement.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/sizes.hpp"
#include "spaix/types.hpp"

namespace spaix {

class LinearInsertion;
class QuadraticSplit;

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
template <class K,
          class D,
          size_t        page_size      = 4096u,
          unsigned      MinFillDivisor = 3u,
          DataPlacement Placement      = DataPlacement::separate,
          class SplitAlgorithm         = QuadraticSplit,
          class InsertionAlgorithm     = LinearInsertion>
struct PageConfiguration
{
  using Box       = decltype(std::declval<K>() | std::declval<K>());
  using Insertion = InsertionAlgorithm;
  using Split     = SplitAlgorithm;

  static constexpr auto dir_fanout = page_internal_fanout<Box>(page_size);
  static constexpr auto dat_fanout =
      page_leaf_fanout<K, D, Placement>(page_size);

  static constexpr auto min_fill_divisor = MinFillDivisor;
  static constexpr auto placement        = Placement;

private:
  using DatNode = DataNode<K, D>;
  using DirNode =
      DirectoryNode<Box, DatNode, Placement, dir_fanout, dat_fanout>;
  using DirEntry = typename DirNode::DirEntry;
  using DatEntry = typename DirNode::DatEntry;

  static_assert(sizeof(DirNode) <= page_size, "");
  static_assert(sizeof(DirNode) > page_size - sizeof(DirEntry), "");
  static_assert(sizeof(DirNode) > page_size - sizeof(DatEntry), "");
};

} // namespace spaix

#endif // SPAIX_PAGECONFIGURATION_HPP
