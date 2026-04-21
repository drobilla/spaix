# Copyright 2019-2026 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

######################
# Compiler and Flags #
######################

cpp = meson.get_compiler('cpp')
warning_level = get_option('warning_level')
cpp_suppressions = []
if cpp.get_id() in ['clang', 'emscripten']
  if warning_level == 'everything'
    cpp_suppressions = [
      '-Wno-c++98-compat-pedantic',
      '-Wno-float-equal',
      '-Wno-implicit-int-float-conversion',
      '-Wno-padded',
      '-Wno-switch-default',
      '-Wno-unsafe-buffer-usage',
    ]

    if not meson.is_cross_build()
      cpp_suppressions += [
        '-Wno-poison-system-directories',
      ]
    endif
  endif

elif cpp.get_id() == 'gcc'
  if warning_level == 'everything'
    cpp_suppressions = [
      '-Wno-conversion',
      '-Wno-effc++',
      '-Wno-float-equal',
      '-Wno-inline',
      '-Wno-maybe-uninitialized',  # Bug
      '-Wno-padded',
      '-Wno-suggest-attribute=const',
      '-Wno-suggest-attribute=pure',
      '-Wno-switch-default',
    ]
  endif

  if host_machine.system() == 'windows'
    cpp_suppressions += [
      '-Wno-strict-overflow',
      '-Wno-useless-cast',
    ]
  endif

elif cpp.get_id() == 'msvc'
  if warning_level == 'everything'
    cpp_suppressions += [
      '/wd4514',  # unreferenced inline function has been removed
      '/wd4623',  # default constructor implicitly deleted
      '/wd4625',  # copy constructor implicitly deleted
      '/wd4626',  # copy assignment operator implicitly deleted
      '/wd4686',  # change in UDT return calling convention
      '/wd4710',  # function not inlined
      '/wd4711',  # function selected for automatic inline expansion
      '/wd4820',  # padding added after construct
      '/wd4868',  # may not enforce left-to-right evaluation order
      '/wd5027',  # move assignment operator implicitly deleted
      '/wd5045',  # will insert Spectre mitigation for memory load
      '/wd5219',  # implicit conversion to float
    ]
  endif

  if warning_level in ['everything', '3']
    cpp_suppressions += [
      '/wd4324',  # structure was padded due to alignment specifier
    ]
  endif
endif

cpp_suppressions = cpp.get_supported_arguments(cpp_suppressions)

########################
# Test Utility Library #
########################

spaix_test_dep = declare_dependency(
  include_directories: include_directories('include'),
)

########
# Lint #
########

# Test that all headers build cleanly
subdir('headers')

# Run unit tests
subdir('unit')

# Check project metadata and scripts
if get_option('lint')
  subdir('lint')
endif

# Build command line utilities and run their tests
if not get_option('tools').disabled()
  subdir('tools')
endif

# Build benchmarks
if not get_option('benchmarks').disabled()
  subdir('benchmark')
endif
