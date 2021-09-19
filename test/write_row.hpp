// Copyright 2013-2020 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TEST_WRITE_ROW_HPP
#define TEST_WRITE_ROW_HPP

#include <iostream>

namespace spaix {
namespace test {

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

} // namespace test
} // namespace spaix

#endif // TEST_WRITE_ROW_HPP
