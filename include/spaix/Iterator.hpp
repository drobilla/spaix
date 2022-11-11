// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_ITERATOR_HPP
#define SPAIX_ITERATOR_HPP

#include "spaix/DataIterator.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/types.hpp"

#include <cassert>
#include <cstddef>
#include <utility>

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

  Iterator(const DataIterator<DirNode, DatNode, max_height>& base,
           Predicate                                         predicate)
    : Base{base}
    , _predicate{predicate}
  {}

  Iterator(const DirEntry& root_entry, Predicate predicate)
    : Base{}
    , _predicate{std::move(predicate)}
  {
    const auto& root = root_entry.node;
    if (root && _predicate.directory(root_entry.key)) {
      const ChildIndex root_child_index = leftmost_child(*root, _predicate);
      if (root_child_index < root->num_children()) {
        Base::stack().emplace_back(Frame{root.get(), root_child_index});
        if (move_down_left() == Status::reached_end) {
          Base::stack().clear();
        }
      }
    }

    assert(this->empty() || _predicate.leaf(entry_key(this->operator*())));
  }

  Iterator& operator++()
  {
    if (increment() == Status::reached_end) {
      assert(this->empty());
    }

    return *this;
  }

private:
  using Base::back;
  using Base::index;
  using Base::node;
  using Base::stack;

  enum class Status {
    success,
    reached_end,
  };

  /**
     Return the index of the leftmost child of `dir` that matches `predicate`.

     Returns the end index (on past the last child) if no child matches.
  */
  static ChildIndex leftmost_child(const DirNode&   dir,
                                   const Predicate& predicate)
  {
    switch (dir.child_type()) {
    case NodeType::directory:
      for (ChildIndex i = 0U; i < dir.dir_children().size(); ++i) {
        if (predicate.directory(dir.dir_children()[i].key)) {
          return i;
        }
      }
      return dir.dir_children().size();

    case NodeType::data:
      for (ChildIndex i = 0U; i < dir.dat_children().size(); ++i) {
        if (predicate.leaf(entry_key(dir.dat_children()[i]))) {
          return i;
        }
      }
      return dir.dat_children().size();
    }

    return 0; // Unreached
  }

  /// Move right until we reach a good leaf or the end of the parent
  [[nodiscard]] Status move_right_leaf()
  {
    assert(node()->child_type() == NodeType::data);
    do {
      ++back().index;
    } while (index() < node()->dat_children().size() &&
             !_predicate.leaf(entry_key(node()->dat_children()[index()])));

    return index() < node()->dat_children().size() ? Status::success
                                                   : Status::reached_end;
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
  [[nodiscard]] Status move_up_right()
  {
    while (!stack().empty() && index() >= node()->num_children()) {
      stack().pop_back(); // Move up
      if (stack().empty()) {
        return Status::reached_end;
      }

      move_right_dir(); // Move to the next child of this node
    }

    return Status::success;
  }

  /// Move down/left until we reach a potentially matching leaf
  [[nodiscard]] Status move_down_left()
  {
    while (node()->child_type() == NodeType::directory) {
      auto* const      dir         = node()->dir_children()[index()].node.get();
      const ChildIndex first_index = leftmost_child(*dir, _predicate);

      stack().emplace_back(Frame{dir, first_index});

      if (first_index >= dir->num_children()) {
        // No matches in this directory node, skip to the next
        if (move_up_right() == Status::reached_end) {
          return Status::reached_end;
        }
      }
    }

    return Status::success;
  }

  Status increment()
  {
    if (move_right_leaf() == Status::success) {
      return Status::success; // Moved to next leaf child
    }

    if (move_up_right() == Status::reached_end) {
      return Status::reached_end; // Reached end of tree
    }

    // Now at a matching directory, and a matching child of that directory
    assert((node()->child_type() == NodeType::directory &&
            _predicate.directory(node()->dir_children()[index()].key)) ||
           (_predicate.leaf(entry_key(node()->dat_children()[index()]))));

    return move_down_left();
  }

  Predicate _predicate;
};

} // namespace spaix

#endif // SPAIX_ITERATOR_HPP
