// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_LINEARINSERTION_HPP
#define SPAIX_LINEARINSERTION_HPP

#include <spaix/types.hpp>
#include <spaix/union.hpp>
#include <spaix/volume.hpp>

#include <limits>
#include <tuple>
#include <utility>

namespace spaix {

/**
   Linear insert position selection.

   From "R-trees: A dynamic index structure for spatial searching", A. Guttman.
*/
class LinearInsertion
{
public:
  /// Choose the best child node to insert/expand by `key`
  template<class Children, class Key>
  std::pair<ChildIndex, UnionOf<Key>> choose(const Children& children,
                                             const Key&      key) noexcept
  {
    using Volume = decltype(volume(std::declval<Key>()));
    using DirKey = UnionOf<Key>;
    using Cost   = std::tuple<Volume, Volume, ChildCount>;

    constexpr auto max_volume   = std::numeric_limits<Volume>::max();
    constexpr auto max_children = std::numeric_limits<ChildCount>::max();

    ChildIndex best_index{};
    DirKey     best_key{children[0].key};
    Cost       best_cost{max_volume, max_volume, max_children};

    for (auto i = ChildIndex{}; i < children.size(); ++i) {
      const auto& child_key       = children[i].key;
      const auto  child_volume    = volume(child_key);
      const auto  new_key         = child_key | key;
      const auto  volume_increase = volume(new_key) - child_volume;

      Cost cost{volume_increase, child_volume, entry_num_children(children[i])};
      if (cost < best_cost) {
        best_cost  = std::move(cost);
        best_index = i;
        best_key   = new_key;
      }
    }

    return {best_index, best_key};
  }
};

} // namespace spaix

#endif // SPAIX_LINEARINSERTION_HPP
