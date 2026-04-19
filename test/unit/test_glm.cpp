// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include <spaix/Config.hpp>
#include <spaix/DataPlacement.hpp>
#include <spaix/LinearInsertion.hpp> // IWYU pragma: keep
#include <spaix/LinearSplit.hpp>     // IWYU pragma: keep
#include <spaix/QuadraticSplit.hpp>  // IWYU pragma: keep
#include <spaix/RTree.hpp>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>

#undef NDEBUG

#include <spaix_test/check.hpp>
#include <spaix_test/options.hpp>

#include <algorithm>
#include <ctime>
#include <exception>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace {

using std::max;
using std::min;

using Scalar = float;
using Data   = size_t;

class Point
{
public:
  constexpr Point(const float point_x, const float point_y)
    : _vec{point_x, point_y}
  {}

  [[nodiscard]] constexpr float x() const { return _vec[0]; }
  [[nodiscard]] constexpr float y() const { return _vec[1]; }

  [[nodiscard]] constexpr const glm::vec2& vec() const { return _vec; }

private:
  glm::vec2 _vec;
};

class Rect
{
public:
  using Volume = float;

  constexpr Rect()
    : _vec{std::numeric_limits<float>::max(),
           std::numeric_limits<float>::lowest(),
           std::numeric_limits<float>::max(),
           std::numeric_limits<float>::lowest()}
  {}

  constexpr explicit Rect(const Point& point)
    : _vec{point.x(), point.x(), point.y(), point.y()}
  {}

  constexpr Rect(const float x_min,
                 const float x_max,
                 const float y_min,
                 const float y_max)
    : _vec{x_min, x_max, y_min, y_max}
  {}

  [[nodiscard]] constexpr Rect with(const Point& rhs) const
  {
    return {min(left(), rhs.x()),
            max(right(), rhs.x()),
            min(top(), rhs.y()),
            max(bottom(), rhs.y())};
  }

  [[nodiscard]] constexpr Rect with(const Rect& rhs) const
  {
    return {min(left(), rhs.left()),
            max(right(), rhs.right()),
            min(top(), rhs.top()),
            max(bottom(), rhs.bottom())};
  }

  [[nodiscard]] constexpr float left() const { return _vec[0]; }
  [[nodiscard]] constexpr float right() const { return _vec[1]; }
  [[nodiscard]] constexpr float top() const { return _vec[2]; }
  [[nodiscard]] constexpr float bottom() const { return _vec[3]; }

  [[nodiscard]] constexpr const glm::vec4& vec() const { return _vec; }

private:
  glm::vec4 _vec;
};

struct Queries {
  static constexpr bool contains(const Rect& parent, const Point& child)
  {
    return (child.x() >= parent.left() && child.x() <= parent.right()) &&
           (child.y() >= parent.top() && child.y() <= parent.bottom());
  }

  [[nodiscard]] static constexpr bool contains(const Rect& parent,
                                               const Rect& child)
  {
    return (child.left() >= parent.left() && child.right() <= parent.right()) &&
           (child.top() >= parent.top() && child.bottom() <= parent.bottom());
  }

  [[nodiscard]] static constexpr bool intersects(const Rect& lhs,
                                                 const Rect& rhs)
  {
    return rhs.left() <= lhs.right() && lhs.left() <= rhs.right() &&
           rhs.top() <= lhs.bottom() && lhs.top() <= rhs.bottom();
  }

  struct Contained {
    explicit Contained(Rect key)
      : _query_key{key}
    {}

    [[nodiscard]] constexpr bool directory(const Rect& k) const
    {
      return intersects(_query_key, k);
    }

    template<class DatKey>
    [[nodiscard]] constexpr bool leaf(const DatKey& k) const
    {
      return contains(_query_key, k);
    }

  private:
    Rect _query_key;
  };
};

struct Ops {
  using Scalar = float;
  using Box    = Rect;
  using Volume = float;

  template<size_t dim>
  [[nodiscard]] static constexpr Scalar lower(const Point& point)
  {
    return point.vec()[dim];
  }

  template<size_t dim>
  [[nodiscard]] static constexpr Scalar upper(const Point& point)
  {
    return point.vec()[dim];
  }

  template<size_t dim>
  [[nodiscard]] static constexpr Scalar lower(const Rect& rect)
  {
    return rect.vec()[2U * dim];
  }

  template<size_t dim>
  [[nodiscard]] static constexpr Scalar upper(const Rect& rect)
  {
    return rect.vec()[(2U * dim) + 1U];
  }

  [[nodiscard]] static constexpr Volume volume(const Point&) { return {}; }

  [[nodiscard]] static constexpr Volume volume(const Rect& rect)
  {
    return (rect.right() - rect.left()) * (rect.bottom() - rect.top());
  }

  [[nodiscard]] static constexpr Rect unify(const Rect& lhs, const Rect& rhs)
  {
    return lhs.with(rhs);
  }

  [[nodiscard]] static constexpr Rect unify(const Rect& lhs, const Point& rhs)
  {
    return lhs.with(rhs);
  }

  static void expand(Rect& lhs, const Rect& rhs) { lhs = unify(lhs, rhs); }

  static void expand(Rect& lhs, const Point& rhs) { lhs = unify(lhs, rhs); }
};

constexpr bool
operator==(const Rect& lhs, const Rect& rhs)
{
  return lhs.vec() == rhs.vec();
}

template<class Key>
Key
make_key(unsigned x, unsigned y);

template<>
Point
make_key<Point>(const unsigned x, const unsigned y)
{
  return Point{static_cast<float>(x), static_cast<float>(y)};
}

template<>
Rect
make_key<Rect>(const unsigned x, const unsigned y)
{
  return Rect{static_cast<float>(x),
              static_cast<float>(x) + 1.0f,
              static_cast<float>(y),
              static_cast<float>(y) + 1.0f};
}

template<class Tree>
void
test_empty_tree(const Tree& tree, const unsigned span)
{
  const Rect everything{
    0.0f, static_cast<float>(span), 0.0f, static_cast<float>(span)};

  CHECK(tree.empty());
  CHECK(tree.begin() == tree.end());
  CHECK(tree.query(Queries::Contained{everything}).empty());
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

  std::iota(y_values.begin(), y_values.end(), 0U);
  std::iota(x_values.begin(), x_values.end(), 0U);

  std::shuffle(y_values.begin(), y_values.end(), rng);
  std::shuffle(x_values.begin(), x_values.end(), rng);

  for (unsigned y = 0; y <= span; ++y) {
    for (unsigned x = 0; x <= span; ++x) {
      tree.insert(make_key<Key>(x_values[x], y_values[y]), (y * span) + x);
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

template<class Tree>
void
test_tree(const unsigned span, const unsigned n_queries)
{
  using Key = typename Tree::Key;

  const auto start_time = static_cast<unsigned>(time(nullptr));
  const auto seed       = std::random_device{}() ^ start_time;

  std::mt19937                            rng{seed};
  std::uniform_int_distribution<unsigned> dist{0, span - 1U};

  auto tree = make_tree<Tree>(rng, span);

  CHECK(tree.cbegin() == tree.cbegin());
  CHECK(tree.cend() == tree.cend());
  CHECK(tree.cbegin() != tree.cend());
  CHECK(std::next(tree.cbegin()) != tree.cbegin());

  size_t n_nodes = 0;
  for (const auto& node : tree) {
    (void)node;
    ++n_nodes;
  }
  CHECK(n_nodes == tree.size());

  unsigned count = 0;

  // Test a query that is in the tree bounds, but has no matches
  const auto mid = static_cast<float>(span) / 2.0f;
  const auto no_matches_query =
    Rect{mid + 0.1f, mid + 0.1f, mid + 0.9f, mid + 0.9f};
  for (const auto& node : tree.query(Queries::Contained{no_matches_query})) {
    ++count;
    (void)node;
  }
  CHECK((count == 0));

  for (auto i = 0U; i < n_queries; ++i) {
    const auto x0     = dist(rng);
    const auto x1     = dist(rng);
    const auto y0     = dist(rng);
    const auto y1     = dist(rng);
    const auto x_low  = std::min(x0, x1);
    const auto x_high = std::max(x0, x1) + 1U;
    const auto y_low  = std::min(y0, y1);
    const auto y_high = std::max(y0, y1) + 1U;

    const auto x_span = x_high - x_low;
    const auto y_span = y_high - y_low;

    const auto query = Rect{static_cast<Scalar>(x_low),
                            static_cast<Scalar>(x_high),
                            static_cast<Scalar>(y_low),
                            static_cast<Scalar>(y_high)};

    const auto expected_count = num_items_in_area<Key>(x_span, y_span);

    const auto verify = [&](const auto& node) {
      CHECK((Queries::contains(query, node.first)));
      CHECK((Queries::contains(tree.bounds(), node.first)));
      ++count;
    };

    // Visitor query
    count = 0;
    tree.visit_matches(Queries::Contained{query}, verify);
    CHECK((count == expected_count));

    // Incremental query
    count = 0;
    for (const auto& node : tree.query(Queries::Contained{query})) {
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
  using Structure = spaix::PageStructure<Rect, Key, Data, page_size, placement>;

  // Test a small tree where the root has leaf children
  test_tree<spaix::RTree<Rect,
                         Key,
                         Data,
                         spaix::Config<Structure,
                                       spaix::LinearSplit<Ops, 2U>,
                                       spaix::LinearInsertion<Ops>>>>(
    2, n_queries);

  test_tree<spaix::RTree<Rect,
                         Key,
                         Data,
                         spaix::Config<Structure,
                                       spaix::LinearSplit<Ops, 2U>,
                                       spaix::LinearInsertion<Ops>>>>(
    span, n_queries);

  test_tree<spaix::RTree<Rect,
                         Key,
                         Data,
                         spaix::Config<Structure,
                                       spaix::QuadraticSplit<Ops>,
                                       spaix::LinearInsertion<Ops>>>>(
    span, n_queries);
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

    test_key<Point>(span, queries);
    test_key<Rect>(span, queries);
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n\n";
    print_usage(argv[0], opts);
    return 1;
  }

  return 0;
}
