# Copyright 2019-2022 David Robillard <d@drobilla.net>
# SPDX-License-Identifier: CC0-1.0 OR GPL-3.0-only

project('spaix', 'cpp',
        version: '0.0.0',
        default_options: [
          'buildtype=debugoptimized',
          'cpp_std=c++17',
          'warning_level=2',
        ],
        meson_version : '>= 0.45.0',
        license: 'GPLv3+')

spaix_src_root = meson.current_source_dir()

cxx = meson.get_compiler('cpp')
versioned_name = 'spaix-@0@'.format(meson.project_version().split('.')[0])

if get_option('strict')
  subdir('warnings')

  if cxx.get_id() == 'clang'
    cxx_warnings += [
      '-Wno-float-equal',
      '-Wno-implicit-int-float-conversion',
      '-Wno-padded',
    ]
  elif cxx.get_id() == 'gcc'
    cxx_warnings += [
      '-Wno-conversion',
      '-Wno-effc++',
      '-Wno-float-equal',
      '-Wno-padded',
      '-Wno-suggest-attribute=const',
      '-Wno-suggest-attribute=pure',
      '-Wno-switch-default',
    ]
  elif cxx.get_id() == 'msvc'
    cxx_warnings += [
      '/wd4514',  # unreferenced inline function has been removed
      '/wd4582',  # constructor is not explicitly called
      '/wd4583',  # destructor is not explicitly called
      '/wd4623',  # default constructor implicitly deleted
      '/wd4625',  # copy constructor implicitly deleted
      '/wd4626',  # copy assignment operator implicitly deleted
      '/wd4710',  # function not inlined
      '/wd4711',  # function selected for automatic inline expansion
      '/wd4820',  # padding added after construct
      '/wd4868',  # may not enforce left-to-right evaluation order
      '/wd5027',  # move assignment operator implicitly deleted
      '/wd5045',  # will insert Spectre mitigation for memory load
    ]
  endif

  add_project_arguments(cxx.get_supported_arguments(cxx_warnings),
                        language: ['cpp'])
endif

headers = [
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
  'include/spaix/StaticVector.hpp',
  'include/spaix/contains.hpp',
  'include/spaix/detail/DirectoryNode.hpp',
  'include/spaix/detail/distribute.hpp',
  'include/spaix/detail/meta.hpp',
  'include/spaix/intersection.hpp',
  'include/spaix/intersects.hpp',
  'include/spaix/search/everything.hpp',
  'include/spaix/search/touching.hpp',
  'include/spaix/search/within.hpp',
  'include/spaix/sizes.hpp',
  'include/spaix/types.hpp',
  'include/spaix/union.hpp',
  'include/spaix/volume.hpp',
]

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

spaix_dep = declare_dependency(
  include_directories: include_directories('include'))

install_headers(headers, subdir: versioned_name + '/spaix')

pkg = import('pkgconfig')
pkg.generate(name: 'Spaix',
             filebase: versioned_name,
             subdirs: [versioned_name],
             version: meson.project_version(),
             description: 'C++ spatial indexing library')

spaix_test_dep = declare_dependency(
  include_directories: include_directories('test/include'))

if get_option('tests')
  subdir('test')
endif

if get_option('tools')
  draw_exe = executable('draw_tree', 'test/tools/draw_tree.cpp',
                        dependencies: [spaix_dep, spaix_test_dep])

  if get_option('tests')
    test('default', draw_exe, suite: 'draw')
    test('linear_insert', draw_exe, args: ['--insert', 'linear'], suite: 'draw')
    test('linear_split', draw_exe, args: ['--split', 'linear'], suite: 'draw')
    test('quadratic_split', draw_exe, args: ['--split', 'quadratic'], suite: 'draw')
    test('page_256', draw_exe, args: ['--page-size', '256'], suite: 'draw')
    test('page_512', draw_exe, args: ['--page-size', '512'], suite: 'draw')
    test('page_1024', draw_exe, args: ['--page-size', '1024'], suite: 'draw')
    test('page_2048', draw_exe, args: ['--page-size', '2048'], suite: 'draw')
    test('page_4096', draw_exe, args: ['--page-size', '4096'], suite: 'draw')

    test('bad_insert', draw_exe, args: ['--insert', 'unknown'], should_fail: true, suite: 'draw')
    test('bad_split', draw_exe, args: ['--split', 'unknown'], should_fail: true, suite: 'draw')
    test('bad_small_page', draw_exe, args: ['--page-size', '128'], should_fail: true, suite: 'draw')
    test('bad_large_page', draw_exe, args: ['--page-size', '8192'], should_fail: true, suite: 'draw')
    test('bad_option', draw_exe, args: ['--bad-arg'], should_fail: true, suite: 'draw')
  endif
endif

if get_option('benchmark')
  subdir('test/benchmark')
endif
