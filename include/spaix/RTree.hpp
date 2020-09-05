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

#ifndef SPAIX_RTREE_HPP
#define SPAIX_RTREE_HPP

#include "spaix/DataNode.hpp"
#include "spaix/DataPlacement.hpp"
#include "spaix/Iterator.hpp"
#include "spaix/LinearInsertion.hpp"
#include "spaix/Point.hpp"
#include "spaix/QuadraticSplit.hpp"
#include "spaix/Rect.hpp"
#include "spaix/contains.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/everything.hpp"
#include "spaix/sizes.hpp"
#include "spaix/types.hpp"
#include "spaix/union.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace spaix {

using NodePath = std::vector<ChildIndex>;

template <class K>
using UnionOf = decltype(std::declval<K>() | std::declval<K>());

/**
   An R-tree which spatially indexes points or rectangles.

   @tparam K Geometric key type for elements.
   @tparam D Data type for elements.
   @tparam C Tree configuration (an instantiation of spaix::Configuration).
*/
template <class K, class D, class C>
class RTree
{
public:
  using Key       = K;
  using Data      = D;
  using Box       = UnionOf<Key>;
  using Config    = C;
  using Insertion = typename Config::Insertion;
  using Split     = typename Config::Split;

  static constexpr auto min_fill_divisor = Config::min_fill_divisor;
  static constexpr auto placement        = Config::placement;
  static constexpr auto dir_fanout       = Config::dir_fanout;
  static constexpr auto dat_fanout       = Config::dat_fanout;
  static constexpr auto min_dir_fanout   = dir_fanout / min_fill_divisor;
  static constexpr auto min_dat_fanout   = dat_fanout / min_fill_divisor;

  static_assert(dir_fanout > 1, "");
  static_assert(dat_fanout > 1, "");
  static_assert(min_dir_fanout > 1, "");
  static_assert(min_dat_fanout > 1, "");

  using DatNode = DataNode<Key, Data>;
  using DirNode =
      DirectoryNode<Box, DatNode, placement, dir_fanout, dat_fanout>;

  using DatNodePtr  = std::unique_ptr<DatNode>;
  using DirNodePtr  = std::unique_ptr<DirNode>;
  using DirEntry    = typename DirNode::DirEntry;
  using DatEntry    = typename DirNode::DatEntry;
  using DirNodePair = std::array<DirEntry, 2>;

  template <class Predicate>
  using Iter = Iterator<Predicate,
                        DirNode,
                        DatNode,
                        max_height(sizeof(DatNode), min_dir_fanout)>;

  template <class Predicate>
  using ConstIter = Iterator<Predicate,
                             const DirNode,
                             const DatNode,
                             max_height(sizeof(DatNode), min_dir_fanout)>;

  /**
     A range in an RTree that matches a predicate.

     This acts like a collection of nodes that match a query predicate,
     particularly for use with range-based for loops.  Note, however, that the
     nodes are not actually contiguous internally as the underlying iterators
     automatically skip nodes that do not match.
  */
  template <class Predicate>
  struct Range
  {
    ConstIter<Predicate>& begin() { return first; }
    ConstIter<Predicate>& end() { return last; }

    bool empty() const { return first == last; }

    ConstIter<Predicate> first;
    ConstIter<Predicate> last;
  };

  using iterator       = Iter<Everything>;
  using const_iterator = ConstIter<Everything>;

  const_iterator begin() const { return const_iterator{_root, Everything{}}; }

  const_iterator end() const
  {
    return const_iterator{{Box{}, nullptr}, Everything{}};
  }

  iterator begin() { return empty() ? end() : iterator{_root, Everything{}}; }

  iterator end() { return iterator{{Box{}, nullptr}, Everything{}}; }

  template <class Predicate>
  Range<Predicate> query(Predicate predicate) const
  {
    if (empty()) {
      return {{{Box{}, nullptr}, predicate}, {{Box{}, nullptr}, predicate}};
    }

    ConstIter<Predicate> first{_root, predicate};
    ConstIter<Predicate> last{{Box{}, nullptr}, predicate};
    if (first != last && !predicate.leaf(first->key)) {
      ++first;
    }

    return {std::move(first), std::move(last)};
  }

  template <class Predicate, class Visitor>
  void fast_query(const Predicate& predicate, const Visitor& visitor) const
  {
    if (_root.node && predicate.directory(_root.key)) {
      fast_query_rec(*_root.node, predicate, visitor);
    }
  }

  /// Remove all items from the tree
  void clear() { _root = {Box{}, nullptr}; }

  /// Return the number of items in the tree
  size_t size() const { return _size; }

  /// Return true iff there are no items in the tree
  bool empty() const { return !_root.node; }

  /// Return the maximum number of children of an internal node
  static constexpr ChildCount internal_fanout() { return dir_fanout; }

  /// Return the maximum number of children of a leaf node
  static constexpr ChildCount leaf_fanout() { return dat_fanout; }

  /// Return an upper bound on the maximum number of elements in a tree
  static constexpr size_t max_size()
  {
    return std::numeric_limits<size_t>::max() / sizeof(DatNode);
  }

  /// Return the maximum height of a tree
  static constexpr size_t max_height()
  {
    return spaix::max_height(sizeof(DatNode), min_dir_fanout);
  }

  /// Return a key that encompasses all items in the tree
  Box bounds() const { return _root.node ? _root.key : Box{}; }

  /// Insert a new item at `key` with `data`
  void insert(const Key& key, const Data& data)
  {
    if (empty()) {
      _root = {Box{key}, std::make_unique<DirNode>(NodeType::data)};
    }

    auto sides = insert_rec(_root, _root.key | key, key, data);
    if (sides[0].node) {
      _root = {sides[0].key | sides[1].key,
               std::make_unique<DirNode>(NodeType::directory)};

      _root.node->append_child(std::move(sides[0]));
      _root.node->append_child(std::move(sides[1]));
      assert(_root.key == ideal_key(*_root.node));
    }

    ++_size;
  }

  using DirVisitor = std::function<
      bool(const Box& key, const NodePath& path, ChildCount n_children)>;

  using DatVisitor = std::function<
      void(const Key& key, const Data& data, const NodePath& path)>;

  void visit_structure(
      DirVisitor visit_dir,
      DatVisitor visit_dat = [](const Key&, const Data&, const NodePath&) {
      }) const
  {
    NodePath path{0};
    visit_structure_rec(
        _root, std::move(visit_dir), std::move(visit_dat), path);
  }

private:
  template <class Children>
  static Box parent_key(const Children& children);

  static Box ideal_key(const DirNode& node);

  static DirNodePair insert_rec(DirEntry&   parent_entry,
                                const Box&  new_parent_key,
                                const Key&  key,
                                const Data& data);

  template <class Predicate, class Visitor>
  void fast_query_rec(const DirNode&   node,
                      const Predicate& predicate,
                      const Visitor&   visitor) const
  {
    switch (node.child_type) {
    case NodeType::directory:
      for (const auto& entry : node.dir_children) {
        if (predicate.directory(entry.key)) {
          fast_query_rec(*entry.node, predicate, visitor);
        }
      }
      break;

    case NodeType::data:
      for (const auto& entry : node.dat_children) {
        if (predicate.leaf(entry_key(entry))) {
          visitor(entry_ref(entry));
        }
      }
    }
  }

  /// Create a new parent seeded with a child
  template <class Entry, ChildCount count>
  static DirEntry new_parent(StaticVector<Entry, ChildCount, count>& deposit,
                             ChildIndex                              index,
                             NodeType child_type);

  /// Split `nodes` plus `node` in two and return the resulting sides
  template <class Entry, ChildCount fanout>
  static DirNodePair split(StaticVector<Entry, ChildCount, fanout>& nodes,
                           Entry                                    entry,
                           const Box&                               bounds,
                           NodeType                                 type);

  static void visit_structure_rec(const DirEntry& entry,
                                  DirVisitor      visit_dir,
                                  DatVisitor      visit_dat,
                                  NodePath&       path);

  size_t   _size{};               ///< Number of elements
  DirEntry _root{Box{}, nullptr}; ///< Key and pointer to root node
};

} // namespace spaix

#include "spaix/RTree.ipp"

#endif // SPAIX_RTREE_HPP
