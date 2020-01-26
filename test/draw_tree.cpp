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

#include "options.hpp"

#include "spaix/LinearInsertion.hpp"
#include "spaix/LinearSplit.hpp"
#include "spaix/Point.hpp"
#include "spaix/QuadraticSplit.hpp"
#include "spaix/RTree.hpp"
#include "spaix/Rect.hpp"
#include "spaix/draw_dot.hpp"
#include "spaix/draw_svg.hpp"

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
using Rect2  = spaix::Rect<Scalar, Scalar>;
using Point2 = spaix::Point<Scalar, Scalar>;

struct Parameters
{
  explicit Parameters(const spaix::test::Arguments& args)
    : n_elements{static_cast<size_t>(std::stoul(args.at("size")))}
    , page_size{static_cast<size_t>(std::stoul(args.at("page-size")))}
    , scale{std::stod(args.at("scale"))}
    , span{std::stod(args.at("span"))}
    , seed{static_cast<uint32_t>(std::stoul(args.at("seed")))}
  {
  }

  size_t   n_elements;
  size_t   page_size;
  double   scale;
  Scalar   span;
  uint32_t seed;
};

template <class Tree>
int
run(const Parameters& params)
{
  std::mt19937                           rng{params.seed};
  std::uniform_real_distribution<Scalar> dist{0, params.span};

  Tree tree;
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

template <class Insertion, class Split, size_t page_size>
int
run(const Parameters& params)
{
  using Config = spaix::Configuration<page_size, 2, Split, Insertion>;

  return run<spaix::RTree<Point2, Scalar, Config>>(params);
}

template <class Insertion, class Split>
int
run(const Parameters& params)
{
  switch (params.page_size) {
  // case 64: return run<Insertion, Split, 64>(params);
  // case 128: return run<Insertion, Split, 128>(params);
  case 256: return run<Insertion, Split, 256>(params);
  case 512: return run<Insertion, Split, 512>(params);
  case 1024: return run<Insertion, Split, 1024>(params);
  case 2048: return run<Insertion, Split, 2048>(params);
  case 4096: return run<Insertion, Split, 4096>(params);
  }

  throw std::runtime_error("Invalid page size '" +
                           std::to_string(params.page_size) + "'");
}

template <class Insertion>
int
run(const Parameters& params, const Args& args)
{
  const auto split = args.at("split");
  if (split == "linear") {
    return run<Insertion, spaix::LinearSplit>(params);
  } else if (split == "quadratic") {
    return run<Insertion, spaix::QuadraticSplit>(params);
  }

  throw std::runtime_error("Unknown split algorithm '" + split + "'");
}

int
run(const Parameters& params, const Args& args)
{
  const auto insert = args.at("insert");
  if (insert == "linear") {
    return run<spaix::LinearInsertion>(params, args);
  }

  throw std::runtime_error("Unknown insert algorithm '" + insert + "'");
}

} // namespace

int
main(int argc, char** argv)
{
  const spaix::test::Options opts{
      {"insert", {"Insert (linear)", "ALGORITHM", "linear"}},
      {"page-size", {"Page size for directory nodes", "BYTES", "512"}},
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
