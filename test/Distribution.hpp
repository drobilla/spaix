// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TEST_DISTRIBUTION_HPP
#define TEST_DISTRIBUTION_HPP

#include <algorithm>
#include <cstddef>

namespace spaix {
namespace test {

/// Distribution of numbers that can be incrementally updated
template<class T>
class Distribution
{
public:
  void update(T x)
  {
    if (_n == 0) {
      _min = _max = _mean = x;
    } else {
      _min  = std::min(_min, x);
      _max  = std::max(_max, x);
      _mean = _mean + (x - _mean) / T(_n + 1);
    }

    ++_n;
  }

  inline size_t n() const { return _n; }
  inline T      min() const { return _min; }
  inline T      max() const { return _max; }
  inline T      mean() const { return _mean; }

private:
  size_t _n{};
  T      _min{};
  T      _max{};
  T      _mean{};
};

} // namespace test
} // namespace spaix

#endif // TEST_DISTRIBUTION_HPP
