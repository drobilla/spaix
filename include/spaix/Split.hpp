// Copyright 2013-2022 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SPLIT_HPP
#define SPAIX_SPLIT_HPP

#include "spaix/expansion.hpp"
#include "spaix/types.hpp"

namespace spaix {

/**
   Utility base class for split algorithms.
*/
class Split
{
protected:
  /// Choose a side to break a tie when the volume comparisons fail
  template<class DirKey, class ChildKey>
  Side tie_side(const DirKey&   lhs_key,
                const DirKey&   rhs_key,
                const ChildKey& child_key)
  {
    const auto l_expansion = spaix::expansion(lhs_key, child_key);
    const auto r_expansion = spaix::expansion(rhs_key, child_key);

    return (l_expansion < r_expansion)   ? Side::left
           : (r_expansion < l_expansion) ? Side::right
                                         : arbitrary_side();
  }

  /// Choose an arbitrary fallback side (which flip-flops to avoid bias)
  Side arbitrary_side()
  {
    return (_bias = (_bias == Side::left ? Side::right : Side::left));
  }

private:
  Side _bias = Side::left;
};

} // namespace spaix

#endif // SPAIX_SPLIT_HPP
