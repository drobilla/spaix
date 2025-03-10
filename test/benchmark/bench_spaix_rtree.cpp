// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#include <spaix_test/BenchParameters.hpp>
#include <spaix_test/Distribution.hpp>
#include <spaix_test/options.hpp>
#include <spaix_test/write_row.hpp>

#include <spaix/Config.hpp>
#include <spaix/DataPlacement.hpp>
#include <spaix/LinearInsertion.hpp>
#include <spaix/LinearSplit.hpp>    // IWYU pragma: keep
#include <spaix/QuadraticSplit.hpp> // IWYU pragma: keep
#include <spaix/RTree.hpp>
#include <spaix/Rect.hpp>
#include <spaix/contains.hpp>
#include <spaix/intersects.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <ratio>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using Args       = spaix::test::Arguments;
using Parameters = spaix::test::BenchParameters;
using Scalar     = float;
using Data       = size_t;
using Rect2      = spaix::Rect<Scalar, Scalar>;

template<class T>
using Distribution = spaix::test::Distribution<T>;

using MinFillRatio = std::ratio<3, 10>;

struct Counts {
  size_t n_checked_dirs = 0U;
  size_t n_checked_dats = 0U;
};

struct BenchmarkWithin {
  BenchmarkWithin(Rect2 query_key, Counts* const counts)
    : _query_key{std::move(query_key)}
    , _counts{counts}
  {}

  template<class DirKey>
  [[nodiscard]] constexpr bool directory(const DirKey& k) const
  {
    ++_counts->n_checked_dirs;
    return intersects(_query_key, k);
  }

  template<class DatKey>
  [[nodiscard]] constexpr bool leaf(const DatKey& k) const
  {
    ++_counts->n_checked_dats;
    return contains(_query_key, k);
  }

private:
  Rect2   _query_key;
  Counts* _counts;
};

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
  QueryMetrics                           metrics{};
  std::uniform_real_distribution<Scalar> dist{0, 1};
  const auto                             query_span{span / 2};

  for (size_t i = 0; i < n_queries; ++i) {
    const auto x0 = dist(rng) * query_span;
    const auto x1 = x0 + (dist(rng) * query_span);
    const auto y0 = dist(rng) * query_span;
    const auto y1 = y0 + (dist(rng) * query_span);

    size_t n_results = 0U;
    Counts counts    = {0U, 0U};

    const auto predicate = BenchmarkWithin{Rect2{{x0, x1}, {y0, y1}}, &counts};

    const auto t_iter_start = std::chrono::steady_clock::now();
#if 0
    for (const auto& node : tree.query(predicate)) {
      volatile auto result = node;

      (void)result;
      ++n_results;
    }
#else
    tree.fast_query(predicate, [&n_results](const auto& node) {
      volatile auto result = node;

      (void)result;
      ++n_results;
    });
#endif
    const auto t_iter_end = std::chrono::steady_clock::now();

    const auto iter_dur =
      std::chrono::duration<double>(t_iter_end - t_iter_start);

    metrics.iter_times.update(iter_dur.count());
    metrics.result_counts.update(static_cast<double>(n_results));
    metrics.checked_dirs.update(static_cast<double>(counts.n_checked_dirs));
    metrics.checked_dats.update(static_cast<double>(counts.n_checked_dats));
  }

  return metrics;
}

template<class Tree>
int
run(const Parameters& params, std::ostream& os)
{
  using Seconds = std::chrono::duration<double>;

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

    const Rect2 key{{std::min(x1, x2), std::max(x1, x2)},
                    {std::min(y1, y2), std::max(y1, y2)}};

    const auto value = i;

    const auto t_start = std::chrono::steady_clock::now();
    t.insert(key, value);
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
                             Tree::Conf::dir_fanout,
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

template<class Insertion,
         class Split,
         spaix::DataPlacement placement,
         size_t               page_size>
int
run(const Parameters& params)
{
  using Conf = spaix::Config<
    spaix::PageStructure<Rect2, Rect2, Data, page_size, placement>,
    Split,
    Insertion,
    MinFillRatio>;

  return run<spaix::RTree<Rect2, Rect2, Data, Conf>>(params, std::cout);
}

template<class Insertion, class Split, spaix::DataPlacement placement>
int
run(const Parameters& params)
{
  switch (params.page_size) {
  case 256:
    return run<Insertion, Split, placement, 256>(params);
  case 512:
    return run<Insertion, Split, placement, 512>(params);
  case 1024:
    return run<Insertion, Split, placement, 1024>(params);
  case 2048:
    return run<Insertion, Split, placement, 2048>(params);
  case 4096:
    return run<Insertion, Split, placement, 4096>(params);
  case 8192:
    return run<Insertion, Split, placement, 8192>(params);
  }

  throw std::runtime_error("Invalid page size '" +
                           std::to_string(params.page_size) + "'");
}

template<class Insertion, class Split>
int
run(const Parameters& params, const Args& args)
{
  const auto placement = args.at("placement");
  if (placement == "inline") {
    return run<Insertion, Split, spaix::DataPlacement::inlined>(params);
  }

  if (placement == "separate") {
    return run<Insertion, Split, spaix::DataPlacement::separate>(params);
  }

  throw std::runtime_error("Invalid placement '" + placement + "'");
}

template<class Insertion>
int
run(const Parameters& params, const Args& args)
{
  const auto split = args.at("split");
  if (split == "linear") {
    return run<Insertion, spaix::LinearSplit>(params, args);
  }

  if (split == "quadratic") {
    return run<Insertion, spaix::QuadraticSplit>(params, args);
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
    {"placement",
     {"Data placement (inline or separate)", "PLACEMENT", "inline"}},
    {"queries", {"Number of queries per step", "COUNT", "100"}},
    {"seed", {"Random number generator seed", "SEED", "5489"}},
    {"size", {"Maximum number of elements", "ELEMENTS", "1000000"}},
    {"span", {"Dimension span", "NUMBER", "10000000"}},
    {"split", {"Split (linear, quadratic)", "ALGORITHM", "quadratic"}},
    {"steps", {"Number of benchmarking steps", "RECORDS", "10"}},
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
