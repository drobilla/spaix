// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

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
enum class DataPlacement : unsigned char {
  inlined,  //!< Data nodes stored within directory node entries
  separate, //!< Data nodes separately allocated
};

} // namespace spaix

#endif // SPAIX_DATAPLACEMENT_HPP
