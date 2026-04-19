// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_INDEX_HPP
#define SPAIX_DETAIL_INDEX_HPP

#include <cstddef>

namespace spaix::detail {

template<size_t i, size_t n>
struct Index {
  constexpr Index<i + 1, n> operator++() const { return {}; }
};

template<size_t n>
using EndIndex = Index<n, n>;

} // namespace spaix::detail

#endif // SPAIX_DETAIL_INDEX_HPP
