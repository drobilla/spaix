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

#ifndef SPAIX_LINEARSPLIT_HPP
#define SPAIX_LINEARSPLIT_HPP

#include "spaix/Rect.hpp"
#include "spaix/detail/meta.hpp"
#include "spaix/types.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

namespace spaix {

// FIXME: node/entry names in here are rotten

/**
   Linear node split.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
class LinearSplit
{
public:
  struct ExtremeIndices
  {
    size_t min_min = 1;
    size_t max_min = 1;
    size_t min_max = 0;
    size_t max_max = 0;
  };

  template <class Children, size_t n_dims>
  static void update_indices(const Children&,
                             const size_t,
                             std::array<ExtremeIndices, n_dims>&,
                             Index<n_dims, n_dims>)
  {
  }

  template <class Children, size_t dim, size_t n_dims>
  static void update_indices(const Children&                     deposit,
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

  template <class T>
  struct MaxSeparation
  {
    size_t dimension  = 0;
    T      separation = {};
  };

  template <class Children, class T, size_t n_dims>
  static void update_max_separation(const Children&,
                                    const std::array<ExtremeIndices, n_dims>&,
                                    MaxSeparation<T>&,
                                    Index<n_dims, n_dims>)
  {
  }

  template <class Children, class T, size_t dim, size_t n_dims>
  static void
  update_max_separation(const Children&                           deposit,
                        const std::array<ExtremeIndices, n_dims>& indices,
                        MaxSeparation<T>&  max_separation,
                        Index<dim, n_dims> index)
  {
    const auto width =
        static_cast<T>(max<dim>(entry_key(deposit[indices[dim].max_max])) -
                       min<dim>(entry_key(deposit[indices[dim].min_min])));

    const auto separation = static_cast<T>(
        abs_diff(max<dim>(entry_key(deposit[indices[dim].min_max])),
                 min<dim>(entry_key(deposit[indices[dim].max_min]))));

    const auto normalized_separation =
        separation / (width > std::numeric_limits<T>::epsilon() ? width : T{1});

    if (normalized_separation > max_separation.separation) {
      max_separation.separation = normalized_separation;
      max_separation.dimension  = dim;
    }

    update_max_separation(deposit, indices, max_separation, ++index);
  }

  /// Return the indices of the children that should be used for split seeds
  template <class Children, class DirKey>
  static std::pair<size_t, size_t>
  pick_seeds(const Children& deposit, const DirKey& bounds)
  {
    std::array<ExtremeIndices, DirKey::size()> indices;
    indices.fill(ExtremeIndices{});

    for (size_t i = 1; i < deposit.size(); ++i) {
      update_indices(deposit, i, indices, bounds.ibegin());
    }

    MaxSeparation<typename CommonScalarType<typename DirKey::Scalars>::type>
        max_separation;
    update_max_separation(deposit, indices, max_separation, bounds.ibegin());

    const std::pair<size_t, size_t> seeds = {
        indices[max_separation.dimension].max_min,
        indices[max_separation.dimension].min_max};

    return seeds.first < seeds.second
               ? seeds
               : std::pair<size_t, size_t>{seeds.second, seeds.first};
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template <class Deposit, class DirNode>
  static void distribute_children(Deposit&         deposit,
                                  DirNode&         lhs,
                                  DirNode&         rhs,
                                  const ChildCount max_fanout)
  {
    auto lhs_volume = volume(lhs.key);
    auto rhs_volume = volume(rhs.key);

    for (size_t i = 0; i < deposit.size(); ++i) {
      if (!&entry_node(deposit[i])) { // FIXME
        continue;
      }

      auto child = std::move(deposit[i]);
      if (lhs.node->num_children() == max_fanout) {
        // Left is full, insert into right
        const auto r_key = rhs.key | entry_key(child);
        distribute_child(rhs, r_key, std::move(child));
        rhs_volume = volume(r_key);
      } else if (rhs.node->num_children() == max_fanout) {
        // Right is full, insert into left
        const auto l_key = lhs.key | entry_key(child);
        distribute_child(lhs, l_key, std::move(child));
        lhs_volume = volume(l_key);
      } else {
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
        } else if (r_volume < l_volume) {
          side = Side::right;
        } else if (lhs.node->num_children() < rhs.node->num_children()) {
          side = Side::left;
        } else {
          side = Side::right;
        }

        if (side == Side::left) {
          distribute_child(lhs, l_key, std::move(child));
          lhs_volume = l_volume;
        } else {
          distribute_child(rhs, r_key, std::move(child));
          rhs_volume = r_volume;
        }
      }
    }
  }

private:
  /// Return |a - b| safely for unsigned types
  template <class T>
  static constexpr T abs_diff(T a, T b)
  {
    return a > b ? a - b : b - a;
  }

  template <class DirNode, class DirKey, class Entry>
  static void
  distribute_child(DirNode& parent, const DirKey& parent_key, Entry&& child)
  {
    parent.key = parent_key;
    parent.node->append_child(std::move(child));
  }
};

} // namespace spaix

#endif // SPAIX_LINEARSPLIT_HPP
