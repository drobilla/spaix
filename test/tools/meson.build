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

test('fanout_4', draw_tree, args: ['--fanout', '4'], suite: 'draw')
test('fanout_8', draw_tree, args: ['--fanout', '8'], suite: 'draw')
test('fanout_12', draw_tree, args: ['--fanout', '12'], suite: 'draw')
test('fanout_16', draw_tree, args: ['--fanout', '16'], suite: 'draw')
test('fanout_20', draw_tree, args: ['--fanout', '20'], suite: 'draw')
test('fanout_24', draw_tree, args: ['--fanout', '24'], suite: 'draw')

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
  'bad_low_fanout',
  draw_tree,
  args: ['--fanout', '3'],
  should_fail: true,
  suite: 'draw',
)
test(
  'bad_high_fanout',
  draw_tree,
  args: ['--fanout', '25'],
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
