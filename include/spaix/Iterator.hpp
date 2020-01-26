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
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace spaix {

template <class Predicate, class DirNode, class DatNode, size_t max_height>
struct Iterator
{
  using iterator_category = std::forward_iterator_tag;
  using value_type        = DatNode;
  using difference_type   = intptr_t;
  using pointer           = const DatNode*;
  using reference         = const DatNode&;

  struct Frame
  {
    const DirNode* node;  ///< Pointer to directory node
    ChildIndex     index; ///< Index of child

    bool operator==(const Frame& rhs) const
    {
      return node == rhs.node && index == rhs.index;
    }
  };

  using Stack    = StaticVector<Frame, size_t, max_height>;
  using DirEntry = typename DirNode::DirEntry;
  using DirKey   = typename DirNode::NodeKey;

  Iterator(const DirEntry& root_entry, Predicate predicate)
    : _stack{}, _predicate{std::move(predicate)}
  {
    const auto& root = root_entry.node;
    if (root && predicate.directory(root_entry.key)) {
      const ChildIndex root_child_index = leftmost_child(*root, predicate);
      if (root_child_index < root->num_children()) {
        _stack.emplace_back(Frame{root.get(), root_child_index});
        move_down_left();
      }
    }
  }

  Iterator& operator++()
  {
    scan_next();
    return *this;
  }

  const DatNode& operator*() const { return *operator->(); }

  const DatNode* operator->() const
  {
    assert(!_stack.empty());
    assert(_stack.back().index < _stack.back().node->num_children());
    assert(_stack.back().node->child_type == NodeType::data);

    return &_stack.back().node->dat_children[_stack.back().index];
  }

  bool operator==(const Iterator& rhs) const
  {
    return _stack.size() == rhs._stack.size() &&
           (_stack.empty() || _stack.back() == rhs._stack.back());
  }

  bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

private:
  static ChildIndex
  leftmost_child(const DirNode& dir, const Predicate& predicate)
  {
    switch (dir.child_type) {
    case NodeType::directory:
      for (size_t i = 0u; i < dir.dir_children.size(); ++i) {
        if (predicate.directory(dir.dir_children[i].key)) {
          return static_cast<ChildIndex>(i);
        }
      }
      return dir.dir_children.size();

    case NodeType::data:
      for (size_t i = 0u; i < dir.dat_children.size(); ++i) {
        if (predicate.leaf(dir.dat_children[i].key)) {
          return static_cast<ChildIndex>(i);
        }
      }
      return dir.dat_children.size();
    }

    return 0; // Unreached
  }

  Frame& back()
  {
    assert(!_stack.empty());
    return _stack.back();
  }

  const DirNode* node() { return back().node; }
  ChildIndex     index() { return back().index; }

  /// Move right until we reach a good leaf or the end of the parent
  bool move_right_leaf()
  {
    assert(node()->child_type == NodeType::data);
    do {
      ++back().index;
    } while (index() < node()->dat_children.size() &&
             !_predicate.leaf(node()->dat_children[index()].key));

    return index() < node()->dat_children.size();
  }

  /// Move right until we reach a good directory or the end of the parent
  void move_right_dir()
  {
    assert(node()->child_type == NodeType::directory);
    do {
      ++back().index;
    } while (index() < node()->dir_children.size() &&
             !_predicate.directory(node()->dir_children[index()].key));
  }

  /// Move up/right until we reach a node we are not at the end of yet
  bool move_up_right()
  {
    while (!_stack.empty() && index() >= node()->num_children()) {
      _stack.pop_back(); // Move up
      if (_stack.empty()) {
        return false; // Reached end of tree
      }

      move_right_dir(); // Move to the next child of this node
    }

    return true;
  }

  /// Move down/left until we reach a potentially matching leaf
  bool move_down_left()
  {
    while (node()->child_type == NodeType::directory) {
      auto* const dir = node()->dir_children[index()].node.get();
      if (dir->child_type == NodeType::data) {
        _stack.emplace_back(Frame{dir, 0});
      } else {
        const ChildIndex index = leftmost_child(*dir, _predicate);
        _stack.emplace_back(Frame{dir, index});

        if (index >= dir->dir_children.size()) {
          // Reached a non-matching directory, skip this subtree
          if (!move_up_right()) {
            return false; // Reached end of tree
          }
        }
      }
    }

    return true;
  }

  bool increment()
  {
    if (move_right_leaf()) {
      return true; // Moved to next leaf child
    } else if (!move_up_right()) {
      return false; // Reached end of tree
    }

    // Now at a matching directory, and a matching child of that directory
    assert((node()->child_type == NodeType::directory &&
            _predicate.directory(node()->dir_children[index()].key)) ||
           (_predicate.leaf(node()->dat_children[index()].key)));

    return move_down_left();
  }

  void scan_next()
  {
    do {
      if (!increment()) {
        return;
      }
    } while (!_predicate.leaf(node()->dat_children[index()].key));
  }

  Stack     _stack;
  Predicate _predicate;
};

} // namespace spaix

#endif // SPAIX_ITERATOR_HPP
