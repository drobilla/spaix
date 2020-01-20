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

#ifndef SPAIX_QUADRATICSPLIT_HPP
#define SPAIX_QUADRATICSPLIT_HPP

#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include <iostream>

namespace spaix {

/**
   Quadratic node split.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
class QuadraticSplit
{
public:
  /// Return the indices of the children that should be used for split seeds
  template <class NodePtr, size_t fanout, class DirKey>
  static std::pair<size_t, size_t>
  pick_seeds(const std::array<NodePtr, fanout>& deposit, const DirKey&)
  {
    using Volume    = decltype(volume(std::declval<DirKey>()));
    using SeedWaste = Volume;

    std::array<Volume, fanout> volumes;
    for (size_t i = 0; i < deposit.size(); ++i) {
      volumes[i] = volume(deposit[i]->key);
    }

    SeedWaste                 max_waste{std::numeric_limits<Volume>::lowest()};
    std::pair<size_t, size_t> seeds{deposit.size(), deposit.size()};
    for (size_t i = 0; i < deposit.size() - 1; ++i) {
      for (size_t j = i + 1; j < deposit.size(); ++j) {
        const auto& k = deposit[i]->key;
        const auto& l = deposit[j]->key;

        const SeedWaste waste = volume(k | l) - volumes[i] - volumes[j];

        if (waste >= max_waste) {
          max_waste = waste;
          seeds     = std::make_pair(i, j);
        }
      }
    }

    assert(seeds.first < deposit.size());
    assert(seeds.second < deposit.size());
    assert(seeds.first != seeds.second);
    assert(deposit[seeds.first]);
    assert(deposit[seeds.second]);
    return seeds;
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template <class Deposit, class DirNode>
  static void distribute_children(Deposit&         deposit,
                                  DirNode&         lhs,
                                  DirNode&         rhs,
                                  const ChildCount min_fanout)
  {
    const auto max_fanout = deposit.size() - min_fanout;

    for (size_t i = 0; i < deposit.size() - 2; ++i) {
      const auto best = pick_next(deposit, lhs, rhs);
      assert(best.child_index < deposit.size());

      auto& parent = best.side == Side::left ? lhs : rhs;

      parent.key = best.new_parent_key;
      parent.append_child(std::move(deposit[best.child_index]));

      if (parent.num_children() == max_fanout) {
        auto& other_parent = best.side == Side::left ? rhs : lhs;
        for (size_t j = 0; j < deposit.size(); ++j) {
          if (deposit[j]) {
            other_parent.key = other_parent.key | deposit[j]->key;
            other_parent.append_child(std::move(deposit[j]));
          }
        }

        return;
      }
    }
  }

private:
  /// Assignment of a child to a parent during a split
  template <class DirKey>
  struct ChildAssignment
  {
    size_t child_index;
    DirKey new_parent_key;
    Side   side;
  };

  /// Return |a - b| safely for unsigned types
  template <class T>
  static constexpr T abs_diff(T a, T b)
  {
    return a > b ? a - b : b - a;
  }

  /// Choose the next child to distribute during a split
  template <class Deposit, class DirNode>
  static ChildAssignment<typename DirNode::NodeKey>
  pick_next(const Deposit& deposit, const DirNode& lhs, const DirNode& rhs)
  {
    using DirKey     = typename DirNode::NodeKey;
    using Volume     = decltype(volume(std::declval<DirKey>()));
    using Result     = ChildAssignment<DirKey>;
    using Preference = std::tuple<Volume, Volume, ChildCount>;

    Preference best_preference{0, 0, 0};
    Result     best{deposit.size(), DirKey{}, Side::left};

    const auto lhs_volume = volume(lhs.key);
    const auto rhs_volume = volume(rhs.key);

    for (size_t i = 0; i < deposit.size(); ++i) {
      const auto& child = deposit[i];
      if (child) {
        const auto l_key       = lhs.key | child->key;
        const auto r_key       = rhs.key | child->key;
        const auto l_volume    = volume(l_key);
        const auto r_volume    = volume(r_key);
        const auto l_expansion = l_volume - lhs_volume;
        const auto r_expansion = r_volume - rhs_volume;

        const Preference preference = {
            abs_diff(l_expansion, r_expansion),
            abs_diff(l_volume, r_volume),
            abs_diff(lhs.num_children(), rhs.num_children())};

        if (preference >= best_preference) {
          best_preference = preference;
          if (l_expansion < r_expansion) {
            best = Result{i, l_key, Side::left};
          } else if (r_expansion < l_expansion) {
            best = Result{i, r_key, Side::right};
          } else if (l_volume < r_volume) {
            best = Result{i, l_key, Side::left};
          } else if (r_volume < l_volume) {
            best = Result{i, r_key, Side::right};
          } else if (lhs.num_children() < rhs.num_children()) {
            best = Result{i, l_key, Side::left};
          } else {
            best = Result{i, r_key, Side::right};
          }
        }
      }
    }

    return best;
  }
};

} // namespace spaix

#endif // SPAIX_QUADRATICSPLIT_HPP
