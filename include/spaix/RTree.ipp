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

#ifndef SPAIX_RTREE_IPP
#define SPAIX_RTREE_IPP

#include "spaix/DataNode.hpp"
#include "spaix/Iterator.hpp"
#include "spaix/LinearInsertion.hpp"
#include "spaix/Point.hpp"
#include "spaix/QuadraticSplit.hpp"
#include "spaix/Rect.hpp"
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

template <class K, class D, class C>
template <class Children>
typename RTree<K, D, C>::Box
RTree<K, D, C>::parent_key(const Children& children)
{
  Box key;

  for (const auto& entry : children) {
    key |= entry_key(entry);
  }

  return key;
}

template <class K, class D, class C>
typename RTree<K, D, C>::Box
RTree<K, D, C>::ideal_key(const DirNode& node)
{
  if (node.child_type == NodeType::directory) {
    return parent_key(node.dir_children);
  } else {
    return parent_key(node.dat_children);
  }
}

template <class K, class D, class C>
typename RTree<K, D, C>::DirNodePair
RTree<K, D, C>::insert_rec(DirEntry&   parent_entry,
                           const Box&  new_parent_key,
                           const Key&  key,
                           const Data& data)
{
  auto& parent = *parent_entry.node;
  if (parent.child_type == NodeType::directory) { // Recursing downwards
    const auto choice   = Insertion::choose(parent.dir_children, key);
    const auto index    = choice.first;
    const auto expanded = choice.second;
    auto&      entry    = parent.dir_children[index];

    auto sides = insert_rec(entry, expanded, key, data);

    if (sides[0].node) { // Child was split, replace it
      parent.dir_children[index] = std::move(sides[0]);
      if (parent.dir_children.size() == dir_fanout) {
        return split(parent.dir_children,
                     std::move(sides[1]),
                     parent_entry.key | key,
                     parent.child_type);
      }

      parent.append_child(std::move(sides[1]));
      parent_entry.key = parent_key(parent.dir_children);
    } else {
      parent_entry.key = new_parent_key;
      assert(parent_entry.key == ideal_key(parent));
    }

  } else if (parent.dat_children.size() < dat_fanout) { // Simple leaf insert
    parent.append_child(DatNode{key, data});
    parent_entry.key = new_parent_key;
    assert(parent_entry.key == ideal_key(parent));

  } else { // Split leaf insert
    return split(parent.dat_children,
                 DatNode{key, data},
                 parent_entry.key | key,
                 parent.child_type);
  }

  return {DirEntry{Box{}, nullptr}, DirEntry{Box{}, nullptr}};
}

/// Create a new parent seeded with a child
template <class K, class D, class C>
template <class Entry, ChildCount count>
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
template <class K, class D, class C>
template <class Entry, ChildCount fanout>
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
  const auto seeds = Split::pick_seeds(deposit, bounds);
  assert(seeds.first < seeds.second);

  // Create left and right parent nodes with seeds
  DirNodePair sides{new_parent(deposit, seeds.second, type),
                    new_parent(deposit, seeds.first, type)};

  // Distribute remaining nodes between seeds
  Split::distribute_children(
      std::move(deposit), sides[0], sides[1], max_fanout);
  assert(sides[0].node->num_children() + sides[1].node->num_children() ==
         fanout + 1);
  assert(sides[0].key == ideal_key(*sides[0].node));
  assert(sides[1].key == ideal_key(*sides[1].node));

  return sides;
}

template <class K, class D, class C>
void
RTree<K, D, C>::visit_structure_rec(const DirEntry& entry,
                                    DirVisitor      visit_dir,
                                    DatVisitor      visit_dat,
                                    NodePath&       path)
{
  const auto& node = *entry.node;
  if (visit_dir(entry.key, path, node.num_children())) {
    for (ChildIndex i = 0u; i < node.num_children(); ++i) {
      path.push_back(i);

      if (node.child_type == NodeType::data) {
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

} // namespace spaix

#endif // SPAIX_RTREE_IPP