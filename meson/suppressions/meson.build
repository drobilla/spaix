# Copyright 2020-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

# Project-specific warning suppressions.
#
# This should be used in conjunction with the generic "warnings" sibling that
# enables all reasonable warnings for the compiler.  It lives here just to keep
# the top-level meson.build more readable.

#######
# C++ #
#######

if is_variable('cpp')
  cpp_suppressions = []

  if get_option('strict')
    if cpp.get_id() in ['clang', 'emscripten']
      cpp_suppressions = [
        '-Wno-float-equal',
        '-Wno-implicit-int-float-conversion',
        '-Wno-padded',
      ]

    elif cpp.get_id() == 'gcc'
      cpp_suppressions = [
        '-Wno-conversion',
        '-Wno-effc++',
        '-Wno-float-equal',
        '-Wno-inline',
        '-Wno-padded',
        '-Wno-suggest-attribute=const',
        '-Wno-suggest-attribute=pure',
        '-Wno-switch-default',
      ]

    elif cpp.get_id() == 'msvc'
      cpp_suppressions = [
        '/wd4514', # unreferenced inline function has been removed
        '/wd4582', # constructor is not explicitly called
        '/wd4583', # destructor is not explicitly called
        '/wd4623', # default constructor implicitly deleted
        '/wd4625', # copy constructor implicitly deleted
        '/wd4626', # copy assignment operator implicitly deleted
        '/wd4710', # function not inlined
        '/wd4711', # function selected for automatic inline expansion
        '/wd4820', # padding added after construct
        '/wd4868', # may not enforce left-to-right evaluation order
        '/wd5027', # move assignment operator implicitly deleted
        '/wd5045', # will insert Spectre mitigation for memory load
        '/wd5219', # implicit conversion to float
      ]
    endif
  endif

  if cpp.get_id() in ['clang', 'emscripten']
    cpp_suppressions += [
      '-Wno-nullability-extension',
    ]
  endif

  cpp_suppressions = cpp.get_supported_arguments(cpp_suppressions)
endif
