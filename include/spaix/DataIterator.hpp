// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DATAITERATOR_HPP
#define SPAIX_DATAITERATOR_HPP

#include <spaix/StaticVector.hpp>
#include <spaix/detail/DirectoryNode.hpp>
#include <spaix/types.hpp>

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

  static DataIterator make_end() { return DataIterator{}; }

  const DatNode& operator*() const { return *operator->(); }

  const DatNode* operator->() const
  {
    assert(!_stack.empty());
    assert(_stack.back().index < _stack.back().node->num_children());
    assert(_stack.back().node->child_type() == NodeType::data);

    return entry_ptr(_stack.back().node->dat_children()[_stack.back().index]);
  }

  template<class RhsDir, class RhsDat>
  bool operator==(const DataIterator<RhsDir, RhsDat, max_height>& rhs) const
  {
    return (empty() || rhs.empty()) ? (empty() && rhs.empty())
                                    : (operator->() == rhs.operator->());
  }

  template<class RhsDir, class RhsDat>
  bool operator!=(const DataIterator<RhsDir, RhsDat, max_height>& rhs) const
  {
    return !operator==(rhs);
  }

  [[nodiscard]] bool empty() const { return _stack.empty(); }

protected:
  struct Frame {
    const DirNode* node;  ///< Pointer to directory node
    ChildIndex     index; ///< Index of child

    friend bool operator==(const Frame& lhs, const Frame& rhs) noexcept
    {
      return lhs.node == rhs.node && lhs.index == rhs.index;
    }
  };

  using Stack = StaticVector<Frame, size_t, max_height>;

  DataIterator() = default;

  [[nodiscard]] Frame& back()
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  [[nodiscard]] const Frame& back() const
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  [[nodiscard]] Stack& stack() { return _stack; }

  [[nodiscard]] const DirNode* node() const { return back().node; }
  [[nodiscard]] ChildIndex     index() const { return back().index; }
  [[nodiscard]] const Stack&   stack() const { return _stack; }

private:
  Stack _stack;
};

} // namespace spaix

#endif // SPAIX_DATAITERATOR_HPP
