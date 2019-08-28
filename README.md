Spaix
=====

Spaix (SPAtial IndeX) is a small header-only C++ library with geometric
primitives and an R-tree which support using different data types for each
dimension.

This is useful when indexing spaces with different ranges in each dimension
(for example doubles on the X axis and bytes on the Y axis), or for using
strong data types that do not allow values from different dimensions to be
erroneously mixed.

The implementation is statically parameterised by fan-out, datatype, and
insert/split algorithm, so it can be tuned for a specific application without
any dynamic overhead.  The downside, as you might expect, is that compile times
can be rough.

Features
--------

This library has a few distinguishing features:

 * Support for indexing points or rectangles in any number of dimensions.

 * Support for arbitrary data types in each dimension.  The types for values in
   a dimension only need to support a few basic operations (product,
   difference, and comparison).

 * Fully `constexpr` point and rectangle types.

 * Test suite with 100% coverage by line.

 * Split and insert selection algorithms are completely separate from the tree
   implementation.  Included are:

   * Classic linear insert selection.
   * Classic linear split.
   * Classic overlap-minimising quadratic split.
   * Overlap-minimising quadratic insert selection.

Building
--------

The library, unit tests, and benchmarks can be built and installed using
[Meson](http://mesonbuild.com/):

    meson . build -Dtest=true -Dbenchmark=true
    ninja -C build test

 -- David Robillard <d@drobilla.net>
