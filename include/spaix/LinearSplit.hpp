// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_LINEARSPLIT_HPP
#define SPAIX_LINEARSPLIT_HPP

#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"
#include "spaix/distribute.hpp"
#include "spaix/types.hpp"

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
  static std::pair<size_t, size_t> pick_seeds(const Entries& deposit,
                                              const DirKey&  bounds)
  {
    using Scalar = typename CommonScalarType<typename DirKey::Scalars>::type;

    std::array<ExtremeIndices, DirKey::size()> indices{};

    for (size_t i = 1; i < deposit.size(); ++i) {
      update_indices(deposit, i, indices, bounds.ibegin());
    }

    MaxSeparation<Scalar> max_separation;
    update_max_separation(deposit, indices, max_separation, bounds.ibegin());

    const auto max_min_index = indices[max_separation.dimension].max_min;
    const auto min_max_index = indices[max_separation.dimension].min_max;
    assert(max_min_index != min_max_index);

    return {std::min(max_min_index, min_max_index),
            std::max(max_min_index, min_max_index)};
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template<class Deposit, class DirNode>
  static void distribute_children(Deposit&&        deposit,
                                  DirNode&         lhs,
                                  DirNode&         rhs,
                                  const ChildCount max_fanout)
  {
    auto lhs_volume = volume(lhs.key);
    auto rhs_volume = volume(rhs.key);

    const size_t n_entries = deposit.size();
    for (size_t i = 0; i < n_entries; ++i) {
      auto child = std::move(deposit.back());
      deposit.pop_back();

      // Insert into parent which causes the least expansion
      const auto l_key       = lhs.key | entry_key(child);
      const auto r_key       = rhs.key | entry_key(child);
      const auto l_volume    = volume(l_key);
      const auto r_volume    = volume(r_key);
      const auto l_expansion = l_volume - lhs_volume;
      const auto r_expansion = r_volume - rhs_volume;

      Side side = Side::left;
      if (l_expansion < r_expansion) {
        side = Side::left;
      } else if (r_expansion < l_expansion) {
        side = Side::right;
      } else if (l_volume < r_volume) {
        side = Side::left;
      } else {
        side = Side::right;
      }

      if (side == Side::left) {
        distribute_child(lhs, l_key, std::move(child));
        if (lhs.node->num_children() == max_fanout) {
          distribute_remaining(rhs, std::forward<Deposit>(deposit));
          return;
        }

        lhs_volume = l_volume;
      } else {
        distribute_child(rhs, r_key, std::move(child));
        if (rhs.node->num_children() == max_fanout) {
          distribute_remaining(lhs, std::forward<Deposit>(deposit));
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
};

} // namespace spaix

#endif // SPAIX_LINEARSPLIT_HPP
