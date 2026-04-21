// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_CONFIG_HPP
#define SPAIX_CONFIG_HPP

#include <spaix/types.hpp>

#include <algorithm>
#include <cstdint>
#include <ratio>
#include <type_traits>

namespace spaix {

enum class DataPlacement : unsigned char;

using DefaultMinFillRatio = std::ratio<3, 10>;

#if SIZE_MAX <= UINT32_MAX
static constexpr auto default_max_tree_height = 6U;
#else
static constexpr auto default_max_tree_height = 12U;
#endif

/**
   A tree structure with specific fanouts.

   Instantiations of this are used as a template parameter for spaix::Config.
   This can be used to configure a tree with specific internal (for directory
   nodes) and leaf (for data nodes) fanouts.

   @tparam max_dir_fanout The maximum number of directory node children.
   @tparam max_dat_fanout The maximum number of data node children.
   @tparam data_placement Where data in the tree is stored.
   @tparam max_tree_height The maximum height of the tree.
*/
template<unsigned      max_dir_fanout,
         unsigned      max_dat_fanout,
         DataPlacement data_placement,
         unsigned      max_tree_height = default_max_tree_height>
struct StaticStructure {
  static constexpr auto placement  = data_placement;
  static constexpr auto dir_fanout = max_dir_fanout;
  static constexpr auto dat_fanout = max_dat_fanout;
  static constexpr auto max_height = max_tree_height;
  static constexpr auto max_fanout = std::max(dir_fanout, dat_fanout);

  using ChildCount = std::conditional_t<(max_fanout < 255U), uint8_t, uint16_t>;
};

/**
   Configuration for an RTree.

   This allows the user to specify the algorithms used, and the structure of
   the tree (fanouts and data node placement) which determines node sizes.

   @tparam TreeStructure The tree structure configuration.

   @tparam SplitAlgorithm Node splitting algorithm, spaix::LinearSplit or
   spaix::QuadraticSplit.

   @tparam InsertionAlgorithm Insert position selection algorithm, generally
   spaix::LinearInsertion;

   @tparam MinFillRatio Minimum node fill ratio to maintain.  This, when
   multiplied by the maximum fanout, gives the minimum number of children that
   a non-root directory node can have.  For example, the default value of 3/10
   means that nodes have at least 3/10th of the maximum fanout.
*/
template<class TreeStructure,
         class SplitAlgorithm,
         class InsertionAlgorithm,
         class MinFillRatio = DefaultMinFillRatio>
struct Config {
  using Structure = TreeStructure;
  using Split     = SplitAlgorithm;
  using Insertion = InsertionAlgorithm;
  using MinFill   = typename MinFillRatio::type;

  static_assert(MinFillRatio::num < MinFillRatio::den);

  static constexpr auto dir_fanout = Structure::dir_fanout;
  static constexpr auto dat_fanout = Structure::dat_fanout;
  static constexpr auto max_height = Structure::max_height;

  static constexpr auto min_dir_fanout =
    std::max<decltype(dir_fanout)>(2U,
                                   (dir_fanout * MinFill::num) / MinFill::den);

  static constexpr auto min_dat_fanout =
    std::max<decltype(dir_fanout)>(2U,
                                   (dat_fanout * MinFill::num) / MinFill::den);

#if !defined(__clang__) && defined(__GNUC__)
  _Pragma("GCC diagnostic push")
  _Pragma("GCC diagnostic ignored \"-Wduplicated-branches\"")
#endif

  static constexpr auto fanout(const NodeType node_type)
  {
    return node_type == NodeType::directory ? dir_fanout : dat_fanout;
  }

  static constexpr auto min_fanout(const NodeType node_type)
  {
    return node_type == NodeType::directory ? min_dir_fanout : min_dat_fanout;
  }

#if !defined(__clang__) && defined(__GNUC__)
  _Pragma("GCC diagnostic pop")
#endif

  static_assert(dir_fanout > 1);
  static_assert(dat_fanout > 1);
  static_assert(min_dir_fanout >= 1);
  static_assert(min_dat_fanout >= 1);
};

} // namespace spaix

#endif // SPAIX_CONFIG_HPP
