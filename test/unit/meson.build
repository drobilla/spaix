# Copyright 2019-2024 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

tests = [
  'Point',
  'Rect',
  'contains',
  'expansion',
  'intersects',
  'union',
  'volume',
]

foreach test : tests
  test(
    test,
    executable(
      'test_@0@'.format(test),
      'test_@0@.cpp'.format(test),
      cpp_args: cpp_suppressions,
      dependencies: [spaix_dep, spaix_test_dep],
    ),
    suite: 'unit',
  )
endforeach

test_RTree = executable(
  'test_RTree',
  'test_RTree.cpp',
  cpp_args: cpp_suppressions,
  dependencies: [spaix_dep, spaix_test_dep],
)

test('RTree', test_RTree, suite: 'unit')
test(
  'bad_args',
  test_RTree,
  args: ['--unknown'],
  should_fail: true,
  suite: 'unit',
)
