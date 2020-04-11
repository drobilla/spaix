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

#ifndef SPAIX_META_HPP
#define SPAIX_META_HPP

#include <cstddef>
#include <ostream>
#include <tuple>
#include <type_traits>

#if defined(__GNUC__)
#  define SPAIX_ALWAYS_INLINE __attribute__((always_inline))
#else
#  define SPAIX_ALWAYS_INLINE
#endif

namespace spaix {

template <size_t n, typename... Ts>
using Nth = typename std::tuple_element<n, std::tuple<Ts...>>::type;

template <class... Ts>
using CommonType = typename std::common_type<Ts...>::type;

template <class Tuple>
struct CommonScalarType
{
};

template <class... Ts>
struct CommonScalarType<std::tuple<Ts...>>
{
  using type = CommonType<Ts...>;
};

template <size_t i, size_t n>
struct Index
{
  constexpr Index<i + 1, n> operator++() const { return {}; }
};

template <size_t n>
using EndIndex = Index<n, n>;

template <size_t i, size_t n>
struct InclusiveIndex
{
  constexpr InclusiveIndex<i + 1, n> operator++() const { return {}; }
};

template <size_t n>
using LastIndex = InclusiveIndex<n, n>;

template <class... Ts>
constexpr Index<0, sizeof...(Ts)>
ibegin()
{
  return {};
}

template <class... Ts>
constexpr InclusiveIndex<0, sizeof...(Ts) - 1>
ibegin_inclusive()
{
  return {};
}

template <class... Ts, size_t last>
constexpr auto
tuple_product(const std::tuple<Ts...>& tuple, InclusiveIndex<last, last>)
{
  return std::get<last>(tuple);
}

template <class... Ts, size_t i, size_t last>
constexpr auto
tuple_product(const std::tuple<Ts...>& tuple, InclusiveIndex<i, last> index)
{
  return std::get<i>(tuple) * tuple_product(tuple, ++index);
}

template <class... Ts>
using ProductOf =
    decltype(tuple_product<Ts...>(std::declval<std::tuple<Ts...>>(),
                                  ibegin_inclusive<Ts...>()));

} // namespace spaix

#endif // SPAIX_POINT_HPP
