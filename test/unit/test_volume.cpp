// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "spaix_test/TestRect.hpp"
#include "spaix_test/check.hpp"

#include "spaix/volume.hpp"

namespace spaix::test {
namespace {

void
test_volume()
{
  STATIC_CHECK((volume(TestRect{{1_xc, 3_xc}, {2.0_yc, 5.0_yc}}) == 6));
  STATIC_CHECK((volume(TestRect{{1_xc, 1_xc}, {2.0_yc, 5.0_yc}}) == 0));
  STATIC_CHECK((volume(TestRect{{1_xc, 3_xc}, {2.0_yc, 2.0_yc}}) == 0));
}

} // namespace
} // namespace spaix::test

int
main()
{
  spaix::test::test_volume();
  return 0;
}
