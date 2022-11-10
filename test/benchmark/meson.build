# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-only

bench_exe = executable(
  'bench_spaix_rtree',
  'bench_spaix_rtree.cpp',
  cpp_args: cpp_suppressions,
  dependencies: [spaix_dep, spaix_test_dep],
)

boost_dep = dependency('boost', required: get_option('benchmarks'))

if boost_dep.found()
  boost_bench_exe = executable(
    'bench_boost_rtree',
    'bench_boost_rtree.cpp',
    cpp_args: cpp_suppressions,
    dependencies: [boost_dep, spaix_dep, spaix_test_dep],
  )
endif

test('bench', bench_exe, args: ['--size', '16', '--insert', 'linear'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--split', 'linear'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--split', 'quadratic'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--page-size', '256'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--page-size', '512'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--page-size', '1024'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--page-size', '2048'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--page-size', '4096'], suite: 'benchmark')
test('bench', bench_exe, args: ['--size', '16', '--page-size', '8192'], suite: 'benchmark')

test('bench', bench_exe, args: ['--insert', 'unknown'], should_fail: true, suite: 'benchmark')
test('bench', bench_exe, args: ['--split', 'unknown'], should_fail: true, suite: 'benchmark')
test('bench', bench_exe, args: ['--page-size', '128'], should_fail: true, suite: 'benchmark')
test('bench', bench_exe, args: ['--page-size', '16384'], should_fail: true, suite: 'benchmark')
test('bench', bench_exe, args: ['--bad-arg'], should_fail: true, suite: 'benchmark')
