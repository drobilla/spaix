# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-only

autoship = find_program('autoship', required: false)
if autoship.found()
  test('autoship', autoship, args: ['test', spaix_src_root], suite: 'data')
endif

reuse = find_program('reuse', required: false)
if reuse.found()
  test('REUSE', reuse, args: ['--root', spaix_src_root, 'lint'], suite: 'data')
endif

test('build', executable('test_build',
                         'test_build.cpp',
                         dependencies: [spaix_dep]))

subdir('unit')
