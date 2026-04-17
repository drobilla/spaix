// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_NODEENTRY_HPP
#define SPAIX_DETAIL_NODEENTRY_HPP

#include <spaix/DataPlacement.hpp>

#include <memory>
#include <utility>

namespace spaix {

template<class ChildKey, class ChildNode>
struct NodePointerEntry {
  using Key  = ChildKey;
  using Node = ChildNode;

  Key                   key{};
  std::unique_ptr<Node> node{};
};

template<class DatNode, DataPlacement placement>
struct DatEntryType;

template<class DatNode>
struct DatEntryType<DatNode, DataPlacement::inlined> {
  using Type = DatNode;

  template<class Key, class Data>
  static Type make(Key key, Data data) noexcept
  {
    return DatNode{std::move(key), std::move(data)};
  }
};

template<class DatNode>
struct DatEntryType<DatNode, DataPlacement::separate> {
  using Type = std::unique_ptr<DatNode>;

  template<class Key, class Data>
  static Type make(Key key, Data data) noexcept
  {
    return std::unique_ptr<DatNode>(
      new DatNode{std::move(key), std::move(data)});
  }
};

template<class DatNode, spaix::DataPlacement placement>
using DataNodeEntry = typename DatEntryType<DatNode, placement>::Type;

} // namespace spaix

#endif // SPAIX_DETAIL_NODEENTRY_HPP
