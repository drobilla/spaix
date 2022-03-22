// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_ITERATOR_HPP
#define SPAIX_ITERATOR_HPP

#include "spaix/DataIterator.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/types.hpp"

#include <cassert>
#include <cstddef>
#include <utility> // IWYU pragma: keep

namespace spaix {

/**
   An iterator for an RTree.

   This iterates over data items in the tree which match the given predicate.
*/
template<class Predicate, class DirNode, class DatNode, size_t max_height>
class Iterator : public DataIterator<DirNode, DatNode, max_height>
{
public:
  using Base     = DataIterator<DirNode, DatNode, max_height>;
  using DirEntry = typename DirNode::DirEntry;
  using DirKey   = typename DirNode::NodeKey;
  using Frame    = typename Base::Frame;

  Iterator(const DirEntry& root_entry, Predicate predicate)
    : Base{}
    , _predicate{std::move(predicate)}
  {
    const auto& root = root_entry.node;
    if (root && _predicate.directory(root_entry.key)) {
      const ChildIndex root_child_index = leftmost_child(*root, _predicate);
      if (root_child_index < root->num_children()) {
        Base::stack().emplace_back(Frame{root.get(), root_child_index});
        move_down_left();
      }
    }
  }

  Iterator& operator++()
  {
    scan_next();
    return *this;
  }

private:
  using Base::back;
  using Base::index;
  using Base::node;
  using Base::stack;

  /**
     Return the index of the leftmost child of `dir` that matches `predicate`.

     Returns the end index (on past the last child) if no child matches.
  */
  static ChildIndex leftmost_child(const DirNode&   dir,
                                   const Predicate& predicate)
  {
    switch (dir.child_type()) {
    case NodeType::directory:
      for (size_t i = 0u; i < dir.dir_children().size(); ++i) {
        if (predicate.directory(dir.dir_children()[i].key)) {
          return i;
        }
      }
      return dir.dir_children().size();

    case NodeType::data:
      for (size_t i = 0u; i < dir.dat_children().size(); ++i) {
        if (predicate.leaf(entry_key(dir.dat_children()[i]))) {
          return i;
        }
      }
      return dir.dat_children().size();
    }

    return 0; // Unreached
  }

  /// Move right until we reach a good leaf or the end of the parent
  bool move_right_leaf()
  {
    assert(node()->child_type() == NodeType::data);
    do {
      ++back().index;
    } while (index() < node()->dat_children().size() &&
             !_predicate.leaf(entry_key(node()->dat_children()[index()])));

    return index() < node()->dat_children().size();
  }

  /// Move right until we reach a good directory or the end of the parent
  void move_right_dir()
  {
    assert(node()->child_type() == NodeType::directory);
    do {
      ++back().index;
    } while (index() < node()->dir_children().size() &&
             !_predicate.directory(node()->dir_children()[index()].key));
  }

  /// Move up/right until we reach a node we are not at the end of yet
  bool move_up_right()
  {
    while (!stack().empty() && index() >= node()->num_children()) {
      stack().pop_back(); // Move up
      if (stack().empty()) {
        return false; // Reached end of tree
      }

      move_right_dir(); // Move to the next child of this node
    }

    return true;
  }

  /// Move down/left until we reach a potentially matching leaf
  bool move_down_left()
  {
    while (node()->child_type() == NodeType::directory) {
      auto* const dir = node()->dir_children()[index()].node.get();
      if (dir->child_type() == NodeType::data) {
        stack().emplace_back(Frame{dir, 0});
      } else {
        const ChildIndex first_index = leftmost_child(*dir, _predicate);
        stack().emplace_back(Frame{dir, first_index});

        if (first_index >= dir->dir_children().size()) {
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
    }

    if (!move_up_right()) {
      return false; // Reached end of tree
    }

    // Now at a matching directory, and a matching child of that directory
    assert((node()->child_type() == NodeType::directory &&
            _predicate.directory(node()->dir_children()[index()].key)) ||
           (_predicate.leaf(entry_key(node()->dat_children()[index()]))));

    return move_down_left();
  }

  void scan_next()
  {
    do {
      if (!increment()) {
        return;
      }
    } while (!_predicate.leaf(entry_key(node()->dat_children()[index()])));
  }

  Predicate _predicate;
};

} // namespace spaix

#endif // SPAIX_ITERATOR_HPP
