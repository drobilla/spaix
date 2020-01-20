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

#include "check.hpp"

#include "spaix/LinearInsertion.hpp"
#include "spaix/LinearSplit.hpp"
#include "spaix/Point.hpp"
#include "spaix/QuadraticSplit.hpp"
#include "spaix/RTree.hpp"
#include "spaix/Rect.hpp"
#include "spaix/within.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace {

using Rect     = spaix::Rect<unsigned, unsigned>;
using Point    = spaix::Point<unsigned, unsigned>;
using Data     = unsigned;
using NodePath = spaix::NodePath;

constexpr size_t page_size = 64u;

template <class Key>
Key
make_key(unsigned x, unsigned y);

template <>
Point
make_key<Point>(const unsigned x, const unsigned y)
{
  return Point{x, y};
}

template <>
Rect
make_key<Rect>(const unsigned x, const unsigned y)
{
  return Rect{{x, x + 1}, {y, y + 1}};
}

template <class Tree>
void
test_empty_tree(const Tree& tree, const unsigned span)
{
  using DirKey  = typename Tree::DirKey;
  using DirNode = typename Tree::DirNode;

  const Rect everything{{0, span}, {0, span}};

  CHECK(tree.empty());

  STATIC_CHECK(sizeof(DirNode) == page_size);
  STATIC_CHECK(Tree::fanout() == spaix::fanout<DirKey>(page_size));

  CHECK(tree.begin() == tree.end());
  CHECK(tree.query(spaix::within(everything)).empty());
}

template <class Tree>
Tree
make_tree(std::mt19937& rng, const unsigned span)
{
  using Key = typename Tree::Key;

  Tree tree;

  test_empty_tree(tree, span);

  std::vector<unsigned> y_values(span + 1);
  std::vector<unsigned> x_values(span + 1);

  std::iota(y_values.begin(), y_values.end(), 0);
  std::iota(x_values.begin(), x_values.end(), 0);

  std::shuffle(y_values.begin(), y_values.end(), rng);
  std::shuffle(x_values.begin(), x_values.end(), rng);

  for (unsigned y = 0; y <= span; ++y) {
    for (unsigned x = 0; x <= span; ++x) {
      tree.insert(make_key<Key>(x_values[x], y_values[y]), y * span + x);
    }
  }

  return tree;
}

template <class Key>
unsigned
num_items_in_area(const unsigned x_span, const unsigned y_span);

template <>
unsigned
num_items_in_area<Point>(const unsigned x_span, const unsigned y_span)
{
  return (x_span + 1) * (y_span + 1);
}

template <>
unsigned
num_items_in_area<Rect>(const unsigned x_span, const unsigned y_span)
{
  return x_span * y_span;
}

template <class NodeKey>
void
check_node(const std::map<NodePath, Rect>& dir_keys,
           const NodeKey&                  key,
           const NodePath&                 path)
{
  // Check that all parent keys are present and contain us
  NodePath parent_path(path.begin(), std::prev(path.end()));
  while (!parent_path.empty()) {
    const auto p = dir_keys.find(parent_path);
    CHECK(p != dir_keys.end());
    CHECK(contains(p->second, key));
    parent_path.pop_back();
  }
}

template <class Tree>
void
test_visit_structure(const Tree& tree)
{
  using DirKey = typename Tree::DirKey;

  std::vector<NodePath> top_paths;

  // Check that structure visitation ends when visitors return false
  tree.visit_structure([&](const DirKey&, const NodePath& path, const size_t) {
    CHECK(path.size() <= 2);
    top_paths.emplace_back(path);
    return path.size() < 2;
  });

  for (const auto& path : top_paths) {
    CHECK(path.size() <= 2);
  }

  // Visit every directory
  size_t n_dirs = 0;
  tree.visit_structure([&](const DirKey&, const NodePath&, const size_t) {
    ++n_dirs;
    return true;
  });

  CHECK(n_dirs > top_paths.size());
}

template <class Tree>
void
test_structure(const Tree& tree)
{
  using DirKey = typename Tree::DirKey;
  using Key    = typename Tree::Key;

  std::map<NodePath, DirKey> dir_keys;
  std::set<NodePath>         dat_paths;

  size_t n_leaves = 0;
  tree.visit_structure(
      [&](const DirKey& key, const NodePath& path, const size_t n_children) {
        CHECK(n_children <= tree.fanout());
        check_node(dir_keys, key, path);
        dir_keys.emplace(path, key);
        return true;
      },
      [&](const Key& key, const Data&, const NodePath& path) {
        check_node(dir_keys, key, path);
        CHECK(dat_paths.find(path) == dat_paths.end());
        dat_paths.emplace(path);
        ++n_leaves;
      });

  CHECK(n_leaves == tree.size());
}

template <class Tree>
void
test_tree(const unsigned span, const unsigned n_queries)
{
  using Scalar = unsigned;
  using Key    = typename Tree::Key;

  std::mt19937                          rng;
  std::uniform_int_distribution<Scalar> dist{0, span - 1};

  auto tree = make_tree<Tree>(rng, span);

  test_visit_structure(tree);
  test_structure(tree);

  CHECK(tree.begin() == tree.begin());
  CHECK(tree.end() == tree.end());
  CHECK(tree.begin() != tree.end());
  CHECK(std::next(tree.begin()) != tree.begin());

  size_t n_nodes = 0;
  for (const auto& node : tree) {
    (void)node;
    ++n_nodes;
  }
  CHECK(n_nodes == tree.size());

  for (auto i = 0u; i < n_queries; ++i) {
    const auto x0     = dist(rng);
    const auto x1     = dist(rng);
    const auto y0     = dist(rng);
    const auto y1     = dist(rng);
    const auto x_low  = std::min(x0, x1);
    const auto x_high = std::max(x0, x1) + 1;
    const auto y_low  = std::min(y0, y1);
    const auto y_high = std::max(y0, y1) + 1;

    const auto     query          = Rect{{x_low, x_high}, {y_low, y_high}};
    const auto     x_span         = spaix::span<0>(query);
    const auto     y_span         = spaix::span<1>(query);
    const unsigned expected_count = num_items_in_area<Key>(x_span, y_span);

    unsigned count = 0;

    const auto verify = [&](const auto& node) {
      CHECK((spaix::min<0>(node.key) >= x_low));
      CHECK((spaix::max<0>(node.key) <= x_high));
      CHECK((spaix::min<1>(node.key) >= y_low));
      CHECK((spaix::max<1>(node.key) <= y_high));
      CHECK((contains(tree.bounds(), node.key)));
      ++count;
    };

    count = 0;
    for (const auto& node : tree.query(spaix::within(query))) {
      verify(node);
    }
    CHECK((count == expected_count));
  }

  tree.clear();
  test_empty_tree(tree, span);
}

template <class Key>
void
test_key(const unsigned span, const unsigned n_queries)
{
  using DirKey = spaix::Rect<unsigned, unsigned>;

  test_tree<spaix::RTree<Key,
                         unsigned,
                         spaix::Rect<unsigned, unsigned>,
                         spaix::fanout<DirKey>(page_size),
                         2,
                         spaix::LinearInsertion,
                         spaix::LinearSplit>>(span, n_queries);

  test_tree<spaix::RTree<Key,
                         unsigned,
                         spaix::Rect<unsigned, unsigned>,
                         spaix::fanout<DirKey>(page_size),
                         2,
                         spaix::LinearInsertion,
                         spaix::QuadraticSplit>>(span, n_queries);
}

} // namespace

int
main()
{
  test_key<spaix::Point<unsigned, unsigned>>(16, 1000);
  test_key<spaix::Rect<unsigned, unsigned>>(16, 1000);

  return 0;
}
