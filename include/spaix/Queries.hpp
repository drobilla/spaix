// Copyright 2022-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_QUERIES_HPP
#define SPAIX_QUERIES_HPP

#include <spaix/search/Everything.hpp>
#include <spaix/search/Exactly.hpp>
#include <spaix/search/Touching.hpp>
#include <spaix/search/Within.hpp>

namespace spaix {

template<class Comparisons>
struct Queries {
  using Box = typename Comparisons::Box;

  static constexpr search::Everything everything()
  {
    return search::Everything{};
  }

  template<class K>
  static constexpr search::Exactly<Comparisons, K> exactly(const K& k)
  {
    return search::Exactly<Comparisons, K>{k};
  }

  template<class K>
  static constexpr search::Within<Comparisons, Box> within(const K& k)
  {
    return search::Within<Comparisons, K>{k};
  }

  template<class K>
  static constexpr search::Touching<Comparisons, Box> touching(const K& k)
  {
    return search::Touching<Comparisons, K>{k};
  }
};

} // namespace spaix

#endif // SPAIX_QUERIES_HPP
