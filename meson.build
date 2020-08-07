project('spaix', 'cpp',
        version: '0.0.0',
        default_options: [
          'buildtype=debugoptimized',
          'cpp_std=c++14',
          'warning_level=3',
        ],
        meson_version : '>= 0.43.0',
        license: 'GPLv3+')

cxx = meson.get_compiler('cpp')
versioned_name = 'spaix-@0@'.format(meson.project_version().split('.')[0])

if get_option('warning_level') == '3'
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
  endif

  add_project_arguments(cxx.get_supported_arguments(cxx_warnings),
                        language: ['cpp'])
endif

headers = [
  'include/spaix/DataNode.hpp',
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
  'include/spaix/draw_dot.hpp',
  'include/spaix/draw_svg.hpp',
  'include/spaix/everything.hpp',
  'include/spaix/intersection.hpp',
  'include/spaix/intersects.hpp',
  'include/spaix/touching.hpp',
  'include/spaix/types.hpp',
  'include/spaix/union.hpp',
  'include/spaix/volume.hpp',
  'include/spaix/within.hpp',
]

if not get_option('no_boost')
  boost_dep = dependency('boost', required: false)
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

if get_option('test')
  tests = [
    'Point',
    'RTree',
    'Rect',
    'contains',
    'intersection',
    'intersects',
    'union'
  ]

  foreach test : tests
    test(test,
         executable('tst_' + test, 'test/tst_@0@.cpp'.format(test),
                    dependencies: spaix_dep))
  endforeach

  draw_exe = executable('draw_tree', 'test/draw_tree.cpp',
                        dependencies:spaix_dep)

  test('draw', draw_exe)
  test('draw', draw_exe, args: ['--insert', 'linear'])
  test('draw', draw_exe, args: ['--split', 'linear'])
  test('draw', draw_exe, args: ['--split', 'quadratic'])
  # test('draw', draw_exe, args: ['--page-size', '64'])
  # test('draw', draw_exe, args: ['--page-size', '128'])
  test('draw', draw_exe, args: ['--page-size', '256'])
  test('draw', draw_exe, args: ['--page-size', '512'])
  test('draw', draw_exe, args: ['--page-size', '1024'])
  test('draw', draw_exe, args: ['--page-size', '2048'])
  test('draw', draw_exe, args: ['--page-size', '4096'])

  test('draw', draw_exe, args: ['--insert', 'unknown'], should_fail: true)
  test('draw', draw_exe, args: ['--split', 'unknown'], should_fail: true)
  test('draw', draw_exe, args: ['--page-size', '16'], should_fail: true)
  test('draw', draw_exe, args: ['--page-size', '8192'], should_fail: true)
  test('draw', draw_exe, args: ['--bad-arg'], should_fail: true)
endif

if get_option('benchmark')
  bench_exe = executable('bench_spaix_rtree', 'test/bench_spaix_rtree.cpp',
                         dependencies: spaix_dep)

  if not get_option('no_boost') and boost_dep.found()
    boost_bench_exe = executable('bench_boost_rtree',
                                 'test/bench_boost_rtree.cpp',
                                 dependencies: [boost_dep, spaix_dep])
  endif

  if get_option('test')
    test('bench', bench_exe, args: ['--size', '16', '--insert', 'linear'])
    test('bench', bench_exe, args: ['--size', '16', '--split', 'linear'])
    test('bench', bench_exe, args: ['--size', '16', '--split', 'quadratic'])
    # test('bench', bench_exe, args: ['--size', '16', '--page-size', '64'])
    # test('bench', bench_exe, args: ['--size', '16', '--page-size', '128'])
    test('bench', bench_exe, args: ['--size', '16', '--page-size', '256'])
    test('bench', bench_exe, args: ['--size', '16', '--page-size', '512'])
    test('bench', bench_exe, args: ['--size', '16', '--page-size', '1024'])
    test('bench', bench_exe, args: ['--size', '16', '--page-size', '2048'])
    test('bench', bench_exe, args: ['--size', '16', '--page-size', '4096'])
    test('bench', bench_exe, args: ['--size', '16', '--page-size', '8192'])

    test('bench', bench_exe, args: ['--insert', 'unknown'], should_fail: true)
    test('bench', bench_exe, args: ['--split', 'unknown'], should_fail: true)
    test('bench', bench_exe, args: ['--page-size', '64'], should_fail: true)
    test('bench', bench_exe, args: ['--page-size', '16384'], should_fail: true)
    test('bench', bench_exe, args: ['--bad-arg'], should_fail: true)

  endif
endif
