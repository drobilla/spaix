// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_RTREE_HPP
#define SPAIX_RTREE_HPP

#include "spaix/DataIterator.hpp"
#include "spaix/DataNode.hpp"
#include "spaix/Iterator.hpp"
#include "spaix/StaticVector.hpp" // IWYU pragma: export
#include "spaix/TreeRange.hpp"
#include "spaix/detail/DirectoryNode.hpp" // IWYU pragma: export
#include "spaix/search/everything.hpp"    // IWYU pragma: keep
#include "spaix/sizes.hpp"
#include "spaix/types.hpp"
#include "spaix/union.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <utility> // IWYU pragma: keep

namespace spaix {

enum class VisitStatus { proceed, finish };

/**
   An R-tree which spatially indexes points or rectangles.

   @tparam K Geometric key type for elements.
   @tparam D Data type for elements.
   @tparam C Tree configuration (an instantiation of spaix::Config).
*/
template<class K, class D, class C>
class RTree
{
public:
  using Key  = K;            ///< Key for a single data element
  using Data = D;            ///< Single data element
  using Box  = UnionOf<Key>; ///< Aggregate key in a directory node

  using Conf      = C;                        ///< Tree configuration
  using Structure = typename Conf::Structure; ///< Tree structure configuration
  using Insertion = typename Conf::Insertion; ///< Insertion algorithm
  using Split     = typename Conf::Split;     ///< Split algorithm

  using DatNode = DataNode<Key, Data>;                    ///< Leaf node
  using DirNode = DirectoryNode<Box, DatNode, Structure>; ///< Internal node

  /// Return the maximum height of a tree
  static constexpr size_t max_height() noexcept
  {
    return max_tree_height<DirNode, DatNode, Conf::placement>(
      Conf::min_dir_fanout);
  }

  /// Return an upper bound on the maximum number of elements in a tree
  static constexpr size_t max_size() noexcept
  {
    return Conf::dat_fanout * power(Conf::dir_fanout, max_height() - 1);
  }

  /// An iterator over a search area
  template<class S>
  using Searcher = Iterator<S, const DirNode, DatNode, max_height()>;

  /// A constant iterator over a search area
  template<class S>
  using ConstSearcher = Iterator<S, const DirNode, const DatNode, max_height()>;

  // STL Container member types
  using iterator       = Searcher<search::Everything>;
  using const_iterator = ConstSearcher<search::Everything>;

  // STL AssociativeContainer member types
  using key_type    = K;
  using mapped_type = D;
  using value_type  = DataNode<K, D>;
  using node_type   = DatNode;

  using NodePath = StaticVector<ChildIndex, ChildCount, max_height()>;

  RTree() = default;

  RTree(Insertion insertion, Split split)
    : _insertion{std::move(insertion)}
    , _split{std::move(split)}
  {}

  RTree(const RTree&)            = delete;
  RTree& operator=(const RTree&) = delete;

  RTree(RTree&&) noexcept            = default;
  RTree& operator=(RTree&&) noexcept = default;

  ~RTree() = default;

  /// Insert a new item with the given `key` and `data`
  void insert(const Key& key, const Data& data);

  /**
     Return a range over all items covered by the given search.

     This returns a facade of a collection that is suitable for use with
     range-based for loops, for example, everything in the tree can be queried
     like:

     @code{.cpp}
     for (const auto& data_node : tree.query(spaix::search::everything())) {
       std::cout << "Key:  " << data_node.key << std::endl;
       std::cout << "Data: " << data_node.data << std::endl;
     }
     @endcode
  */
  template<class S>
  TreeRange<ConstSearcher<S>> query(S search) const;

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

  /// Return a key that encompasses all items in the tree
  Box bounds() const { return _root.node ? _root.key : Box{}; }

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

  using EndIterator = DataIterator<const DirNode, const DatNode, max_height()>;

  const_iterator begin() const { return const_iterator{_root, {}}; }
  EndIterator    end() const { return EndIterator::make_end(); }
  const_iterator cbegin() const { return begin(); }
  EndIterator    cend() const { return EndIterator::make_end(); }
  iterator       begin() { return iterator{_root, {}}; }

private:
  using DatNodePtr  = std::unique_ptr<DatNode>;
  using DirNodePtr  = std::unique_ptr<DirNode>;
  using DirEntry    = typename DirNode::DirEntry;
  using DatEntry    = typename DirNode::DatEntry;
  using DirNodePair = std::array<DirEntry, 2>;

  static constexpr auto placement      = Conf::placement;
  static constexpr auto dir_fanout     = Conf::dir_fanout;
  static constexpr auto dat_fanout     = Conf::dat_fanout;
  static constexpr auto min_dir_fanout = dir_fanout / Conf::min_fill_divisor;
  static constexpr auto min_dat_fanout = dat_fanout / Conf::min_fill_divisor;

  static_assert(min_dir_fanout > 1);
  static_assert(min_dat_fanout > 1);

  static_assert(sizeof(DirNode) <= Structure::max_dir_node_size);
  static_assert(sizeof(DirNode) >= Structure::min_dir_node_size);

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
                      const Visitor&   visitor) const;

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
