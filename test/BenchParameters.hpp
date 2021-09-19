// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TEST_BENCHPARAMETERS_HPP
#define TEST_BENCHPARAMETERS_HPP

#include "options.hpp"

#include <cstddef>
#include <cstdint>

namespace spaix {
namespace test {

struct BenchParameters {
  explicit BenchParameters(const spaix::test::Arguments& args)
    : n_elements{std::stoul(args.at("size"))}
    , n_queries{std::stoul(args.at("queries"))}
    , n_steps{std::stoul(args.at("steps"))}
    , span{std::stof(args.at("span"))}
    , seed{static_cast<uint32_t>(std::stoul(args.at("seed")))}
    , page_size{std::stoul(args.at("page-size"))}
  {}

  size_t   n_elements;
  size_t   n_queries;
  size_t   n_steps;
  float    span;
  uint32_t seed;
  size_t   page_size;
};

} // namespace test
} // namespace spaix

#endif // TEST_BENCHPARAMETERS_HPP
