// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TYPES_HPP
#define SPAIX_TYPES_HPP

#include <cstddef>
#include <utility>

namespace spaix {

using ChildIndex = size_t;
using ChildCount = size_t;

template<class T>
using Range = std::pair<T, T>;

enum class NodeType : size_t {
  directory, ///< Internal directory node
  data,      ///< Leaf data node
};

enum class Side { left, right };

} // namespace spaix

#endif // SPAIX_TYPES_HPP
