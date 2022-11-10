// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_LINEARSPLIT_HPP
#define SPAIX_LINEARSPLIT_HPP

#include "spaix/SplitSeeds.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/detail/distribute.hpp"
#include "spaix/detail/meta.hpp"
#include "spaix/expansion.hpp"
#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

namespace spaix {

/**
   Linear node split.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
class LinearSplit
{
public:
  template<class DirKey>
  using VolumeOf = decltype(volume(std::declval<DirKey>()));

  template<class DirKey>
  using SeedsFor = SplitSeeds<VolumeOf<DirKey>>;

  /// Return the indices of the children that should be used for split seeds
  template<class DirKey, class Entries>
  SeedsFor<DirKey> pick_seeds(const Entries& deposit)
  {
    using std::max;
    using std::min;

    using Scalar = typename CommonElementType<typename DirKey::Scalars>::type;

    std::array<ExtremeIndices, DirKey::size()> indices{};
    const Index<0U, DirKey::size()>            dim_begin{};

    for (size_t i = 1; i < deposit.size(); ++i) {
      update_indices(deposit, i, indices, dim_begin);
    }

    MaxSeparation<Scalar> max_separation;
    update_max_separation(deposit, indices, max_separation, dim_begin);

    const auto max_min_index = indices[max_separation.dimension].max_min;
    const auto min_max_index = indices[max_separation.dimension].min_max;
    assert(max_min_index != min_max_index);

    const auto lhs_index = min(max_min_index, min_max_index);
    const auto rhs_index = max(max_min_index, min_max_index);

    return {lhs_index,
            rhs_index,
            volume(entry_key(deposit[lhs_index])),
            volume(entry_key(deposit[rhs_index]))};
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template<class Deposit, class DirNode>
  void distribute_children(SeedsFor<typename DirNode::Key>& seeds,
                           Deposit&&                        deposit,
                           DirNode&                         lhs,
                           DirNode&                         rhs,
                           const ChildCount                 max_fanout)
  {
    // Scan the deposit entries once, sending each left or right immediately
    const size_t n_entries = deposit.size();
    for (size_t i = 0; i < n_entries; ++i) {
      auto child = std::move(deposit.back());
      deposit.pop_back();

      // Calculate the change in volume from inserting left or right
      const auto& child_key  = entry_key(child);
      const auto  l_key      = lhs.key | child_key;
      const auto  r_key      = rhs.key | child_key;
      const auto  l_volume   = volume(l_key);
      const auto  r_volume   = volume(r_key);
      const auto  d_l_volume = l_volume - seeds.lhs_volume;
      const auto  d_r_volume = r_volume - seeds.rhs_volume;

      // Choose the side with the least volume increase, then least volume
      const Side side = (d_l_volume < d_r_volume)   ? Side::left
                        : (d_r_volume < d_l_volume) ? Side::right
                        : (l_volume < r_volume)     ? Side::left
                        : (r_volume < l_volume)
                          ? Side::right
                          : tie_side(lhs.key, rhs.key, child_key);

      // Distribute the child to the chosen side and update its volume
      if (side == Side::left) {
        const auto n = detail::distribute_child(lhs, l_key, std::move(child));
        if (n == max_fanout) {
          detail::distribute_remaining(rhs, std::forward<Deposit>(deposit));
          return;
        }

        seeds.lhs_volume = l_volume;
      } else {
        const auto n = detail::distribute_child(rhs, r_key, std::move(child));
        if (n == max_fanout) {
          detail::distribute_remaining(lhs, std::forward<Deposit>(deposit));
          return;
        }

        seeds.rhs_volume = r_volume;
      }
    }
  }

private:
  struct ExtremeIndices {
    size_t min_min = 1;
    size_t max_min = 1;
    size_t min_max = 0;
    size_t max_max = 0;
  };

  template<class T>
  struct MaxSeparation {
    size_t dimension  = 0;
    T      separation = std::numeric_limits<T>::min();
  };

  template<class Entries, size_t n_dims>
  static void update_indices(const Entries&,
                             const size_t,
                             std::array<ExtremeIndices, n_dims>&,
                             Index<n_dims, n_dims>)
  {}

  template<class Entries, size_t dim, size_t n_dims>
  static void update_indices(const Entries&                      deposit,
                             const size_t                        child_index,
                             std::array<ExtremeIndices, n_dims>& indices,
                             Index<dim, n_dims>                  index)
  {
    const auto& child      = deposit[child_index];
    const auto& child_key  = entry_key(child);
    const auto [low, high] = range<dim>(child_key);
    auto& extremes         = indices[dim];

    const auto& min_min = entry_key(deposit[extremes.min_min]);
    const auto& max_min = entry_key(deposit[extremes.max_min]);
    const auto& min_max = entry_key(deposit[extremes.min_max]);
    const auto& max_max = entry_key(deposit[extremes.max_max]);

    if (low <= range<dim>(min_min).lower) {
      extremes.min_min = child_index;
    }

    if (low >= range<dim>(max_min).lower) {
      extremes.max_min = child_index;
    }

    if (high <= range<dim>(min_max).upper && child_index != extremes.max_min) {
      extremes.min_max = child_index;
    }

    if (high >= range<dim>(max_max).upper) {
      extremes.max_max = child_index;
    }

    update_indices(deposit, child_index, indices, ++index);
  }

  template<class Entries, class T, size_t n_dims>
  static void update_max_separation(const Entries&,
                                    const std::array<ExtremeIndices, n_dims>&,
                                    MaxSeparation<T>&,
                                    Index<n_dims, n_dims>)
  {}

  template<class Entries, class T, size_t dim, size_t n_dims>
  static void update_max_separation(
    const Entries&                            deposit,
    const std::array<ExtremeIndices, n_dims>& indices,
    MaxSeparation<T>&                         max_separation,
    Index<dim, n_dims>                        index)
  {
    const auto& min_min = entry_key(deposit[indices[dim].min_min]);
    const auto& max_min = entry_key(deposit[indices[dim].max_min]);
    const auto& min_max = entry_key(deposit[indices[dim].min_max]);
    const auto& max_max = entry_key(deposit[indices[dim].max_max]);

    const auto width =
      static_cast<T>(range<dim>(max_max).upper - range<dim>(min_min).lower);

    const auto separation =
      static_cast<T>(range<dim>(min_max).upper - range<dim>(max_min).lower);

    const auto normalized_separation =
      separation / (width > std::numeric_limits<T>::epsilon() ? width : T{1});

    if (normalized_separation > max_separation.separation) {
      max_separation.separation = normalized_separation;
      max_separation.dimension  = dim;
    }

    update_max_separation(deposit, indices, max_separation, ++index);
  }

  /// Choose a side to break a tie when the volume comparisons fail
  template<class DirKey, class ChildKey>
  Side tie_side(const DirKey&   lhs_key,
                const DirKey&   rhs_key,
                const ChildKey& child_key)
  {
    const auto l_expansion = spaix::expansion(lhs_key, child_key);
    const auto r_expansion = spaix::expansion(rhs_key, child_key);

    return (l_expansion < r_expansion)   ? Side::left
           : (r_expansion < l_expansion) ? Side::right
                                         : arbitrary_side();
  }

  /// Choose an arbitrary fallback side (which flip-flops to avoid bias)
  Side arbitrary_side()
  {
    return (_bias = (_bias == Side::left ? Side::right : Side::left));
  }

  Side _bias = Side::left;
};

} // namespace spaix

#endif // SPAIX_LINEARSPLIT_HPP
