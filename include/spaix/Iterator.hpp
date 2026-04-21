// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_ITERATOR_HPP
#define SPAIX_ITERATOR_HPP

#include <spaix/DataIterator.hpp>
#include <spaix/detail/entry.hpp>
#include <spaix/types.hpp>

#include <cassert>
#include <cstdint>
#include <iterator>
#include <utility>

namespace spaix {

/**
   An iterator for an RTree.

   This iterates over data elements (key/value pairs) in the tree that match a
   given predicate.
*/
template<class Predicate, class DirNode, class DatNode, unsigned max_height>
class Iterator : public DataIterator<DirNode, DatNode, max_height>
{
public:
  using Base       = DataIterator<DirNode, DatNode, max_height>;
  using DirEntry   = typename DirNode::DirEntry;
  using ChildIndex = typename Base::ChildIndex;

  using iterator_category = std::forward_iterator_tag;
  using difference_type   = intptr_t;

  Iterator(const DataIterator<DirNode, DatNode, max_height>& base,
           Predicate                                         predicate) noexcept
    : Base{base}
    , _predicate{predicate}
  {}

  Iterator(const DirEntry& root_entry, Predicate predicate) noexcept
    : Base{}
    , _predicate{std::move(predicate)}
  {
    const auto& root = root_entry.node;
    if (root && _predicate.directory(root_entry.key)) {
      const ChildIndex root_child_index = leftmost_child(*root, _predicate);
      if (root_child_index < root->num_children()) {
        this->step_down(root.get(), root_child_index);
        if (move_down_left() == Status::reached_end) {
          this->clear();
        }
      }
    }

    assert(this->empty() ||
           _predicate.leaf(detail::entry_key(this->operator*())));
  }

  Iterator& operator++() noexcept
  {
    if (increment() == Status::reached_end) {
      assert(this->empty());
    }

    return *this;
  }

private:
  using Base::empty;
  using Base::index;
  using Base::parent;

  enum class Status : unsigned char {
    success,
    reached_end,
  };

  /**
     Return the index of the leftmost child of `dir` that matches `predicate`.

     Returns the end index (on past the last child) if no child matches.
  */
  static ChildIndex leftmost_child(const DirNode&   dir,
                                   const Predicate& predicate) noexcept
  {
    if (dir.child_type() == NodeType::directory) {
      for (ChildIndex i = 0U; i < dir.num_children(); ++i) {
        if (predicate.directory(dir.dir_children()[i].key)) {
          return i;
        }
      }
      return dir.num_children();
    }

    for (ChildIndex i = 0U; i < dir.dat_children().size(); ++i) {
      if (predicate.leaf(detail::entry_key(dir.dat_children()[i]))) {
        return i;
      }
    }
    return dir.dat_children().size();
  }

  /// Move right until we reach a good leaf or the end of the parent
  [[nodiscard]] Status move_right_leaf() noexcept
  {
    assert(parent()->child_type() == NodeType::data);
    do {
      Base::step_right();
    } while (
      index() < parent()->num_children() &&
      !_predicate.leaf(detail::entry_key(parent()->dat_children()[index()])));

    return index() < parent()->num_children() ? Status::success
                                              : Status::reached_end;
  }

  /// Move right until we reach a good directory or the end of the parent
  void move_right_dir() noexcept
  {
    assert(parent()->child_type() == NodeType::directory);
    do {
      Base::step_right();
    } while (index() < parent()->num_children() &&
             !_predicate.directory(parent()->dir_children()[index()].key));
  }

  /// Move up/right until we reach a node we are not at the end of yet
  [[nodiscard]] Status move_up_right() noexcept
  {
    assert(!empty());

    while (index() >= parent()->num_children()) {
      Base::step_up(); // Step up to parent
      if (empty()) {
        return Status::reached_end;
      }

      move_right_dir(); // Move to the next child of this node
    }

    return Status::success;
  }

  /// Move down/left until we reach a potentially matching leaf
  [[nodiscard]] Status move_down_left() noexcept
  {
    while (parent()->child_type() == NodeType::directory) {
      auto* const      dir = parent()->dir_children()[index()].node.get();
      const ChildIndex first_index = leftmost_child(*dir, _predicate);

      Base::step_down(dir, first_index);

      if (first_index >= dir->num_children()) {
        // No matches in this directory node, skip to the next
        if (move_up_right() == Status::reached_end) {
          return Status::reached_end;
        }
      }
    }

    return Status::success;
  }

  Status increment() noexcept
  {
    if (move_right_leaf() == Status::success) {
      return Status::success; // Moved to next leaf child
    }

    if (move_up_right() == Status::reached_end) {
      return Status::reached_end; // Reached end of tree
    }

    // Now at a matching directory, and a matching child of that directory
    assert(
      (parent()->child_type() == NodeType::directory &&
       _predicate.directory(parent()->dir_children()[index()].key)) ||
      (_predicate.leaf(detail::entry_key(parent()->dat_children()[index()]))));

    return move_down_left();
  }

  Predicate _predicate;
};

} // namespace spaix

#endif // SPAIX_ITERATOR_HPP
