// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include "check.hpp"
#include "options.hpp"

#include "spaix/DataPlacement.hpp"
#include "spaix/LinearInsertion.hpp" // IWYU pragma: keep
#include "spaix/LinearSplit.hpp"     // IWYU pragma: keep
#include "spaix/PageConfiguration.hpp"
#include "spaix/Point.hpp"
#include "spaix/QuadraticSplit.hpp" // IWYU pragma: keep
#include "spaix/RTree.hpp"
#include "spaix/Rect.hpp"
#include "spaix/contains.hpp"
#include "spaix/within.hpp"

#include <algorithm>
#include <ctime>
#include <exception>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

using Scalar   = float;
using Rect     = spaix::Rect<float, float>;
using Point    = spaix::Point<float, float>;
using Data     = size_t;
using NodePath = spaix::NodePath;

/* constexpr size_t page_size = 512u; */

template<class Key>
Key
make_key(unsigned x, unsigned y);

template<>
Point
make_key<Point>(const unsigned x, const unsigned y)
{
  return Point{float(x), float(y)};
}

template<>
Rect
make_key<Rect>(const unsigned x, const unsigned y)
{
  return Rect{{float(x), float(x) + 1.0f}, {float(y), float(y) + 1.0f}};
}

template<class Tree>
void
test_empty_tree(const Tree& tree, const unsigned span)
{
  const Rect everything{{0.0f, float(span)}, {0.0f, float(span)}};

  CHECK(tree.empty());
  CHECK(tree.begin() == tree.end());
  CHECK(tree.query(spaix::within(everything)).empty());
}

template<class Tree>
Tree
make_tree(std::mt19937& rng, const unsigned span)
{
  using Key = typename Tree::Key;

  Tree tree;

  test_empty_tree(tree, span);

  std::vector<unsigned> y_values(span + 1);
  std::vector<unsigned> x_values(span + 1);

  std::iota(y_values.begin(), y_values.end(), 0u);
  std::iota(x_values.begin(), x_values.end(), 0u);

  std::shuffle(y_values.begin(), y_values.end(), rng);
  std::shuffle(x_values.begin(), x_values.end(), rng);

  for (unsigned y = 0; y <= span; ++y) {
    for (unsigned x = 0; x <= span; ++x) {
      tree.insert(make_key<Key>(x_values[x], y_values[y]), y * span + x);
    }
  }

  return tree;
}

template<class Key>
unsigned
num_items_in_area(unsigned x_span, unsigned y_span);

template<>
unsigned
num_items_in_area<Point>(const unsigned x_span, const unsigned y_span)
{
  return (x_span + 1) * (y_span + 1);
}

template<>
unsigned
num_items_in_area<Rect>(const unsigned x_span, const unsigned y_span)
{
  return x_span * y_span;
}

template<class NodeKey>
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

template<class Tree>
void
test_visit(const Tree& tree)
{
  using DirKey = typename Tree::Box;

  std::vector<NodePath> top_paths;

  // Check that structure visitation ends when visitors return false
  tree.visit([&](const NodePath& path, const DirKey&, const size_t) {
    CHECK(path.size() <= 2);
    top_paths.emplace_back(path);
    return path.size() < 2 ? spaix::VisitStatus::proceed
                           : spaix::VisitStatus::finish;
  });

  for (const auto& path : top_paths) {
    CHECK(path.size() <= 2);
  }

  // Visit every directory
  size_t n_dirs = 0;
  tree.visit([&](const NodePath&, const DirKey&, const size_t) {
    ++n_dirs;
    return spaix::VisitStatus::proceed;
  });

  CHECK(n_dirs >= top_paths.size());

  // Visit every leaf
  size_t n_leaves = 0;
  tree.visit(
    [&](const NodePath&, const DirKey&, const size_t) {
      return spaix::VisitStatus::proceed;
    },
    [&](const NodePath&, const typename Tree::Key&, const Data&) {
      return (++n_leaves == tree.size() / 2) ? spaix::VisitStatus::finish
                                             : spaix::VisitStatus::proceed;
    });

  CHECK(n_leaves == tree.size() / 2);
}

template<class Tree>
void
test_structure(const Tree& tree)
{
  using DirKey = typename Tree::Box;
  using Key    = typename Tree::Key;

  std::map<NodePath, DirKey> dir_keys;
  std::set<NodePath>         dat_paths;

  size_t n_leaves = 0;
  tree.visit(
    [&](const NodePath& path, const DirKey& key, const size_t n_children) {
      CHECK(n_children <= std::max(Tree::internal_fanout(), Tree::leaf_fanout()));
      check_node(dir_keys, key, path);
      dir_keys.emplace(path, key);
      return spaix::VisitStatus::proceed;
    },
    [&](const NodePath& path, const Key& key, const Data&) {
      check_node(dir_keys, key, path);
      CHECK(dat_paths.find(path) == dat_paths.end());
      dat_paths.emplace(path);
      ++n_leaves;
      return spaix::VisitStatus::proceed;
    });

  CHECK(n_leaves == tree.size());
}

template<class Tree>
void
test_tree(const unsigned span, const unsigned n_queries)
{
  using Key = typename Tree::Key;

  const auto start_time = static_cast<unsigned>(time(nullptr));
  const auto seed       = std::random_device{}() ^ start_time;

  std::mt19937                            rng{seed};
  std::uniform_int_distribution<unsigned> dist{0, span - 1u};

  auto tree = make_tree<Tree>(rng, span);

  test_visit(tree);
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

  unsigned count = 0;

  // Test a query that is in the tree bounds, but has no matches
  const auto mid = float(span) / 2.0f;
  const auto no_matches_query =
    Rect{{mid + 0.1f, mid + 0.1f}, {mid + 0.9f, mid + 0.9f}};
  for (const auto& node : tree.query(spaix::within(no_matches_query))) {
    ++count;
    (void)node;
  }
  CHECK((count == 0));

  for (auto i = 0u; i < n_queries; ++i) {
    const auto x0     = dist(rng);
    const auto x1     = dist(rng);
    const auto y0     = dist(rng);
    const auto y1     = dist(rng);
    const auto x_low  = std::min(x0, x1);
    const auto x_high = std::max(x0, x1) + 1u;
    const auto y_low  = std::min(y0, y1);
    const auto y_high = std::max(y0, y1) + 1u;

    const auto x_span = x_high - x_low;
    const auto y_span = y_high - y_low;
    const auto query =
      Rect{{Scalar(x_low), Scalar(x_high)}, {Scalar(y_low), Scalar(y_high)}};

    const auto expected_count = num_items_in_area<Key>(x_span, y_span);

    const auto verify = [&](const auto& node) {
      CHECK((spaix::min<0>(node.key) >= x_low));
      CHECK((spaix::max<0>(node.key) <= x_high));
      CHECK((spaix::min<1>(node.key) >= y_low));
      CHECK((spaix::max<1>(node.key) <= y_high));
      CHECK((contains(tree.bounds(), node.key)));
      ++count;
    };

    // Fast visitor query
    count = 0;
    tree.fast_query(spaix::within(query), verify);
    CHECK((count == expected_count));

    // Incremental query
    count = 0;
    for (const auto& node : tree.query(spaix::within(query))) {
      verify(node);
    }
    CHECK((count == expected_count));
  }

  tree.clear();
  test_empty_tree(tree, span);
}

template<class Key, spaix::DataPlacement placement, size_t page_size>
void
test_page_size(const unsigned span, const unsigned n_queries)
{
  // Test a small tree where the root has leaf children
  test_tree<spaix::RTree<Key,
                         Data,
                         spaix::PageConfiguration<spaix::LinearSplit,
                                                  spaix::LinearInsertion,
                                                  Key,
                                                  Data,
                                                  page_size,
                                                  3,
                                                  placement>>>(2, n_queries);

  test_tree<spaix::RTree<Key,
                         Data,
                         spaix::PageConfiguration<spaix::LinearSplit,
                                                  spaix::LinearInsertion,
                                                  Key,
                                                  Data,
                                                  page_size,
                                                  3,
                                                  placement>>>(span, n_queries);

  test_tree<spaix::RTree<Key,
                         Data,
                         spaix::PageConfiguration<spaix::QuadraticSplit,
                                                  spaix::LinearInsertion,
                                                  Key,
                                                  Data,
                                                  page_size,
                                                  3,
                                                  placement>>>(span, n_queries);
}

template<class Key, spaix::DataPlacement placement>
void
test_placement(const unsigned span, const unsigned n_queries)
{
  test_page_size<Key, placement, 256>(span, n_queries);
  test_page_size<Key, placement, 512>(span, n_queries);
  test_page_size<Key, placement, 4096>(span, n_queries);
}

template<class Key>
void
test_key(const unsigned span, const unsigned n_queries)
{
  test_placement<Key, spaix::DataPlacement::inlined>(span, n_queries);
  test_placement<Key, spaix::DataPlacement::separate>(span, n_queries);
}

} // namespace

int
main(int argc, char** argv)
{
  const spaix::test::Options opts{
    {"span", {"Dimension span", "NUMBER", "20"}},
    {"queries", {"Number of queries", "COUNT", "400"}}};

  try {
    const auto args    = parse_options(opts, argc, argv);
    const auto span    = static_cast<unsigned>(std::stoul(args.at("span")));
    const auto queries = static_cast<unsigned>(std::stoul(args.at("queries")));

    test_key<spaix::Point<float, float>>(span, queries);
    test_key<spaix::Rect<float, float>>(span, queries);

  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n\n";
    print_usage(argv[0], opts);
    return 1;
  }

  return 0;
}
