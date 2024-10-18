// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_NODEPOINTERENTRY_HPP
#define SPAIX_DETAIL_NODEPOINTERENTRY_HPP

#include <memory>

namespace spaix {

template<class ChildKey, class ChildNode>
struct NodePointerEntry {
  using Key  = ChildKey;
  using Node = ChildNode;

  Key                   key{};
  std::unique_ptr<Node> node{};
};

} // namespace spaix

#endif // SPAIX_DETAIL_NODEPOINTERENTRY_HPP
