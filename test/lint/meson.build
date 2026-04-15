# Copyright 2019-2026 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

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
