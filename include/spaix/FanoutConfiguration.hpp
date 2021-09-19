// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_FANOUTCONFIGURATION_HPP
#define SPAIX_FANOUTCONFIGURATION_HPP

#include "spaix/DataPlacement.hpp"
#include "spaix/types.hpp"

namespace spaix {

class LinearInsertion;
class QuadraticSplit;

/**
   Configuration for an RTree based on internal and leaf fanouts.

   This allows the user to specify specific fanouts to use for nodes.  To
   choose fanouts based on a page size, use spaix::PageConfiguration instead.

   @tparam DirFanout Maximum number of directory (internal) children of a node.

   @tparam DatFanout Maximum number of data (leaf) children of a node.

   @tparam MinFillDivisor Minimum fill divisor when splitting nodes.  The
   maximum fanout divided by this is the minimum number of children that a
   split node will receive.  For example, the default value of 3 means that
   split nodes must have at least a third of the maximum fanout.

   @tparam SplitAlgorithm Node splitting algorithm, spaix::LinearSplit or
   spaix::QuadraticSplit.

   @tparam InsertionAlgorithm Insert position selection algorithm.
*/
template<ChildCount    DirFanout,
         ChildCount    DatFanout,
         unsigned      MinFillDivisor = 3u,
         DataPlacement Placement      = DataPlacement::separate,
         class SplitAlgorithm         = QuadraticSplit,
         class InsertionAlgorithm     = LinearInsertion>
struct FanoutConfiguration {
  static constexpr auto       dir_fanout       = DirFanout;
  static constexpr auto       dat_fanout       = DatFanout;
  static constexpr const auto min_fill_divisor = MinFillDivisor;
  static constexpr const auto placement        = Placement;

  using Insertion = InsertionAlgorithm;
  using Split     = SplitAlgorithm;
};

} // namespace spaix

#endif // SPAIX_FANOUTCONFIGURATION_HPP
