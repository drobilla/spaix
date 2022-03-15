// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_META_HPP
#define SPAIX_DETAIL_META_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

#if defined(__GNUC__)
#  define SPAIX_ALWAYS_INLINE __attribute__((always_inline))
#else
#  define SPAIX_ALWAYS_INLINE
#endif

namespace spaix {

template<size_t n, typename... Ts>
using Nth = typename std::tuple_element<n, std::tuple<Ts...>>::type;

template<class... Ts>
using CommonType = typename std::common_type<Ts...>::type;

template<class Tuple>
struct CommonScalarType {};

template<class... Ts>
struct CommonScalarType<std::tuple<Ts...>> {
  using type = CommonType<Ts...>;
};

template<size_t i, size_t n>
struct Index {
  constexpr Index<i + 1, n> operator++() const { return {}; }
};

template<size_t n>
using EndIndex = Index<n, n>;

template<size_t i, size_t n>
struct InclusiveIndex {
  constexpr InclusiveIndex<i + 1, n> operator++() const { return {}; }
};

template<size_t n>
using LastIndex = InclusiveIndex<n, n>;

template<class... Ts>
constexpr Index<0, sizeof...(Ts)>
ibegin()
{
  return {};
}

template<class... Ts>
constexpr InclusiveIndex<0, sizeof...(Ts) - 1>
ibegin_inclusive()
{
  return {};
}

template<class... Ts, size_t last>
constexpr auto
tuple_product(const std::tuple<Ts...>& tuple, InclusiveIndex<last, last>)
{
  return std::get<last>(tuple);
}

template<class... Ts, size_t i, size_t last>
constexpr auto
tuple_product(const std::tuple<Ts...>& tuple, InclusiveIndex<i, last> index)
{
  return std::get<i>(tuple) * tuple_product(tuple, ++index);
}

template<class... Ts>
using ProductOf =
  decltype(tuple_product<Ts...>(std::declval<std::tuple<Ts...>>(),
                                ibegin_inclusive<Ts...>()));

} // namespace spaix

#endif // SPAIX_DETAIL_META_HPP
