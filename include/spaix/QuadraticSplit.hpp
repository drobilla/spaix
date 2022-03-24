// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_QUADRATICSPLIT_HPP
#define SPAIX_QUADRATICSPLIT_HPP

#include "spaix/SplitSeeds.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/detail/distribute.hpp"
#include "spaix/expansion.hpp"
#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility> // IWYU pragma: keep

namespace spaix {

template<class T, class Size, Size Capacity>
class StaticVector;

/**
   Quadratic node split.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
class QuadraticSplit
{
public:
  /// Return the indices of the children that should be used for split seeds
  template<class Entry, ChildCount count, class DirKey>
  SplitSeeds<DirKey> pick_seeds(
    const StaticVector<Entry, ChildCount, count>& deposit,
    const DirKey&)
  {
    using Volume    = decltype(volume(std::declval<DirKey>()));
    using SeedWaste = Volume;

    assert(deposit.size() == deposit.capacity());
    Volume volumes[count];
    for (size_t i = 0; i < deposit.size(); ++i) {
      volumes[i] = volume(entry_key(deposit[i]));
    }

    SeedWaste          max_waste{std::numeric_limits<Volume>::lowest()};
    SplitSeeds<DirKey> seeds{deposit.size(), deposit.size(), {}, {}};
    for (size_t i = 0; i < deposit.size() - 1; ++i) {
      for (size_t j = i + 1; j < deposit.size(); ++j) {
        const auto& k = entry_key(deposit[i]);
        const auto& l = entry_key(deposit[j]);

        const SeedWaste waste = volume(k | l) - volumes[i] - volumes[j];

        if (waste >= max_waste) {
          max_waste       = waste;
          seeds.lhs_index = i;
          seeds.rhs_index = j;
        }
      }
    }

    seeds.lhs_volume = volumes[seeds.lhs_index];
    seeds.rhs_volume = volumes[seeds.rhs_index];

    assert(seeds.lhs_index < deposit.size());
    assert(seeds.rhs_index < deposit.size());
    assert(seeds.lhs_index < seeds.rhs_index);
    return seeds;
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template<class Deposit, class DirEntry>
  void distribute_children(SplitSeeds<typename DirEntry::Key>& seeds,
                           Deposit&&                           deposit,
                           DirEntry&                           lhs,
                           DirEntry&                           rhs,
                           const ChildCount                    max_fanout)
  {
    const size_t n_entries = deposit.size();
    for (size_t i = 0; i < n_entries; ++i) {
      const auto  best   = pick_next(seeds, deposit, lhs, rhs);
      auto* const iter   = deposit.begin() + best.child_index;
      auto&       parent = best.side == Side::left ? lhs : rhs;

      assert(best.child_index < deposit.size());

      const auto n_children =
        detail::distribute_child(parent, best.new_parent_key, std::move(*iter));

      deposit.pop(iter);

      if (n_children == max_fanout) {
        detail::distribute_remaining(best.side == Side::left ? rhs : lhs,
                                     deposit);
        return;
      }

      if (best.side == Side::left) {
        seeds.lhs_volume = best.new_parent_volume;
      } else {
        seeds.rhs_volume = best.new_parent_volume;
      }
    }
  }

private:
  template<class DirKey>
  using VolumeOf = decltype(volume(std::declval<DirKey>()));

  /// Assignment of a child to a parent during a split
  template<class DirKey>
  struct ChildAssignment {
    size_t           child_index;
    DirKey           new_parent_key;
    VolumeOf<DirKey> new_parent_volume;
    Side             side;
  };

  /// Return |a - b| safely for unsigned types
  template<class T>
  static constexpr T abs_diff(T a, T b)
  {
    return a > b ? a - b : b - a;
  }

  /// Choose the next child to distribute during a split
  template<class Deposit, class DirEntry>
  ChildAssignment<typename DirEntry::Key> pick_next(
    const SplitSeeds<typename DirEntry::Key>& seeds,
    const Deposit&                            deposit,
    const DirEntry&                           lhs,
    const DirEntry&                           rhs)
  {
    using DirNode    = typename DirEntry::Node;
    using DirKey     = typename DirNode::NodeKey;
    using Volume     = VolumeOf<DirKey>;
    using Result     = ChildAssignment<DirKey>;
    using Preference = Volume;

    Preference best_preference{0};
    Result     best{deposit.size(), DirKey{}, Volume{}, Side::left};

    for (size_t i = 0; i < deposit.size(); ++i) {
      const auto& child      = deposit[i];
      const auto& child_key  = entry_key(child);
      const auto  l_key      = lhs.key | child_key;
      const auto  r_key      = rhs.key | child_key;
      const auto  l_volume   = volume(l_key);
      const auto  r_volume   = volume(r_key);
      const auto  l_d_volume = l_volume - seeds.lhs_volume;
      const auto  r_d_volume = r_volume - seeds.rhs_volume;

      const Preference preference = abs_diff(l_d_volume, r_d_volume);
      if (preference >= best_preference) {
        const Side best_side = (l_d_volume < r_d_volume)   ? Side::left
                               : (r_d_volume < l_d_volume) ? Side::right
                               : (seeds.lhs_volume < seeds.rhs_volume)
                                 ? Side::left
                               : (seeds.rhs_volume < seeds.lhs_volume)
                                 ? Side::right
                                 : tie_side(lhs.key, rhs.key, child_key);

        best_preference = preference;
        best            = (best_side == Side::left)
                            ? Result{i, l_key, l_volume, Side::left}
                            : Result{i, r_key, r_volume, Side::right};
      }
    }

    return best;
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

#endif // SPAIX_QUADRATICSPLIT_HPP
