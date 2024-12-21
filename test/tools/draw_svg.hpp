// Copyright 2013-2024 David Robillard <d@drobilla.net>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SPAIX_DRAW_SVG_HPP
#define SPAIX_DRAW_SVG_HPP

#include <spaix/Point.hpp>
#include <spaix/RTree.hpp>
#include <spaix/Rect.hpp>
#include <spaix/types.hpp>

#include <array>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <ostream>
#include <string>

namespace spaix {
namespace svg {

constexpr double pad = 8;

template<class NodePath>
inline std::string
color(const NodePath& path, const double alpha)
{
  constexpr uint32_t fanout = 4;
  constexpr uint32_t u8max  = std::numeric_limits<uint8_t>::max();

  std::array<uint32_t, 3U> components{};
  unsigned                 c = 0U;
  for (const ChildIndex index : path) {
    components[c] += ((static_cast<uint32_t>(index) * u8max) / ((fanout - 1U)));
    c = (c + 1) % 3;
  }

  std::array<char, 36> buf{};
  (void)snprintf(buf.data(),
                 buf.size(),
                 "#%02X%02X%02X%02X",
                 (components[0] % u8max),
                 (components[1] % u8max),
                 (components[2] % u8max),
                 static_cast<unsigned>(alpha * u8max));

  return {buf.data()};
}

template<size_t axis, class Key, class DirKey>
inline double
coord(const Key& key, const DirKey& bounds, const double scale)
{
  return (range<axis>(key).lower - range<axis>(bounds).lower) * scale + pad;
}

template<class T>
inline void
write_attr(std::ostream& os, const std::string& key, const T& value)
{
  os << " " << key << "=\"" << value << "\"";
}

template<class DirKey, class NodePath>
inline void
draw_dir(std::ostream&   os,
         const DirKey&   key,
         const NodePath& path,
         const DirKey&   bounds,
         const double    scale)
{
  if (path.size() == 1) {
    return;
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
}

template<class... Values, class NodePath>
inline void
draw_dat(std::ostream&          os,
         const Rect<Values...>& key,
         const NodePath&        path,
         const Rect<Values...>& bounds,
         const double           scale)
{
  draw_dir(os, key, path, bounds, scale);
}

template<class... Values, class NodePath>
inline void
draw_dat(std::ostream&           os,
         const Point<Values...>& key,
         const NodePath&         path,
         const Rect<Values...>&  bounds,
         const double            scale)
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

template<class Tree>
void
draw_svg(std::ostream&  os,
         const Tree&    tree,
         const double   scale     = 1.0,
         const unsigned max_depth = 0)
{
  using Key      = typename Tree::Key;
  using Data     = typename Tree::Data;
  using DirKey   = typename Tree::Box;
  using NodePath = typename Tree::NodePath;

  const auto bounds = tree.bounds();

  os << "<svg";
  svg::write_attr(os, "xmlns", std::string{"http://www.w3.org/2000/svg"});
  svg::write_attr(os, "width", (span<0>(bounds) * scale) + (2 * svg::pad));
  svg::write_attr(os, "height", (span<1>(bounds) * scale) + (2 * svg::pad));
  os << ">\n";

  tree.visit(
    [&os, bounds, scale, max_depth](
      const NodePath& path, const DirKey& key, size_t) {
      svg::draw_dir(os, key, path, bounds, scale);
      return (!max_depth || path.size() <= max_depth) ? VisitStatus::proceed
                                                      : VisitStatus::finish;
    },
    [&os, bounds, scale](const NodePath& path, const Key& key, const Data&) {
      svg::draw_dat(os, key, path, bounds, scale);
      return VisitStatus::proceed;
    });

  os << "</svg>\n";
}

} // namespace spaix

#endif // SPAIX_DRAW_SVG_HPP
