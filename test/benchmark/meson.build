# Copyright 2019-2026 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

boost_dep = disabler()
if not get_option('boost').disabled()
  boost_dep = dependency(
    'boost',
    include_type: 'system',
    required: get_option('boost')
  )
endif

bench_exe = executable(
  'bench_spaix_rtree',
  'bench_spaix_rtree.cpp',
  cpp_args: cpp_suppressions,
  dependencies: [spaix_dep, spaix_test_dep],
)

if boost_dep.found()
  boost_bench_exe = executable(
    'bench_boost_rtree',
    'bench_boost_rtree.cpp',
    cpp_args: cpp_suppressions,
    dependencies: [boost_dep, spaix_dep, spaix_test_dep],
  )
endif

test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--insert', 'linear'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--split', 'linear'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--split', 'quadratic'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--fanout', '4'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--fanout', '8'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--fanout', '12'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--fanout', '16'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--fanout', '20'],
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--fanout', '24'],
  suite: 'benchmark',
)

test(
  'bench',
  bench_exe,
  args: ['--size', '16', '--placement', 'separate'],
  suite: 'benchmark',
)

test(
  'bench',
  bench_exe,
  args: ['--insert', 'unknown'],
  should_fail: true,
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--split', 'unknown'],
  should_fail: true,
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--placement', 'unknown'],
  should_fail: true,
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--fanout', '3'],
  should_fail: true,
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--fanout', '25'],
  should_fail: true,
  suite: 'benchmark',
)
test(
  'bench',
  bench_exe,
  args: ['--bad-arg'],
  should_fail: true,
  suite: 'benchmark',
)
