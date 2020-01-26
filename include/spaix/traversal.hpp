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

#ifndef SPAIX_TRAVERSAL_HPP
#define SPAIX_TRAVERSAL_HPP

#include "spaix/types.hpp"

#include <cstddef>

namespace spaix {

/**
   Return the index of the leftmost child of `dir` that matches `predicate`.

   Returns the end index (on past the last child) if no child matches.
*/
template <class DirNode, class Predicate>
ChildIndex
leftmost_child(const DirNode& dir, const Predicate& predicate)
{
  switch (dir.child_type) {
  case NodeType::directory:
    for (size_t i = 0u; i < dir.dir_children.size(); ++i) {
      if (predicate.directory(dir.dir_children[i].key)) {
        return static_cast<ChildIndex>(i);
      }
    }
    return dir.dir_children.size();

  case NodeType::data:
    for (size_t i = 0u; i < dir.dat_children.size(); ++i) {
      if (predicate.leaf(dir.dat_children[i].key)) {
        return static_cast<ChildIndex>(i);
      }
    }
    return dir.dat_children.size();
  }

  return 0; // Unreached
}

} // namespace spaix

#endif // SPAIX_TRAVERSAL_HPP
