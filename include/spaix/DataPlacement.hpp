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

#ifndef SPAIX_DATAPLACEMENT_HPP
#define SPAIX_DATAPLACEMENT_HPP

namespace spaix {

/**
   Policy for the allocation and placement of data nodes.

   This can be used to control whether data nodes are stored within the entries
   of their directory nodes (which makes sense if the key and data types are
   small), or allocated as separate nodes.

   Pointers to data nodes are only stable across tree modifications if they are
   allocated separately.
*/
enum class DataPlacement {
  inlined,  //!< Data nodes stored within directory node entries
  separate, //!< Data nodes separately allocated
};

} // namespace spaix

#endif // SPAIX_DATAPLACEMENT_HPP
