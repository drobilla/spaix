// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_LINEARINSERTION_HPP
#define SPAIX_LINEARINSERTION_HPP

#include <cstddef>
#include <limits>
#include <type_traits>
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
  static std::pair<size_t, decltype(std::declval<Key>() | std::declval<Key>())>
  choose(const Children& children, const Key& key)
  {
    using Volume = decltype(volume(std::declval<Key>()));
    using DirKey = decltype(std::declval<Key>() | std::declval<Key>());
    using Sizes  = std::pair<Volume, Volume>;

    size_t best_index{0};
    DirKey best_key{children[0].key};
    Sizes  best_sizes{std::numeric_limits<Volume>::max(),
                     std::numeric_limits<Volume>::max()};

    for (size_t i = 0; i < children.size(); ++i) {
      const auto& child_key    = children[i].key;
      const auto  child_volume = volume(child_key);
      const auto  expanded     = child_key | key;
      const auto  expansion    = volume(expanded) - child_volume;

      Sizes sizes{expansion, child_volume};
      if (sizes < best_sizes) {
        best_sizes = std::move(sizes);
        best_index = i;
        best_key   = expanded;
      }
    }

    return {best_index, best_key};
  }
};

} // namespace spaix

#endif // SPAIX_LINEARINSERTION_HPP
