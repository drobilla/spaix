# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

tests = [
  'test_Point',
  'test_RTree',
  'test_Rect',
  'test_contains',
  'test_expansion',
  'test_intersects',
  'test_union',
  'test_volume',
]

foreach test : tests
  test(
    test,
    executable(
      test,
      test + '.cpp',
      cpp_args: cpp_suppressions,
      dependencies: [spaix_dep, spaix_test_dep],
    ),
    suite: 'unit',
  )
endforeach

