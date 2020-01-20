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

#ifndef SPAIX_LINEARINSERTION_HPP
#define SPAIX_LINEARINSERTION_HPP

#include "spaix/types.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace spaix {

struct LinearInsertion
{
  /// Choose the best child node to insert/expand by `key`
  template <class Children, class Key>
  static std::pair<size_t, decltype(std::declval<Key>() | std::declval<Key>())>
  choose(const Children& children, const Key& key)
  {
    using Volume = decltype(volume(std::declval<Key>()));
    using DirKey = decltype(std::declval<Key>() | std::declval<Key>());
    using Sizes  = std::tuple<Volume, Volume, ChildCount>;

    size_t best_index = 0;
    DirKey best_key   = children[0]->key;
    Sizes  best_sizes{std::numeric_limits<Volume>::max(),
                     std::numeric_limits<Volume>::max(),
                     std::numeric_limits<ChildCount>::max()};

    const size_t n_children = children.size();
    for (size_t i = 0; i < n_children && children[i]; ++i) {
      const auto& child           = children[i];
      const auto  expanded        = child->key | key;
      const auto  expanded_volume = volume(expanded);

      const Volume child_expansion =
          ((expanded != child->key) ? (expanded_volume - volume(child->key))
                                    : 0);

      Sizes sizes{child_expansion, expanded_volume, child->num_children()};
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
