// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TREERANGE_HPP
#define SPAIX_TREERANGE_HPP

#include <utility> // IWYU pragma: keep

/**
   A range in a tree.

   This acts like a collection of nodes that match a query predicate,
   particularly for use with range-based for loops.  Note, however, that the
   nodes are not actually contiguous internally as the underlying iterators
   automatically skip nodes that do not match.
*/
template<class IteratorType>
struct TreeRange {
  TreeRange(IteratorType first, IteratorType last)
    : _first{std::move(first)}
    , _last{std::move(last)}
  {}

  TreeRange(const TreeRange&)            = delete;
  TreeRange& operator=(const TreeRange&) = delete;

  TreeRange(TreeRange&&) noexcept            = default;
  TreeRange& operator=(TreeRange&&) noexcept = default;

  ~TreeRange() = default;

  IteratorType&       begin() { return _first; }
  const IteratorType& end() { return _last; }

  bool empty() const { return _first == _last; }

private:
  IteratorType _first;
  IteratorType _last;
};

#endif // SPAIX_TREERANGE_HPP
