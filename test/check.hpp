// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TEST_CHECK_HPP
#define TEST_CHECK_HPP

#undef NDEBUG

#include <cassert>

#ifdef _MSC_VER
#  define STATIC_CHECK(condition)     \
    do {                              \
      assert(condition); /* NOLINT */ \
    } while (0)
#else
#  define STATIC_CHECK(condition)     \
    do {                              \
      static_assert(condition);       \
      assert(condition); /* NOLINT */ \
    } while (0)
#endif

#define CHECK(condition) assert(condition)

#endif // TEST_CHECK_HPP
