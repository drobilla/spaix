// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_QUADRATICSPLIT_HPP
#define SPAIX_QUADRATICSPLIT_HPP

#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/detail/distribute.hpp"
#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

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
  std::pair<ChildIndex, ChildIndex> pick_seeds(
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

    SeedWaste                 max_waste{std::numeric_limits<Volume>::lowest()};
    std::pair<size_t, size_t> seeds{deposit.size(), deposit.size()};
    for (size_t i = 0; i < deposit.size() - 1; ++i) {
      for (size_t j = i + 1; j < deposit.size(); ++j) {
        const auto& k = entry_key(deposit[i]);
        const auto& l = entry_key(deposit[j]);

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
    return seeds;
  }

  /// Distribute nodes in `deposit` between parents `lhs` and `rhs`
  template<class Deposit, class DirEntry>
  void distribute_children(Deposit&&        deposit,
                           DirEntry&        lhs,
                           DirEntry&        rhs,
                           const ChildCount max_fanout)
  {
    const size_t n_entries = deposit.size();
    for (size_t i = 0; i < n_entries; ++i) {
      const auto  best   = pick_next(deposit, lhs, rhs);
      auto* const iter   = deposit.begin() + best.child_index;
      auto&       parent = best.side == Side::left ? lhs : rhs;

      assert(best.child_index < deposit.size());
      detail::distribute_child(parent, best.new_parent_key, std::move(*iter));
      deposit.pop(iter);

      if (parent.node->num_children() == max_fanout) {
        detail::distribute_remaining(best.side == Side::left ? rhs : lhs,
                                     deposit);
        return;
      }
    }
  }

private:
  /// Assignment of a child to a parent during a split
  template<class DirKey>
  struct ChildAssignment {
    size_t child_index;
    DirKey new_parent_key;
    Side   side;
  };

  /// Return |a - b| safely for unsigned types
  template<class T>
  static constexpr T abs_diff(T a, T b)
  {
    return a > b ? a - b : b - a;
  }

  /// Choose the next child to distribute during a split
  template<class Deposit, class DirEntry>
  static ChildAssignment<typename DirEntry::Key>
  pick_next(const Deposit& deposit, const DirEntry& lhs, const DirEntry& rhs)
  {
    using DirNode    = typename DirEntry::Node;
    using DirKey     = typename DirNode::NodeKey;
    using Volume     = decltype(volume(std::declval<DirKey>()));
    using Result     = ChildAssignment<DirKey>;
    using Preference = Volume;

    Preference best_preference{0};
    Result     best{deposit.size(), DirKey{}, Side::left};

    const auto lhs_volume = volume(lhs.key);
    const auto rhs_volume = volume(rhs.key);

    for (size_t i = 0; i < deposit.size(); ++i) {
      const auto& child       = deposit[i];
      const auto  l_key       = lhs.key | entry_key(child);
      const auto  r_key       = rhs.key | entry_key(child);
      const auto  l_volume    = volume(l_key);
      const auto  r_volume    = volume(r_key);
      const auto  l_expansion = l_volume - lhs_volume;
      const auto  r_expansion = r_volume - rhs_volume;

      const Preference preference = abs_diff(l_expansion, r_expansion);
      if (preference >= best_preference) {
        best_preference = preference;
        best = (l_expansion < r_expansion)   ? Result{i, l_key, Side::left}
               : (r_expansion < l_expansion) ? Result{i, r_key, Side::right}
               : (lhs_volume < rhs_volume)   ? Result{i, l_key, Side::left}
                                             : Result{i, r_key, Side::right};
      }
    }

    return best;
  }
};

} // namespace spaix

#endif // SPAIX_QUADRATICSPLIT_HPP
