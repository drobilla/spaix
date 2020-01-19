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

#ifndef TEST_BENCHPARAMETERS_HPP
#define TEST_BENCHPARAMETERS_HPP

#include "options.hpp"

#include <cstddef>
#include <cstdint>

namespace spaix {
namespace test {

struct BenchParameters
{
  BenchParameters(const spaix::test::Arguments& args)
    : n_elements{static_cast<size_t>(std::stoul(args.at("size")))}
    , n_queries{static_cast<size_t>(std::stoul(args.at("queries")))}
    , n_steps{static_cast<size_t>(std::stoul(args.at("steps")))}
    , span{std::stof(args.at("span"))}
    , seed{static_cast<uint32_t>(std::stoul(args.at("seed")))}
    , page_size{static_cast<size_t>(std::stoul(args.at("page-size")))}
  {
  }

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
