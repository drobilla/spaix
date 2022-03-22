// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DATAITERATOR_HPP
#define SPAIX_DATAITERATOR_HPP

#include "spaix/StaticVector.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/types.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace spaix {

/// An iterator that points at a data node in an RTree
template<class DirNode, class DatNode, size_t max_height>
class DataIterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type        = DatNode;
  using difference_type   = intptr_t;
  using pointer           = const DatNode*;
  using reference         = const DatNode&;

  const DatNode& operator*() const { return *operator->(); }

  const DatNode* operator->() const
  {
    assert(!_stack.empty());
    assert(_stack.back().index < _stack.back().node->num_children());
    assert(_stack.back().node->child_type() == NodeType::data);

    return entry_ptr(_stack.back().node->dat_children()[_stack.back().index]);
  }

  bool operator==(const DataIterator& rhs) const
  {
    return _stack.size() == rhs._stack.size() &&
           (_stack.empty() || _stack.back() == rhs._stack.back());
  }

  bool operator!=(const DataIterator& rhs) const { return !operator==(rhs); }

protected:
  struct Frame {
    const DirNode* node;  ///< Pointer to directory node
    ChildIndex     index; ///< Index of child

    inline friend bool operator==(const Frame& lhs, const Frame& rhs)
    {
      return lhs.node == rhs.node && lhs.index == rhs.index;
    }
  };

  using Stack = StaticVector<Frame, size_t, max_height>;

  DataIterator() = default;

  Frame& back()
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  const Frame& back() const
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  Stack& stack() { return _stack; }

  const DirNode* node() const { return back().node; }
  ChildIndex     index() const { return back().index; }
  const Stack&   stack() const { return _stack; }

private:
  Stack _stack;
};

} // namespace spaix

#endif // SPAIX_DATAITERATOR_HPP
