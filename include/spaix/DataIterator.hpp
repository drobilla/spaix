// Copyright 2013-2026 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DATAITERATOR_HPP
#define SPAIX_DATAITERATOR_HPP

#include <spaix/EntryIterator.hpp>
#include <spaix/detail/entry.hpp>
#include <spaix/types.hpp>

#include <cassert>

namespace spaix {

/**
   Base class for iterators that refer to data entries.

   This is either empty, or can be dereferenced to access a data entry.
*/
template<class DirNode, class DatNode, unsigned max_height>
class DataIterator : public EntryIterator<DirNode, max_height>
{
public:
  using value_type = DatNode;
  using pointer    = const DatNode*;
  using reference  = const DatNode&;

  DataIterator() noexcept = default;

  [[nodiscard]] DatNode& operator*() const noexcept { return *operator->(); }

  [[nodiscard]] DatNode* operator->() const noexcept
  {
    assert(!this->empty());
    assert(this->index() < this->parent()->num_children());
    assert(this->parent()->child_type() == NodeType::data);

    return detail::entry_ptr(this->parent()->dat_children()[this->index()]);
  }
};

} // namespace spaix

#endif // SPAIX_DATAITERATOR_HPP
