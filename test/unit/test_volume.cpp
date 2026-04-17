// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include <spaix_test/Scalar.hpp>
#include <spaix_test/TestRect.hpp>
#include <spaix_test/check.hpp>

#include <spaix/Operations.hpp>

namespace spaix::test {
namespace {

template<typename Ops, typename TestRect>
constexpr void
test_volume()
{
  auto volume = [](const auto& k) { return Ops::volume(k); };

  STATIC_CHECK((volume(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}}) == 6));
  STATIC_CHECK((volume(TestRect{{1_xc, 1_xc}, {2.0_yc, 5.0_yc}}) == 0));
  STATIC_CHECK((volume(TestRect{{1_xc, 3_xc}, {2.0_yc, 2.0_yc}}) == 0));
}

constexpr void
run()
{
  test_volume<Operations<XCoord, YCoord>, TestRect>();
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::run();
  return 0;
}
