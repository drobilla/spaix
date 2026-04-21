// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DATAITERATOR_HPP
#define SPAIX_DATAITERATOR_HPP

#include <spaix/StaticVector.hpp>
#include <spaix/detail/entry.hpp>
#include <spaix/types.hpp>

#include <cassert>

namespace spaix {

/**
   Base class for iterators that refer to data entries.

   This is either empty, or can be dereferenced to access a data entry.
*/
template<class DirNode, class DatNode, unsigned max_height>
class DataIterator
{
public:
  using value_type = DatNode;
  using pointer    = const DatNode*;
  using reference  = const DatNode&;

  DataIterator() noexcept = default;

  const DatNode& operator*() const noexcept { return *operator->(); }

  const DatNode* operator->() const noexcept
  {
    assert(!empty());
    assert(back().index < back().node->num_children());
    assert(back().node->child_type() == NodeType::data);

    return detail::entry_ptr(back().node->dat_children()[back().index]);
  }

  template<class RhsDir, class RhsDat>
  bool operator==(
    const DataIterator<RhsDir, RhsDat, max_height>& rhs) const noexcept
  {
    return (empty() || rhs.empty()) ? (empty() && rhs.empty())
                                    : (operator->() == rhs.operator->());
  }

  template<class RhsDir, class RhsDat>
  bool operator!=(
    const DataIterator<RhsDir, RhsDat, max_height>& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  [[nodiscard]] bool empty() const noexcept { return _stack.empty(); }

protected:
  void step_down(const DirNode* const node, const ChildIndex index) noexcept
  {
    _stack.emplace_back(Frame{node, index});
  }

  void step_up() noexcept { _stack.pop_back(); }

  void step_right() noexcept { ++back().index; }

  void clear() noexcept { _stack.clear(); }

  [[nodiscard]] const DirNode* parent() const noexcept { return back().node; }
  [[nodiscard]] ChildIndex     index() const noexcept { return back().index; }

private:
  struct Frame {
    const DirNode* node;  ///< Pointer to directory node
    ChildIndex     index; ///< Index of child

    friend bool operator==(const Frame& lhs, const Frame& rhs) noexcept
    {
      return lhs.node == rhs.node && lhs.index == rhs.index;
    }
  };

  [[nodiscard]] Frame& back() noexcept
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  [[nodiscard]] const Frame& back() const noexcept
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  StaticVector<Frame, unsigned, max_height + 1U> _stack;
};

} // namespace spaix

#endif // SPAIX_DATAITERATOR_HPP
