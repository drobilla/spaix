// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_ATTRIBUTES_HPP
#define SPAIX_DETAIL_ATTRIBUTES_HPP

#ifdef __GNUC__
#  define SPAIX_ALWAYS_INLINE __attribute__((always_inline))
#else
#  define SPAIX_ALWAYS_INLINE
#endif

#endif // SPAIX_DETAIL_ATTRIBUTES_HPP
