// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_QUADRATICSPLIT_HPP
#define SPAIX_QUADRATICSPLIT_HPP

#include "spaix/SideChooser.hpp"
#include "spaix/SplitSeeds.hpp"
#include "spaix/StaticVector.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/detail/distribute.hpp"
#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <array>
#include <cassert>
#include <limits>
#include <utility>

namespace spaix {

/**
   Quadratic node split.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
class QuadraticSplit
{
public:
  template<class DirKey>
  using VolumeOf = decltype(volume(std::declval<DirKey>()));

  template<class DirKey>
  using SeedsFor = SplitSeeds<VolumeOf<DirKey>>;

  /// Return the indices of the children that should be used for split seeds
  template<class DirKey, class Entry, ChildCount count>
  SeedsFor<DirKey> pick_seeds(
    const StaticVector<Entry, ChildCount, count>& deposit)
  {
    using Volume = decltype(volume(std::declval<DirKey>()));

    assert(deposit.size() > 1U);
    assert(deposit.size() == deposit.capacity());

    std::array<Volume, count> volumes{};
    for (ChildIndex i = 0; i < deposit.size(); ++i) {
      volumes[i] = volume(entry_key(deposit[i]));
    }

    Volume           max_waste{std::numeric_limits<Volume>::lowest()};
    SeedsFor<DirKey> seeds{};
    for (ChildIndex i = 0; i < deposit.size() - 1; ++i) {
      for (ChildIndex j = i + 1; j < deposit.size(); ++j) {
        const auto& k = entry_key(deposit[i]);
        const auto& l = entry_key(deposit[j]);

        const Volume waste = volume(k | l) - volumes[i] - volumes[j];

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
  void distribute_children(SeedsFor<typename DirEntry::Key>& seeds,
                           Deposit&&                         deposit,
                           DirEntry&                         lhs,
                           DirEntry&                         rhs,
                           const ChildCount                  max_fanout)
  {
    const ChildIndex n_entries = deposit.size();
    for (ChildIndex i = 0; i < n_entries; ++i) {
      const auto  best   = pick_next(seeds, deposit, lhs, rhs);
      auto* const iter   = deposit.begin() + best.child_index;
      auto&       parent = best.side == Side::left ? lhs : rhs;

      assert(best.child_index < deposit.size());

      const auto n_children =
        detail::distribute_child(parent, best.new_parent_key, std::move(*iter));

      deposit.pop(iter);

      if (n_children == max_fanout) {
        detail::distribute_remaining(best.side == Side::left ? rhs : lhs,
                                     std::forward<Deposit>(deposit));
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
  /// Assignment of a child to a parent during a split
  template<class DirKey>
  struct ChildAssignment {
    ChildIndex       child_index;
    DirKey           new_parent_key;
    VolumeOf<DirKey> new_parent_volume;
    Side             side;
  };

  /// Choose the next child to distribute during a split
  template<class Deposit, class DirEntry>
  ChildAssignment<typename DirEntry::Key> pick_next(
    const SeedsFor<typename DirEntry::Key>& seeds,
    const Deposit&                          deposit,
    const DirEntry&                         lhs,
    const DirEntry&                         rhs)
  {
    using DirNode    = typename DirEntry::Node;
    using DirKey     = typename DirNode::DirKey;
    using Volume     = VolumeOf<DirKey>;
    using Result     = ChildAssignment<DirKey>;
    using Preference = Volume;

    Preference best_preference{0};
    Result     best{deposit.size(), DirKey{}, Volume{}, Side::left};

    for (ChildIndex i = 0; i < deposit.size(); ++i) {
      const auto& key     = entry_key(deposit[i]);
      auto        chooser = make_side_chooser(seeds, lhs.key, rhs.key, key);

      const auto preference = chooser.preference();
      if (preference >= best_preference) {
        const Side best_side = chooser.choose_side();
        const auto outcome   = chooser.outcome(best_side);

        best_preference = preference;
        best            = Result{i, outcome.key, outcome.volume, best_side};
      }
    }

    return best;
  }
};

} // namespace spaix

#endif // SPAIX_QUADRATICSPLIT_HPP
