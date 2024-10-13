// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_SIDECHOOSER_HPP
#define SPAIX_SIDECHOOSER_HPP

#include "spaix/expansion.hpp"
#include "spaix/types.hpp"
#include "spaix/volume.hpp"

#include <utility>

namespace spaix {

template<typename Volume>
struct SplitSeeds;

template<class DirKey, class ChildKey>
class SideChooser
{
public:
  using Volume = decltype(volume(std::declval<DirKey>()));

  struct Outcome {
    Volume volume;
    DirKey key;
  };

  SideChooser(const DirKey&   lhs_key,
              const Volume    lhs_volume,
              const DirKey&   rhs_key,
              const Volume    rhs_volume,
              const ChildKey& child_key) noexcept
    : _l_key{lhs_key | child_key}
    , _r_key{rhs_key | child_key}
    , _child_key{child_key}
    , _lhs_volume{lhs_volume}
    , _rhs_volume{rhs_volume}
    , _l_volume{volume(_l_key)}
    , _r_volume{volume(_r_key)}
    , _d_l_volume{_l_volume - lhs_volume}
    , _d_r_volume{_r_volume - rhs_volume}
  {}

  /// Return how much more preferable it is to choose one side over the other
  [[nodiscard]] Volume preference() const noexcept
  {
    return abs_diff(_d_l_volume, _d_r_volume);
  }

  /// Return the best side to distribute the child to
  [[nodiscard]] Side choose_side() noexcept
  {
    return (_d_l_volume < _d_r_volume)   ? Side::left
           : (_d_r_volume < _d_l_volume) ? Side::right
           : (_lhs_volume < _rhs_volume) ? Side::left
           : (_rhs_volume < _lhs_volume) ? Side::right
                                         : tie_side(_l_key, _r_key, _child_key);
  }

  /// Return the outcome (volume and key) of adding the child to the given side
  [[nodiscard]] Outcome outcome(const Side side) const noexcept
  {
    return side == Side::left ? Outcome{_l_volume, _l_key}
                              : Outcome{_r_volume, _r_key};
  }

private:
  /// Return |a - b| safely for unsigned types
  template<class T>
  static constexpr T abs_diff(T a, T b) noexcept
  {
    return a > b ? a - b : b - a;
  }

  /// Choose a side to break a tie when the volume comparisons fail
  Side tie_side(const DirKey&   lhs_key,
                const DirKey&   rhs_key,
                const ChildKey& child_key) noexcept
  {
    const auto l_expansion = spaix::expansion(lhs_key, child_key);
    const auto r_expansion = spaix::expansion(rhs_key, child_key);

    return (l_expansion < r_expansion)   ? Side::left
           : (r_expansion < l_expansion) ? Side::right
           : ((++_tie_phase & 1U) == 0U) ? Side::left
                                         : Side::right;
  }

  DirKey   _l_key;
  DirKey   _r_key;
  ChildKey _child_key;
  Volume   _lhs_volume;
  Volume   _rhs_volume;
  Volume   _l_volume;
  Volume   _r_volume;
  Volume   _d_l_volume;
  Volume   _d_r_volume;
  unsigned _tie_phase{};
};

template<class Volume, class DirKey, class ChildKey>
SideChooser<DirKey, ChildKey>
make_side_chooser(const SplitSeeds<Volume>& seeds,
                  const DirKey&             lhs_key,
                  const DirKey&             rhs_key,
                  const ChildKey&           child_key) noexcept
{
  return {lhs_key, seeds.lhs_volume, rhs_key, seeds.rhs_volume, child_key};
}

} // namespace spaix

#endif // SPAIX_SIDECHOOSER_HPP
