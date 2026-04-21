// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_ENTRYITERATOR_HPP
#define SPAIX_ENTRYITERATOR_HPP

#include <spaix/types.hpp>

#include <array>
#include <cassert>

namespace spaix {

/**
   Base class for any RTree iterator.

   This is either empty, or refers to an entry in a directory node.
   Implementation details are hidden behind a simple interface that provides
   access to the parent node and entry index, and basic tree movement
   operations.

   This base class doesn't provide a C++ iterator interface because the type of
   the node being referred to is unknown.

   The implementation is a simple stack, but the interface is designed so it
   could be implemented in other ways (for example, for a tree with parent
   pointers).
*/
template<class DirNode, unsigned max_height>
class EntryIterator
{
public:
  EntryIterator() noexcept = default;
  ~EntryIterator()         = default;

  EntryIterator(const EntryIterator&)            = default;
  EntryIterator& operator=(const EntryIterator&) = default;

  EntryIterator(EntryIterator&&) noexcept            = default;
  EntryIterator& operator=(EntryIterator&&) noexcept = default;

  bool operator==(const EntryIterator& rhs) const noexcept
  {
    return empty()       ? rhs.empty()
           : rhs.empty() ? empty()
                         : (parent() == rhs.parent() && index() == rhs.index());
  }

  bool operator!=(const EntryIterator& rhs) const noexcept
  {
    return !(*this == rhs);
  }

  [[nodiscard]] DirNode* parent() const noexcept
  {
    assert(_size);
    return _parents[_size - 1];
  }

  [[nodiscard]] ChildIndex index() const noexcept
  {
    assert(_size);
    return _indexes[_size - 1];
  }

  [[nodiscard]] bool empty() const noexcept { return _size == 0; }

  void step_up() noexcept
  {
    assert(_size);
    --_size;
  }

  void step_down(DirNode* const node, const ChildIndex index) noexcept
  {
    assert(_size < max_height);
    _parents[_size] = node;
    _indexes[_size] = index;
    ++_size;
  }

  void step_right() noexcept
  {
    assert(_size);
    ++_indexes[_size - 1];
  }

  void clear() noexcept { _size = 0; }

private:
  static constexpr auto max_size = max_height + 1U;

  std::array<DirNode*, max_size>   _parents{};
  unsigned                         _size{};
  std::array<ChildIndex, max_size> _indexes{};
};

} // namespace spaix

#endif // SPAIX_ENTRYITERATOR_HPP
