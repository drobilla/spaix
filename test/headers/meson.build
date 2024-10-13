# Copyright 2019-2024 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

warning_level = get_option('warning_level')
header_suppressions = []
if cpp.get_id() in ['clang', 'emscripten']
  if warning_level == 'everything'
    header_suppressions = ['-Wno-c++98-compat-pedantic']

    if not meson.is_cross_build()
      header_suppressions += ['-Wno-poison-system-directories']
    endif
  endif

elif cpp.get_id() == 'gcc'
  if warning_level == 'everything'
    header_suppressions = ['-Wno-effc++', '-Wno-suggest-attribute=const']
  endif

elif cpp.get_id() == 'msvc'
  header_suppressions += [
    '/experimental:external',
    '/external:W0',
    '/external:anglebrackets',
  ]

  if warning_level == 'everything'
    header_suppressions = [
      '/wd4514', # unreferenced inline function has been removed
      '/wd4582', # constructor is not explicitly called
      '/wd4583', # destructor is not explicitly called
      '/wd4623', # default constructor implicitly deleted
      '/wd4625', # copy constructor implicitly deleted
      '/wd4626', # copy assignment operator implicitly deleted
      '/wd4710', # function not inlined
      '/wd4711', # function selected for automatic inline expansion
      '/wd4868', # may not enforce left-to-right evaluation order
      '/wd5027', # move assignment operator implicitly deleted
      '/wd5045', # will insert Spectre mitigation for memory load
    ]
  endif
endif

test(
  'build',
  executable(
    'test_headers',
    files('test_headers.cpp'),
    cpp_args: header_suppressions,
    dependencies: [spaix_dep],
  ),
)
