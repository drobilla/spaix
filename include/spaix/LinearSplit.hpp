// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_LINEARSPLIT_HPP
#define SPAIX_LINEARSPLIT_HPP

#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/detail/distribute.hpp"
#include "spaix/detail/meta.hpp"
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
  /// Return the indices of the children that should be used for split seeds
  template<class Entries, class DirKey>
  std::pair<size_t, size_t> pick_seeds(const Entries& deposit,
                                       const DirKey&  bounds)
  {
    using std::max;
    using std::min;

    using Scalar = typename CommonElementType<typename DirKey::Scalars>::type;

    std::array<ExtremeIndices, DirKey::size()> indices{};

    for (size_t i = 1; i < deposit.size(); ++i) {
      update_indices(deposit, i, indices, bounds.ibegin());
    }

    MaxSeparation<Scalar> max_separation;
    update_max_separation(deposit, indices, max_separation, bounds.ibegin());

    const auto max_min_index = indices[max_separation.dimension].max_min;
    const auto min_max_index = indices[max_separation.dimension].min_max;
    assert(max_min_index != min_max_index);

    return {min(max_min_index, min_max_index),
            max(max_min_index, min_max_index)};
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template<class Deposit, class DirNode>
  void distribute_children(Deposit&&        deposit,
                           DirNode&         lhs,
                           DirNode&         rhs,
                           const ChildCount max_fanout)
  {
    // Calculate the initial side volumes, which will be updated as we go
    auto lhs_volume = volume(lhs.key);
    auto rhs_volume = volume(rhs.key);

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
      const auto  d_l_volume = l_volume - lhs_volume;
      const auto  d_r_volume = r_volume - rhs_volume;

      // Choose the side with the least volume increase, then least volume
      const Side side = (d_l_volume < d_r_volume)   ? Side::left
                        : (d_r_volume < d_l_volume) ? Side::right
                        : (l_volume < r_volume)     ? Side::left
                        : (r_volume < l_volume)     ? Side::right
                                                    : arbitrary_side();

      // Distribute the child to the chosen side and update its volume
      if (side == Side::left) {
        const auto n = detail::distribute_child(lhs, l_key, std::move(child));
        if (n == max_fanout) {
          detail::distribute_remaining(rhs, std::forward<Deposit>(deposit));
          return;
        }

        lhs_volume = l_volume;
      } else {
        const auto n = detail::distribute_child(rhs, r_key, std::move(child));
        if (n == max_fanout) {
          detail::distribute_remaining(lhs, std::forward<Deposit>(deposit));
          return;
        }

        rhs_volume = r_volume;
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
    const auto& child = deposit[child_index];
    const auto  low   = min<dim>(entry_key(child));
    const auto  high  = max<dim>(entry_key(child));

    if (low <= min<dim>(entry_key(deposit[indices[dim].min_min]))) {
      indices[dim].min_min = child_index;
    }

    if (low >= min<dim>(entry_key(deposit[indices[dim].max_min]))) {
      indices[dim].max_min = child_index;
    }

    if (high <= max<dim>(entry_key(deposit[indices[dim].min_max])) &&
        child_index != indices[dim].max_min) {
      indices[dim].min_max = child_index;
    }

    if (high >= max<dim>(entry_key(deposit[indices[dim].max_max]))) {
      indices[dim].max_max = child_index;
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
    const auto width =
      static_cast<T>(max<dim>(entry_key(deposit[indices[dim].max_max])) -
                     min<dim>(entry_key(deposit[indices[dim].min_min])));

    const auto separation =
      static_cast<T>(max<dim>(entry_key(deposit[indices[dim].min_max])) -
                     min<dim>(entry_key(deposit[indices[dim].max_min])));

    const auto normalized_separation =
      separation / (width > std::numeric_limits<T>::epsilon() ? width : T{1});

    if (normalized_separation > max_separation.separation) {
      max_separation.separation = normalized_separation;
      max_separation.dimension  = dim;
    }

    update_max_separation(deposit, indices, max_separation, ++index);
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
