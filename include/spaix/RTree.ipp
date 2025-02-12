// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_RTREE_IPP
#define SPAIX_RTREE_IPP

#include <spaix/StaticVector.hpp>
#include <spaix/TreeRange.hpp>
#include <spaix/detail/DirectoryNode.hpp>
#include <spaix/types.hpp>
#include <spaix/union.hpp>

#include <algorithm>
#include <cassert>
#include <memory>
#include <numeric>
#include <utility>

namespace spaix {

template<class B, class K, class D, class C>
RTree<B, K, D, C>::RTree(Insertion insertion, Split split)
  : _insertion{std::move(insertion)}
  , _split{std::move(split)}
{}

template<class B, class K, class D, class C>
void
RTree<B, K, D, C>::insert(const Key& key, const Data& data)
{
  if (empty()) {
    _root = {Box{key}, std::make_unique<DirNode>(NodeType::data)};
  }

  auto sides = insert_rec(_root, _root.key | key, key, data);
  if (sides[0].node) {
    const auto root_key  = sides[0].key | sides[1].key;
    auto       root_node = std::make_unique<DirNode>(NodeType::directory);

    root_node->append_child(std::move(sides[0]));
    root_node->append_child(std::move(sides[1]));

    assert(root_key == ideal_key(*root_node));
    _root = {root_key, std::move(root_node)};
  }

  ++_size;
}

template<class B, class K, class D, class C>
template<class S>
auto
RTree<B, K, D, C>::query(S search) const -> TreeRange<ConstSearcher<S>>
{
  if (empty()) {
    return {{{Box{}, nullptr}, search}, {{Box{}, nullptr}, search}};
  }

  ConstSearcher<S> first{_root, search};
  ConstSearcher<S> last{{Box{}, nullptr}, search};

  assert(first == last || search.leaf(first->key));

  return {std::move(first), std::move(last)};
}

template<class B, class K, class D, class C>
template<class Children>
auto
RTree<B, K, D, C>::parent_key(const Children& children) -> Box
{
  return std::accumulate(
    children.begin(),
    children.end(),
    Box{},
    [](const Box& box, const auto& entry) { return box | entry_key(entry); });
}

template<class B, class K, class D, class C>
auto
RTree<B, K, D, C>::ideal_key(const DirNode& node) -> Box
{
  if (node.child_type() == NodeType::directory) {
    return parent_key(node.dir_children());
  }

  return parent_key(node.dat_children());
}

template<class B, class K, class D, class C>
auto
RTree<B, K, D, C>::insert_rec(DirEntry&   parent_entry,
                              const Box&  new_parent_key,
                              const Key&  key,
                              const Data& data) -> DirNodePair
{
  auto& parent = *parent_entry.node;
  if (parent.child_type() == NodeType::directory) { // Recursing downwards
    auto& children               = parent.dir_children();
    const auto [index, expanded] = _insertion.choose(children, key);
    auto& entry                  = children[index];
    auto  sides                  = insert_rec(entry, expanded, key, data);

    if (sides[0].node) { // Child was split, replace it
      children[index] = std::move(sides[0]);
      if (children.size() == Conf::dir_fanout) {
        return split(children, std::move(sides[1]), parent.child_type());
      }

      parent.append_child(std::move(sides[1]));
      parent_entry.key = parent_key(children);
    } else {
      parent_entry.key = new_parent_key;
      assert(parent_entry.key == ideal_key(parent));
    }

  } else if (parent.dat_children().size() <
             Conf::dat_fanout) { // Simple leaf insert
    parent.append_child(DirNode::make_dat_entry(key, data));
    parent_entry.key = new_parent_key;
    assert(parent_entry.key == ideal_key(parent));

  } else { // Split leaf insert
    return split(parent.dat_children(),
                 DirNode::make_dat_entry(key, data),
                 parent.child_type());
  }

  return {DirEntry{Box{}, nullptr}, DirEntry{Box{}, nullptr}};
}

template<class B, class K, class D, class C>
template<class Predicate, class Visitor>
void
RTree<B, K, D, C>::fast_query(const Predicate& predicate,
                              const Visitor&   visitor) const
{
  if (_root.node && predicate.directory(_root.key)) {
    fast_query_rec(*_root.node, predicate, visitor);
  }
}

template<class B, class K, class D, class C>
template<class Predicate, class Visitor>
void
RTree<B, K, D, C>::fast_query_rec(const DirNode&   node,
                                  const Predicate& predicate,
                                  const Visitor&   visitor) const
{
  if (node.child_type() == NodeType::directory) {
    for (const auto& entry : node.dir_children()) {
      if (predicate.directory(entry.key)) {
        fast_query_rec(*entry.node, predicate, visitor);
      }
    }
  } else {
    for (const auto& entry : node.dat_children()) {
      if (predicate.leaf(entry_key(entry))) {
        visitor(entry_ref(entry));
      }
    }
  }
}

/// Create a new parent seeded with a child
template<class B, class K, class D, class C>
template<class Entry, ChildCount count>
auto
RTree<B, K, D, C>::new_parent(StaticVector<Entry, ChildCount, count>& deposit,
                              ChildIndex                              index,
                              NodeType child_type) -> DirEntry
{
  auto* const iter{deposit.begin() + index};
  const Box   key{entry_key(*iter)};
  auto        node{std::make_unique<DirNode>(child_type)};

  node->append_child(std::move(*iter));
  deposit.pop(iter);

  return {key, std::move(node)};
}

/// Split `nodes` plus `node` in two and return the resulting sides
template<class B, class K, class D, class C>
template<class Entry, ChildCount fanout>
auto
RTree<B, K, D, C>::split(StaticVector<Entry, ChildCount, fanout>& nodes,
                         Entry                                    entry,
                         const NodeType type) -> DirNodePair
{
  constexpr auto max_fanout =
    fanout - (fanout * Conf::MinFillRatio::num / Conf::MinFillRatio::den);

  // Make an array of all nodes to deposit
  StaticVector<Entry, ChildCount, fanout + 1> deposit;
  std::for_each(nodes.begin(), nodes.end(), [&deposit](auto& node) {
    deposit.emplace_back(std::move(node));
  });
  deposit.emplace_back(std::move(entry));

  // Pick two nodes to seed the left and right groups
  auto seeds = _split.template pick_seeds<B>(deposit);
  assert(seeds.lhs_index < seeds.rhs_index);

  // Create left/right parent nodes with right/left seeds
  // The largest index is popped first to avoid invalidating the other
  DirNodePair sides{new_parent(deposit, seeds.rhs_index, type),
                    new_parent(deposit, seeds.lhs_index, type)};

  // Distribute remaining nodes between seeds
  _split.distribute_children(
    seeds, std::move(deposit), sides[0], sides[1], max_fanout);

  assert(sides[0].node->num_children() + sides[1].node->num_children() ==
         fanout + 1);
  assert(sides[0].key == ideal_key(*sides[0].node));
  assert(sides[1].key == ideal_key(*sides[1].node));

  return sides;
}

namespace detail {

template<typename Children, typename NodePath, typename ChildFunc>
VisitStatus
visit_children(const Children& children, NodePath& path, ChildFunc child_func)
{
  VisitStatus status = VisitStatus::proceed;

  for (ChildIndex i = 0U; i < children.size() && status == VisitStatus::proceed;
       ++i) {
    path.emplace_back(i);
    status = child_func(children[i]);
    path.pop_back();
  }

  return status;
}

template<class Key,
         class Node,
         typename DirVisitor,
         typename DatVisitor,
         typename NodePath>
VisitStatus
visit_dir_entry(const NodePointerEntry<Key, Node>& entry,
                DirVisitor                         visit_dir,
                DatVisitor                         visit_dat,
                NodePath&                          path)
{
  const auto& node = *entry.node;

  if (visit_dir(path, entry.key, node.num_children()) == VisitStatus::finish) {
    return VisitStatus::finish;
  }

  return node.child_type() == NodeType::directory
           ? visit_children(node.dir_children(),
                            path,
                            [&](const auto& child) {
                              return visit_dir_entry(
                                child, visit_dir, visit_dat, path);
                            })
           : visit_children(node.dat_children(), path, [&](const auto& child) {
               return visit_dat(path, entry_key(child), entry_data(child));
             });
}

} // namespace detail

template<class B, class K, class D, class C>
template<typename DirVisitor, typename DatVisitor>
void
RTree<B, K, D, C>::visit(DirVisitor&& visit_dir, DatVisitor&& visit_dat) const
{
  NodePath path;
  path.emplace_back(ChildIndex{});
  detail::visit_dir_entry(_root,
                          std::forward<DirVisitor>(visit_dir),
                          std::forward<DatVisitor>(visit_dat),
                          path);
}

} // namespace spaix

#endif // SPAIX_RTREE_IPP
