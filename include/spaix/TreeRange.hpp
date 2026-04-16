// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TREERANGE_HPP
#define SPAIX_TREERANGE_HPP

#include <utility>

/**
   A range in a tree.

   This acts like a collection of nodes that match a query predicate,
   particularly for use with range-based for loops.  Note, however, that the
   nodes are not actually contiguous internally as the underlying iterators
   automatically skip nodes that do not match.
*/
template<class IteratorType>
struct TreeRange {
  TreeRange(IteratorType first, IteratorType last) noexcept
    : _first{std::move(first)}
    , _last{std::move(last)}
  {}

  TreeRange(const TreeRange&)            = delete;
  TreeRange& operator=(const TreeRange&) = delete;

  TreeRange(TreeRange&&) noexcept            = default;
  TreeRange& operator=(TreeRange&&) noexcept = default;

  ~TreeRange() = default;

  [[nodiscard]] IteratorType&       begin() noexcept { return _first; }
  [[nodiscard]] const IteratorType& end() noexcept { return _last; }

  [[nodiscard]] bool empty() const noexcept { return _first == _last; }

private:
  IteratorType _first;
  IteratorType _last;
};

#endif // SPAIX_TREERANGE_HPP
