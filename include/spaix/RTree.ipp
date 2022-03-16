// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_RTREE_IPP
#define SPAIX_RTREE_IPP

#include "spaix/StaticVector.hpp"
#include "spaix/detail/DirectoryNode.hpp"
#include "spaix/types.hpp"
#include "spaix/union.hpp" // IWYU pragma: keep

#include <cassert>
#include <memory>
#include <utility>

namespace spaix {

template<class K, class D, class C>
template<class Children>
typename RTree<K, D, C>::Box
RTree<K, D, C>::parent_key(const Children& children)
{
  Box key;

  for (const auto& entry : children) {
    key |= entry_key(entry);
  }

  return key;
}

template<class K, class D, class C>
typename RTree<K, D, C>::Box
RTree<K, D, C>::ideal_key(const DirNode& node)
{
  if (node.child_type() == NodeType::directory) {
    return parent_key(node.dir_children());
  }

  return parent_key(node.dat_children());
}

template<class K, class D, class C>
typename RTree<K, D, C>::DirNodePair
RTree<K, D, C>::insert_rec(DirEntry&   parent_entry,
                           const Box&  new_parent_key,
                           const Key&  key,
                           const Data& data)
{
  auto& parent = *parent_entry.node;
  if (parent.child_type() == NodeType::directory) { // Recursing downwards
    auto&      children = parent.dir_children();
    const auto choice   = _insertion.choose(children, key);
    const auto index    = choice.first;
    const auto expanded = choice.second;
    auto&      entry    = children[index];

    auto sides = insert_rec(entry, expanded, key, data);

    if (sides[0].node) { // Child was split, replace it
      children[index] = std::move(sides[0]);
      if (children.size() == Config::dir_fanout) {
        return split(children,
                     std::move(sides[1]),
                     parent_entry.key | key,
                     parent.child_type());
      }

      parent.append_child(std::move(sides[1]));
      parent_entry.key = parent_key(children);
    } else {
      parent_entry.key = new_parent_key;
      assert(parent_entry.key == ideal_key(parent));
    }

  } else if (parent.dat_children().size() <
             Config::dat_fanout) { // Simple leaf insert
    parent.append_child(DirNode::make_dat_entry(key, data));
    parent_entry.key = new_parent_key;
    assert(parent_entry.key == ideal_key(parent));

  } else { // Split leaf insert
    return split(parent.dat_children(),
                 DirNode::make_dat_entry(key, data),
                 parent_entry.key | key,
                 parent.child_type());
  }

  return {DirEntry{Box{}, nullptr}, DirEntry{Box{}, nullptr}};
}

/// Create a new parent seeded with a child
template<class K, class D, class C>
template<class Entry, ChildCount count>
typename RTree<K, D, C>::DirEntry
RTree<K, D, C>::new_parent(StaticVector<Entry, ChildCount, count>& deposit,
                           ChildIndex                              index,
                           NodeType                                child_type)
{
  const auto iter{deposit.begin() + index};
  Box        key{entry_key(*iter)};
  DirNodePtr node{std::make_unique<DirNode>(child_type)};

  node->append_child(std::move(*iter));
  deposit.pop(iter);

  return {key, std::move(node)};
}

/// Split `nodes` plus `node` in two and return the resulting sides
template<class K, class D, class C>
template<class Entry, ChildCount fanout>
typename RTree<K, D, C>::DirNodePair
RTree<K, D, C>::split(StaticVector<Entry, ChildCount, fanout>& nodes,
                      Entry                                    entry,
                      const Box&                               bounds,
                      const NodeType                           type)
{
  constexpr auto max_fanout = fanout - (fanout / min_fill_divisor);

  // Make an array of all nodes to deposit
  StaticVector<Entry, ChildCount, fanout + 1> deposit;
  for (auto&& e : nodes) {
    deposit.emplace_back(std::move(e));
  }
  deposit.emplace_back(std::move(entry));
  assert(bounds == parent_key(deposit));

  // Pick two nodes to seed the left and right groups
  const auto seeds = _split.pick_seeds(deposit, bounds);
  assert(seeds.first < seeds.second);

  // Create left and right parent nodes with seeds
  DirNodePair sides{new_parent(deposit, seeds.second, type),
                    new_parent(deposit, seeds.first, type)};

  // Distribute remaining nodes between seeds
  _split.distribute_children(
    std::move(deposit), sides[0], sides[1], max_fanout);
  assert(sides[0].node->num_children() + sides[1].node->num_children() ==
         fanout + 1);
  assert(sides[0].key == ideal_key(*sides[0].node));
  assert(sides[1].key == ideal_key(*sides[1].node));

  return sides;
}

namespace detail {

template<typename Children, typename ChildFunc>
VisitStatus
visit_children(const Children& children, NodePath& path, ChildFunc&& child_func)
{
  VisitStatus status = VisitStatus::proceed;

  for (ChildIndex i = 0u; i < children.size() && status == VisitStatus::proceed;
       ++i) {
    path.push_back(i);
    status = child_func(children[i]);
    path.pop_back();
  }

  return status;
}

template<class Key, class Node, typename DirVisitor, typename DatVisitor>
VisitStatus
visit_dir_entry(const NodePointerEntry<Key, Node>& entry,
                DirVisitor&&                       visit_dir,
                DatVisitor&&                       visit_dat,
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

template<class K, class D, class C>
template<typename DirVisitor, typename DatVisitor>
void
RTree<K, D, C>::visit(DirVisitor&& visit_dir, DatVisitor&& visit_dat) const
{
  NodePath path({0}, NodePath::allocator_type{});
  detail::visit_dir_entry(_root,
                          std::forward<DirVisitor>(visit_dir),
                          std::forward<DatVisitor>(visit_dat),
                          path);
}

template<class K, class D, class C>
template<typename DirVisitor>
void
RTree<K, D, C>::visit(DirVisitor&& visit_dir) const
{
  NodePath path({0}, NodePath::allocator_type{});
  detail::visit_dir_entry(
    _root,
    std::forward<DirVisitor>(visit_dir),
    [](auto, auto, auto) { return VisitStatus::proceed; },
    path);
}

} // namespace spaix

#endif // SPAIX_RTREE_IPP
