// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_DATENTRYTYPE_HPP
#define SPAIX_DETAIL_DATENTRYTYPE_HPP

#include <spaix/DataPlacement.hpp>

#include <memory>

namespace spaix {

template<class DatNode, DataPlacement placement>
struct DatEntryType;

template<class DatNode>
struct DatEntryType<DatNode, DataPlacement::inlined> {
  using Type = DatNode;

  template<class Key, class Data>
  static Type make(Key key, Data data)
  {
    return DatNode{std::move(key), std::move(data)};
  }
};

template<class DatNode>
struct DatEntryType<DatNode, DataPlacement::separate> {
  using Type = std::unique_ptr<DatNode>;

  template<class Key, class Data>
  static Type make(Key key, Data data)
  {
    return std::unique_ptr<DatNode>(
      new DatNode{std::move(key), std::move(data)});
  }
};

} // namespace spaix

#endif // SPAIX_DETAIL_DATENTRYTYPE_HPP
