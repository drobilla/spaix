# Copyright 2019-2024 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

spaix_test_dep = declare_dependency(
  include_directories: include_directories('include'),
)

if get_option('lint')
  if not meson.is_subproject()
    # Check project metadata
    autoship = find_program('autoship', required: false)
    if autoship.found()
      test('autoship', autoship, args: ['test', spaix_src_root], suite: 'data')
    endif
  endif

  # Check licensing metadata
  reuse = find_program('reuse', required: false)
  if reuse.found()
    test(
      'REUSE',
      reuse,
      args: ['--root', spaix_src_root, 'lint'],
      suite: 'data',
    )
  endif

  # Check scripts for errors with pylint
  pylint = find_program('pylint', required: false)
  if pylint.found()
    pymod = import('python')
    plot_py = pymod.find_installation(
      'python3',
      modules: ['matplotlib', 'pandas'],
      required: false,
    )

    if plot_py.found()
      test(
        'pylint',
        pylint,
        args: files('../scripts/benchmark.py'),
        suite: 'scripts',
      )
    endif
  endif
endif

# Test that all headers build cleanly
subdir('headers')

# Run unit tests
subdir('unit')

# Build command line utilities and run their tests
if get_option('tools')
  draw_exe = executable(
    'draw_tree',
    'tools/draw_tree.cpp',
    cpp_args: cpp_suppressions,
    dependencies: [spaix_dep, spaix_test_dep],
  )

  if not get_option('tests').disabled()
    test('default', draw_exe, suite: 'draw')
    test('linear_insert', draw_exe, args: ['--insert', 'linear'], suite: 'draw')
    test('linear_split', draw_exe, args: ['--split', 'linear'], suite: 'draw')
    test(
      'quadratic_split',
      draw_exe,
      args: ['--split', 'quadratic'],
      suite: 'draw',
    )
    test('page_256', draw_exe, args: ['--page-size', '256'], suite: 'draw')
    test('page_512', draw_exe, args: ['--page-size', '512'], suite: 'draw')
    test('page_1024', draw_exe, args: ['--page-size', '1024'], suite: 'draw')
    test('page_2048', draw_exe, args: ['--page-size', '2048'], suite: 'draw')
    test('page_4096', draw_exe, args: ['--page-size', '4096'], suite: 'draw')

    test(
      'bad_insert',
      draw_exe,
      args: ['--insert', 'unknown'],
      should_fail: true,
      suite: 'draw',
    )
    test(
      'bad_split',
      draw_exe,
      args: ['--split', 'unknown'],
      should_fail: true,
      suite: 'draw',
    )
    test(
      'bad_small_page',
      draw_exe,
      args: ['--page-size', '128'],
      should_fail: true,
      suite: 'draw',
    )
    test(
      'bad_large_page',
      draw_exe,
      args: ['--page-size', '8192'],
      should_fail: true,
      suite: 'draw',
    )
    test(
      'bad_option',
      draw_exe,
      args: ['--bad-arg'],
      should_fail: true,
      suite: 'draw',
    )
  endif
endif

# Build benchmarks
if not get_option('benchmarks').disabled()
  subdir('benchmark')
endif
