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

#ifndef SPAIX_DISTRIBUTE_HPP
#define SPAIX_DISTRIBUTE_HPP

#include <cstddef>
#include <utility>

namespace spaix {

template<class DirEntry, class DirKey, class Entry>
static void
distribute_child(DirEntry& parent, const DirKey& new_parent_key, Entry&& child)
{
  parent.key = new_parent_key;
  parent.node->append_child(std::forward<Entry>(child));
}

template<class DirEntry, class Deposit>
static void
distribute_remaining(DirEntry& parent, Deposit&& deposit)
{
  for (size_t i = 0; i < deposit.size(); ++i) {
    parent.key |= entry_key(deposit[i]);
    parent.node->append_child(std::move(deposit[i]));
  }
}

} // namespace spaix

#endif // SPAIX_DISTRIBUTE_HPP
