/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TEST_SCALAR_HPP
#define TEST_SCALAR_HPP

#include <iosfwd>
#include <limits>

#undef NDEBUG

namespace spaix {
namespace test {

template <class Tag, class Rep>
struct Scalar
{
  constexpr Scalar() : _value{} {}
  constexpr Scalar(const Rep v) : _value{v} {}

  constexpr bool operator==(const Scalar& rhs) const
  {
    return _value == rhs._value;
  }

  constexpr bool operator<(const Scalar& rhs) const
  {
    return _value < rhs._value;
  }

  constexpr Scalar operator*(const Scalar& rhs) const
  {
    return _value * rhs._value;
  }

  constexpr Scalar operator-(const Scalar& rhs) const
  {
    return _value - rhs._value;
  }

  constexpr Rep value() const { return _value; }

private:
  Rep _value;
};

template <class Tag, class Rep>
std::ostream&
operator<<(std::ostream& os, const Scalar<Tag, Rep>& scalar)
{
  return os << scalar.value();
}

} // namespace test
} // namespace spaix

namespace std {

template <class Tag, class Rep>
class numeric_limits<spaix::test::Scalar<Tag, Rep>> : public numeric_limits<Rep>
{
};

} // namespace std

#endif // TEST_SCALAR_HPP
