// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_DISTRIBUTION_HPP
#define SPAIX_TEST_DISTRIBUTION_HPP

#include <algorithm>
#include <cstddef>

namespace spaix::test {

/// Distribution of numbers that can be incrementally updated
template<class T>
class Distribution
{
public:
  void update(T x)
  {
    using std::max;
    using std::min;

    if (_n == 0) {
      _min = _max = _mean = x;
    } else {
      _min  = min(_min, x);
      _max  = max(_max, x);
      _mean = _mean + (x - _mean) / T(_n + 1);
    }

    ++_n;
  }

  size_t n() const { return _n; }
  T      min() const { return _min; }
  T      max() const { return _max; }
  T      mean() const { return _mean; }

private:
  size_t _n{};
  T      _min{};
  T      _max{};
  T      _mean{};
};

} // namespace spaix::test

#endif // SPAIX_TEST_DISTRIBUTION_HPP
