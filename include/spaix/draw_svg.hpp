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

#ifndef SPAIX_DRAW_SVG_HPP
#define SPAIX_DRAW_SVG_HPP

#include "spaix/Rect.hpp"
#include "spaix/types.hpp"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

namespace spaix {

using NodePath = std::vector<ChildIndex>;

namespace svg {

constexpr double pad = 8;

static inline std::string
color(const NodePath& path, const double alpha)
{
  constexpr ChildCount fanout = 4;
  constexpr uint32_t   max    = std::numeric_limits<uint8_t>::max();

  uint32_t components[3] = {0, 0, 0};
  int      c             = 0;
  for (ChildIndex index : path) {
    components[c] += ((index * max) / ((fanout - 1)));
    c = (c + 1) % 3;
  }

  char buf[10];
  snprintf(buf,
           sizeof(buf),
           "#%02X%02X%02X%02X",
           (components[0] % max),
           (components[1] % max),
           (components[2] % max),
           static_cast<unsigned>(alpha * max));

  return buf;
}

template <size_t axis, class Key, class DirKey>
static inline double
coord(const Key& key, const DirKey& bounds, const double scale)
{
  return (min<axis>(key) - min<axis>(bounds)) * scale + pad;
}

template <class T>
static inline void
write_attr(std::ostream& os, const std::string& key, const T& value)
{
  os << " " << key << "=\"" << value << "\"";
}

template <class DirKey>
static inline bool
draw_dir(std::ostream&   os,
         const DirKey&   key,
         const NodePath& path,
         const size_t,
         const DirKey& bounds,
         const double  scale)
{
  if (path.size() == 1) {
    return true;
  }

  const auto style =
      "fill: " + color(path, 0.2) + "; stroke: " + color(path, 1.0);

  os << "  <rect";
  write_attr(os, "style", style);
  write_attr(os, "x", coord<0>(key, bounds, scale));
  write_attr(os, "y", coord<1>(key, bounds, scale));
  write_attr(os, "width", span<0>(key) * scale);
  write_attr(os, "height", span<1>(key) * scale);
  os << "/>\n";

  return true;
}

template <class... Values, class Data>
static inline void
draw_dat(std::ostream&          os,
         const Rect<Values...>& key,
         const Data&,
         const NodePath&        path,
         const Rect<Values...>& bounds,
         const double           scale)
{
  draw_dir(os, key, path, bounds, scale);
}

template <class... Values, class Data>
static inline void
draw_dat(std::ostream&           os,
         const Point<Values...>& key,
         const Data&,
         const NodePath&        path,
         const Rect<Values...>& bounds,
         const double           scale)
{
  const auto style = "fill: " + color(path, 1.0) + "; stroke: black";

  os << "  <circle";
  write_attr(os, "style", style);
  write_attr(os, "cx", coord<0>(key, bounds, scale));
  write_attr(os, "cy", coord<1>(key, bounds, scale));
  write_attr(os, "r", 2.0);
  os << "/>\n";
}

} // namespace svg

template <class Tree>
void
draw_svg(std::ostream&  os,
         const Tree&    tree,
         const double   scale     = 1.0,
         const unsigned max_depth = 0)
{
  using Key    = typename Tree::Key;
  using Data   = typename Tree::Data;
  using DirKey = typename Tree::DirKey;

  const auto bounds = tree.bounds();

  os << "<svg";
  svg::write_attr(os, "xmlns", "http://www.w3.org/2000/svg");
  svg::write_attr(os, "width", (span<0>(bounds) * scale) + (2 * svg::pad));
  svg::write_attr(os, "height", (span<1>(bounds) * scale) + (2 * svg::pad));
  os << ">\n";

  tree.visit_structure(
      [&os, bounds, scale, max_depth](
          const DirKey& key, const NodePath& path, const size_t n_children) {
        return svg::draw_dir(os, key, path, n_children, bounds, scale) &&
               (!max_depth || path.size() <= max_depth);
      },
      [&os, bounds, scale](
          const Key& key, const Data& data, const NodePath& path) {
        svg::draw_dat(os, key, data, path, bounds, scale);
      });

  os << "</svg>\n";
}

} // namespace spaix

#endif // SPAIX_DRAW_SVG_HPP
