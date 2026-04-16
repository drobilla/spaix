// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DATANODE_HPP
#define SPAIX_DATANODE_HPP

#include <utility>

namespace spaix {

/// A data record in a tree
template<class Key, class Data>
using DataNode = std::pair<Key, Data>;

} // namespace spaix

#endif // SPAIX_DATANODE_HPP
