// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_HETEROX_DETAIL_META_HPP
#define SPAIX_HETEROX_DETAIL_META_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include <spaix/detail/Index.hpp>

namespace spaix::heterox::detail {

template<size_t n, typename... Ts>
using Nth = typename std::tuple_element_t<n, std::tuple<Ts...>>;

template<size_t i, size_t n>
struct InclusiveIndex {
  constexpr InclusiveIndex<i + 1, n> operator++() const { return {}; }
};

template<size_t n>
using LastIndex = InclusiveIndex<n, n>;

template<class... Ts>
constexpr ::spaix::detail::Index<0, sizeof...(Ts)>
ibegin() noexcept
{
  return {};
}

template<class... Ts>
constexpr InclusiveIndex<0, sizeof...(Ts) - 1>
ibegin_inclusive() noexcept
{
  return {};
}

template<class... Ts>
using ProductOf = decltype((std::declval<Ts>() * ...));

} // namespace spaix::heterox::detail

#endif // SPAIX_HETEROX_DETAIL_META_HPP
