# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

project('spaix', ['cpp'],
        version: '0.0.0',
        license: 'GPLv3+',
        meson_version : '>= 0.49.2',
        default_options: [
          'b_ndebug=if-release',
          'buildtype=release',
          'cpp_std=c++17',
        ])

spaix_src_root = meson.current_source_dir()
versioned_name = 'spaix-@0@'.format(meson.project_version().split('.')[0])

#######################
# Compilers and Flags #
#######################

# Required tools
pkg = import('pkgconfig')
cpp = meson.get_compiler('cpp')

# Set global warning flags
if get_option('strict') and not meson.is_subproject()
  subdir('meson/warnings')
  add_project_arguments(all_cpp_warnings, language: ['cpp'])
endif
subdir('meson/suppressions')

################
# Dependencies #
################

# Boost for comparison benchmark program
boost_dep = []
enable_boost = get_option('boost')
have_boost = false
if enable_boost == 'no'
  message('Boost support explicitly disabled')
else
  boost_dep = dependency('boost', required: false)
  if boost_dep.found()
    have_boost = true
  elif enable_boost == 'yes' and not have_boost
    error('Boost support not found, but was explicitly requested.')
  endif
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
  include_directories: include_directories('include'))

# Generage pkg-config file for external dependants
pkg.generate(name: 'Spaix',
             description: 'C++ spatial indexing library',
             filebase: versioned_name,
             subdirs: [versioned_name],
             version: meson.project_version())

# Install headers to a versioned include directory
install_headers(headers, subdir: versioned_name + '/spaix')

#########
# Tests #
#########

if not get_option('tests').disabled()
  subdir('test')
endif
