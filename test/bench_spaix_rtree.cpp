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

#include "BenchParameters.hpp"
#include "Distribution.hpp"
#include "options.hpp"
#include "write_row.hpp"

#include "spaix/LinearInsertion.hpp"
#include "spaix/LinearSplit.hpp"
#include "spaix/QuadraticSplit.hpp"
#include "spaix/RTree.hpp"
#include "spaix/Rect.hpp"
#include "spaix/within.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using Args       = spaix::test::Arguments;
using Parameters = spaix::test::BenchParameters;
using Scalar     = float;
using Data       = float;
using Rect2      = spaix::Rect<Scalar, Scalar>;

template <class T>
using Distribution = spaix::test::Distribution<T>;

struct Counts
{
  size_t n_checked_dirs = 0u;
  size_t n_checked_dats = 0u;
};

template <class QueryKey>
struct BenchmarkWithin
{
  template <class DirKey>
  constexpr bool directory(const DirKey& k) const
  {
    ++counts->n_checked_dirs;
    return spaix::intersects(key, k);
  }

  template <class DatKey>
  constexpr bool leaf(const DatKey& k) const
  {
    ++counts->n_checked_dats;
    return spaix::contains(key, k);
  }

  const QueryKey key;
  Counts*        counts{};
};

constexpr unsigned min_fill_divisor = 4;

template <class Last>
void
write_row(std::ostream& os, Last last)
{
  os << last << '\n';
}

template <class First, class... Rest>
void
write_row(std::ostream& os, First first, Rest... rest)
{
  os << first << '\t';
  write_row(os, std::forward<Rest>(rest)...);
}

struct QueryMetrics
{
  Distribution<double> iter_times;
  Distribution<double> checked_dirs;
  Distribution<double> checked_dats;
  Distribution<double> result_counts;
};

template <class Tree, class Scalar>
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
    const auto x1 = x0 + dist(rng) * query_span;
    const auto y0 = dist(rng) * query_span;
    const auto y1 = y0 + dist(rng) * query_span;

    const typename Rect2::Tuple ranges{{x0, x1}, {y0, y1}};

    size_t n_results = 0u;
    Counts counts    = {0u, 0u};

    const auto predicate = BenchmarkWithin<Rect2>{Rect2{ranges}, &counts};

    const auto t_iter_start = std::chrono::steady_clock::now();
    for (const auto& node : tree.query(predicate)) {
      (void)node;
      ++n_results;
    }
    const auto t_iter_end = std::chrono::steady_clock::now();

    const auto iter_dur =
        std::chrono::duration<double>(t_iter_end - t_iter_start);
    metrics.iter_times.update(
        iter_dur.count()); // / std::max(1.0, double(n_results)));

    metrics.result_counts.update(n_results);
    metrics.checked_dirs.update(counts.n_checked_dirs);
    metrics.checked_dats.update(counts.n_checked_dats);
  }

  return metrics;
}

template <class Tree>
int
run(const Parameters& params, std::ostream& os)
{
  const auto span         = params.span;
  const auto n_per_record = params.n_elements / params.n_steps;

  std::mt19937                           rng{params.seed};
  std::uniform_real_distribution<Scalar> dist{0.0f, span};

  Tree                 t;
  Distribution<double> times;
  Distribution<double> total_times;

  write_row(os,
            "n",
            "page_size",
            "fanout",
            "elapsed",
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
  for (size_t i = 0; i < params.n_elements; ++i) {
    const auto x1 = floor(dist(rng));
    const auto x2 = floor(dist(rng));
    const auto y1 = floor(dist(rng));
    const auto y2 = floor(dist(rng));

    const Rect2 key{{std::min(x1, x2), std::max(x1, x2)},
                    {std::min(y1, y2), std::max(y1, y2)}};

    const auto value = static_cast<Data>(i);

    const auto t_start = std::chrono::steady_clock::now();
    t.insert(std::move(key), std::move(value));
    const auto t_end = std::chrono::steady_clock::now();

    const auto d = std::chrono::duration<double>(t_end - t_start).count();
    times.update(d);
    total_times.update(d);

    if (i % n_per_record == n_per_record - 1) {
      const auto metrics = benchmark_queries(rng, t, span, params.n_queries);

      write_row(os,
                total_times.n(),
                params.page_size,
                t.fanout(),
                std::chrono::duration<double>(t_end - t_bench_start).count(),
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
      times = {};
    }
  }

  return 0;
}

template <class Insertion, class Split, size_t page_size>
int
run(const Parameters& params)
{
  return run<spaix::RTree<Rect2,
                          Scalar,
                          Rect2,
                          spaix::fanout<Rect2>(page_size),
                          min_fill_divisor,
                          Insertion,
                          Split>>(params, std::cout);
}

template <class Insertion, class Split>
int
run(const Parameters& params)
{
  switch (params.page_size) {
  case 64: return run<Insertion, Split, 64>(params);
  case 128: return run<Insertion, Split, 128>(params);
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

  throw std::runtime_error("Unknown algorithm '" + split + "'");
}

int
run(const Parameters& params, const Args& args)
{
  const auto insert = args.at("insert");
  if (insert == "linear") {
    return run<spaix::LinearInsertion>(params, args);
  }

  throw std::runtime_error("Unknown algorithm '" + insert + "'");
}

} // namespace

int
main(int argc, char** argv)
{
  const spaix::test::Options opts{
      {"insert", {"Insert (linear)", "ALGORITHM", "linear"}},
      {"page-size", {"Page size for directory nodes", "BYTES", "128"}},
      {"queries", {"Number of queries per step", "COUNT", "100"}},
      {"seed", {"Random number generator seed", "SEED", "5489"}},
      {"size", {"Maximum number of elements", "ELEMENTS", "1000000"}},
      {"span", {"Dimension span", "NUMBER", "1000000"}},
      {"split", {"Split (linear, quadratic)", "ALGORITHM", "linear"}},
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
