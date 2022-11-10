// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SPLITSEEDS_HPP
#define SPAIX_SPLITSEEDS_HPP

#include "spaix/types.hpp"

namespace spaix {

template<typename Volume>
struct SplitSeeds {
  ChildIndex lhs_index;
  ChildIndex rhs_index;
  Volume     lhs_volume;
  Volume     rhs_volume;
};

} // namespace spaix

#endif // SPAIX_SPLITSEEDS_HPP
