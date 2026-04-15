# Copyright 2019-2026 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

draw_tree = executable(
  'draw_tree',
  files('draw_tree.cpp'),
  cpp_args: cpp_suppressions,
  dependencies: [spaix_dep, spaix_test_dep],
)

test('default', draw_tree, suite: 'draw')
test('linear_insert', draw_tree, args: ['--insert', 'linear'], suite: 'draw')

test('linear_split', draw_tree, args: ['--split', 'linear'], suite: 'draw')
test(
  'quadratic_split',
  draw_tree,
  args: ['--split', 'quadratic'],
  suite: 'draw',
)

test('page_256', draw_tree, args: ['--page-size', '256'], suite: 'draw')
test('page_512', draw_tree, args: ['--page-size', '512'], suite: 'draw')
test('page_1024', draw_tree, args: ['--page-size', '1024'], suite: 'draw')
test('page_2048', draw_tree, args: ['--page-size', '2048'], suite: 'draw')
test('page_4096', draw_tree, args: ['--page-size', '4096'], suite: 'draw')

test(
  'bad_insert',
  draw_tree,
  args: ['--insert', 'unknown'],
  should_fail: true,
  suite: 'draw',
)
test(
  'bad_split',
  draw_tree,
  args: ['--split', 'unknown'],
  should_fail: true,
  suite: 'draw',
)
test(
  'bad_small_page',
  draw_tree,
  args: ['--page-size', '128'],
  should_fail: true,
  suite: 'draw',
)
test(
  'bad_large_page',
  draw_tree,
  args: ['--page-size', '8192'],
  should_fail: true,
  suite: 'draw',
)
test(
  'bad_option',
  draw_tree,
  args: ['--bad-arg'],
  should_fail: true,
  suite: 'draw',
)
