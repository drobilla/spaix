// Copyright 2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DETAIL_ENTRYTRACKER_HPP
#define SPAIX_DETAIL_ENTRYTRACKER_HPP

#include <spaix/SplitParts.hpp>

namespace spaix::detail {

template<class Entry, class ChildCount>
class EntryTracker
{
public:
  EntryTracker(SplitParts<Entry, ChildCount>& parts, bool tracked) noexcept
    : _parts{&parts}
    , _tracked{tracked}
  {}

  ~EntryTracker()                              = default;
  EntryTracker(const EntryTracker&)            = delete;
  EntryTracker& operator=(const EntryTracker&) = delete;
  EntryTracker(EntryTracker&&)                 = delete;
  EntryTracker& operator=(EntryTracker&&)      = delete;

  void track(const Side side, const ChildCount index) noexcept
  {
    if (!_tracked) {
      _tracked              = true;
      _parts->tracked_side  = side;
      _parts->tracked_index = index;
    }
  }

  explicit operator bool() const noexcept { return _tracked; }

private:
  SplitParts<Entry, ChildCount>* _parts;
  bool                           _tracked;
};

} // namespace spaix::detail

#endif // SPAIX_DETAIL_ENTRYTRACKER_HPP
