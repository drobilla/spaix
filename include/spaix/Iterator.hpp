/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPAIX_ITERATOR_HPP
#define SPAIX_ITERATOR_HPP

#include "spaix/StaticVector.hpp"
#include "spaix/types.hpp"

#include <cassert>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>

namespace spaix {

template <class Node>
struct StackFrame
{
  const Node* node;  ///< Pointer to directory node
  ChildIndex  index; ///< Index of node child

  bool operator==(const StackFrame& rhs) const
  {
    return node == rhs.node && index == rhs.index;
  }
};

template <class Predicate, class DirNode, class DatNode, size_t max_height>
struct Iterator : public std::iterator<std::forward_iterator_tag,
                                       DatNode,
                                       intptr_t,
                                       const DatNode*,
                                       const DatNode&>
{
  using Frame = StackFrame<DirNode>;
  using Stack = StaticVector<Frame, size_t, max_height>;

  Iterator(Stack&& stack, Predicate predicate)
    : _stack{std::move(stack)}, _predicate{std::move(predicate)}
  {
  }

  Iterator& operator++()
  {
    scan_next();
    return *this;
  }

  const DatNode& operator*() const
  {
    assert(!_stack.empty());
    assert(_stack.back().index < _stack.back().node->n_children);

    return *_stack.back().node->dat_children[_stack.back().index];
  }

  const DatNode* operator->() const
  {
    assert(!_stack.empty());
    assert(_stack.back().index < _stack.back().node->n_children);
    assert(_stack.back().node->child_type == NodeType::DAT);

    return _stack.back().node->dat_children[_stack.back().index].get();
  }

  bool operator==(const Iterator& rhs) const { return _stack == rhs._stack; }

  bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

private:
  Frame& back()
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  /// Move up until we reach a node we are not at the end of yet
  void move_up()
  {
    while (!_stack.empty() && back().index >= back().node->n_children - 1) {
      _stack.pop_back();
    }
  }

  /// Move right until we reach a good leaf or the end of the parent
  bool move_right_leaf()
  {
    assert(back().node->child_type == NodeType::DAT);
    do {
      ++back().index;
    } while (back().index < back().node->n_children &&
             !_predicate.leaf(back().node->dat_children[back().index]->key));

    return back().index < back().node->n_children;
  }

  /// Move right until we reach a good directory or the end of the parent
  bool move_right_dir()
  {
    assert(back().node->child_type == NodeType::DIR);
    do {
      ++back().index;
    } while (
        back().index < back().node->n_children &&
        !_predicate.directory(back().node->dir_children[back().index]->key));

    return back().index < back().node->n_children;
  }

  bool increment()
  {
    if (move_right_leaf()) {
      return true; // Moved to next leaf child
    }

    while (!_stack.empty() && back().index >= back().node->n_children) {
      move_up();
      if (_stack.empty()) {
        return false; // Reached end of tree
      }

      move_right_dir();
    }

    _stack.emplace_back(
        Frame{back().node->dir_children[back().index].get(), 0});

    // Move down and left until we are at the next leaf
    while (back().node->child_type == NodeType::DIR) {
      _stack.emplace_back(Frame{back().node->dir_children[0].get(), 0});
    }

    return true;
  }

  void scan_next()
  {
    do {
      if (!increment()) {
        return;
      }
    } while (!_predicate.leaf(back().node->dat_children[back().index]->key));
  }

  Stack     _stack;
  Predicate _predicate;
};

} // namespace spaix

#endif // SPAIX_ITERATOR_HPP
