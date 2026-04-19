# Copyright 2019-2026 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: 0BSD OR GPL-3.0-only

project(
  'spaix',
  ['cpp'],
  version: '0.0.1',
  license: 'GPLv3+',
  meson_version: '>= 0.54.0',
  default_options: [
    'b_ndebug=if-release',
    'buildtype=release',
    'cpp_std=c++17',
  ],
)

spaix_src_root = meson.current_source_dir()
versioned_name = 'spaix-@0@'.format(meson.project_version().split('.')[0])

pkg = import('pkgconfig')

###########
# Library #
###########

headers = files(
  'include/spaix/Comparisons.hpp',
  'include/spaix/Config.hpp',
  'include/spaix/DataIterator.hpp',
  'include/spaix/DataNode.hpp',
  'include/spaix/DataPlacement.hpp',
  'include/spaix/Iterator.hpp',
  'include/spaix/LinearInsertion.hpp',
  'include/spaix/LinearSplit.hpp',
  'include/spaix/Operations.hpp',
  'include/spaix/Point.hpp',
  'include/spaix/QuadraticSplit.hpp',
  'include/spaix/Queries.hpp',
  'include/spaix/RTree.hpp',
  'include/spaix/RTree.ipp',
  'include/spaix/Rect.hpp',
  'include/spaix/SideChooser.hpp',
  'include/spaix/SplitSeeds.hpp',
  'include/spaix/StaticVector.hpp',
  'include/spaix/TreeRange.hpp',
  'include/spaix/detail/DatEntryType.hpp',
  'include/spaix/detail/DirectoryNode.hpp',
  'include/spaix/detail/NodePointerEntry.hpp',
  'include/spaix/detail/attributes.hpp',
  'include/spaix/detail/distribute.hpp',
  'include/spaix/detail/entry.hpp',
  'include/spaix/detail/meta.hpp',
  'include/spaix/search/Everything.hpp',
  'include/spaix/search/Touching.hpp',
  'include/spaix/search/Within.hpp',
  'include/spaix/sizes.hpp',
  'include/spaix/types.hpp',
)

# Declare dependency for internal meson dependants
spaix_dep = declare_dependency(
  include_directories: include_directories('include'),
)

# Override pkg-config dependency for internal meson dependants
meson.override_dependency(versioned_name, spaix_dep)

if not get_option('install').disabled()
  # Generate pkg-config file for external dependants
  pkg.generate(
    name: 'Spaix',
    description: 'C++ spatial indexing library',
    filebase: versioned_name,
    subdirs: [versioned_name],
    version: meson.project_version(),
  )

  # Install headers to a versioned include directory
  install_headers(headers, subdir: versioned_name + '/spaix')
endif

#########
# Tests #
#########

if not get_option('tests').disabled()
  subdir('test')
endif
