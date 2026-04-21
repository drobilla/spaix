// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include "draw_dot.hpp"
#include "draw_svg.hpp"

#include <spaix_test/options.hpp>

#include <spaix/Config.hpp>
#include <spaix/DataPlacement.hpp>
#include <spaix/LinearInsertion.hpp> // IWYU pragma: keep
#include <spaix/LinearSplit.hpp>     // IWYU pragma: keep
#include <spaix/QuadraticSplit.hpp>  // IWYU pragma: keep
#include <spaix/RTree.hpp>
#include <spaix/homox/Operations.hpp>
#include <spaix/homox/Point.hpp>
#include <spaix/homox/Rect.hpp>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace {

using Args   = spaix::test::Arguments;
using Scalar = double;
using Rect2  = spaix::homox::Rect<Scalar, 2U>;
using Point2 = spaix::homox::Point<Scalar, 2U>;
using Ops    = spaix::homox::Operations<Scalar, 2U>;

struct Parameters {
  explicit Parameters(const spaix::test::Arguments& args)
    : n_elements{std::stoul(args.at("size"))}
    , fanout{static_cast<uint32_t>(std::stoul(args.at("fanout")))}
    , scale{std::stod(args.at("scale"))}
    , span{std::stod(args.at("span"))}
    , seed{static_cast<uint32_t>(std::stoul(args.at("seed")))}
  {}

  size_t   n_elements;
  unsigned fanout;
  double   scale;
  Scalar   span;
  uint32_t seed;
};

template<class Tree>
int
run(const Parameters& params)
{
  std::mt19937                           rng{params.seed};
  std::uniform_real_distribution<Scalar> dist{0, params.span};

  Tree tree{typename Tree::Insertion{}, typename Tree::Split{}};
  for (size_t i = 0; i < params.n_elements; ++i) {
    tree.insert(Point2{floor(dist(rng)), floor(dist(rng))},
                static_cast<double>(i));
  }

  {
    std::ofstream dot{"r_tree.dot"};
    draw_dot(dot, tree);
    std::cerr << "Wrote r_tree.dot\n";
  }
  {
    std::ofstream svg{"r_tree.svg"};
    draw_svg(svg, tree, params.scale);
    std::cerr << "Wrote r_tree.svg\n";
  }

  return 0;
}

template<class Insertion, class Split, unsigned fanout>
int
run(const Parameters& params)
{
  using Structure =
    spaix::StaticStructure<fanout, fanout, spaix::DataPlacement::separate>;

  using Config = spaix::Config<Structure, Split, Insertion>;

  return run<spaix::RTree<Rect2, Point2, Scalar, Config>>(params);
}

template<class Insertion, class Split>
int
run(const Parameters& params)
{
  switch (params.fanout) {
  case 4:
    return run<Insertion, Split, 4>(params);
  case 8:
    return run<Insertion, Split, 8>(params);
  case 12:
    return run<Insertion, Split, 12>(params);
  case 16:
    return run<Insertion, Split, 16>(params);
  case 20:
    return run<Insertion, Split, 20>(params);
  case 24:
    return run<Insertion, Split, 24>(params);
  }

  throw std::runtime_error("Invalid fanout '" + std::to_string(params.fanout) +
                           "'");
}

template<class Insertion>
int
run(const Parameters& params, const Args& args)
{
  const auto split = args.at("split");
  if (split == "linear") {
    return run<Insertion, spaix::LinearSplit<Ops, 2U>>(params);
  }

  if (split == "quadratic") {
    return run<Insertion, spaix::QuadraticSplit<Ops>>(params);
  }

  throw std::runtime_error("Unknown split algorithm '" + split + "'");
}

int
run(const Parameters& params, const Args& args)
{
  const auto insert = args.at("insert");
  if (insert == "linear") {
    return run<spaix::LinearInsertion<Ops>>(params, args);
  }

  throw std::runtime_error("Unknown insert algorithm '" + insert + "'");
}

} // namespace

int
main(int argc, char** argv)
{
  const spaix::test::Options opts{
    {"insert", {"Insert (linear)", "ALGORITHM", "linear"}},
    {"fanout", {"Fanout for directory nodes", "COUNT", "8"}},
    {"scale", {"Scale factor for SVG output", "NUMBER", "1.0"}},
    {"seed", {"Random number generator seed", "SEED", "5489"}},
    {"size", {"Number of elements", "ELEMENTS", "1024"}},
    {"span", {"Dimension span", "NUMBER", "1024"}},
    {"split", {"Split (linear, quadratic)", "ALGORITHM", "quadratic"}},
  };

  try {
    const auto args = parse_options(opts, argc, argv);
    run(Parameters{args}, args);
  } catch (const std::runtime_error& e) {
    std::cerr << "error: " << e.what() << "\n\n";
    print_usage(argv[0], opts);
    return 1;
  }

  return 0;
}
