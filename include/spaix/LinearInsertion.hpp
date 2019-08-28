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
  static size_t choose(const Children& children, const Key& key)
  {
    using Volume = decltype(volume(std::declval<Key>()));
    using Sizes  = std::tuple<Volume, ChildCount>;

    size_t best_index = 0;
    Sizes  best_sizes{std::numeric_limits<Volume>::max(),
                     std::numeric_limits<ChildCount>::max()};

    for (size_t i = 0; i < children.size() && children[i]; ++i) {
      const auto& child    = children[i];
      const auto  expanded = child->key | key;

      Volume child_expansion = 0;
      if (expanded != child->key) {
        child_expansion = volume(expanded) - volume(child->key);
      }

      Sizes sizes{child_expansion, child->num_children()};
      if (sizes < best_sizes) {
        best_sizes = std::move(sizes);
        best_index = i;
      }
    }

    return best_index;
  }
};

} // namespace spaix

#endif // SPAIX_LINEARINSERTION_HPP
