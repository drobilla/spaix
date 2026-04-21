// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_BENCHPARAMETERS_HPP
#define SPAIX_TEST_BENCHPARAMETERS_HPP

#include "options.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace spaix::test {

struct BenchParameters {
  explicit BenchParameters(const spaix::test::Arguments& args)
    : n_elements{std::stoul(args.at("size"))}
    , n_queries{std::stoul(args.at("queries"))}
    , n_steps{std::stoul(args.at("steps"))}
    , span{std::stof(args.at("span"))}
    , seed{static_cast<uint32_t>(std::stoul(args.at("seed")))}
    , fanout{static_cast<uint32_t>(std::stoul(args.at("fanout")))}
  {}

  size_t   n_elements;
  size_t   n_queries;
  size_t   n_steps;
  float    span;
  uint32_t seed;
  uint32_t fanout;
};

} // namespace spaix::test

#endif // SPAIX_TEST_BENCHPARAMETERS_HPP
