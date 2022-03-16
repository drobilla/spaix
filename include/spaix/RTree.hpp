// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_RTREE_HPP
#define SPAIX_RTREE_HPP

#include "spaix/DataNode.hpp"
#include "spaix/Iterator.hpp"
#include "spaix/StaticVector.hpp"         // IWYU pragma: export
#include "spaix/detail/DirectoryNode.hpp" // IWYU pragma: export
#include "spaix/sizes.hpp"
#include "spaix/traverse/everything.hpp"
#include "spaix/types.hpp"
#include "spaix/union.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility> // IWYU pragma: keep
#include <vector>

// IWYU pragma: no_include <algorithm>

namespace spaix {

using NodePath = std::vector<ChildIndex>;

enum class VisitStatus { proceed, finish };

/**
   An R-tree which spatially indexes points or rectangles.

   @tparam K Geometric key type for elements.
   @tparam D Data type for elements.
   @tparam C Tree configuration (an instantiation of spaix::Configuration).
*/
template<class K, class D, class C>
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
  static constexpr auto min_dir_fanout = Config::dir_fanout / min_fill_divisor;
  static constexpr auto min_dat_fanout = Config::dat_fanout / min_fill_divisor;

  static_assert(Config::dir_fanout > 1);
  static_assert(Config::dat_fanout > 1);
  static_assert(min_dir_fanout > 1);
  static_assert(min_dat_fanout > 1);

  using DatNode = DataNode<Key, Data>;
  using DirNode = DirectoryNode<Box,
                                DatNode,
                                Config::placement,
                                Config::dir_fanout,
                                Config::dat_fanout>;

  using DatNodePtr  = std::unique_ptr<DatNode>;
  using DirNodePtr  = std::unique_ptr<DirNode>;
  using DirEntry    = typename DirNode::DirEntry;
  using DatEntry    = typename DirNode::DatEntry;
  using DirNodePair = std::array<DirEntry, 2>;

  template<class Predicate>
  using Iter =
    Iterator<Predicate, DirNode, DatNode, max_height<DatNode>(min_dir_fanout)>;

  template<class Predicate>
  using ConstIter = Iterator<Predicate,
                             const DirNode,
                             const DatNode,
                             max_height<DatNode>(min_dir_fanout)>;

  /**
     A range in an RTree that matches a predicate.

     This acts like a collection of nodes that match a query predicate,
     particularly for use with range-based for loops.  Note, however, that the
     nodes are not actually contiguous internally as the underlying iterators
     automatically skip nodes that do not match.
  */
  template<class Predicate>
  struct Range {
    Range(ConstIter<Predicate> first, ConstIter<Predicate> last)
      : _first{std::move(first)}
      , _last{std::move(last)}
    {}

    ConstIter<Predicate>& begin() { return _first; }
    ConstIter<Predicate>& end() { return _last; }

    bool empty() const { return _first == _last; }

  private:
    ConstIter<Predicate> _first;
    ConstIter<Predicate> _last;
  };

  using Everything = traverse::Everything;

  using iterator       = Iter<Everything>;
  using const_iterator = ConstIter<Everything>;

  RTree() = default;

  RTree(Insertion insertion, Split split)
    : _insertion{std::move(insertion)}
    , _split{std::move(split)}
  {}

  const_iterator begin() const { return const_iterator{_root, {}}; }
  const_iterator end() const { return const_iterator{{Box{}, nullptr}, {}}; }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }
  iterator       begin() { return empty() ? end() : iterator{_root, {}}; }
  iterator       end() { return iterator{{Box{}, nullptr}, {}}; }

  template<class Predicate>
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

  template<class Predicate, class Visitor>
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
  static constexpr ChildCount internal_fanout() { return Config::dir_fanout; }

  /// Return the maximum number of children of a leaf node
  static constexpr ChildCount leaf_fanout() { return Config::dat_fanout; }

  /// Return an upper bound on the maximum number of elements in a tree
  static constexpr size_t max_size()
  {
    return std::numeric_limits<size_t>::max() / sizeof(DatNode);
  }

  /// Return the maximum height of a tree
  static constexpr size_t max_height()
  {
    return spaix::max_height<DatNode>(min_dir_fanout);
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

  /**
     Visit every node in the tree.

     The traversal is terminated immediately if any visitor returns
     VisitStatus::finish.

     @param visit_dir Function called for every internal directory node, with
     prototype VisitStatus(const NodePath&, const Box&, ChildCount)>.

     @param visit_dat Function called for every internal directory node, with
     prototype VisitStatus(const NodePath&, const Key&, const Data&)>.
  */
  template<typename DirVisitor, typename DatVisitor>
  void visit(DirVisitor&& visit_dir, DatVisitor&& visit_dat) const;

  template<typename DirVisitor>
  void visit(DirVisitor&& visit_dir) const;

private:
  template<class Children>
  static Box parent_key(const Children& children);

  static Box ideal_key(const DirNode& node);

  DirNodePair insert_rec(DirEntry&   parent_entry,
                         const Box&  new_parent_key,
                         const Key&  key,
                         const Data& data);

  template<class Predicate, class Visitor>
  void fast_query_rec(const DirNode&   node,
                      const Predicate& predicate,
                      const Visitor&   visitor) const
  {
    switch (node.child_type()) {
    case NodeType::directory:
      for (const auto& entry : node.dir_children()) {
        if (predicate.directory(entry.key)) {
          fast_query_rec(*entry.node, predicate, visitor);
        }
      }
      break;

    case NodeType::data:
      for (const auto& entry : node.dat_children()) {
        if (predicate.leaf(entry_key(entry))) {
          visitor(entry_ref(entry));
        }
      }
    }
  }

  /// Create a new parent seeded with a child
  template<class Entry, ChildCount count>
  static DirEntry new_parent(StaticVector<Entry, ChildCount, count>& deposit,
                             ChildIndex                              index,
                             NodeType child_type);

  /// Split `nodes` plus `node` in two and return the resulting sides
  template<class Entry, ChildCount fanout>
  DirNodePair split(StaticVector<Entry, ChildCount, fanout>& nodes,
                    Entry                                    entry,
                    const Box&                               bounds,
                    NodeType                                 type);

  Insertion _insertion{};          ///< Insertion algorithm
  Split     _split{};              ///< Split algorithm
  size_t    _size{};               ///< Number of elements
  DirEntry  _root{Box{}, nullptr}; ///< Key and pointer to root node
};

} // namespace spaix

#include "spaix/RTree.ipp" // IWYU pragma: export

#endif // SPAIX_RTREE_HPP
