// Copyright 2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_CONCEPTS_HPP
#define SPAIX_CONCEPTS_HPP

namespace spaix {

#ifndef SPAIX_NO_CONCEPTS
#  define SPAIX_USE_CONCEPTS __cplusplus >= 202002L
#endif

#if SPAIX_USE_CONCEPTS

/**
   Has a static bool contains(Box, Key) method for some Key.

   Required operation for search::Within.
*/
template<typename Comps, class Key>
concept ChecksContains = requires(typename Comps::Box box, Key key) {
  { Comps::contains(box, key) } -> std::same_as<bool>;
};

/**
   Has a static bool intersects(Box, Key) method for some key.

   Required operation for any search.
*/
template<typename Comps, class Key>
concept ChecksIntersects = requires(typename Comps::Box box, Key key) {
  { Comps::intersects(box, key) } -> std::same_as<bool>;
};

/**
   Has static lower<dim>(Key) and upper<dim>(Key) methods for some Key.

   Required operation for #LinearSplit.
*/
template<typename Ops, class Key>
concept MeasuresSpan = requires(Key key) {
  {
    Ops::template lower<0>(key)
  } -> std::same_as<decltype(Ops::template lower<0>(key))>;
  {
    Ops::template upper<0>(key)
  } -> std::same_as<decltype(Ops::template upper<0>(key))>;
};

/**
   Has a static Volume volume(Box) method.

   Required operation for any insertion.
*/
template<typename Ops>
concept MeasuresVolume = requires(typename Ops::Box box) {
  { Ops::volume(box) } -> std::same_as<typename Ops::Volume>;
};

/**
   Has a static Box unify<Key>(Box, Key) method.

   Required operation for any insertion.
*/
template<typename Ops, class Key>
concept Unifies = requires(typename Ops::Box box, Key key) {
  { Ops::unify(box, key) } -> std::same_as<typename Ops::Box>;
};

#endif

} // namespace spaix

#endif // SPAIX_CONCEPTS_HPP
