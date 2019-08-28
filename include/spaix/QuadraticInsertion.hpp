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

#ifndef SPAIX_QUADRATICINSERTION_HPP
#define SPAIX_QUADRATICINSERTION_HPP

#include "spaix/intersection.hpp"
#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <utility>

namespace spaix {

struct QuadraticInsertion
{
  /// Choose the best child node to insert/expand by `key`
  template <class Children, class Key>
  static size_t choose(const Children& children, const Key& key)
  {
    using Volume = decltype(volume(std::declval<Key>()));

    // overlap, expansion, volume, n_children, in decreasing order of priority
    using Sizes = std::tuple<Volume, Volume, Volume, ChildCount>;

    size_t best_index = 0;
    Sizes  best_sizes{std::numeric_limits<Volume>::max(),
                     std::numeric_limits<Volume>::max(),
                     std::numeric_limits<Volume>::max(),
                     std::numeric_limits<ChildCount>::max()};

    for (size_t i = 0; i < children.size() && children[i]; ++i) {
      const auto& child    = children[i];
      const auto  expanded = child->key | key;

      Volume       child_overlap   = 0;
      Volume       child_expansion = 0;
      const Volume key_volume      = volume(expanded);
      if (expanded != child->key) {
        child_expansion = key_volume - volume(child->key);
        for (size_t j = 0; j < children.size() && children[j]; ++j) {
          if (i != j) {
            child_overlap += volume(expanded & children[j]->key);
          }
        }
      }

      Sizes sizes{
          child_overlap, child_expansion, key_volume, child->num_children()};

      if (sizes < best_sizes) {
        best_sizes = std::move(sizes);
        best_index = i;
      }
    }

    return best_index;
  }
};

} // namespace spaix

#endif // SPAIX_QUADRATICINSERTION_HPP
