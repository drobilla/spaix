# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-only

tests = [
  'test_Point',
  'test_RTree',
  'test_Rect',
  'test_contains',
  'test_expansion',
  'test_intersection',
  'test_intersects',
  'test_union',
  'test_volume',
]

foreach test : tests
  test(test,
       executable(test,
                  test + '.cpp',
                  dependencies: [spaix_dep, spaix_test_dep]),
       suite: 'unit')
endforeach

