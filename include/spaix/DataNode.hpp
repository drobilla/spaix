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

#ifndef SPAIX_DATANODE_HPP
#define SPAIX_DATANODE_HPP

#include <cstddef>

namespace spaix {

template <class Key, class Data>
struct DataNode
{
  Key  key; // FIXME? const?
  Data data;
};

} // namespace spaix

#endif // SPAIX_DATANODE_HPP
