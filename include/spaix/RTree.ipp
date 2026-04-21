// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_RTREE_IPP
#define SPAIX_RTREE_IPP

#include <spaix/StaticVectorView.hpp>
#include <spaix/TreeRange.hpp>
#include <spaix/detail/DirectoryNode.hpp>
#include <spaix/detail/entry.hpp>
#include <spaix/types.hpp>

#include <algorithm>
#include <cassert>
#include <memory>
#include <numeric>
#include <utility>

namespace spaix {

template<class B, class K, class D, class C>
RTree<B, K, D, C>::RTree(Insertion insertion, Split split) noexcept
  : _insertion{std::move(insertion)}
  , _split{std::move(split)}
{}

template<class B, class K, class D, class C>
void
RTree<B, K, D, C>::insert(const Key& key, const Data& data)
{
  if (empty()) {
    _root = {B{key}, std::make_unique<DirNode>(NodeType::data)};
    ++_height;
  }

  insert_entry(_height - 1U, DirNode::make_dat_entry(key, data));
  ++_size;
}

template<class B, class K, class D, class C>
template<class Entry>
void
RTree<B, K, D, C>::insert_entry(const unsigned depth, Entry entry) noexcept
{
  const auto root_key = Ops::unify(_root.key, detail::entry_key(entry));
  auto       sides    = insert_rec(depth, _root, root_key, std::move(entry));

  if (sides[0].node) {
    const auto new_root_key = Ops::unify(sides[0].key, sides[1].key);
    auto       root_node    = std::make_unique<DirNode>(NodeType::directory);

    root_node->append_child(std::move(sides[0]));
    root_node->append_child(std::move(sides[1]));

    assert(new_root_key == ideal_key(*root_node));
    _root = {new_root_key, std::move(root_node)};
    ++_height;
  }
}

template<class B, class K, class D, class C>
void
RTree<B, K, D, C>::erase(data_iterator& i)
{
  // Drop leaf entry
  i.parent()->dat_children().pop_at(i.index());
  i.step_up();
  if (--_size == 0U) {
    clear();
    return;
  }

  // Condense tree by re-inserting any under-full directory nodes
  condense_tree(i);

  assert(!(_root.node->child_type() == NodeType::directory &&
           _root.node->num_children() == 1));
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

  assert(first == last || search.leaf(first->first));

  return {std::move(first), std::move(last)};
}

template<class B, class K, class D, class C>
template<class S>
auto
RTree<B, K, D, C>::query(S search) -> TreeRange<Searcher<S>>
{
  if (empty()) {
    return {{{Box{}, nullptr}, search}, {{Box{}, nullptr}, search}};
  }

  Searcher<S> first{_root, search};
  Searcher<S> last{{Box{}, nullptr}, search};

  assert(first == last || search.leaf(first->first));

  return {std::move(first), std::move(last)};
}

template<class B, class K, class D, class C>
template<class Children>
auto
RTree<B, K, D, C>::parent_key(const Children& children) noexcept -> Box
{
  return std::accumulate(children.begin(),
                         children.end(),
                         Box{},
                         [](const Box& box, const auto& entry) {
                           return Ops::unify(box, detail::entry_key(entry));
                         });
}

template<class B, class K, class D, class C>
auto
RTree<B, K, D, C>::ideal_key(const DirNode& node) noexcept -> Box
{
  if (node.child_type() == NodeType::directory) {
    return parent_key(node.dir_children());
  }

  return parent_key(node.dat_children());
}

template<class B, class K, class D, class C>
template<class Entry>
auto
RTree<B, K, D, C>::insert_rec(const unsigned depth,
                              DirEntry&      parent_entry,
                              const Box&     new_parent_key,
                              Entry          element) noexcept -> DirNodePair
{
  auto& parent = *parent_entry.node;
  if (depth) {
    // Recursing downwards
    auto children = parent.dir_children();
    const auto [index, expanded] =
      _insertion.choose(children, detail::entry_key(element));

    auto sides =
      insert_rec(depth - 1U, children[index], expanded, std::move(element));
    if (sides[0].node) { // Child was split, replace it
      children[index] = std::move(sides[0]);
      if (children.size() == Conf::dir_fanout) {
        return split(
          std::move(children), std::move(sides[1]), parent.child_type());
      }

      parent.append_child(std::move(sides[1]));
      parent_entry.key = parent_key(children);
    } else {
      parent_entry.key = new_parent_key;
      assert(parent_entry.key == ideal_key(parent));
    }

  } else if (parent.num_children() < Conf::fanout(parent.child_type())) {
    // Simple insert
    parent.append_child(std::move(element));
    parent_entry.key = new_parent_key;
    assert(parent_entry.key == ideal_key(parent));

  } else {
    // Split insert
    return split_node(parent, std::move(element));
  }

  return {DirEntry{Box{}, nullptr}, DirEntry{Box{}, nullptr}};
}

template<class B, class K, class D, class C>
void
RTree<B, K, D, C>::condense_tree(entry_iterator& i) noexcept
{
  // Condense upwards by removing under-filled directory nodes
  StaticVector<std::unique_ptr<DirNode>, unsigned, max_height()> removed;
  bool condensing = true;
  for (; !i.empty() && condensing; i.step_up()) {
    auto& e = i.parent()->dir_children()[i.index()];
    if (e.node->num_children() < Conf::min_fanout(e.node->child_type())) {
      // This entry's node is under-filled, remove it and continue
      removed.emplace_back(std::move(e.node));
      i.parent()->dir_children().pop_at(i.index());
    } else {
      // Recompute this entry's key and continue if it shrank
      const auto new_key = ideal_key(*e.node);
      condensing         = (new_key != e.key);
      if (condensing) {
        e.key = new_key;
      }
    }
  }

  // Shrink the root key if necessary
  if (condensing) {
    assert(i.empty() || i.parent() == _root.node.get());
    _root.key = ideal_key(*_root.node);
  }
  assert(_root.key == ideal_key(*_root.node));

  // Reinsert the children of all the removed directories
  for (auto h = 0U; h < removed.size(); ++h) {
    if (removed[h]->child_type() == NodeType::directory) {
      reinsert_children(h, removed[h]->dir_children());
    } else {
      reinsert_children(h, removed[h]->dat_children());
    }
  }

  // Remove superfluous root if possible
  if (_root.node->child_type() == NodeType::directory &&
      _root.node->num_children() == 1) {
    _root = std::move(_root.node->dir_children()[0]);
    --_height;
  }
}

template<class B, class K, class D, class C>
template<class Predicate, class Visitor>
void
RTree<B, K, D, C>::visit_matches(const Predicate& predicate,
                                 const Visitor&   visitor) const noexcept
{
  if (_root.node && predicate.directory(_root.key)) {
    visit_matches_rec(*_root.node, predicate, visitor);
  }
}

template<class B, class K, class D, class C>
template<class Predicate, class Visitor>
void
RTree<B, K, D, C>::visit_matches_rec(const DirNode&   node,
                                     const Predicate& predicate,
                                     const Visitor&   visitor) const noexcept
{
  if (node.child_type() == NodeType::directory) {
    for (const auto& entry : node.dir_children()) {
      if (predicate.directory(entry.key)) {
        visit_matches_rec(*entry.node, predicate, visitor);
      }
    }
  } else {
    for (const auto& entry : node.dat_children()) {
      if (predicate.leaf(detail::entry_key(entry))) {
        visitor(detail::entry_ref(entry));
      }
    }
  }
}

/// Create a new parent seeded with a child
template<class B, class K, class D, class C>
template<class Entry, class Count, Count count>
auto
RTree<B, K, D, C>::new_parent(StaticVector<Entry, Count, count>& deposit,
                              ChildIndex                         index,
                              NodeType child_type) noexcept -> DirEntry
{
  auto* const iter{deposit.begin() + index};
  const Box   key{detail::entry_key(*iter)};
  auto        node{std::make_unique<DirNode>(child_type)};

  node->append_child(std::move(*iter));
  deposit.pop_at(index);

  return {key, std::move(node)};
}

template<class B, class K, class D, class C>
auto
RTree<B, K, D, C>::split_node(DirNode& node, DirEntry entry) noexcept
  -> DirNodePair
{
  return split(node.dir_children(), std::move(entry), NodeType::directory);
}

template<class B, class K, class D, class C>
auto
RTree<B, K, D, C>::split_node(DirNode& node, DatEntry entry) noexcept
  -> DirNodePair
{
  return split(node.dat_children(), std::move(entry), NodeType::data);
}

/// Split `nodes` plus `node` in two and return the resulting sides
template<class B, class K, class D, class C>
template<class Entry, class Count, Count fanout>
auto
RTree<B, K, D, C>::split(StaticVectorView<Entry, Count, fanout> nodes,
                         Entry                                  entry,
                         const NodeType type) noexcept -> DirNodePair
{
  constexpr auto max_fanout =
    fanout - (fanout * Conf::MinFill::num / Conf::MinFill::den);

  // Make an array of all nodes to deposit
  StaticVector<Entry, ChildCount, static_cast<ChildCount>(fanout + 1U)> deposit;
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

template<class B, class K, class D, class C>
template<class Entry, class Fanout, Fanout fanout>
void
RTree<B, K, D, C>::reinsert_children(
  const unsigned                          skip,
  StaticVectorView<Entry, Fanout, fanout> nodes) noexcept
{
  for (Fanout i = 0U; i < nodes.size(); ++i) {
    Entry& e = nodes[i];
    assert(_height >= 1U + skip);
    insert_entry(_height - 1U - skip, std::move(e));
  }
}

namespace detail {

template<typename Children, typename NodePath, typename ChildFunc>
VisitStatus
visit_children(const Children& children, NodePath& path, ChildFunc child_func)
{
  using ChildIndex = typename Children::size_type;

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

  if (visit_dir(path, entry.key, node.child_type(), node.num_children()) ==
      VisitStatus::finish) {
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
