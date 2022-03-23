// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#undef NDEBUG

#include "spaix_test/TestRect.hpp"
#include "spaix_test/check.hpp"

#include "spaix/volume.hpp"

namespace spaix::test {

static void
test_volume()
{
  STATIC_CHECK((volume(TestRect{{1, 3}, {2, 5}}) == 6));
  STATIC_CHECK((volume(TestRect{{1, 1}, {2, 5}}) == 0));
  STATIC_CHECK((volume(TestRect{{1, 3}, {2, 2}}) == 0));
}

} // namespace spaix::test

int
main()
{
  spaix::test::test_volume();
  return 0;
}
