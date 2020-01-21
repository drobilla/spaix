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
#include "spaix/Iterator.hpp"
#include "spaix/LinearInsertion.hpp"
#include "spaix/Point.hpp"
#include "spaix/QuadraticSplit.hpp"
#include "spaix/Rect.hpp"
#include "spaix/contains.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/everything.hpp"
#include "spaix/traversal.hpp"
#include "spaix/types.hpp"
#include "spaix/union.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef NDEBUG
#  include "spaix/contains.hpp"
#endif

namespace spaix {

using NodePath = std::vector<ChildIndex>;

struct Everything;

/// Return a fanout so that directory nodes fit within `page_size` bytes
template <class DirKey>
constexpr ChildCount
fanout(const size_t page_size = 128u)
{
  return static_cast<ChildCount>(
      (page_size - sizeof(NodeType) - sizeof(ChildCount)) /
      (sizeof(DirKey) + sizeof(std::unique_ptr<void*>)));
}

/// Return log_2(n)
template <class T>
constexpr T
log_2(const T n)
{
  return (n < 2) ? 1 : 1 + log_2(n / 2);
}

/// Return log_b(n)
template <class T>
constexpr T
log_b(const T n, const T b)
{
  return log_2(n) / log_2(b);
}

/// Return an upper bound on the maximum number of elements in a tree
static constexpr size_t
max_size(const size_t dat_size)
{
  return std::numeric_limits<size_t>::max() / dat_size;
}

/// Return the maximum height of a tree
static constexpr size_t
max_height(const size_t dat_size, const ChildCount min_fanout)
{
  return log_b(max_size(dat_size), size_t{min_fanout});
}

template <class Key>
struct RectFor
{
};

template <class... Values>
struct RectFor<Point<Values...>>
{
  using type = Rect<Values...>;
};

template <class... Values>
struct RectFor<Rect<Values...>>
{
  using type = Rect<Values...>;
};

/**
   An R-tree which spatially indexes points or rectangles.

   @tparam LeafKey The key type used for data items stored in tree leaves.
   @tparam LeafData The type of values stored in the tree.

*/
template <class LeafKey,
          class LeafData,
          class DirectoryKey        = typename RectFor<LeafKey>::type,
          ChildCount Fanout         = fanout<DirectoryKey>(),
          unsigned   MinFillDivisor = 4,
          class Insertion           = LinearInsertion,
          class Split               = QuadraticSplit>
class RTree
{
public:
  static_assert(Fanout > 1, "");

  using Data   = LeafData;
  using DirKey = DirectoryKey;
  using Key    = LeafKey;

  using DatNode = DataNode<Key, Data>;
  using DirNode = DirectoryNode<DirKey, DatNode, Fanout>;

  using DatNodePtr = std::unique_ptr<DatNode>;
  using DirNodePtr = std::unique_ptr<DirNode>;
  using DirEntry   = typename DirNode::DirEntry;

  using DirNodePair = std::array<DirEntry, 2>;
  using Frame       = StackFrame<DirNode>;

  static_assert(
      sizeof(DirectoryNode<DirKey, DatNode, fanout<DirectoryKey>(512)>) <=
              512 &&
          sizeof(DirectoryNode<DirKey, DatNode, fanout<DirectoryKey>(512)>) >
              512 - sizeof(DirKey) - sizeof(Data),
      "");

  static_assert(
      sizeof(DirectoryNode<DirKey, DatNode, fanout<DirectoryKey>(4096)>) <=
              4096 &&
          sizeof(DirectoryNode<DirKey, DatNode, fanout<DirectoryKey>(4096)>) >
              4096 - sizeof(DirKey) - sizeof(Data),
      "");

  static constexpr auto min_fanout = Fanout / MinFillDivisor;

  RTree() {}

  ~RTree() = default;

  RTree(const RTree&)            = delete;
  RTree& operator=(const RTree&) = delete;

  RTree(RTree&&) noexcept            = default;
  RTree& operator=(RTree&&) noexcept = default;

  template <class Predicate>
  using Iter = Iterator<Predicate,
                        DirNode,
                        DatNode,
                        max_height(sizeof(DatNode), min_fanout)>;

  template <class Predicate>
  using ConstIter = Iterator<Predicate,
                             DirNode,
                             const DatNode,
                             max_height(sizeof(DatNode), min_fanout)>;

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
    return const_iterator{{DirKey{}, nullptr}, Everything{}};
  }

  iterator begin() { return empty() ? end() : iterator{_root, Everything{}}; }

  iterator end() { return iterator{{DirKey{}, nullptr}, Everything{}}; }

  template <class Predicate>
  Range<Predicate> query(Predicate predicate) const
  {
    if (empty()) {
      return {{{DirKey{}, nullptr}, predicate},
              {{DirKey{}, nullptr}, predicate}};
    }

    ConstIter<Predicate> first{_root, predicate};
    ConstIter<Predicate> last{{DirKey{}, nullptr}, predicate};
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

  template <class Predicate, class Visitor>
  void fast_query_rec(const DirNode&   node,
                      const Predicate& predicate,
                      const Visitor&   visitor) const
  {
    switch (node.child_type) {
    case NodeType::DIR:
      for (ChildIndex i = 0u; i < node.num_children(); ++i) {
        if (predicate.directory(node.dir_children[i].key)) {
          fast_query_rec(*node.dir_children[i].node, predicate, visitor);
        }
      }
      break;

    case NodeType::DAT:
      for (ChildIndex i = 0u; i < node.num_children(); ++i) {
        if (predicate.leaf(node.dat_children[i].key)) {
          visitor(node.dat_children[i]);
        }
      }
    }
  }

  /// Remove all items from the tree
  void clear() { _root = {DirKey{}, nullptr}; }

  /// Return the number of items in the tree
  size_t size() const { return _size; }

  /// Return true iff there are no items in the tree
  bool empty() const { return !_root.node; }

  /// Return the maximum number of children of a node
  static constexpr ChildCount fanout() { return Fanout; }

  /// Return an upper bound on the maximum number of elements in a tree
  static constexpr size_t max_size()
  {
    return std::numeric_limits<size_t>::max() / sizeof(DatNode);
  }

  /// Return the maximum height of a tree
  static constexpr size_t max_height()
  {
    return spaix::max_height(sizeof(DatNode), min_fanout);
  }

  /// Return a key that encompasses all items in the tree
  DirKey bounds() const { return _root.node ? _root.key : DirKey{}; }

  /// Insert a new item at `key` with `data`
  void insert(const Key& key, const Data& data)
  {
    if (empty()) {
      _root = {DirKey{key}, DirNodePtr{new DirNode{NodeType::DAT}}};
    }

    auto sides = insert_rec(_root, key, data);
    if (sides[0].node) {
      _root = {sides[0].key | sides[1].key,
               DirNodePtr{new DirNode{NodeType::DIR}}};

      _root.node->append_child(std::move(sides[0]));
      _root.node->append_child(std::move(sides[1]));
      assert(_root.key == ideal_key(*_root.node));
    } else {
      _root.key = _root.key | key;
    }

    ++_size;
  }

  using DirVisitor = std::function<
      bool(const DirKey& key, const NodePath& path, ChildCount n_children)>;

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
  static DirKey parent_key(const Children& children)
  {
    DirKey key;

    for (const auto& entry : children) {
      key = key | entry_key(entry);
    }

    return key;
  }

  static DirKey ideal_key(const DirNode& node)
  {
    if (node.child_type == NodeType::DIR) {
      return parent_key(node.dir_children);
    } else {
      return parent_key(node.dat_children);
    }
  }

  void replace_key(DirEntry& entry, const DirKey& new_key)
  {
    if (new_key != entry.key) {
      entry.key = new_key;
    }
  }

  DirNodePair
  insert_rec(DirEntry& parent_entry, const Key& key, const Data& data)
  {
    auto& parent = *parent_entry.node;
    if (parent.child_type == NodeType::DIR) { // Recursing downwards
      const auto choice   = Insertion::choose(parent.dir_children, key);
      const auto index    = choice.first;
      const auto expanded = choice.second;
      auto&      entry    = parent.dir_children[index];

      entry.key = expanded;

      auto sides = insert_rec(entry, key, data);

      if (sides[0].node) { // Child was split, replace it
        parent.dir_children[index] = std::move(sides[0]);
        if (parent.num_children() == Fanout) {
          return split(parent.dir_children,
                       std::move(sides[1]),
                       parent_entry.key | key,
                       parent.child_type);
        }

        parent.append_child(std::move(sides[1]));
        replace_key(parent_entry, parent_key(parent.dir_children));
      } else {
        parent_entry.key = parent_entry.key | key;
        assert(parent_entry.key == ideal_key(parent));
      }

    } else if (parent.num_children() < Fanout) { // Simple leaf insert
      parent.append_child(DatNode{key, data});
      parent_entry.key = parent_entry.key | key;
      assert(parent_entry.key == ideal_key(parent));

    } else { // Split leaf insert
      return split(parent.dat_children,
                   DatNode{key, data},
                   parent_entry.key | key,
                   parent.child_type);
    }

    return {DirEntry{DirKey{}, nullptr}, DirEntry{DirKey{}, nullptr}};
  }

  /// Create a new parent seeded with a child
  template <class Entry>
  static DirEntry
  new_parent(StaticVector<Entry, ChildCount, Fanout + 1>& deposit,
             ChildIndex                                   index,
             NodeType                                     child_type)
  {
    if (index != deposit.size() - 1) {
      std::iter_swap(deposit.begin() + index,
                     deposit.begin() + deposit.size() - 1);
    }

    DirKey     key{entry_key(deposit.back())};
    DirNodePtr node{new DirNode(child_type)};
    node->append_child(std::move(deposit.back()));
    deposit.pop_back();
    return {key, std::move(node)};
  }

  /// Split `nodes` plus `node` in two and return the resulting sides
  template <class Entry>
  static DirNodePair split(StaticVector<Entry, ChildCount, Fanout>& nodes,
                           Entry                                    entry,
                           const DirKey&                            bounds,
                           const NodeType                           type)
  {
    // assert(node);
    // assert(std::find(nodes.begin(), nodes.end(), nullptr) == nodes.end());

    // Make an array of all nodes to deposit
    StaticVector<Entry, ChildCount, Fanout + 1> deposit;
    for (auto&& e : nodes) {
      deposit.emplace_back(std::move(e));
    }
    deposit.emplace_back(std::move(entry));
    // std::array<Entry, Fanout + 1> deposit;
    // std::move(nodes.begin(), nodes.end(), deposit.begin());
    // deposit[nodes.size()] = std::move(entry);
    if (bounds != parent_key(deposit)) {
      std::cerr << "bounds: " << bounds << std::endl;
      std::cerr << "ideal:  " << parent_key(deposit) << std::endl;
    }
    // FIXME
    assert(bounds == parent_key(deposit));

    // Pick two nodes to seed the left and right groups
    const auto seeds = Split::pick_seeds(deposit, bounds);
    assert(seeds.first < seeds.second);

    // Create left and right parent nodes with seeds
    DirNodePair sides{new_parent(deposit, seeds.second, type),
                      new_parent(deposit, seeds.first, type)};

    // Distribute remaining nodes between seeds
    Split::distribute_children(
        deposit, sides[0], sides[1], Fanout - min_fanout);
    assert(sides[0].node->num_children() + sides[1].node->num_children() ==
           Fanout + 1);
    sides[0].key = ideal_key(*sides[0].node);
    sides[1].key = ideal_key(*sides[1].node);

    // fprintf(
    //     stderr, "Balance: %u %u\n", sides[0]->num_children(),
    //     sides[1]->num_children());
    // fprintf(
    //     stderr, "Overlap: %f\n", double(volume(sides[0]->key &
    //     sides[1]->key)));
    return sides;
  }

  void visit_structure_rec(const DirEntry& entry,
                           DirVisitor      visit_dir,
                           DatVisitor      visit_dat,
                           NodePath&       path) const
  {
    const auto& node = *entry.node;
    if (visit_dir(entry.key, path, node.num_children())) {
      for (ChildIndex i = 0u; i < node.num_children(); ++i) {
        path.push_back(i);

        if (node.child_type == NodeType::DAT) {
          const auto& child = node.dat_children[i];
          visit_dat(child.key, child.data, path);
        } else {
          const auto& child = node.dir_children[i];
          visit_structure_rec(child, visit_dir, visit_dat, path);
        }

        path.pop_back();
      }
    }
  }

  size_t   _size{};                  ///< Number of elements
  DirEntry _root{DirKey{}, nullptr}; ///< Key and pointer to root node
};

} // namespace spaix

#endif // SPAIX_RTREE_HPP
