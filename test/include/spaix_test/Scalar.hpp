// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_SCALAR_HPP
#define SPAIX_TEST_SCALAR_HPP

#include <iosfwd>
#include <limits>
#include <utility>

namespace spaix::test {

template<class Tag, class Rep>
struct Scalar {
  constexpr Scalar() = default;

  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  constexpr Scalar(const Rep v)
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

  constexpr explicit operator Rep() const { return _value; }

private:
  Rep _value{};
};

template<class Rep>
struct SimpleVolume {
  constexpr SimpleVolume() = default;

  constexpr SimpleVolume(const Rep v) // NOLINT
    : _value{v}
  {}

  [[nodiscard]] constexpr Rep value() const { return _value; }

  [[nodiscard]] constexpr bool operator==(const Rep rep) const
  {
    return _value == rep;
  }

private:
  Rep _value{};
};

template<class Rep>
std::ostream&
operator<<(std::ostream& os, const SimpleVolume<Rep>& volume)
{
  return os << volume.value();
}

template<class LhsTag, class LhsRep, class RhsTag, class RhsRep>
SimpleVolume<decltype(std::declval<LhsRep>() * std::declval<RhsRep>())>
operator*(const Scalar<LhsTag, LhsRep>& lhs, const Scalar<RhsTag, RhsRep>& rhs)
{
  return {lhs.value() * rhs.value()};
}

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
{
public:
  static constexpr spaix::test::Scalar<Tag, Rep> lowest() noexcept
  {
    return spaix::test::Scalar<Tag, Rep>{numeric_limits<Rep>::lowest()};
  }

  static constexpr spaix::test::Scalar<Tag, Rep> max() noexcept
  {
    return spaix::test::Scalar<Tag, Rep>{numeric_limits<Rep>::max()};
  }
};

} // namespace std

#endif // SPAIX_TEST_SCALAR_HPP
