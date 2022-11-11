// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include "spaix_test/BenchParameters.hpp"
#include "spaix_test/Distribution.hpp"
#include "spaix_test/options.hpp"
#include "spaix_test/write_row.hpp"

#include "spaix/types.hpp"

#ifdef __APPLE__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Weverything\"")
#endif

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>

#ifdef __APPLE__
_Pragma("clang diagnostic pop")
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

// IWYU pragma: no_include <map>

namespace {
namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;

using Args       = spaix::test::Arguments;
using Parameters = spaix::test::BenchParameters;
using Scalar     = float;
using Data       = size_t;

using Point = bg::model::point<Scalar, 2, bg::cs::cartesian>;
using Box   = bg::model::box<Point>;

template<class T>
using Distribution = spaix::test::Distribution<T>;

constexpr unsigned min_fill_divisor = 3;

struct QueryMetrics {
  Distribution<double> iter_times;
  Distribution<double> checked_dirs;
  Distribution<double> checked_dats;
  Distribution<double> result_counts;
};

template<class Tree, class Scalar>
QueryMetrics
benchmark_queries(std::mt19937& rng,
                  const Tree&   tree,
                  const Scalar  span,
                  const size_t  n_queries)
{
  (void)rng;
  (void)tree;
  (void)span;
  (void)n_queries;

  QueryMetrics                           metrics{};
  std::uniform_real_distribution<Scalar> dist{0, 1};
  const auto                             query_span{span / 2};

  for (size_t i = 0; i < n_queries; ++i) {
    const auto x0 = dist(rng) * query_span;
    const auto x1 = x0 + dist(rng) * query_span;
    const auto y0 = dist(rng) * query_span;
    const auto y1 = y0 + dist(rng) * query_span;

    size_t n_results = 0U;

    const Box query_box{{x0, y0}, {x1, y1}};

    const auto t_iter_start = std::chrono::steady_clock::now();
    auto       iter         = tree.qbegin(bgi::within(query_box));

    while (iter != tree.qend()) {
      volatile Box  box   = iter->first;
      volatile Data value = iter->second;

      (void)box;
      (void)value;

      ++n_results;
      ++iter;
    }

    const auto t_iter_end = std::chrono::steady_clock::now();

    const auto iter_dur =
      std::chrono::duration<double>(t_iter_end - t_iter_start);
    metrics.iter_times.update(
      iter_dur.count()); // / std::max(1.0, double(n_results)));

    metrics.result_counts.update(static_cast<double>(n_results));
  }

  return metrics;
}

template<class Algorithm>
int
run(const Parameters& params)
{
  using Seconds = std::chrono::duration<double>;

  using Value = std::pair<Box, Data>;
  using Tree  = bgi::rtree<Value, Algorithm>;

  auto&      os           = std::cout;
  const auto span         = params.span;
  const auto n_per_record = params.n_elements / params.n_steps;

  std::mt19937                           rng{params.seed};
  std::uniform_real_distribution<Scalar> dist{0.0f, span};

  Tree                 t;
  Distribution<double> times;
  Distribution<double> total_times;

  spaix::test::write_row(os,
                         "n",
                         "page_size",
                         "fanout",
                         "elapsed",
                         "throughput",
                         "t_ins",
                         "t_ins_min",
                         "t_ins_max",
                         "t_iter",
                         "t_iter_min",
                         "t_iter_max",
                         "q_dirs",
                         "q_dirs_min",
                         "q_dirs_max",
                         "q_dats",
                         "q_dats_min",
                         "q_dats_max",
                         "n_results");

  const auto t_bench_start = std::chrono::steady_clock::now();
  auto       last_row_time = t_bench_start;
  size_t     last_row_n    = 0U;
  for (size_t i = 0; i < params.n_elements; ++i) {
    const auto x1 = floorf(dist(rng));
    const auto x2 = floorf(dist(rng));
    const auto y1 = floorf(dist(rng));
    const auto y2 = floorf(dist(rng));

    const Box key{{std::min(x1, x2), std::min(y1, y2)},
                  {std::max(x1, x2), std::max(y1, y2)}};

    const auto value = i;

    const auto t_start = std::chrono::steady_clock::now();
    t.insert(std::make_pair(key, value));
    const auto t_end = std::chrono::steady_clock::now();

    const auto d = Seconds(t_end - t_start).count();
    times.update(d);
    total_times.update(d);

    if (i % n_per_record == n_per_record - 1) {
      const auto metrics = benchmark_queries(rng, t, span, params.n_queries);

      const auto total_elapsed = t_end - t_bench_start;
      const auto row_elapsed   = t_end - last_row_time;
      const auto row_count     = static_cast<double>(i - last_row_n);

      spaix::test::write_row(os,
                             total_times.n(),
                             params.page_size,
                             t.parameters().max_elements,
                             Seconds(total_elapsed).count(),
                             row_count / Seconds(row_elapsed).count(),
                             times.mean(),
                             times.min(),
                             times.max(),
                             metrics.iter_times.mean(),
                             metrics.iter_times.min(),
                             metrics.iter_times.max(),
                             metrics.checked_dirs.mean(),
                             metrics.checked_dirs.min(),
                             metrics.checked_dirs.max(),
                             metrics.checked_dats.mean(),
                             metrics.checked_dats.min(),
                             metrics.checked_dats.max(),
                             metrics.result_counts.mean());
      times         = {};
      last_row_n    = i;
      last_row_time = t_end;
    }
  }

  return 0;
}

template<size_t page_size>
int
run(const Parameters& params, const Args& args)
{
  static constexpr auto overhead =
    sizeof(spaix::NodeType) + sizeof(spaix::ChildCount);

  static constexpr auto entry_space    = page_size - overhead;
  static constexpr auto dir_entry_size = sizeof(Box) + sizeof(void*);
  static constexpr auto dir_fanout     = entry_space / dir_entry_size;

  constexpr auto max_fill = dir_fanout;
  constexpr auto min_fill = std::max(size_t(1), max_fill / min_fill_divisor);

  const auto split = args.at("split");
  if (split == "linear") {
    return run<bgi::linear<max_fill, min_fill>>(params);
  }

  if (split == "quadratic") {
    return run<bgi::quadratic<max_fill, min_fill>>(params);
  }

  throw std::runtime_error("Unknown algorithm '" + split + "'");
}

int
run(const Parameters& params, const Args& args)
{
  switch (params.page_size) {
  // case 128: return run<128>(params, args);
  case 256:
    return run<256>(params, args);
  case 512:
    return run<512>(params, args);
  case 1024:
    return run<1024>(params, args);
  case 2048:
    return run<2048>(params, args);
  case 4096:
    return run<4096>(params, args);
  case 8192:
    return run<8192>(params, args);
  }

  throw std::runtime_error("Invalid page size '" +
                           std::to_string(params.page_size) + "'");
}

} // namespace

int
main(int argc, char** argv)
{
  const spaix::test::Options opts{
    {"insert", {"Insert (linear, quadratic)", "ALGORITHM", "linear"}},
    {"page-size", {"Page size for directory nodes", "BYTES", "512"}},
    {"placement", {"Data placement (ignored)", "PLACEMENT", "inline"}},
    {"queries", {"Number of queries per step", "COUNT", "100"}},
    {"seed", {"Random number generator seed", "SEED", "5489"}},
    {"size", {"Maximum number of elements", "ELEMENTS", "1000000"}},
    {"span", {"Dimension span", "NUMBER", "10000000"}},
    {"split", {"Split (linear, quadratic)", "ALGORITHM", "quadratic"}},
    {"steps", {"Number of benchmarking steps", "RECORDS", "10"}},
  };

  try {
    const auto args = parse_options(opts, argc, argv);

    if (args.at("insert") != "linear") {
      std::cerr << "warning: boost rtree only has linear insert selection\n";
      return 0;
    }

    run(Parameters{args}, args);

  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n\n";
    print_usage(argv[0], opts);
    return 1;
  }

  return 0;
}
