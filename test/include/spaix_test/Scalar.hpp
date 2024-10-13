// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_SCALAR_HPP
#define SPAIX_TEST_SCALAR_HPP

#include <iosfwd>
#include <limits>

#undef NDEBUG

namespace spaix::test {

template<class Tag, class Rep>
struct Scalar {
  constexpr Scalar() = default;

  constexpr Scalar(const Rep v) // NOLINT
    : _value{v}
  {}

  constexpr bool operator==(const Scalar& rhs) const
  {
    return _value == rhs._value;
  }

  constexpr bool operator<(const Scalar& rhs) const
  {
    return _value < rhs._value;
  }

  constexpr auto operator*(const Scalar& rhs) const
  {
    return _value * rhs._value;
  }

  constexpr auto operator-(const Scalar& rhs) const
  {
    return _value - rhs._value;
  }

  [[nodiscard]] constexpr Rep value() const { return _value; }

private:
  Rep _value{};
};

template<class Tag, class Rep>
std::ostream&
operator<<(std::ostream& os, const Scalar<Tag, Rep>& scalar)
{
  return os << scalar.value();
}

} // namespace spaix::test

namespace std {

template<class Tag, class Rep>
class numeric_limits<spaix::test::Scalar<Tag, Rep>> : public numeric_limits<Rep>
{};

} // namespace std

#endif // SPAIX_TEST_SCALAR_HPP
