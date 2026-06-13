// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_QUADRATICSPLIT_HPP
#define SPAIX_QUADRATICSPLIT_HPP

#include <spaix/SideChooser.hpp>
#include <spaix/SplitSeeds.hpp>
#include <spaix/StaticVector.hpp>
#include <spaix/detail/EntryTracker.hpp>
#include <spaix/detail/distribute.hpp>
#include <spaix/detail/entry.hpp>
#include <spaix/types.hpp>

#include <array>
#include <cassert>
#include <limits>
#include <utility>

namespace spaix {

/**
   Quadratic node split.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
template<typename Ops>
class QuadraticSplit
{
public:
  using Volume = typename Ops::Volume;

  /// Return the indices of the children that should be used for split seeds
  template<class DirKey, class Entry, class ChildCount, ChildCount count>
  SplitSeeds<ChildCount, Volume> pick_seeds(
    const StaticVector<Entry, ChildCount, count>& deposit) noexcept
  {
    using ChildIndex = ChildCount;

    assert(deposit.size() > 1U);
    assert(deposit.size() == deposit.capacity());

    std::array<Volume, count> volumes{};
    for (ChildIndex i = 0; i < deposit.size(); ++i) {
      volumes[i] = Ops::volume(detail::entry_key(deposit[i]));
    }

    Volume max_waste{std::numeric_limits<Volume>::lowest()};
    SplitSeeds<ChildCount, Volume> seeds{};
    for (ChildIndex i = 0; i < deposit.size() - 1; ++i) {
      for (auto j = static_cast<ChildIndex>(i + 1); j < deposit.size(); ++j) {
        const auto& k = detail::entry_key(deposit[i]);
        const auto& l = detail::entry_key(deposit[j]);

        const auto both  = Ops::unify(DirKey{k}, l);
        const auto waste = Ops::volume(both) - volumes[i] - volumes[j];

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
  template<class Deposit, class DirEntry, class ChildCount>
  void distribute_children(
    SplitSeeds<ChildCount, Volume>&                    seeds,
    Deposit&&                                          deposit,
    typename Deposit::size_type                        track_index,
    SplitParts<DirEntry, typename Deposit::size_type>& parts,
    const unsigned                                     max_fanout) noexcept
  {
    DirEntry& lhs = parts.sides[0];
    DirEntry& rhs = parts.sides[1];

    detail::EntryTracker<DirEntry, ChildCount> tracker{
      parts, track_index >= deposit.size()};

    const auto n_entries = deposit.size();
    for (auto i = 0U; i < n_entries; ++i) {
      const auto  best   = pick_next(seeds, deposit, lhs, rhs);
      auto* const iter   = deposit.begin() + best.child_index;
      auto&       parent = best.side == Side::left ? lhs : rhs;
      if (best.child_index == track_index) {
        tracker.track(best.side, parent.node->num_children());
      }

      assert(best.child_index < deposit.size());

      const auto n_children =
        detail::distribute_child(parent, best.new_parent_key, std::move(*iter));

      deposit.pop_at(best.child_index);
      if (track_index == deposit.size()) {
        track_index = best.child_index;
      }

      if (n_children == max_fanout) {
        const auto new_index =
          detail::distribute_remaining<Ops>(best.side == Side::left ? rhs : lhs,
                                            std::forward<Deposit>(deposit),
                                            track_index);
        tracker.track((best.side == Side::left) ? Side::right : Side::left,
                      new_index);
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
  template<class ChildIndex, class DirKey>
  struct ChildAssignment {
    ChildIndex child_index;
    DirKey     new_parent_key;
    Volume     new_parent_volume;
    Side       side;
  };

  /// Choose the next child to distribute during a split
  template<class Deposit, class DirEntry>
  ChildAssignment<typename Deposit::size_type, typename DirEntry::Key>
  pick_next(const SplitSeeds<typename Deposit::size_type, Volume>& seeds,
            const Deposit&                                         deposit,
            const DirEntry&                                        lhs,
            const DirEntry&                                        rhs) noexcept
  {
    using ChildIndex = typename Deposit::size_type;
    using DirNode    = typename DirEntry::Node;
    using DirKey     = typename DirNode::DirKey;
    using Result     = ChildAssignment<ChildIndex, DirKey>;
    using Preference = Volume;

    Preference best_preference{0};
    Result     best{deposit.size(), DirKey{}, Volume{}, Side::left};

    for (ChildIndex i = 0; i < deposit.size(); ++i) {
      auto chooser = make_side_chooser<Ops>(seeds,
                                            lhs.key,
                                            detail::entry_num_children(lhs),
                                            rhs.key,
                                            detail::entry_num_children(rhs),
                                            detail::entry_key(deposit[i]));

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
