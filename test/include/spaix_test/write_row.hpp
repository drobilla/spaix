// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_TEST_WRITE_ROW_HPP
#define SPAIX_TEST_WRITE_ROW_HPP

#include <iostream>

namespace spaix::test {

template<class Last>
void
write_row(std::ostream& os, Last last)
{
  os << last << '\n';
}

template<class First, class... Rest>
void
write_row(std::ostream& os, First first, Rest... rest)
{
  os << first << '\t';
  write_row(os, std::forward<Rest>(rest)...);
}

} // namespace spaix::test

#endif // SPAIX_TEST_WRITE_ROW_HPP
