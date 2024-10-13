<!-- Copyright 2020-2022 David Robillard <d@drobilla.net> -->
<!-- SPDX-License-Identifier: 0BSD or GPL-3.0-only -->

Spaix
=====

Spaix (SPAtial IndeX) is a header-only C++ library with geometric primitives
and an R-tree which support using different data types for each dimension.

This is useful when indexing spaces with different ranges in each dimension
(for example doubles on the X axis and bytes on the Y axis), or for using
strong data types that do not allow mixing values in different dimensions.

The implementation is statically parameterised by maximum and minimum fan-out,
key/value datatypes, and insert/split algorithm, so it can be tuned for a
specific application without any dynamic overhead.  The downside, as you might
expect, is that compile times can be high compared to a more dynamic
implementation.

Features
--------

This library has a few distinguishing features:

 * Support for indexing points or rectangles in any number of dimensions.

 * Support for arbitrary data types in each dimension.  Data types only need to
   support a few basic operations: product, difference, and comparison.

 * Fully `constexpr` point and rectangle types.

 * Test suite with 100% coverage by line.

 * Completely separate and pluggable split and insert selection algorithms.
   Included are the classic (Guttman) algorithms:

   * Linear insert selection.
   * Linear split.
   * Area-minimising quadratic split.

Building
--------

The library, unit tests, and benchmarks can be built and installed using
[Meson](http://mesonbuild.com/):

    meson setup build -Dbenchmark=true
    ninja -C build test

Benchmarking
------------

The included benchmarking script can be used to benchmark the various
algorithms and parameters, for example:

    scripts/benchmark.py --page-size 512 --size 1000000 --queries 1000

This will produce an HTML page which contains several plots for various
operations.  See the help output for details.

 -- David Robillard <d@drobilla.net>
