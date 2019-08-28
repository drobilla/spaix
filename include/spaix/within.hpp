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

#ifndef SPAIX_WITHIN_HPP
#define SPAIX_WITHIN_HPP

#include "spaix/Rect.hpp"
#include "spaix/contains.hpp"
#include "spaix/intersects.hpp"

namespace spaix {

template <typename QueryKey>
struct Within
{
  template <class DirKey>
  constexpr bool directory(const DirKey& k) const
  {
    return intersects(key, k);
  }

  template <class DatKey>
  constexpr bool leaf(const DatKey& k) const
  {
    return contains(key, k);
  }

  const QueryKey key;
};

/// Return a query predicate that matches items contained within a region
template <class QueryKey>
Within<QueryKey>
within(QueryKey key)
{
  return Within<QueryKey>{std::move(key)};
}

} // namespace spaix

#endif // SPAIX_WITHIN_HPP
