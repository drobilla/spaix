// Copyright 2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SPLITPARTS_HPP
#define SPAIX_SPLITPARTS_HPP

#include <spaix/types.hpp>

#include <array>

namespace spaix {

template<class DirEntry, class ChildIndex>
struct SplitParts {
  std::array<DirEntry, 2> sides;
  ChildIndex              tracked_index;
  Side                    tracked_side;
};

} // namespace spaix

#endif // SPAIX_SPLITPARTS_HPP
