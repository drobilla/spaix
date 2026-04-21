// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_POWER_HPP
#define SPAIX_DETAIL_POWER_HPP

namespace spaix::detail {

/// Return b^e (b raised to the power e)
template<class T>
constexpr T
power(const T b, const T e) noexcept
{
  return !e ? 1 : b * power(b, e - 1);
}

} // namespace spaix::detail

#endif // SPAIX_DETAIL_POWER_HPP
