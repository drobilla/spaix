# Copyright 2019-2024 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

project(
  'spaix',
  ['cpp'],
  version: '0.0.0',
  license: 'GPLv3+',
  meson_version: '>= 0.52.0',
  default_options: [
    'b_ndebug=if-release',
    'buildtype=release',
    'cpp_std=c++17',
  ],
)

spaix_src_root = meson.current_source_dir()
versioned_name = 'spaix-@0@'.format(meson.project_version().split('.')[0])

#######################
# Compilers and Flags #
#######################

# Required tools
pkg = import('pkgconfig')
cpp = meson.get_compiler('cpp')

# Set global warning suppressions
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

  if warning_level in ['everything', '3']
    cpp_suppressions += [
      '-Wno-nullability-extension',
    ]
  endif

elif cpp.get_id() == 'gcc'
  if warning_level == 'everything'
    cpp_suppressions = [
      '-Wno-conversion',
      '-Wno-effc++',
      '-Wno-float-equal',
      '-Wno-inline',
      '-Wno-maybe-uninitialized', # Bug
      '-Wno-padded',
      '-Wno-suggest-attribute=const',
      '-Wno-suggest-attribute=pure',
      '-Wno-switch-default',
    ]
  endif

elif cpp.get_id() == 'msvc'
  cpp_suppressions += [
    '/experimental:external',
    '/external:W0',
    '/external:anglebrackets',
  ]

  if warning_level == 'everything'
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

cpp_suppressions = cpp.get_supported_arguments(cpp_suppressions)

################
# Dependencies #
################

# Boost for comparison benchmark program
boost_dep = disabler()
if not (get_option('benchmarks').disabled() or get_option('boost').disabled())
  boost_dep = dependency(
    'boost',
    include_type: 'system',
    required: get_option('boost')
  )
endif

###########
# Library #
###########

headers = files(
  'include/spaix/Config.hpp',
  'include/spaix/DataIterator.hpp',
  'include/spaix/DataNode.hpp',
  'include/spaix/DataPlacement.hpp',
  'include/spaix/Iterator.hpp',
  'include/spaix/LinearInsertion.hpp',
  'include/spaix/LinearSplit.hpp',
  'include/spaix/Point.hpp',
  'include/spaix/QuadraticSplit.hpp',
  'include/spaix/RTree.hpp',
  'include/spaix/RTree.ipp',
  'include/spaix/Rect.hpp',
  'include/spaix/SplitSeeds.hpp',
  'include/spaix/StaticVector.hpp',
  'include/spaix/TreeRange.hpp',
  'include/spaix/contains.hpp',
  'include/spaix/detail/DirectoryNode.hpp',
  'include/spaix/detail/distribute.hpp',
  'include/spaix/detail/meta.hpp',
  'include/spaix/expansion.hpp',
  'include/spaix/intersects.hpp',
  'include/spaix/search/everything.hpp',
  'include/spaix/search/touching.hpp',
  'include/spaix/search/within.hpp',
  'include/spaix/sizes.hpp',
  'include/spaix/types.hpp',
  'include/spaix/union.hpp',
  'include/spaix/volume.hpp',
)

# Declare dependency for internal meson dependants
spaix_dep = declare_dependency(
  include_directories: include_directories('include'),
)

# Generage pkg-config file for external dependants
pkg.generate(
  name: 'Spaix',
  description: 'C++ spatial indexing library',
  filebase: versioned_name,
  subdirs: [versioned_name],
  version: meson.project_version(),
)

# Install headers to a versioned include directory
install_headers(headers, subdir: versioned_name + '/spaix')

#########
# Tests #
#########

if not get_option('tests').disabled()
  subdir('test')
endif
