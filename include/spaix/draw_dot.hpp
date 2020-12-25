/*
  Copyright 2013-2020 David Robillard <d@drobilla.net>

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SPAIX_DRAW_DOT_HPP
#define SPAIX_DRAW_DOT_HPP

#include "spaix/RTree.hpp"
#include "spaix/types.hpp"

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

namespace spaix {

using NodePath = std::vector<ChildIndex>;

namespace detail {

static inline std::string
to_string(const NodePath& path)
{
  std::string s{"n"};
  for (auto i = path.begin(); i != path.end(); ++i) {
    s += ((i == path.begin()) ? "" : "_") + std::to_string(*i);
  }

  return s;
}

template<class DirKey>
static inline bool
draw_dir_dot(std::ostream&   os,
             const DirKey&   key,
             const NodePath& path,
             const size_t    n_children)
{
  const auto id = to_string(path);
  os << "  " << id << "[label=\"" << key << "\"];\n";

  auto child_path{path};
  for (ChildCount i = 0; i < n_children; ++i) {
    child_path.push_back(i);
    os << "  " << id << " -- " << to_string(child_path) << ";\n";
    child_path.pop_back();
  }

  return true;
}

template<class Key, class Data>
static inline void
draw_dat_dot(std::ostream& os,
             const Key&    key,
             const Data&,
             const NodePath& path)
{
  os << "  " << to_string(path) << "[label=\"" << key << "\"];\n";
}

} // namespace detail

template<class Tree>
void
draw_dot(std::ostream& os, const Tree& tree, const size_t max_depth = 0u)
{
  using Key    = typename Tree::Key;
  using Data   = typename Tree::Data;
  using DirKey = typename Tree::Box;

  using spaix::VisitStatus;

  os << "graph Tree {\n";
  os << "  node [shape=box];\n";
  tree.visit(
    [&os, max_depth](
      const NodePath& path, const DirKey& key, const size_t n_children) {
      return detail::draw_dir_dot(os, key, path, n_children) &&
                 (!max_depth || path.size() <= max_depth)
               ? VisitStatus::proceed
               : VisitStatus::finish;
    },
    [&os](const NodePath& path, const Key& key, const Data& data) {
      detail::draw_dat_dot(os, key, data, path);
      return VisitStatus::proceed;
    });

  os << "}\n";
}

} // namespace spaix

#endif // SPAIX_DRAW_DOT_HPP
