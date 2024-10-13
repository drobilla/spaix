// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DATANODE_HPP
#define SPAIX_DATANODE_HPP

namespace spaix {

/// A data record in a tree
template<class Key, class Data>
struct DataNode {
  Key  key;  ///< Point or Rect key
  Data data; ///< Arbitrary user data
};

} // namespace spaix

#endif // SPAIX_DATANODE_HPP
