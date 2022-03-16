// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SEARCH_EVERYTHING_HPP
#define SPAIX_SEARCH_EVERYTHING_HPP

namespace spaix::search {

struct Everything {
  template<class DirKey>
  static constexpr bool directory(const DirKey&)
  {
    return true;
  }

  template<class DatKey>
  static constexpr bool leaf(const DatKey&)
  {
    return true;
  }
};

/// Return a tree query predicate that matches everything
constexpr Everything
everything()
{
  return Everything{};
}

} // namespace spaix::search

#endif // SPAIX_SEARCH_EVERYTHING_HPP
