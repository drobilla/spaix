// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/check.hpp>
#include <spaix_test/hetero2.hpp>
#include <spaix_test/homo2.hpp>

namespace spaix::test {
namespace {

template<typename Ops, typename TestRect>
constexpr void
test_volume()
{
  auto volume = [](const auto& k) { return Ops::volume(k); };

  STATIC_CHECK((volume(TestRect{{1, 3}, {2.0, 5.0}}) == 6));
  STATIC_CHECK((volume(TestRect{{1, 1}, {2.0, 5.0}}) == 0));
  STATIC_CHECK((volume(TestRect{{1, 3}, {2.0, 2.0}}) == 0));
  STATIC_CHECK((volume(TestRect{}) == 0));
}

constexpr void
run()
{
  test_volume<hetero2::Ops, hetero2::Rect>();
  test_volume<homo2::Ops, homo2::Rect>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
